/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-17
 * Description : digital camera controller
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006      by Stephan Kulow <coolo at kde dot org>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cameracontroller.h"

// Qt includes

#include <QApplication>
#include <QMutex>
#include <QWaitCondition>
#include <QVariant>
#include <QImage>
#include <QFile>
#include <QRegularExpression>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "actionthreadbase.h"
#include "template.h"
#include "templatemanager.h"
#include "gpcamera.h"
#include "umscamera.h"
#include "jpegutils.h"
#include "dfileoperations.h"
#include "filereadwritelock.h"

namespace Digikam
{

class Q_DECL_HIDDEN CameraCommand
{
public:

    enum Action
    {
        cam_none = 0,
        cam_connect,
        cam_cancel,
        cam_cameraInformation,
        cam_listfolders,
        cam_listfiles,
        cam_download,
        cam_upload,
        cam_delete,
        cam_lock,
        cam_thumbsinfo,
        cam_metadata,
        cam_open,
        cam_freeSpace,
        cam_preview,
        cam_capture
    };

public:

    CameraCommand() = default;

public:

    Action                  action = cam_none;
    QMap<QString, QVariant> map;
};

class Q_DECL_HIDDEN CameraController::Private
{
public:

    Private() = default;

    bool                  close     = false;
    bool                  canceled  = false;
    bool                  running   = false;

    QStringList           folderList;

    QWidget*              parent    = nullptr;

    QTimer*               timer     = nullptr;

    DKCamera*             camera    = nullptr;

    QMutex                mutex;
    QWaitCondition        condVar;

    QList<CameraCommand*> cmdThumbs;
    QList<CameraCommand*> commands;
};

CameraController::CameraController(QWidget* const parent,
                                   const QString& title,
                                   const QString& model,
                                   const QString& port,
                                   const QString& path)
    : QThread(parent),
      d      (new Private)
{
    d->parent = parent;

    // URL parsing

    if (path.startsWith(QLatin1String("camera:/")))
    {
        QUrl url(path);
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "path " << path << " " << url <<  " " << url.host();
        QString xport = url.host();

        if (xport.startsWith(QLatin1String("usb:")))
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "xport " << xport;
            QRegularExpression x(QLatin1String("(usb:[0-9,]*)"));
            QRegularExpressionMatch match = x.match(xport);

            if (match.hasMatch())
            {
                QString usbport = match.captured(1);
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "USB " << xport << " " << usbport;

                //if ((xport == usbport) || ((count == 1) && (xport == "usb:")))
                //{
                //   model = xmodel;
                d->camera = new GPCamera(title, url.userName(), QLatin1String("usb:"), QLatin1String("/"));
                //}
            }
        }
    }

    if (!d->camera)
    {
        if (model.toLower() == QLatin1String("directory browse"))
        {
            d->camera = new UMSCamera(title, model, port, path);
        }
        else
        {
            d->camera = new GPCamera(title, model, port, path);
        }
    }

    connect(d->camera, SIGNAL(signalFolderList(QStringList)),
            this, SIGNAL(signalFolderList(QStringList)));

    // setup inter-thread signals

    qRegisterMetaType<CamItemInfo>("CamItemInfo");
    qRegisterMetaType<CamItemInfoList>("CamItemInfoList");

    connect(this, SIGNAL(signalInternalDownloadFailed(QString,QString)),
            this, SLOT(slotDownloadFailed(QString,QString)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalUploadFailed(QString,QString,QString)),
            this, SLOT(slotUploadFailed(QString,QString,QString)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalDeleteFailed(QString,QString)),
            this, SLOT(slotDeleteFailed(QString,QString)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalLockFailed(QString,QString)),
            this, SLOT(slotLockFailed(QString,QString)),
            Qt::BlockingQueuedConnection);

    d->running = true;
}

CameraController::~CameraController()
{
    // clear commands, stop camera

    slotCancel();

    // stop thread
    {
        QMutexLocker lock(&d->mutex);
        d->running = false;
        d->condVar.wakeAll();
    }
    wait();

    delete d->camera;
    delete d;
}

bool CameraController::cameraThumbnailSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->thumbnailSupport();
}

bool CameraController::cameraDeleteSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->deleteSupport();
}

bool CameraController::cameraUploadSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->uploadSupport();
}

bool CameraController::cameraMkDirSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->mkDirSupport();
}

bool CameraController::cameraDelDirSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->delDirSupport();
}

bool CameraController::cameraCaptureImageSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->captureImageSupport();
}

bool CameraController::cameraCaptureImagePreviewSupport() const
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->captureImageSupport() && d->camera->captureImagePreviewSupport();
}

QString CameraController::cameraPath() const
{
    if (!d->camera)
    {
        return QString();
    }

    return d->camera->path();
}

QString CameraController::cameraTitle() const
{
    if (!d->camera)
    {
        return QString();
    }

    return d->camera->title();
}

DKCamera::CameraDriverType CameraController::cameraDriverType() const
{
    if (!d->camera)
    {
        return DKCamera::UMSDriver;
    }

    return d->camera->cameraDriverType();
}

QByteArray CameraController::cameraMD5ID() const
{
    if (!d->camera)
    {
        return QByteArray();
    }

    return d->camera->cameraMD5ID();
}

QIcon CameraController::mimeTypeThumbnail(const QString& itemName) const
{
    if (!d->camera)
    {
        return QPixmap();
    }

    QFileInfo fi(itemName);
    QString mime = d->camera->mimeType(fi.suffix().toLower());

    if      (mime.startsWith(QLatin1String("image/x-raw")))
    {
        return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
    }
    else if (mime.startsWith(QLatin1String("image/")))
    {
        return QIcon::fromTheme(QLatin1String("view-preview"));
    }
    else if (mime.startsWith(QLatin1String("video/")))
    {
        return QIcon::fromTheme(QLatin1String("video-x-generic"));
    }
    else if (mime.startsWith(QLatin1String("audio/")))
    {
        return QIcon::fromTheme(QLatin1String("audio-x-generic"));
    }

    return QIcon::fromTheme(QLatin1String("unknown"));
}

void CameraController::slotCancel()
{
    d->canceled = true;
    d->camera->cancel();

    QMutexLocker lock(&d->mutex);

    qDeleteAll(d->cmdThumbs);
    qDeleteAll(d->commands);
    d->cmdThumbs.clear();
    d->commands.clear();
}

void CameraController::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("CameraController"));       // To customize thread name

    while (d->running)
    {
        CameraCommand* command = nullptr;

        {
            QMutexLocker lock(&d->mutex);

            if (!d->commands.isEmpty())
            {
                command = d->commands.takeFirst();
                Q_EMIT signalBusy(true);
            }
            else
            {
                Q_EMIT signalBusy(false);
            }

            if (!command && !d->cmdThumbs.isEmpty())
            {
                command = d->cmdThumbs.takeFirst();
            }

            if (!command)
            {
                d->condVar.wait(&d->mutex);
                continue;
            }
        }

        // cppcheck-suppress knownConditionTrueFalse
        if (command)
        {
            executeCommand(command);
            delete command;
        }
    }

    Q_EMIT signalBusy(false);
}

void CameraController::executeCommand(CameraCommand* const cmd)
{
    if (!cmd)
    {
        return;
    }

    switch (cmd->action)
    {
        case (CameraCommand::cam_connect):
        {
            sendLogMsg(i18n("Connecting to camera..."));

            bool result = d->camera->doConnect();

            Q_EMIT signalConnected(result);

            if (result)
            {
                d->camera->printSupportedFeatures();
                sendLogMsg(i18n("Connection established."));
            }
            else
            {
                sendLogMsg(i18n("Connection failed."));
            }

            break;
        }

        case (CameraCommand::cam_cameraInformation):
        {
            QString summary, manual, about;

            d->camera->cameraSummary(summary);
            d->camera->cameraManual(manual);
            d->camera->cameraAbout(about);

            Q_EMIT signalCameraInformation(summary, manual, about);

            break;
        }

        case (CameraCommand::cam_freeSpace):
        {
            qint64 bytesSize  = 0;
            qint64 bytesAvail = 0;

            if (!d->camera->getFreeSpace(bytesSize, bytesAvail))
            {
                sendLogMsg(i18n("Failed to get free space from camera"),
                           DHistoryView::ErrorEntry);
            }

            Q_EMIT signalFreeSpace(bytesSize, bytesAvail);

            break;
        }

        case (CameraCommand::cam_preview):
        {
            QImage preview;

            if (!d->camera->getPreview(preview))
            {
                sendLogMsg(i18n("Failed to get preview from camera"),
                           DHistoryView::ErrorEntry);
            }

            Q_EMIT signalPreview(preview);

            break;
        }

        case (CameraCommand::cam_capture):
        {
            CamItemInfo itemInfo;

            if (!d->camera->capture(itemInfo))
            {
                sendLogMsg(i18n("Failed to process capture from camera"),
                           DHistoryView::ErrorEntry);
            }

            Q_EMIT signalUploaded(itemInfo);

            break;
        }

        case (CameraCommand::cam_listfolders):
        {
            QString folder = cmd->map[QLatin1String("folder")].toString();

            if (!d->camera->getFolders(folder))
            {
                sendLogMsg(xi18n("Failed to list folder <filename>%1</filename>", folder),
                           DHistoryView::ErrorEntry);
            }

            break;
        }

        case (CameraCommand::cam_listfiles):
        {
            QString folder   = cmd->map[QLatin1String("folder")].toString();
            bool useMetadata = cmd->map[QLatin1String("useMetadata")].toBool();

            CamItemInfoList itemsList;

            if (!d->camera->getItemsInfoList(folder, useMetadata, itemsList))
            {
                sendLogMsg(xi18n("Failed to list files in <filename>%1</filename>", folder),
                           DHistoryView::ErrorEntry);
            }

            // TODO would it be okay to pass this to the ImportItemModel and let it filter it for us?

            for (CamItemInfoList::iterator it = itemsList.begin() ; it != itemsList.end() ; )
            {
                const CamItemInfo& info = (*it);

                if (info.mime.isEmpty())
                {
                    it = itemsList.erase(it);

                    continue;
                }

                ++it;
            }

            Q_EMIT signalFileList(itemsList);

            break;
        }

        case (CameraCommand::cam_thumbsinfo):
        {
            QList<QVariant> list = cmd->map[QLatin1String("list")].toList();
            int thumbSize        = cmd->map[QLatin1String("thumbSize")].toInt();

            for (QList<QVariant>::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
            {
                if (d->canceled)
                {
                    break;
                }

                QString folder = (*it).toStringList().at(0);
                QString file   = (*it).toStringList().at(1);

                CamItemInfo info;
                info.folder = folder;
                info.name = file;
                QImage thumbnail;

                if (d->camera->getThumbnail(folder, file, thumbnail))
                {
                    thumbnail = thumbnail.scaled(thumbSize, thumbSize, Qt::KeepAspectRatio,
                                                                       Qt::SmoothTransformation);
                    Q_EMIT signalThumbInfo(folder, file, info, thumbnail);
                }
                else
                {
                    sendLogMsg(xi18n("Failed to get thumbnail for <filename>%1</filename>", file),
                               DHistoryView::ErrorEntry, folder, file);
                    Q_EMIT signalThumbInfoFailed(folder, file, info);
                }
            }

            break;
        }

        case (CameraCommand::cam_metadata):
        {
            QString folder = cmd->map[QLatin1String("folder")].toString();
            QString file   = cmd->map[QLatin1String("file")].toString();

            QScopedPointer<DMetadata> meta(new DMetadata);

            if (!d->camera->getMetadata(folder, file, *meta))
            {
                sendLogMsg(xi18n("Failed to get Metadata for <filename>%1</filename>", file),
                           DHistoryView::ErrorEntry, folder, file);
            }

            Q_EMIT signalMetadata(folder, file, meta.data()->data());

            break;
        }

        case (CameraCommand::cam_download):
        {
            QString   folder         = cmd->map[QLatin1String("folder")].toString();
            QString   file           = cmd->map[QLatin1String("file")].toString();
            QString   mime           = cmd->map[QLatin1String("mime")].toString();
            QString   dest           = cmd->map[QLatin1String("dest")].toString();
            bool      documentName   = cmd->map[QLatin1String("documentName")].toBool();
            bool      fixDateTime    = cmd->map[QLatin1String("fixDateTime")].toBool();
            QDateTime newDateTime    = cmd->map[QLatin1String("newDateTime")].toDateTime();
            QString   templateTitle  = cmd->map[QLatin1String("template")].toString();
            bool      convertJpeg    = cmd->map[QLatin1String("convertJpeg")].toBool();
            QString   losslessFormat = cmd->map[QLatin1String("losslessFormat")].toString();

#ifdef HAVE_JXL

            bool      convertDng     = cmd->map[QLatin1String("convertDng")].toBool();
            bool      backupRaw      = cmd->map[QLatin1String("backupRaw")].toBool();
            bool      compressDng    = cmd->map[QLatin1String("compressDng")].toBool();
            int       previewMode    = cmd->map[QLatin1String("previewMode")].toInt();

#endif

            QString   script         = cmd->map[QLatin1String("script")].toString();
            int       pickLabel      = cmd->map[QLatin1String("pickLabel")].toInt();
            int       colorLabel     = cmd->map[QLatin1String("colorLabel")].toInt();
            int       rating         = cmd->map[QLatin1String("rating")].toInt();

            // download to a temp file

            Q_EMIT signalDownloaded(folder, file, QString(), CamItemInfo::DownloadStarted);

            const QString tempTemplate(QLatin1String("%1Camera-XXXXXX.digikamtempfile.%2"));
            SafeTemporaryFile* const temp = new SafeTemporaryFile(tempTemplate.arg(dest).arg(file));
            temp->setAutoRemove(false);
            temp->open();
            QString tempFile              = temp->safeFilePath();
            delete temp;

            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Downloading: " << file << " using " << tempFile;

            bool result = d->camera->downloadItem(folder, file, tempFile);

            if      (!result || d->canceled)
            {
                QFile::remove(tempFile);

                sendLogMsg(xi18n("Failed to download <filename>%1</filename>", file),
                           DHistoryView::ErrorEntry, folder, file);

                Q_EMIT signalDownloaded(folder, file, QString(), CamItemInfo::DownloadFailed);

                break;
            }
            else if (mime == QLatin1String("image/jpeg"))
            {
                // Possible modification operations. Only apply it to JPEG for the moment.

                qCDebug(DIGIKAM_IMPORTUI_LOG) << "Set metadata from: " << file << " using " << tempFile;

                QScopedPointer<DMetadata> metadata(new DMetadata(tempFile));
                bool applyChanges = false;

                if (documentName)
                {
                    metadata->setExifTagString("Exif.Image.DocumentName", file);
                    applyChanges = true;
                }

                if (fixDateTime)
                {
                    metadata->setImageDateTime(newDateTime, true);
                    applyChanges = true;
                }

                // TODO: Set image tags using DMetadata.

                if (colorLabel > NoColorLabel)
                {
                    metadata->setItemColorLabel(colorLabel);
                    applyChanges = true;
                }

                if (pickLabel > NoPickLabel)
                {
                    metadata->setItemPickLabel(pickLabel);
                    applyChanges = true;
                }

                if (rating > RatingMin)
                {
                    metadata->setItemRating(rating);
                    applyChanges = true;
                }

                if (!templateTitle.isEmpty())
                {
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Metadata template title:" << templateTitle;

                    if (templateTitle == Template::removeTemplateTitle())
                    {
                        metadata->removeMetadataTemplate();
                        applyChanges = true;
                    }
                    else
                    {
                        Template t = metadata->getMetadataTemplate();
                        t.merge(TemplateManager::defaultManager()->findByTitle(templateTitle));

                        metadata->setMetadataTemplate(t);
                        applyChanges = true;
                    }
                }

                if (applyChanges)
                {
                    metadata->applyChanges();
                }

                // Convert JPEG file to lossless format if wanted,
                // and move converted image to destination.

                if (convertJpeg)
                {
                    // When converting a file, we need to set the new format extension..

                    QFileInfo convInfo(file);
                    QString convFile               = convInfo.completeBaseName() +
                                                     QLatin1Char('.') + losslessFormat.toLower();
                    SafeTemporaryFile* const temp2 = new SafeTemporaryFile(tempTemplate.arg(dest).arg(convFile));
                    temp2->setAutoRemove(false);
                    temp2->open();
                    convFile                       = temp2->safeFilePath();
                    delete temp2;

                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Convert to LossLess: " << file;

                    if (!JPEGUtils::jpegConvert(tempFile, convFile, file, losslessFormat))
                    {
                        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Convert failed to JPEG!";

                        // convert failed. delete the temp file

                        QFile::remove(convFile);

                        sendLogMsg(xi18n("Failed to convert file <filename>%1</filename> to JPEG", file),
                                   DHistoryView::ErrorEntry, folder, file);
                    }
                    else
                    {
                        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Done, removing the temp file: " << tempFile;

                        // Else remove only the first temp file.

                        QFile::remove(tempFile);
                        tempFile = convFile;
                    }
                }
            }

#ifdef HAVE_JXL

            else if (convertDng && (mime == QLatin1String("image/x-raw")))
            {
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "Convert to DNG: " << file;

                QFileInfo dngInfo(file);

                if  (dngInfo.suffix().toUpper() != QLatin1String("DNG"))
                {
                    QString dngFile                = dngInfo.completeBaseName() + QLatin1String(".dng");
                    SafeTemporaryFile* const temp3 = new SafeTemporaryFile(tempTemplate.arg(dest).arg(dngFile));
                    temp3->setAutoRemove(false);
                    temp3->open();
                    dngFile                        = temp3->safeFilePath();
                    delete temp3;

                    DNGWriter dngWriter;

                    dngWriter.setInputFile(tempFile);
                    dngWriter.setOutputFile(dngFile);
                    dngWriter.setBackupOriginalRawFile(backupRaw);
                    dngWriter.setCompressLossLess(compressDng);
                    dngWriter.setPreviewMode(previewMode);

                    if (dngWriter.convert() != DNGWriter::PROCESS_COMPLETE)
                    {
                        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Convert failed to DNG!";

                        // convert failed. delete the temp file

                        QFile::remove(dngFile);

                        sendLogMsg(xi18n("Failed to convert file <filename>%1</filename> to DNG", file),
                                   DHistoryView::ErrorEntry, folder, file);
                    }
                    else
                    {
                        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Done, removing the temp file: " << tempFile;

                        // Else remove only the first temp file.

                        QFile::remove(tempFile);
                        tempFile = dngFile;
                    }
                }
                else
                {
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Convert skipped to DNG";

                    sendLogMsg(xi18n("Skipped to convert file <filename>%1</filename> to DNG", file),
                               DHistoryView::WarningEntry, folder, file);
                }
            }

#endif
            // Run script

            if (!script.isEmpty())
            {
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "Got a script, processing:" << script;

                QString s;

                if (QFileInfo::exists(script))
                {
                    QFile sFile(script);

                    if (!sFile.open(QIODevice::ReadOnly))
                    {
                        sendLogMsg(xi18n("Failed to open script for <filename>%1</filename>", file),
                                         DHistoryView::ErrorEntry,  folder, file);
                    }
                    else
                    {
                        s = QString::fromUtf8(sFile.readAll());
                    }
                }

                if (s.isEmpty())
                {
                    s = script;
                }

                if (s.indexOf(QLatin1Char('%')) > -1)
                {
                    // %filename must be replaced before %file

                    QFileInfo fileInfo(tempFile);
                    s.replace(QLatin1String("%orgfilename"), file,                Qt::CaseSensitive);
                    s.replace(QLatin1String("%filename"),    fileInfo.fileName(), Qt::CaseSensitive);
                    s.replace(QLatin1String("%orgpath"),     folder,              Qt::CaseSensitive);
                    s.replace(QLatin1String("%path"),        fileInfo.path(),     Qt::CaseSensitive);
                    s.replace(QLatin1String("%file"),        tempFile,            Qt::CaseSensitive);
                }
                else
                {
                    if      (s.endsWith(QLatin1String("\r\n")))
                    {
                        s.chop(2);
                    }
                    else if (s.endsWith(QLatin1String("\n")))
                    {
                        s.chop(1);
                    }

                    s.append(QLatin1String(" \"") + tempFile + QLatin1String("\""));
                }

                // Start the process

                QProcess process;
                process.setProcessChannelMode(QProcess::SeparateChannels);
                process.setProcessEnvironment(adjustedEnvironmentForAppImage());

#ifdef Q_OS_WIN

                QString dir                   = QDir::temp().path();
                SafeTemporaryFile* const temp = new SafeTemporaryFile(dir + QLatin1String("/ImportScript-XXXXXX.cmd"));
                temp->setAutoRemove(false);
                temp->open();
                QString scriptPath            = temp->safeFilePath();

                // Crash fix: a QTemporaryFile is not properly closed until its destructor is called.

                delete temp;

                QFile tempFile(scriptPath);

                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(s.toUtf8());
                    tempFile.close();
                }

                process.start(QLatin1String("cmd.exe"), QStringList() << QLatin1String("/C") << scriptPath);

#else

                process.start(QLatin1String("/bin/bash"), QStringList() << QLatin1String("-c") << s);

#endif
                if (!process.waitForFinished(60000))
                {
                    sendLogMsg(xi18n("Timeout from script for <filename>%1</filename>", file),
                                     DHistoryView::ErrorEntry,  folder, file);
                    process.kill();
                }

                if (process.exitCode() != 0)
                {
                    sendLogMsg(xi18n("Failed to run script for <filename>%1</filename>", file),
                                     DHistoryView::ErrorEntry,  folder, file);
                }

                qCDebug(DIGIKAM_IMPORTUI_LOG) << "stdout" << process.readAllStandardOutput();
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "stderr" << process.readAllStandardError();

#ifdef Q_OS_WIN

                tempFile.remove();

#endif

            }

            Q_EMIT signalDownloaded(folder, file, tempFile, CamItemInfo::DownloadedYes);

            break;
        }

        case (CameraCommand::cam_upload):
        {
            QString folder = cmd->map[QLatin1String("destFolder")].toString();

            // We will using the same source file name to create the dest file
            // name in camera.

            QString file   = cmd->map[QLatin1String("destFile")].toString();

            // The source file path to download in camera.

            QString src    = cmd->map[QLatin1String("srcFilePath")].toString();

            CamItemInfo itemsInfo;

            bool result    = d->camera->uploadItem(folder, file, src, itemsInfo);

            if (result)
            {
                Q_EMIT signalUploaded(itemsInfo);
            }
            else
            {
                Q_EMIT signalInternalUploadFailed(folder, file, src);
            }

            break;
        }

        case (CameraCommand::cam_delete):
        {
            QString folder = cmd->map[QLatin1String("folder")].toString();
            QString file   = cmd->map[QLatin1String("file")].toString();
            bool result    = d->camera->deleteItem(folder, file);

            if (result)
            {
                Q_EMIT signalDeleted(folder, file, true);
            }
            else
            {
                Q_EMIT signalInternalDeleteFailed(folder, file);
            }

            break;
        }

        case (CameraCommand::cam_lock):
        {
            QString folder = cmd->map[QLatin1String("folder")].toString();
            QString file   = cmd->map[QLatin1String("file")].toString();
            bool    lock   = cmd->map[QLatin1String("lock")].toBool();
            bool result    = d->camera->setLockItem(folder, file, lock);

            if (result)
            {
                Q_EMIT signalLocked(folder, file, true);
            }
            else
            {
                Q_EMIT signalInternalLockFailed(folder, file);
            }

            break;
        }

        default:
        {
            qCWarning(DIGIKAM_IMPORTUI_LOG) << " unknown action specified";
            break;
        }
    }
}

void CameraController::sendLogMsg(const QString& msg, DHistoryView::EntryType type,
                                  const QString& folder, const QString& file)
{
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Log (" << file << " " << folder << ": " << msg;

    if (!d->canceled)
    {
        Q_EMIT signalLogMsg(msg, type, folder, file);
    }
}

void CameraController::slotDownloadFailed(const QString& folder, const QString& file)
{
    sendLogMsg(xi18n("Failed to download <filename>%1</filename>", file),
               DHistoryView::ErrorEntry, folder, file);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            QMessageBox::critical(d->parent, qApp->applicationName(),
                                  i18n("Failed to download file <b>%1</b>.", file));
        }
        else
        {
            const QString msg = i18n("Failed to download file <b>%1</b>. Do you want to continue?", file);
            int result        = QMessageBox::warning(d->parent, qApp->applicationName(),
                                                     msg, QMessageBox::Yes | QMessageBox::Cancel);

            if (result != QMessageBox::Yes)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotUploadFailed(const QString& folder, const QString& file, const QString& src)
{
    Q_UNUSED(folder);
    Q_UNUSED(src);

    sendLogMsg(xi18n("Failed to upload <filename>%1</filename>", file),
               DHistoryView::ErrorEntry);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            QMessageBox::critical(d->parent, qApp->applicationName(),
                                  i18n("Failed to upload file <b>%1</b>.", file));
        }
        else
        {
            const QString msg = i18n("Failed to upload file <b>%1</b>. Do you want to continue?", file);
            int result        = QMessageBox::warning(d->parent, qApp->applicationName(),
                                                     msg, QMessageBox::Yes | QMessageBox::Cancel);

            if (result != QMessageBox::Yes)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotDeleteFailed(const QString& folder, const QString& file)
{
    Q_EMIT signalDeleted(folder, file, false);

    sendLogMsg(xi18n("Failed to delete <filename>%1</filename>", file),
               DHistoryView::ErrorEntry, folder, file);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            QMessageBox::critical(d->parent, qApp->applicationName(),
                                  i18n("Failed to delete file <b>%1</b>.", file));
        }
        else
        {
            const QString msg = i18n("Failed to delete file <b>%1</b>. Do you want to continue?", file);
            int result        = QMessageBox::warning(d->parent, qApp->applicationName(),
                                                     msg, QMessageBox::Yes | QMessageBox::Cancel);

            if (result != QMessageBox::Yes)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotLockFailed(const QString& folder, const QString& file)
{
    Q_EMIT signalLocked(folder, file, false);

    sendLogMsg(xi18n("Failed to lock <filename>%1</filename>", file),
               DHistoryView::ErrorEntry, folder, file);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            QMessageBox::critical(d->parent, qApp->applicationName(),
                                  i18n("Failed to toggle lock file <b>%1</b>.", file));
        }
        else
        {
            const QString msg = i18n("Failed to toggle lock file <b>%1</b>. Do you want to continue?", file);
            int result        = QMessageBox::warning(d->parent, qApp->applicationName(),
                                                     msg, QMessageBox::Yes | QMessageBox::Cancel);

            if (result != QMessageBox::Yes)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::addCommand(CameraCommand* const cmd)
{
    QMutexLocker lock(&d->mutex);

    if (cmd->action == CameraCommand::cam_thumbsinfo)
    {
        d->cmdThumbs << cmd;
    }
    else
    {
        d->commands << cmd;
    }

    d->condVar.wakeAll();
}

void CameraController::moveThumbsInfo(CameraCommand* const cmd)
{
    QMutexLocker lock(&d->mutex);

    int index = d->cmdThumbs.indexOf(cmd);

    if (index > 0)
    {
        d->cmdThumbs.move(index, 0);
    }
}

bool CameraController::queueIsEmpty() const
{
    QMutexLocker lock(&d->mutex);

    return (d->commands.isEmpty() && d->cmdThumbs.isEmpty());
}

void CameraController::slotConnect()
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_connect;
    addCommand(cmd);
}

void CameraController::listRootFolder(bool useMetadata)
{
    listFolders(d->camera->path());
    listFiles(d->camera->path(), useMetadata);
}

void CameraController::listFolders(const QString& folder)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_listfolders;
    cmd->map.insert(QLatin1String("folder"), QVariant(folder));

    addCommand(cmd);
}

void CameraController::listFiles(const QString& folder, bool useMetadata)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_listfiles;
    cmd->map.insert(QLatin1String("folder"),      QVariant(folder));
    cmd->map.insert(QLatin1String("useMetadata"), QVariant(useMetadata));
    addCommand(cmd);
}

CameraCommand* CameraController::getThumbsInfo(const CamItemInfoList& list, int thumbSize)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_thumbsinfo;

    QList<QVariant> itemsList;

    for (const CamItemInfo& info : std::as_const(list))
    {
        itemsList.append(QStringList() << info.folder << info.name);
    }

    cmd->map.insert(QLatin1String("list"),      QVariant(itemsList));
    cmd->map.insert(QLatin1String("thumbSize"), QVariant(thumbSize));
    addCommand(cmd);

    return cmd;
}

void CameraController::getMetadata(const QString& folder, const QString& file)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_metadata;
    cmd->map.insert(QLatin1String("folder"), QVariant(folder));
    cmd->map.insert(QLatin1String("file"),   QVariant(file));
    addCommand(cmd);
}

void CameraController::getCameraInformation()
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_cameraInformation;
    addCommand(cmd);
}

void CameraController::getFreeSpace()
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_freeSpace;
    addCommand(cmd);
}

void CameraController::getPreview()
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_preview;
    addCommand(cmd);
}

void CameraController::capture()
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_capture;
    addCommand(cmd);
}

void CameraController::upload(const QFileInfo& srcFileInfo, const QString& destFile, const QString& destFolder)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_upload;
    cmd->map.insert(QLatin1String("srcFilePath"), QVariant(srcFileInfo.filePath()));
    cmd->map.insert(QLatin1String("destFile"),    QVariant(destFile));
    cmd->map.insert(QLatin1String("destFolder"),  QVariant(destFolder));
    addCommand(cmd);
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Uploading '" << srcFileInfo.filePath()
                                  << "' into camera : '" << destFolder
                                  << "' (" << destFile << ")";
}

void CameraController::download(const DownloadSettingsList& list)
{
    for (const DownloadSettings& downloadSettings : std::as_const(list))
    {
        download(downloadSettings);
    }
}

void CameraController::download(const DownloadSettings& downloadSettings)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_download;
    cmd->map.insert(QLatin1String("folder"),            QVariant(downloadSettings.folder));
    cmd->map.insert(QLatin1String("file"),              QVariant(downloadSettings.file));
    cmd->map.insert(QLatin1String("mime"),              QVariant(downloadSettings.mime));
    cmd->map.insert(QLatin1String("dest"),              QVariant(downloadSettings.dest));
    cmd->map.insert(QLatin1String("documentName"),      QVariant(downloadSettings.documentName));
    cmd->map.insert(QLatin1String("fixDateTime"),       QVariant(downloadSettings.fixDateTime));
    cmd->map.insert(QLatin1String("newDateTime"),       QVariant(downloadSettings.newDateTime));
    cmd->map.insert(QLatin1String("template"),          QVariant(downloadSettings.templateTitle));
    cmd->map.insert(QLatin1String("convertJpeg"),       QVariant(downloadSettings.convertJpeg));
    cmd->map.insert(QLatin1String("losslessFormat"),    QVariant(downloadSettings.losslessFormat));

#ifdef HAVE_JXL

    cmd->map.insert(QLatin1String("convertDng"),        QVariant(downloadSettings.convertDng));
    cmd->map.insert(QLatin1String("backupRaw"),         QVariant(downloadSettings.backupRaw));
    cmd->map.insert(QLatin1String("compressDng"),       QVariant(downloadSettings.compressDng));
    cmd->map.insert(QLatin1String("previewMode"),       QVariant(downloadSettings.previewMode));

#endif

    cmd->map.insert(QLatin1String("script"),            QVariant(downloadSettings.script));
    cmd->map.insert(QLatin1String("pickLabel"),         QVariant(downloadSettings.pickLabel));
    cmd->map.insert(QLatin1String("colorLabel"),        QVariant(downloadSettings.colorLabel));
    cmd->map.insert(QLatin1String("rating"),            QVariant(downloadSettings.rating));
/*
    cmd->map.insert(QLatin1String("tagIds"),            QVariant(downloadSettings.tagIds));
*/
    addCommand(cmd);
}

void CameraController::deleteFile(const QString& folder, const QString& file)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_delete;
    cmd->map.insert(QLatin1String("folder"), QVariant(folder));
    cmd->map.insert(QLatin1String("file"),   QVariant(file));
    addCommand(cmd);
}

void CameraController::lockFile(const QString& folder, const QString& file, bool locked)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_lock;
    cmd->map.insert(QLatin1String("folder"), QVariant(folder));
    cmd->map.insert(QLatin1String("file"),   QVariant(file));
    cmd->map.insert(QLatin1String("lock"),   QVariant(locked));
    addCommand(cmd);
}

void CameraController::openFile(const QString& folder, const QString& file)
{
    d->canceled              = false;
    CameraCommand* const cmd = new CameraCommand;
    cmd->action              = CameraCommand::cam_open;
    cmd->map.insert(QLatin1String("folder"), QVariant(folder));
    cmd->map.insert(QLatin1String("file"),   QVariant(file));
    cmd->map.insert(QLatin1String("dest"),   QVariant(QDir::tempPath() + QLatin1Char('/') + file));
    addCommand(cmd);
}

} // namespace Digikam

#include "moc_cameracontroller.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "umscamera.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QTextDocument>
#include <QtGlobal>
#include <QScopedPointer>
#include <QCryptographicHash>
#include <qplatformdefs.h>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Solid includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wnonportable-include-path"
#endif

#include <solid/device.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "dimg.h"
#include "dmetadata.h"
#include "itemscanner.h"
#include "dfileoperations.h"

#ifdef HAVE_MEDIAPLAYER
#   include "videothumbnailer.h"
#   include "videostripfilter.h"
#endif

namespace Digikam
{

UMSCamera::UMSCamera(const QString& title, const QString& model,
                     const QString& port, const QString& path)
    : DKCamera(title, model, port, path)
{
    getUUIDFromSolid();
}

/// Method not supported by UMS camera.
bool UMSCamera::getPreview(QImage& /*preview*/)
{
    return false;
}

/// Method not supported by UMS camera.
bool UMSCamera::capture(CamItemInfo& /*itemInfo*/)
{
    return false;
}

DKCamera::CameraDriverType UMSCamera::cameraDriverType()
{
    return DKCamera::UMSDriver;
}

QByteArray UMSCamera::cameraMD5ID()
{
    QString camData;

    // Camera media UUID is used (if available) to improve fingerprint value
    // registration to database. We want to be sure that MD5 sum is really unique.
    // We don't use camera information here. media UUID is enough because it came be
    // mounted by a card reader or a camera. In this case, "already downloaded" flag will
    // be independent of the device used to mount memory card.

    camData.append(uuid());
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(camData.toUtf8());

    return md5.result().toHex();
}

/// @note implemented in gui, outside the camera thread.
bool UMSCamera::getFreeSpace(qint64& /*bytesSize*/, qint64& /*bytesAvail*/)
{
    return false;
}

bool UMSCamera::doConnect()
{
    QFileInfo dir(m_path);

    if (!dir.exists() || !dir.isReadable() || !dir.isDir())
    {
        return false;
    }

    if (dir.isWritable())
    {
        m_deleteSupport = true;
        m_uploadSupport = true;
        m_mkDirSupport  = true;
        m_delDirSupport = true;
    }
    else
    {
        m_deleteSupport = false;
        m_uploadSupport = false;
        m_mkDirSupport  = false;
        m_delDirSupport = false;
    }

    m_thumbnailSupport    = true;   // UMS camera always support thumbnails.
    m_captureImageSupport = false;  // UMS camera never support capture mode.

    return true;
}

void UMSCamera::cancel()
{
    // set the cancel flag

    m_cancel = true;
}

bool UMSCamera::getFolders(const QString& folder)
{
    if (m_cancel)
    {
        return false;
    }

    QDirIterator it(folder, QDir::Dirs |
                            QDir::NoDotAndDotDot);

    QStringList subFolderList;

    // cppcheck-suppress knownConditionTrueFalse
    while (it.hasNext() && !m_cancel)
    {
        subFolderList << it.next();
    }

    if (subFolderList.isEmpty())
    {
        return true;
    }

    Q_EMIT signalFolderList(subFolderList);

    return true;
}

bool UMSCamera::getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList)
{
    m_cancel = false;
    infoList.clear();

    if (!QFileInfo::exists(folder))
    {
        return false;
    }

    QDirIterator it(folder, QDir::Files |
                            QDir::NoDotAndDotDot);

    while (it.hasNext() && !m_cancel)
    {
        it.next();
        CamItemInfo info;
        getItemInfo(folder, it.fileName(), info, useMetadata);
        infoList.append(info);
    }

    return true;
}

void UMSCamera::getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata)
{
    info.folder           = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
    info.name             = itemName;

    QFileInfo fi(info.folder + info.name);
    info.size             = fi.size();
    info.readPermissions  = fi.isReadable();
    info.writePermissions = fi.isWritable();
    info.mime             = mimeType(fi.suffix().toLower());

    if (!info.mime.isEmpty())
    {
        if (useMetadata)
        {
            // Try to use file metadata

            QScopedPointer<DMetadata> meta(new DMetadata);
            getMetadata(folder, itemName, *meta);
            fillItemInfoFromMetadata(info, *meta);

            // Fall back to file system info

            if (info.ctime.isNull())
            {
                info.ctime = ItemScanner::creationDateFromFilesystem(fi);
            }
        }
        else
        {
            // Only use file system date

            info.ctime = ItemScanner::creationDateFromFilesystem(fi);
        }
    }

    // if we have an image, allow previews
    // TODO allow video previews at some point?

    if (info.mime.startsWith(QLatin1String("image/")) ||
        info.mime.startsWith(QLatin1String("video/")))
    {
        info.previewPossible = true;
    }
}

bool UMSCamera::getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)
{
    m_cancel     = false;
    QString path = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
    path        += itemName;

    // Try to get preview from Exif data (good quality). Can work with Raw files

    QScopedPointer<DMetadata> metadata(new DMetadata(path));
    metadata->getItemPreview(thumbnail);

    if (!thumbnail.isNull())
    {
        return true;
    }

    // RAW files : try to extract embedded thumbnail using RawEngine

    DRawDecoder::loadRawPreview(thumbnail, path);

    if (!thumbnail.isNull())
    {
        return true;
    }

    KSharedConfig::Ptr config  = KSharedConfig::openConfig();
    KConfigGroup group         = config->group(QLatin1String("Camera Settings"));
    bool turnHighQualityThumbs = group.readEntry(QLatin1String("TurnHighQualityThumbs"), false);

    // Try to get thumbnail from Exif data (poor quality).

    if (!turnHighQualityThumbs)
    {
        thumbnail = metadata->getExifThumbnail(true);

        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // THM files: try to get thumbnail from '.thm' files if we didn't manage to get
    // thumbnail from Exif. Any cameras provides *.thm files like JPEG files with RAW files.
    // Using this way is always speed up than ultimate loading using DImg.
    // Note: the thumbnail extracted with this method can be in poor quality.
    // 2006/27/01 - Gilles - Tested with my Minolta Dynax 5D USM camera.

    QFileInfo fi(path);

    if      (thumbnail.load(fi.path() + fi.baseName() + QLatin1String(".thm")))   // Lowercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }
    else if (thumbnail.load(fi.path() + fi.baseName() + QLatin1String(".THM")))   // Uppercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // Finally, we trying to get thumbnail using DImg API (slow).

    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Use DImg loader to get thumbnail from:" << path;

    DImg dimgThumb;

    // skip loading the data we don't need to speed it up.

    dimgThumb.load(path, false /*loadMetadata*/, false /*loadICCData*/, false /*loadUniqueHash*/, false /*loadHistory*/);

    if (!dimgThumb.isNull())
    {
        thumbnail = dimgThumb.copyQImage();

        return true;
    }

#ifdef HAVE_MEDIAPLAYER

    VideoThumbnailer thumbnailer;
    VideoStripFilter videoStrip;

    thumbnailer.addFilter(&videoStrip);
    thumbnailer.generateThumbnail(path, thumbnail);

    if (!thumbnail.isNull())
    {
        return true;
    }

#endif

    return false;
}

bool UMSCamera::getMetadata(const QString& folder, const QString& itemName, DMetadata& meta)
{
    QString path = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
    QFileInfo fi, thmlo, thmup;
    bool ret     = false;

    fi.setFile(path    + itemName);
    thmlo.setFile(path + fi.baseName() + QLatin1String(".thm"));
    thmup.setFile(path + fi.baseName() + QLatin1String(".THM"));

    if      (thmlo.exists())
    {
        // Try thumbnail sidecar files with lowercase extension.

        ret = meta.load(thmlo.filePath());
    }
    else if (thmup.exists())
    {
        // Try thumbnail sidecar files with uppercase extension.

        ret = meta.load(thmup.filePath());
    }
    else
    {
        // If no thumbnail sidecar file available, try to load image metadata for files.

        ret = meta.load(fi.filePath(), true);
    }

    return ret;
}

bool UMSCamera::downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)
{
    m_cancel     = false;
    QString src  = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
    src         += itemName;
    QString dest = saveFile;

    QFile sFile(src);
    QFile dFile(dest);

    if (!sFile.open(QIODevice::ReadOnly))
    {
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Failed to open source file for reading:" << src;

        return false;
    }

    if (!dFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered))
    {
        sFile.close();
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Failed to open destination file for writing:" << dest;

        return false;
    }

    const int  MAX_IPC_SIZE = (1024 * 32);
    QByteArray buffer(MAX_IPC_SIZE, '\0');
    qint64     len;

    while (((len = sFile.read(buffer.data(), MAX_IPC_SIZE)) != 0) && !m_cancel)
    {
        if ((len == -1) || (dFile.write(buffer.data(), len) != len))
        {
            sFile.close();
            dFile.close();

            return false;
        }
    }

    sFile.close();
    dFile.close();

    // Set the file modification time of the downloaded file to the original file.
    // NOTE: this behavior don't need to be managed through Setup/Metadata settings.

    dFile.setPermissions(sFile.permissions());
    DFileOperations::copyModificationTime(src, dest);

    return true;
}

bool UMSCamera::setLockItem(const QString& folder, const QString& itemName, bool lock)
{
    QString src = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
    src        += itemName;

    if (lock)
    {
        // Lock the file to set read only flag

        if (::chmod(src.toUtf8().constData(), S_IREAD) == -1)
        {
            return false;
        }
    }
    else
    {
        // Unlock the file to set read/write flag

        if (::chmod(src.toUtf8().constData(), S_IREAD | S_IWRITE) == -1)
        {
            return false;
        }
    }

    return true;
}

bool UMSCamera::deleteItem(const QString& folder, const QString& itemName)
{
    m_cancel     = false;
    QString path = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;

    // Any camera provide THM (thumbnail) file with real image. We need to remove it also.

    QFileInfo fi(path + itemName);

    QFileInfo thmLo(path + fi.baseName() + QLatin1String(".thm"));          // Lowercase

    if (thmLo.exists())
    {
        QFile::remove(thmLo.filePath());
    }

    QFileInfo thmUp(path + fi.baseName() + QLatin1String(".THM"));          // Uppercase

    if (thmUp.exists())
    {
        QFile::remove(thmUp.filePath());
    }

    // Remove the real image.

    return QFile::remove(path + itemName);
}

bool UMSCamera::uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& info)
{
    m_cancel     = false;
    QString dest = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
    dest        += itemName;
    QString src  = localFile;

    QFile sFile(src);
    QFile dFile(dest);

    if (!sFile.open(QIODevice::ReadOnly))
    {
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Failed to open source file for reading:" << src;

        return false;
    }

    if (!dFile.open(QIODevice::WriteOnly))
    {
        sFile.close();
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Failed to open destination file for writing:" << dest;

        return false;
    }

    const int MAX_IPC_SIZE = (1024 * 32);

    char buffer[MAX_IPC_SIZE];

    qint64 len;

    while (((len = sFile.read(buffer, MAX_IPC_SIZE)) != 0) && !m_cancel)
    {
        if ((len == -1) || (dFile.write(buffer, (quint64)len) == -1))
        {
            sFile.close();
            dFile.close();

            return false;
        }
    }

    sFile.close();
    dFile.close();

    // Set the file modification time of the uploaded file to original file.
    // NOTE: this behavior don't need to be managed through Setup/Metadata settings.

    DFileOperations::copyModificationTime(src, dest);

    // Get new camera item information.

    PhotoInfoContainer        pInfo;
    QScopedPointer<DMetadata> meta(new DMetadata);
    QFileInfo                 fi(dest);
    QString                   mime = mimeType(fi.suffix().toLower());

    if (!mime.isEmpty())
    {
        QSize     dims;
        QDateTime dt;

        // Try to load image metadata.

        meta->load(fi.filePath());
        dt    = meta->getItemDateTime();
        dims  = meta->getItemDimensions();
        pInfo = meta->getPhotographInformation();

        if (dt.isNull()) // fall back to file system info
        {
            dt = ItemScanner::creationDateFromFilesystem(fi);
        }

        info.name             = fi.fileName();
        info.folder           = !folder.endsWith(QLatin1Char('/')) ? folder + QLatin1Char('/') : folder;
        info.mime             = mime;
        info.ctime            = dt;
        info.size             = fi.size();
        info.width            = dims.width();
        info.height           = dims.height();
        info.downloaded       = CamItemInfo::DownloadUnknown;
        info.readPermissions  = fi.isReadable();
        info.writePermissions = fi.isWritable();
        info.photoInfo        = pInfo;
    }

    return true;
}

bool UMSCamera::cameraSummary(QString& summary)
{
    summary =  QString(i18nc("@info", "\"Mounted Camera\" driver for USB/IEEE1394 mass storage cameras and "
                                      "Flash disk card readers.<br><br>"));

    // we do not expect title/model/etc. to contain newlines,
    // so we just escape HTML characters

    summary += i18nc("@info List of device properties",
                     "Title: \"%1\"<br>"
                     "Model: \"%2\"<br>"
                     "Port: \"%3\"<br>"
                     "Path: \"%4\"<br>"
                     "UUID: \"%5\"<br><br>",
                     title(), model(), port(), path(), uuid());

    summary += i18nc("@info List of supported device operations",
                     "Thumbnails: \"%1\"<br>"
                     "Capture image: \"%2\"<br>"
                     "Delete items: \"%3\"<br>"
                     "Upload items: \"%4\"<br>"
                     "Create directories: \"%5\"<br>"
                     "Delete directories: \"%6\"<br><br>",
                     thumbnailSupport()    ? i18nc("@info: ums backend feature", "yes") : i18nc("@info: ums backend feature", "no"),
                     captureImageSupport() ? i18nc("@info: ums backend feature", "yes") : i18nc("@info: ums backend feature", "no"),
                     deleteSupport()       ? i18nc("@info: ums backend feature", "yes") : i18nc("@info: ums backend feature", "no"),
                     uploadSupport()       ? i18nc("@info: ums backend feature", "yes") : i18nc("@info: ums backend feature", "no"),
                     mkDirSupport()        ? i18nc("@info: ums backend feature", "yes") : i18nc("@info: ums backend feature", "no"),
                     delDirSupport()       ? i18nc("@info: ums backend feature", "yes") : i18nc("@info: ums backend feature", "no"));
    return true;
}

bool UMSCamera::cameraManual(QString& manual)
{
    manual = QString(i18nc("@info", "For more information about the \"Mounted Camera\" driver, "
                                    "please read the \"Supported Digital Still "
                                    "Cameras\" section in the digiKam manual."));
    return true;
}

bool UMSCamera::cameraAbout(QString& about)
{
    about = QString(i18nc("@info", "The \"Mounted Camera\" driver is a simple interface to a camera disk "
                                   "mounted locally on your system.<br><br>"
                                   "It does not use libgphoto2 drivers.<br><br>"
                                   "To report any problems with this driver, please contact the digiKam team at:<br><br>"
                                   "https://www.digikam.org/support/contact/"));
    return true;
}

void UMSCamera::getUUIDFromSolid()
{
    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    for (const Solid::Device& accessDevice : std::as_const(devices))
    {
        // check for StorageAccess

        if (!accessDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        const Solid::StorageAccess* const access = accessDevice.as<Solid::StorageAccess>();

        if (!access->isAccessible())
        {
            continue;
        }

        // check for StorageDrive

        Solid::Device driveDevice;

        for (Solid::Device currentDevice = accessDevice ;
             currentDevice.isValid() ;
             currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageDrive>())
            {
                driveDevice = currentDevice;
                break;
            }
        }

        if (!driveDevice.isValid())
        {
            continue;
        }

        // check for StorageVolume

        Solid::Device volumeDevice;

        for (Solid::Device currentDevice = accessDevice ;
             currentDevice.isValid() ;
             currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageVolume>())
            {
                volumeDevice = currentDevice;
                break;
            }
        }

        if (!volumeDevice.isValid())
        {
            continue;
        }

        Solid::StorageVolume* const volume = volumeDevice.as<Solid::StorageVolume>();

        if (m_path.startsWith(QDir::fromNativeSeparators(access->filePath())) &&
            (QDir::fromNativeSeparators(access->filePath()) != QLatin1String("/")))
        {
            m_uuid = volume->uuid();
        }
    }
}

} // namespace Digikam

#include "moc_umscamera.cpp"

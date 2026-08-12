/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Parthasarathy Gopavarapu <gparthasarathy93 at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2016 by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawikiwindow.h"

// Qt includes

#include <QWindow>
#include <QPointer>
#include <QLayout>
#include <QCloseEvent>
#include <QFileInfo>
#include <QFile>
#include <QMenu>
#include <QUrl>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// MediaWiki includes

#include "mediawiki_login.h"
#include "mediawiki_iface.h"

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
#include "wstoolutils.h"
#include "ditemslist.h"
#include "previewloadthread.h"
#include "mediawikiwidget.h"
#include "mediawikitalker.h"
#include "dfileoperations.h"

using namespace MediaWiki;

namespace DigikamGenericMediaWikiPlugin
{

class Q_DECL_HIDDEN MediaWikiWindow::Private
{
public:

    Private() = default;

public:

    QString          tmpDir;
    QString          tmpPath;
    QString          login;
    QString          pass;
    QString          wikiName;
    QUrl             wikiUrl;

    MediaWikiWidget* widget         = nullptr;
    Iface*           MediaWiki      = nullptr;
    DInfoInterface*  iface          = nullptr;
    MediaWikiTalker* uploadTalker   = nullptr;
};

MediaWikiWindow::MediaWikiWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("MediaWiki export dialog")),
      d           (new Private)
{
    d->tmpPath.clear();
    d->tmpDir       = WSToolUtils::makeTemporaryDir("MediaWiki").absolutePath() + QLatin1Char('/');
    d->widget       = new MediaWikiWidget(iface, this);
    d->uploadTalker = nullptr;
    d->login        = QString();
    d->pass         = QString();
    d->iface        = iface;

    setMainWidget(d->widget);
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Export to MediaWiki"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to MediaWiki"));

    startButton()->setEnabled(false);

    d->widget->setMinimumSize(700, 500);
    d->widget->installEventFilter(this);

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(d->widget, SIGNAL(signalChangeUserRequest()),
            this, SLOT(slotChangeUserClicked()));

    connect(d->widget, SIGNAL(signalLoginRequest(QString,QString,QString,QUrl)),
            this, SLOT(slotDoLogin(QString,QString,QString,QUrl)));

    connect(d->widget->progressBar(), SIGNAL(signalProgressCanceled()),
            this, SLOT(slotProgressCanceled()));

    readSettings();
    reactivate();
}

MediaWikiWindow::~MediaWikiWindow()
{
    delete d;
}

void MediaWikiWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void MediaWikiWindow::reactivate()
{
    d->widget->imagesList()->listView()->clear();
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->loadItemInfoFirstLoad();
    d->widget->clearEditFields();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "imagesList items count:" << d->widget->imagesList()->listView()->topLevelItemCount();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "imagesList url length:"  << d->widget->imagesList()->imageUrls(false).size();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "allImagesDesc length:"   << d->widget->allImagesDesc().size();
    show();
}

void MediaWikiWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("MediaWiki export settings"));

    d->widget->readSettings(group);
}

void MediaWikiWindow::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("MediaWiki export settings"));

    d->widget->saveSettings(group);
}

void MediaWikiWindow::slotFinished()
{
    d->widget->progressBar()->progressCompleted();
    saveSettings();
}

void MediaWikiWindow::slotProgressCanceled()
{
    slotFinished();
    reject();
}

bool MediaWikiWindow::prepareImageForUpload(const QString& imgPath)
{
    // Create temporary directory if it does not exist

    if (!QDir(d->tmpDir).exists())
    {
        QDir().mkpath(d->tmpDir);
    }

    // Get temporary file name

    d->tmpPath = d->tmpDir + QFileInfo(imgPath).baseName().trimmed() + QLatin1String(".jpg");

    QImage image;

    // Rescale image if requested: metadata is lost

    if (d->widget->resize())
    {
        image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

        if (image.isNull())
        {
            image.load(imgPath);
        }

        if (image.isNull())
        {
            return false;
        }

        int maxDim = d->widget->dimension();

        if (d->widget->resize() && ((image.width() > maxDim) || (image.height() > maxDim)))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to" << maxDim;
            image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file:" << d->tmpPath;
        image.save(d->tmpPath, "JPEG", d->widget->quality());
    }
    else
    {
        // file is copied with its embedded metadata

        if (!DFileOperations::copyFile(imgPath, d->tmpPath))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "File copy error from:" << imgPath << "to" << d->tmpPath;
            return false;
        }
    }

    // NOTE : In case of metadata are saved to tmp file, we will override metadata processor settings from host
    // to write metadata to image file rather than sidecar file, to be effective with remote web service.

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);

    if (d->widget->removeMeta())
    {
        // save empty metadata to erase them
        meta->save(d->tmpPath);
    }
    else
    {
        // copy meta data from initial to temporary image

        if (meta->load(imgPath))
        {
            if (d->widget->resize())
            {
                meta->setItemDimensions(image.size());
            }

            if (d->widget->removeGeo())
            {
                meta->removeGPSInfo();
            }

            meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta->save(d->tmpPath, true);
        }
    }

    return true;
}

void MediaWikiWindow::slotStartTransfer()
{
    saveSettings();
    QList<QUrl> urls                                    = d->widget->imagesList()->imageUrls(false);
    QMap <QString, QMap <QString, QString> > imagesDesc = d->widget->allImagesDesc();

    for (int i = 0 ; i < urls.size() ; ++i)
    {
        if (d->widget->resize() || d->widget->removeMeta() || d->widget->removeGeo())
        {
            prepareImageForUpload(urls.at(i).toLocalFile());
            imagesDesc.insert(d->tmpPath, imagesDesc.take(urls.at(i).toLocalFile()));
        }
    }

    d->uploadTalker->setImageMap(imagesDesc);

    d->widget->progressBar()->setRange(0, 100);
    d->widget->progressBar()->setValue(0);

    connect(d->uploadTalker, SIGNAL(signalUploadProgress(int)),
            d->widget->progressBar(), SLOT(setValue(int)));

    connect(d->uploadTalker, SIGNAL(signalEndUpload()),
            this, SLOT(slotEndUpload()));

    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("MediaWiki export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(
        QIcon::fromTheme(QLatin1String("dk-mediawiki")).pixmap(22, 22));
    d->uploadTalker->slotBegin();
}

void MediaWikiWindow::slotChangeUserClicked()
{
    startButton()->setEnabled(false);
    d->widget->invertAccountLoginBox();
}

void MediaWikiWindow::slotDoLogin(const QString& login, const QString& pass, const QString& wikiName, const QUrl& wikiUrl)
{
    d->login              = login;
    d->pass               = pass;
    d->wikiName           = wikiName;
    d->wikiUrl            = wikiUrl;
    d->MediaWiki          = new Iface(wikiUrl);
    Login* const loginJob = new Login(*d->MediaWiki, login, pass);

    connect(loginJob, SIGNAL(result(KJob*)),
            this, SLOT(slotLoginHandle(KJob*)));

    loginJob->start();
}

int MediaWikiWindow::slotLoginHandle(KJob* loginJob)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << loginJob->error() << loginJob->errorString() << loginJob->errorText();

    if (loginJob->error())
    {
        d->login.clear();
        d->pass.clear();
        d->uploadTalker = nullptr;
        QMessageBox::critical(this, i18nc("@title:window", "Login Error"), i18n("Please check your credentials and try again."));
    }
    else
    {
        d->uploadTalker = new MediaWikiTalker(d->iface, d->MediaWiki, this);
        startButton()->setEnabled(true);
        d->widget->invertAccountLoginBox();
        d->widget->updateLabels(d->login, d->wikiName, d->wikiUrl.toString());
    }

    return loginJob->error();
}

void MediaWikiWindow::slotEndUpload()
{
    disconnect(d->uploadTalker, SIGNAL(signalUploadProgress(int)),
               d->widget->progressBar(),SLOT(setValue(int)));

    disconnect(d->uploadTalker, SIGNAL(signalEndUpload()),
               this, SLOT(slotEndUpload()));

    QMessageBox::information(this, QString(), i18n("Upload finished with no errors."));
    d->widget->progressBar()->hide();
    d->widget->progressBar()->progressCompleted();
}

bool MediaWikiWindow::eventFilter(QObject* /*obj*/, QEvent* event)
{
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* const c = dynamic_cast<QKeyEvent *>(event);

        if (c && (c->key() == Qt::Key_Return))
        {
            event->ignore();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Key event pass";
            return false;
        }
    }

    return true;
}

} // namespace DigikamGenericMediaWikiPlugin

#include "moc_mediawikiwindow.cpp"

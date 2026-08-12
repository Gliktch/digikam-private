/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a tool to export items to a local storage
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fcexportwindow.h"

// Qt includes

#include <QDir>
#include <QCloseEvent>
#include <QMessageBox>
#include <QHBoxLayout>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "fcexportwidget.h"
#include "ditemslist.h"
#include "fcthread.h"
#include "fctask.h"

namespace DigikamGenericFileCopyPlugin
{

class Q_DECL_HIDDEN FCExportWindow::Private
{
public:

    Private() = default;

public:

    const QString TARGET_URL_PROPERTY           = QLatin1String("targetUrl");
    const QString TARGET_SIDECARS               = QLatin1String("sidecars");
    const QString TARGET_WRITE_METADATA_TO_FILE = QLatin1String("writeMetadataToFile");
    const QString TARGET_OVERWRITE              = QLatin1String("overwrite");
    const QString TARGET_ALBUMPATH              = QLatin1String("albumPath");
    const QString TARGET_BEHAVIOR               = QLatin1String("targetBehavior");
    const QString CONFIG_GROUP                  = QLatin1String("FileCopyExport");

    const QString CHANGE_IMAGE_PROPERTIES       = QLatin1String("changeImageProperties");
    const QString IMAGE_RESIZE                  = QLatin1String("imageResize");
    const QString IMAGE_FORMAT                  = QLatin1String("imageFormat");
    const QString IMAGE_COMPRESSION             = QLatin1String("imageCompression");
    const QString REMOVE_METADATA               = QLatin1String("removeMetadata");

public:

    DItemsList*          imagesList             = nullptr;
    FCExportWidget*      exportWidget           = nullptr;
    FCThread*            thread                 = nullptr;
};

FCExportWindow::FCExportWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("FileCopy Export Dialog")),
      d           (new Private)
{
    QWidget* const page = new QWidget(this);

    // setup image list

    d->imagesList        = new DItemsList(page);
    d->imagesList->setObjectName(QLatin1String("FCExport ImagesList"));
    d->imagesList->setAllowRAW(true);
    d->imagesList->setIface(iface);
    d->imagesList->listView()->setWhatsThis(i18n("This is the list of items to copy "
                                                 "to the specified target."));

    if (iface->forceAlbumSelection)
    {
        iface->forceAlbumSelection = false;
        d->imagesList->loadImagesFromCurrentAlbum();
    }
    else
    {
        d->imagesList->loadImagesFromCurrentSelection();
    }

    // Setup Settings Widget

    d->exportWidget         = new FCExportWidget(iface, page);

    QHBoxLayout* const hlay = new QHBoxLayout(page);
    hlay->addWidget(d->imagesList);
    hlay->addWidget(d->exportWidget);
    hlay->setContentsMargins(QMargins(0, 0, 0, 0));
    hlay->setStretchFactor(d->imagesList,   6);
    hlay->setStretchFactor(d->exportWidget, 4);

    setMainWidget(page);

    // -- Window setup ------------------------------------------------------

    setWindowTitle(i18nc("@title:window", "Export to Local Storage"));
    setModal(false);

    startButton()->setText(i18nc("@action:button", "Start Export"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start export to the specified target"));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelCopy()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotCopy()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(d->imagesList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->exportWidget, SIGNAL(signalTargetUrlChanged(QUrl)),
            this, SLOT(slotTargetUrlChanged(QUrl)));

    // -- initial sync ------------------------------------------------------

    restoreSettings();
    updateUploadButton();
}

FCExportWindow::~FCExportWindow()
{
    delete d;
}

void FCExportWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FCExportWindow::reactivate()
{
    d->imagesList->loadImagesFromCurrentSelection();
    show();
}

void FCExportWindow::restoreSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(d->CONFIG_GROUP);

    FCContainer settings;

    settings.destUrl               = group.readEntry(d->TARGET_URL_PROPERTY,           QUrl());
    settings.behavior              = group.readEntry(d->TARGET_BEHAVIOR,               (int)FCContainer::CopyFile);
    settings.imageFormat           = group.readEntry(d->IMAGE_FORMAT,                  (int)FCContainer::JPEG);
    settings.imageResize           = group.readEntry(d->IMAGE_RESIZE,                  1024);
    settings.imageCompression      = group.readEntry(d->IMAGE_COMPRESSION,             75);
    settings.sidecars              = group.readEntry(d->TARGET_SIDECARS,               false);
    settings.writeMetadataToFile   = group.readEntry(d->TARGET_WRITE_METADATA_TO_FILE, false);
    settings.overwrite             = group.readEntry(d->TARGET_OVERWRITE,              false);
    settings.albumPath             = group.readEntry(d->TARGET_ALBUMPATH,              false);
    settings.removeMetadata        = group.readEntry(d->REMOVE_METADATA,               false);
    settings.changeImageProperties = group.readEntry(d->CHANGE_IMAGE_PROPERTIES,       false);

    d->exportWidget->setSettings(settings);
}

void FCExportWindow::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(d->CONFIG_GROUP);
    FCContainer settings    = d->exportWidget->getSettings();

    group.writeEntry(d->TARGET_URL_PROPERTY,           settings.destUrl);
    group.writeEntry(d->TARGET_BEHAVIOR,               settings.behavior);
    group.writeEntry(d->IMAGE_FORMAT,                  settings.imageFormat);
    group.writeEntry(d->IMAGE_RESIZE,                  settings.imageResize);
    group.writeEntry(d->IMAGE_COMPRESSION,             settings.imageCompression);
    group.writeEntry(d->TARGET_SIDECARS,               settings.sidecars);
    group.writeEntry(d->TARGET_WRITE_METADATA_TO_FILE, settings.writeMetadataToFile);
    group.writeEntry(d->TARGET_OVERWRITE,              settings.overwrite);
    group.writeEntry(d->TARGET_ALBUMPATH,              settings.albumPath);
    group.writeEntry(d->REMOVE_METADATA,               settings.removeMetadata);
    group.writeEntry(d->CHANGE_IMAGE_PROPERTIES,       settings.changeImageProperties);
}

void FCExportWindow::slotCopy()
{
    saveSettings();

    int loopCheck      = 0;
    QString targetPath = d->exportWidget->getSettings().destUrl.toLocalFile();

    while (!QFile::exists(targetPath))
    {
        ++loopCheck;

        if (loopCheck == 1)
        {
            if (
                QMessageBox::question(this, i18nc("@title:window", "Target Location Error"),
                                      i18n("The target folder does not exist. "
                                           "Should it be created?"))
                == QMessageBox::Yes
               )
            {
                QDir().mkpath(targetPath);
            }
            else
            {
                ++loopCheck;
                break;
            }
        }
        else
        {
            QMessageBox::warning(this, i18nc("@title:window", "Target Location Error"),
                                 i18n("The target folder could not be "
                                      "created and does not exist."));
            break;
        }
    }

    if (loopCheck > 1)
    {
        return;
    }

    // start copying and react on signals

    startButton()->setEnabled(false);
    d->exportWidget->setEnabled(false);
    setRejectButtonMode(QDialogButtonBox::Cancel);

    if (d->thread)
    {
        d->thread->cancel();
    }
    else
    {
        d->thread = new FCThread(this);

        connect(d->thread, SIGNAL(finished()),
                this, SLOT(slotCopyingFinished()));

        connect(d->thread, SIGNAL(signalUrlProcessed(QUrl,QUrl)),
                this, SLOT(slotCopyingDone(QUrl,QUrl)));
    }

    d->thread->createCopyJobs(d->imagesList->imageUrls(),
                              d->exportWidget->getSettings());

    d->thread->start();
}

void FCExportWindow::slotImageListChanged()
{
    updateUploadButton();
}

void FCExportWindow::slotTargetUrlChanged(const QUrl& target)
{
    Q_UNUSED(target);

    updateUploadButton();
}

void FCExportWindow::updateUploadButton()
{
    bool listNotEmpty = !d->imagesList->imageUrls().isEmpty();
    startButton()->setEnabled(listNotEmpty && d->exportWidget->targetUrl().isValid());
/*
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Updated upload button with listNotEmpty ="
                                     << listNotEmpty << ", targetUrl().isValid() ="
                                     << d->exportWidget->targetUrl().isValid();
*/
}

void FCExportWindow::slotCopyingDone(const QUrl& from, const QUrl& to)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Copied to:" << to.toLocalFile();

    d->imagesList->blockSignals(true);
    d->imagesList->removeItemByUrl(from);
    d->imagesList->blockSignals(false);
}

void FCExportWindow::slotCopyingFinished()
{
    updateUploadButton();
    d->exportWidget->setEnabled(true);
    setRejectButtonMode(QDialogButtonBox::Close);

    if (!d->imagesList->imageUrls().isEmpty())
    {
        QMessageBox::information(this, i18nc("@title:window", "Copy not Completed"),
                                 i18n("Some of the items have not been copied "
                                      "and are still in the list. "
                                      "You can retry to copy these items now."));
    }
}

void FCExportWindow::slotCancelCopy()
{
    if (d->thread)
    {
        d->thread->cancel();
    }
}

void FCExportWindow::slotFinished()
{
    saveSettings();
    updateUploadButton();
    d->imagesList->listView()->clear();
}

} // namespace DigikamGenericFileCopyPlugin

#include "moc_fcexportwindow.cpp"

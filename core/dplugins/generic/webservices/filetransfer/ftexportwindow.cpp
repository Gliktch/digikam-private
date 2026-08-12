/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible location
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ftexportwindow.h"

// Qt includes

#include <QCloseEvent>
#include <QMessageBox>
#include <QHBoxLayout>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kio/job.h>
#include <kio/copyjob.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "ditemslist.h"
#include "ftexportwidget.h"

namespace DigikamGenericFileTransferPlugin
{

class Q_DECL_HIDDEN FTExportWindow::Private
{
public:

    Private() = default;

public:

    const QString TARGET_URL_PROPERTY   = QLatin1String("targetUrl");
    const QString HISTORY_URL_PROPERTY  = QLatin1String("historyUrls");
    const QString CONFIG_GROUP          = QLatin1String("KioExport");

public:

    DItemsList*     imagesList          = nullptr;
    FTExportWidget* exportWidget        = nullptr;
};

FTExportWindow::FTExportWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Kio Export Dialog")),
      d           (new Private)
{
    QWidget* const page     = new QWidget(this);

    // Setup image list

    d->imagesList           = new DItemsList(page);
    d->imagesList->setObjectName(QLatin1String("FTExport ImagesList"));
    d->imagesList->setIface(iface);
    d->imagesList->loadImagesFromCurrentSelection();
    d->imagesList->setAllowRAW(true);
    d->imagesList->listView()->setWhatsThis(i18n("This is the list of images to upload "
                                                 "to the specified target."));

    d->exportWidget         = new FTExportWidget(iface, page);

    QHBoxLayout* const hlay = new QHBoxLayout(page);
    hlay->addWidget(d->imagesList);
    hlay->addWidget(d->exportWidget);
    hlay->setContentsMargins(QMargins(0, 0, 0, 0));
    hlay->setStretchFactor(d->imagesList,   6);
    hlay->setStretchFactor(d->exportWidget, 4);

    setMainWidget(page);

    // -- Window setup ------------------------------------------------------

    setWindowTitle(i18nc("@title:window", "Export to Remote Storage"));
    setModal(false);

    startButton()->setText(i18nc("@action:button", "Start Export"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start export to the specified target"));

    const auto btns = m_buttons->buttons();

    for (QAbstractButton* const btn : btns)
    {
        static_cast<QPushButton*>(btn)->setDefault(false);
    }

    // ---

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotUpload()));

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

FTExportWindow::~FTExportWindow()
{
    delete d;
}

void FTExportWindow::slotFinished()
{
    saveSettings();
    d->imagesList->listView()->clear();
}

void FTExportWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FTExportWindow::reactivate()
{
    d->imagesList->loadImagesFromCurrentSelection();
    show();
}

void FTExportWindow::restoreSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(d->CONFIG_GROUP);
    d->exportWidget->setHistory(group.readEntry(d->HISTORY_URL_PROPERTY, QList<QUrl>()));
    d->exportWidget->setTargetUrl(group.readEntry(d->TARGET_URL_PROPERTY, QUrl()));
}

void FTExportWindow::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(d->CONFIG_GROUP);
    group.writeEntry(d->HISTORY_URL_PROPERTY, d->exportWidget->history());
    group.writeEntry(d->TARGET_URL_PROPERTY,  d->exportWidget->targetUrl().url());
}

void FTExportWindow::slotImageListChanged()
{
    updateUploadButton();
}

void FTExportWindow::slotTargetUrlChanged(const QUrl& target)
{
    Q_UNUSED(target);

    updateUploadButton();
}

void FTExportWindow::updateUploadButton()
{
    bool listNotEmpty = !d->imagesList->imageUrls().isEmpty();
    startButton()->setEnabled(listNotEmpty && d->exportWidget->targetUrl().isValid());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Updated upload button with listNotEmpty = "
                                     << listNotEmpty << ", targetUrl().isValid() = "
                                     << d->exportWidget->targetUrl().isValid();
}

void FTExportWindow::slotCopyingDone(KIO::Job* job,
                                     const QUrl& from,
                                     const QUrl& to,
                                     const QDateTime& mtime,
                                     bool directory,
                                     bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(to);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "copied " << to.toDisplayString();

    d->imagesList->removeItemByUrl(from);
}

void FTExportWindow::slotCopyingFinished(KJob* job)
{
    Q_UNUSED(job);

    setEnabled(true);

    if (!d->imagesList->imageUrls().isEmpty())
    {
        QMessageBox::information(this, i18nc("@title:window", "Upload not Completed"),
                                 i18n("Some of the images have not been transferred "
                                      "and are still in the list. "
                                      "You can retry to export these images now."));
    }
}

void FTExportWindow::slotUpload()
{
    saveSettings();

    // start copying and react on signals

    setEnabled(false);
    KIO::CopyJob* const copyJob = KIO::copy(d->imagesList->imageUrls(),
                                            d->exportWidget->targetUrl());

    connect(copyJob, SIGNAL(copyingDone(KIO::Job*,QUrl,QUrl,QDateTime,bool,bool)),
            this, SLOT(slotCopyingDone(KIO::Job*,QUrl,QUrl,QDateTime,bool,bool)));

    connect(copyJob, SIGNAL(result(KJob*)),
            this, SLOT(slotCopyingFinished(KJob*)));
}

} // namespace DigikamGenericFileTransferPlugin

#include "moc_ftexportwindow.cpp"

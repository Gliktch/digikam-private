/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 04.10.2009
 * Description : a tool to import items from a KIO accessible location
 *
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ftimportwindow.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QCloseEvent>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "ftimportwidget.h"
#include "ditemslist.h"

namespace DigikamGenericFileTransferPlugin
{

class Q_DECL_HIDDEN FTImportWindow::Private
{
public:

    Private() = default;

public:

    const QString SOURCE_URL_PROPERTY   = QLatin1String("sourceUrl");
    const QString HISTORY_URL_PROPERTY  = QLatin1String("historyUrls");
    const QString CONFIG_GROUP          = QLatin1String("KioImport");

public:

    FTImportWidget* importWidget    = nullptr;
    DInfoInterface* iface           = nullptr;
};

FTImportWindow::FTImportWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Kio Import Dialog")),
      d           (new Private)
{
    d->iface        = iface;
    d->importWidget = new FTImportWidget(this, d->iface);
    setMainWidget(d->importWidget);

    // window setup

    setWindowTitle(i18nc("@title:window", "Import from Remote Storage"));
    setModal(false);
    startButton()->setEnabled(false);

    startButton()->setText(i18nc("@action:button", "Start Import"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start importing the specified images "
                                   "into the currently selected album."));

    // connections

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotImport()));

    connect(d->importWidget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotSourceAndTargetUpdated()));

    connect(d->iface, SIGNAL(signalUploadUrlChanged()),
            this, SLOT(slotSourceAndTargetUpdated()));

    // -- initial sync ------------------------------------------------------

    restoreSettings();

    slotSourceAndTargetUpdated();
}

FTImportWindow::~FTImportWindow()
{
    delete d;
}

void FTImportWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    saveSettings();
    e->accept();
}

void FTImportWindow::slotImport()
{
    QUrl url = d->iface->uploadUrl();

    if (!url.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "starting to import urls: " << d->importWidget->sourceUrls();

        // start copying and react on signals

        setEnabled(false);

        KIO::CopyJob* const copyJob = KIO::copy(d->importWidget->imagesList()->imageUrls(), url);

        connect(copyJob, SIGNAL(copyingDone(KIO::Job*,QUrl,QUrl,QDateTime,bool,bool)),
                this, SLOT(slotCopyingDone(KIO::Job*,QUrl,QUrl,QDateTime,bool,bool)));

        connect(copyJob, SIGNAL(result(KJob*)),
                this, SLOT(slotCopyingFinished(KJob*)));
    }
}

void FTImportWindow::slotCopyingDone(KIO::Job* job,
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

    d->importWidget->imagesList()->removeItemByUrl(from);
}

void FTImportWindow::slotCopyingFinished(KJob* job)
{
    Q_UNUSED(job);

    setEnabled(true);

    if (!d->importWidget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::information(this, i18nc("@title:window", "Import not Completed"),
                                 i18n("Some of the images have not been transferred "
                                      "and are still in the list. "
                                      "You can retry to import these images now."));
    }

    saveSettings();
}

void FTImportWindow::slotSourceAndTargetUpdated()
{
    bool hasUrlToImport = !d->importWidget->sourceUrls().isEmpty();
    bool hasTarget      = !d->iface->uploadUrl().isEmpty();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "switching import button activity with: hasUrlToImport = "
                                     << hasUrlToImport << ", hasTarget = " << hasTarget;

    startButton()->setEnabled(hasUrlToImport && hasTarget);
}

void FTImportWindow::restoreSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(d->CONFIG_GROUP);
    d->importWidget->setHistory(group.readEntry(d->HISTORY_URL_PROPERTY, QList<QUrl>()));
    d->importWidget->setSourceUrl(group.readEntry(d->SOURCE_URL_PROPERTY, QUrl()));
}

void FTImportWindow::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(d->CONFIG_GROUP);
    group.writeEntry(d->HISTORY_URL_PROPERTY, d->importWidget->history());
    group.writeEntry(d->SOURCE_URL_PROPERTY,  d->importWidget->sourceUrl().url());
}

} // namespace DigikamGenericFileTransferPlugin

#include "moc_ftimportwindow.cpp"

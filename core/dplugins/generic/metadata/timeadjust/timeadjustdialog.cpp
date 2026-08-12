/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : dialog to set time stamp of picture files.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2005 by Jesper Pedersen <blackie at kde dot org>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustdialog.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QTimer>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QDateTime>
#include <QTimeEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QMessageBox>
#include <QWindow>
#include <QScopedPointer>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "digikam_version.h"
#include "dmetadata.h"
#include "dprogresswdg.h"
#include "dxmlguiwindow.h"
#include "timeadjustlist.h"
#include "timeadjustthread.h"

namespace DigikamGenericTimeAdjustPlugin
{

class Q_DECL_HIDDEN TimeAdjustDialog::Private
{

public:

    Private() = default;

    TimeAdjustSettings*   settingsView  = nullptr;

    QMap<QUrl, int>       itemsUsedMap;           // Map of item urls and index.

    QTimer*               previewTimer  = nullptr;
    QTimer*               updateTimer   = nullptr;

    bool                  exitAfterOk   = false;
    bool                  isProcessed   = false;

    DProgressWdg*         progressBar   = nullptr;
    TimeAdjustList*       listView      = nullptr;

    TimeAdjustThread*     thread        = nullptr;

    DInfoInterface*       iface         = nullptr;
};

TimeAdjustDialog::TimeAdjustDialog(QWidget* const parent, DInfoInterface* const iface)
    : DPluginDialog(parent, QLatin1String("Time Adjust Dialog")),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "Adjust Time & Date"));
    setMinimumSize(900, 500);
    setModal(true);

    d->iface = iface;

    m_buttons->addButton(QDialogButtonBox::Yes);
    m_buttons->addButton(QDialogButtonBox::Apply);
    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->button(QDialogButtonBox::Yes)->setText(i18n("OK"));
    m_buttons->button(QDialogButtonBox::Yes)->setAutoDefault(false);
    m_buttons->button(QDialogButtonBox::Apply)->setAutoDefault(false);
    m_buttons->button(QDialogButtonBox::Close)->setAutoDefault(false);

    m_buttons->button(QDialogButtonBox::Yes)->setToolTip(i18nc("@info:tooltip",
                                                               "Write the corrected date and time for each image and close dialog"));
    m_buttons->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip",
                                                                 "Write the corrected date and time for each image"));


    QWidget* const mainWidget = new QWidget(this);
    QVBoxLayout* const vbx    = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(m_buttons);
    setLayout(vbx);

    QGridLayout* const mainLayout = new QGridLayout(mainWidget);
    d->listView                   = new TimeAdjustList(mainWidget);
    d->settingsView               = new TimeAdjustSettings(mainWidget, true);
    d->progressBar                = new DProgressWdg(mainWidget);
    d->progressBar->reset();
    d->progressBar->hide();

    // ----------------------------------------------------------------------------

    mainLayout->addWidget(d->listView,     0, 0, 2, 1);
    mainLayout->addWidget(d->settingsView, 0, 1, 1, 1);
    mainLayout->addWidget(d->progressBar,  1, 1, 1, 1);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setContentsMargins(QMargins());

    // ----------------------------------------------------------------------------

    d->previewTimer = new QTimer(this);
    d->previewTimer->setSingleShot(true);
    d->previewTimer->setInterval(500);

    d->updateTimer = new QTimer(this);
    d->updateTimer->setSingleShot(true);
    d->updateTimer->setInterval(250);

    connect(d->previewTimer, SIGNAL(timeout()),
            this, SLOT(slotPreviewTimer()));

    connect(d->updateTimer, SIGNAL(timeout()),
            this, SLOT(slotUpdateTimer()));

    // -- Thread Slots/Signals ----------------------------------------------

    d->thread = new TimeAdjustThread(this, d->iface);

    connect(d->thread, SIGNAL(signalProcessEnded(QUrl,QDateTime,QDateTime,int)),
            this, SLOT(slotProcessEnded(QUrl,QDateTime,QDateTime,int)));

    connect(d->thread, SIGNAL(signalPreviewReady(QUrl,QDateTime,QDateTime)),
            this, SLOT(slotPreviewReady(QUrl,QDateTime,QDateTime)));

    connect(d->thread, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)),
            d->iface, SLOT(slotDateTimeForUrl(QUrl,QDateTime,bool)));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotCancelThread()));

    connect(d->thread, SIGNAL(signalProcessStarted(QUrl)),
            this, SLOT(slotProcessStarted(QUrl)));

    connect(d->thread, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));

    // -- Dialog Slots/Signals -----------------------------------------------

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(slotCancelThread()));

    connect(m_buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotUpdateTimestamps()));

    connect(m_buttons->button(QDialogButtonBox::Yes), SIGNAL(clicked()),
            this, SLOT(slotOkExitTimestamps()));

    connect(d->settingsView, SIGNAL(signalSrcTimestampChanged()),
            d->thread, SLOT(slotSrcTimestampChanged()));

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            d->previewTimer, SLOT(start()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotDialogFinished()));

    connect(d->listView->listView(), &DItemsListView::itemSelectionChanged,
            this, [this]()
        {
            d->settingsView->setCurrentItemUrl(d->listView->getCurrentUrl());
        }
    );

    // -----------------------------------------------------------------------

    setBusy(false);
    readSettings();

    int index = 0;

    d->listView->setIface(d->iface);
    d->listView->loadImagesFromCurrentSelection();
    const auto urls = d->listView->imageUrls();

    for (const QUrl& url : urls)
    {
        d->itemsUsedMap.insert(url, index);
        ++index;
    }

    d->previewTimer->start();
}

TimeAdjustDialog::~TimeAdjustDialog()
{
    if (d->thread->isRunning())
    {
        d->thread->cancel();
        d->thread->wait();
    }

    delete d;
}

void TimeAdjustDialog::closeEvent(QCloseEvent* e)
{
    if (d->thread->isRunning())
    {
        d->thread->cancel();
        d->thread->wait();

        e->ignore();

        return;
    }

    slotDialogFinished();

    e->accept();
}

void TimeAdjustDialog::slotDialogFinished()
{
    saveSettings();
}

void TimeAdjustDialog::readSettings()
{
    TimeAdjustContainer prm;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Time Adjust Settings"));

    prm.customDate     = group.readEntry(QLatin1String("Custom Date"),                     QDateTime());
    prm.customDate     = asDateTimeUTC(prm.customDate);
    prm.customTime     = group.readEntry(QLatin1String("Custom Time"),                     QDateTime());
    prm.customTime     = asDateTimeUTC(prm.customTime);
    prm.adjustmentType = group.readEntry(QLatin1String("Adjustment Type"),                 0);
    prm.adjustmentDays = group.readEntry(QLatin1String("Adjustment Days"),                 0);
    prm.adjustmentTime = group.readEntry(QLatin1String("Adjustment Time"),                 QDateTime());
    prm.adjustmentTime = asDateTimeUTC(prm.adjustmentTime);

    prm.updUseExifTool = group.readEntry(QLatin1String("Update Timestamps With ExifTool"), false);
    prm.updIfAvailable = group.readEntry(QLatin1String("Update Only If Available Time"),   true);
    prm.updFileModDate = group.readEntry(QLatin1String("Update File Modification Time"),   true);
    prm.updEXIFModDate = group.readEntry(QLatin1String("Update EXIF Modification Time"),   true);
    prm.updEXIFOriDate = group.readEntry(QLatin1String("Update EXIF Original Time"),       true);
    prm.updEXIFDigDate = group.readEntry(QLatin1String("Update EXIF Digitization Time"),   true);
    prm.updEXIFThmDate = group.readEntry(QLatin1String("Update EXIF Thumbnail Time"),      true);
    prm.updIPTCDate    = group.readEntry(QLatin1String("Update IPTC Time"),                true);
    prm.updXMPVideo    = group.readEntry(QLatin1String("Update XMP Video Time"),           true);
    prm.updXMPDate     = group.readEntry(QLatin1String("Update XMP Creation Time"),        true);

    prm.dateSource     = group.readEntry(QLatin1String("Use Timestamp Type"),              0);
    prm.metadataSource = group.readEntry(QLatin1String("Meta Timestamp Type"),             0);
    prm.fileDateSource = group.readEntry(QLatin1String("File Timestamp Type"),             0);

    d->settingsView->setSettings(prm);
}

void TimeAdjustDialog::saveSettings()
{
    TimeAdjustContainer prm   = d->settingsView->settings();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QLatin1String("Time Adjust Settings"));

    group.writeEntry(QLatin1String("Custom Date"),                     prm.customDate);
    group.writeEntry(QLatin1String("Custom Time"),                     prm.customTime);

    group.writeEntry(QLatin1String("Adjustment Type"),                 prm.adjustmentType);
    group.writeEntry(QLatin1String("Adjustment Days"),                 prm.adjustmentDays);
    group.writeEntry(QLatin1String("Adjustment Time"),                 prm.adjustmentTime);

    group.writeEntry(QLatin1String("Update Timestamps With ExifTool"), prm.updUseExifTool);
    group.writeEntry(QLatin1String("Update Only If Available Time"),   prm.updIfAvailable);
    group.writeEntry(QLatin1String("Update File Modification Time"),   prm.updFileModDate);
    group.writeEntry(QLatin1String("Update EXIF Modification Time"),   prm.updEXIFModDate);
    group.writeEntry(QLatin1String("Update EXIF Original Time"),       prm.updEXIFOriDate);
    group.writeEntry(QLatin1String("Update EXIF Digitization Time"),   prm.updEXIFDigDate);
    group.writeEntry(QLatin1String("Update EXIF Thumbnail Time"),      prm.updEXIFThmDate);
    group.writeEntry(QLatin1String("Update IPTC Time"),                prm.updIPTCDate);
    group.writeEntry(QLatin1String("Update XMP Video Time"),           prm.updXMPVideo);
    group.writeEntry(QLatin1String("Update XMP Creation Time"),        prm.updXMPDate);

    group.writeEntry(QLatin1String("Use Timestamp Type"),              prm.dateSource);
    group.writeEntry(QLatin1String("Meta Timestamp Type"),             prm.metadataSource);
    group.writeEntry(QLatin1String("File Timestamp Type"),             prm.fileDateSource);
}

void TimeAdjustDialog::slotUpdateTimestamps()
{
    if (d->thread->isRunning())
    {
        d->thread->cancel();
        d->thread->wait();
    }

    d->isProcessed = true;
    d->updateTimer->start();
}

void TimeAdjustDialog::slotOkExitTimestamps()
{
    if (d->isProcessed)
    {
        accept();

        return;
    }

    d->exitAfterOk = true;
    slotUpdateTimestamps();
}

void TimeAdjustDialog::slotPreviewTimer()
{
    if      (d->thread->isRunning() && d->isProcessed)
    {
        return;
    }
    else if (d->thread->isRunning())
    {
        d->thread->cancel();
        d->thread->wait();
    }

    d->listView->setWaitStatus();

    d->thread->setSettings(d->settingsView->settings());
    d->thread->setPreviewDates(d->itemsUsedMap);
    d->thread->start();
}

void TimeAdjustDialog::slotUpdateTimer()
{
    d->listView->setWaitStatus();

    d->progressBar->show();
    d->progressBar->reset();
    d->progressBar->setMaximum(d->itemsUsedMap.size());
    d->progressBar->progressScheduled(i18nc("@info", "Adjust Time and Date"), true, true);
    d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("appointment-new")).pixmap(22, 22));

    d->thread->setSettings(d->settingsView->settings());
    d->thread->setUpdatedDates(d->itemsUsedMap);
    d->thread->start();

    setBusy(true);
}

void TimeAdjustDialog::slotCancelThread()
{
    if (d->thread->isRunning())
    {
        d->thread->cancel();
        d->thread->wait();
    }

    d->exitAfterOk = false;
    d->isProcessed = false;

    if (m_buttons->button(QDialogButtonBox::Yes)->isEnabled())
    {
        accept();
    }
}

void TimeAdjustDialog::setBusy(bool busy)
{
    if (busy)
    {
        m_buttons->button(QDialogButtonBox::Close)->setText(i18nc("@action", "Cancel"));
        m_buttons->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));
        m_buttons->button(QDialogButtonBox::Close)->setToolTip(i18nc("@info", "Cancel current operation"));
    }
    else
    {
        m_buttons->button(QDialogButtonBox::Close)->setText(i18nc("@action", "Close"));
        m_buttons->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QLatin1String("window-close")));
        m_buttons->button(QDialogButtonBox::Close)->setToolTip(i18nc("@info", "Close window"));
    }

    m_buttons->button(QDialogButtonBox::Apply)->setEnabled(!d->isProcessed && !busy);
    m_buttons->button(QDialogButtonBox::Yes)->setEnabled(!busy);
    d->settingsView->setEnabled(!busy);
}

void TimeAdjustDialog::slotProcessStarted(const QUrl& url)
{
    d->listView->processing(url);
}

void TimeAdjustDialog::slotPreviewReady(const QUrl& url,
                                        const QDateTime& org,
                                        const QDateTime& adj)
{
    d->listView->setStatus(url, org, adj);
    d->isProcessed = false;
}

void TimeAdjustDialog::slotProcessEnded(const QUrl& url,
                                        const QDateTime& org,
                                        const QDateTime& adj, int status)
{
    d->listView->processed(url, (status == TimeAdjustList::NOPROCESS_ERROR));
    d->listView->setStatus(url, org, adj, status);

    d->progressBar->setValue(d->progressBar->value() + 1);
}

void TimeAdjustDialog::slotThreadFinished()
{
    setBusy(false);
    d->progressBar->hide();
    d->progressBar->progressCompleted();

    if (d->exitAfterOk)
    {
        accept();
    }
}

} // namespace DigikamGenericTimeAdjustPlugin

#include "moc_timeadjustdialog.cpp"

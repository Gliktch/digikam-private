/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG converter batch dialog
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Veaceslav Munteanu <slavuttici at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngconverterdialog.h"

// Qt includes

#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QPixmap>
#include <QTimer>
#include <QTreeWidgetItemIterator>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>
#include <QCursor>
#include <QBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dngconverteractions.h"
#include "dngconverterthread.h"
#include "dngconverterlist.h"
#include "filesaveconflictbox.h"
#include "dngsettings.h"
#include "dprogresswdg.h"
#include "dngwriter.h"
#include "dmetadata.h"
#include "digikam_debug.h"
#include "imagedialog.h"
#include "dexpanderbox.h"
#include "dfileoperations.h"

using namespace Digikam;

namespace DigikamGenericDNGConverterPlugin
{

class Q_DECL_HIDDEN DNGConverterDialog::Private
{
public:

    Private() = default;

public:

    bool                      busy              = false;

    QStringList               fileList;

    DProgressWdg*             progressBar       = nullptr;

    DNGConverterList*         listView          = nullptr;

    DNGConverterActionThread* thread            = nullptr;

    DNGSettings*              dngSettings       = nullptr;

    FileSaveConflictBox*      conflictSettings  = nullptr;

    DInfoInterface*           iface             = nullptr;
};

DNGConverterDialog::DNGConverterDialog(QWidget* const parent, DInfoInterface* const iface)
    : DPluginDialog(parent, QLatin1String("DNG Converter Dialog")),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "DNG Converter"));
    setMinimumSize(900, 500);
    setModal(true);

    d->iface                  = iface;

    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->addButton(QDialogButtonBox::Ok);
    m_buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button", "&Convert"));

    QWidget* const mainWidget = new QWidget(this);
    QVBoxLayout* const vbx    = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(m_buttons);
    setLayout(vbx);

    //---------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(mainWidget);
    d->listView                   = new DNGConverterList(mainWidget);
    d->progressBar                = new DProgressWdg(mainWidget);
    d->progressBar->reset();
    d->progressBar->hide();

    d->listView->appendControlButtonsWidget(d->progressBar);
    QBoxLayout* const blay        = d->listView->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    blay->setStretchFactor(d->progressBar, 20);

    d->dngSettings                = new DNGSettings(this);
    DLineWidget* const line       = new DLineWidget(Qt::Horizontal, this);
    d->conflictSettings           = new FileSaveConflictBox(this);

    mainLayout->addWidget(d->listView,         0, 0, 5, 1);
    mainLayout->addWidget(d->dngSettings,      0, 1, 1, 1);
    mainLayout->addWidget(line,                1, 1, 1, 1);
    mainLayout->addWidget(d->conflictSettings, 2, 1, 1, 1);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setRowStretch(3, 10);
    mainLayout->setContentsMargins(QMargins());

    // ---------------------------------------------------------------

    d->thread = new DNGConverterActionThread(this);

    connect(d->thread, SIGNAL(signalStarting(DigikamGenericDNGConverterPlugin::DNGConverterActionData)),
            this, SLOT(slotDNGConverterAction(DigikamGenericDNGConverterPlugin::DNGConverterActionData)));

    connect(d->thread, SIGNAL(signalFinished(DigikamGenericDNGConverterPlugin::DNGConverterActionData)),
            this, SLOT(slotDNGConverterAction(DigikamGenericDNGConverterPlugin::DNGConverterActionData)));

    connect(d->thread, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));

    // ---------------------------------------------------------------

    connect(m_buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotStartStop()));

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(slotClose()));

    connect(d->listView, SIGNAL(signalImageListChanged()),
            this, SLOT(slotIdentify()));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStartStop()));

    connect(d->dngSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotIdentify()));

    connect(d->dngSettings, SIGNAL(signalSetupExifTool()),
            this, SLOT(slotSetupExifTool()));

    connect(d->conflictSettings, SIGNAL(signalConflictButtonChanged(int)),
            this, SLOT(slotIdentify()));

    // ---------------------------------------------------------------

    d->listView->setIface(d->iface);
    d->listView->loadImagesFromCurrentSelection();

    setBusy(false);
    readSettings();
}

DNGConverterDialog::~DNGConverterDialog()
{
    delete d;
}

void DNGConverterDialog::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    // Stop current conversion if necessary

    if (d->busy)
    {
        slotStartStop();
    }

    saveSettings();
    d->listView->listView()->clear();
    e->accept();
}

void DNGConverterDialog::slotSetupExifTool()
{
    if (d->iface)
    {
        connect(d->iface, SIGNAL(signalSetupChanged()),
                d->dngSettings, SLOT(slotSetupChanged()));

        d->iface->openSetupPage(DInfoInterface::ExifToolPage);
    }
}

void DNGConverterDialog::slotClose()
{
    // Stop current conversion if necessary

    if (d->busy)
    {
        slotStartStop();
    }

    saveSettings();
    d->listView->listView()->clear();
    d->fileList.clear();
    accept();
}

void DNGConverterDialog::slotDefault()
{
    d->dngSettings->setDefaultSettings();
    d->conflictSettings->resetToDefault();
}

void DNGConverterDialog::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("DNGConverter Settings"));

    d->dngSettings->setBackupOriginalRawFile(group.readEntry("BackupOriginalRawFile", false));
    d->dngSettings->setCompressLossLess(group.readEntry("CompressLossLess",           true));
    d->dngSettings->setPreviewMode(group.readEntry("PreviewMode",                     (int)(DNGWriter::FULL_SIZE)));
    d->conflictSettings->readSettings(group);
}

void DNGConverterDialog::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("DNGConverter Settings"));

    group.writeEntry("BackupOriginalRawFile", d->dngSettings->backupOriginalRawFile());
    group.writeEntry("CompressLossLess",      d->dngSettings->compressLossLess());
    group.writeEntry("PreviewMode",           (int)d->dngSettings->previewMode());
    d->conflictSettings->writeSettings(group);
}

void DNGConverterDialog::slotStartStop()
{
    if (!d->busy)
    {
        d->fileList.clear();

        QTreeWidgetItemIterator it(d->listView->listView());

        while (*it)
        {
            DNGConverterListViewItem* const lvItem = dynamic_cast<DNGConverterListViewItem*>(*it);

            if (lvItem)
            {
                if (!lvItem->isDisabled() && (lvItem->state() != DNGConverterListViewItem::Success))
                {
                    lvItem->setIcon(1, QIcon());
                    lvItem->setState(DNGConverterListViewItem::Waiting);
                    d->fileList.append(lvItem->url().path());
                }
            }

            ++it;
        }

        if (d->fileList.empty())
        {
            QMessageBox::information(this, i18nc("@title:window", "DNG Converter"), i18n("The list does not contain any Raw files to process."));
            setBusy(false);
            slotAborted();
            return;
        }

        d->progressBar->setMaximum(d->fileList.count());
        d->progressBar->setValue(0);
        d->progressBar->show();
        d->progressBar->progressScheduled(i18n("DNG Converter"), true, true);
        d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("image-x-adobe-dng")).pixmap(22, 22));

        processAll();
    }
    else
    {
        d->fileList.clear();
        d->thread->cancel();
        setBusy(false);

        d->listView->cancelProcess();

        QTimer::singleShot(500, this, SLOT(slotAborted()));
    }
}

void DNGConverterDialog::addItems(const QList<QUrl>& itemList)
{
    d->listView->slotAddImages(itemList);
}

void DNGConverterDialog::slotAborted()
{
    d->progressBar->setValue(0);
    d->progressBar->hide();
    d->progressBar->progressCompleted();
}

void DNGConverterDialog::slotIdentify()
{
    QList<QUrl> urlList = d->listView->imageUrls(true);

    for (QList<QUrl>::const_iterator  it = urlList.constBegin() ; it != urlList.constEnd() ; ++it)
    {
        QFileInfo fi((*it).path());

        if (d->conflictSettings->conflictRule() == FileSaveConflictBox::OVERWRITE)
        {
            QString dest                         = fi.completeBaseName() + QLatin1String(".dng");
            DNGConverterListViewItem* const item = dynamic_cast<DNGConverterListViewItem*>(d->listView->listView()->findItem(*it));

            if (item)
            {
                item->setDestFileName(dest);
            }
        }
        else
        {
            QString dest      = fi.absolutePath() + QLatin1String("/") + fi.completeBaseName() + QLatin1String(".dng");
            QFileInfo a(dest);
            bool fileNotFound = (a.exists());

            if (!fileNotFound)
            {
                dest = fi.completeBaseName() + QLatin1String(".dng");
            }

            else
            {
                int i = 0;

                while(fileNotFound)
                {
                    a = QFileInfo(dest);

                    if (!a.exists())
                    {
                        fileNotFound = false;
                    }
                    else
                    {
                        i++;
                        dest = fi.absolutePath()     +
                               QLatin1String("/")    +
                               fi.completeBaseName() +
                               QLatin1String("_")    +
                               QString::number(i)    +
                               QLatin1String(".dng");
                    }
                }

                dest = fi.completeBaseName() + QLatin1String("_") + QString::number(i) + QLatin1String(".dng");
            }

            DNGConverterListViewItem* const item = dynamic_cast<DNGConverterListViewItem*>(d->listView->listView()->findItem(*it));

            if (item)
            {
                item->setDestFileName(dest);
            }
        }
    }

    if (!urlList.empty())
    {
        d->thread->identifyRawFiles(urlList);

        if (!d->thread->isRunning())
        {
            d->thread->start();
        }
    }
}

void DNGConverterDialog::processAll()
{
    d->thread->setBackupOriginalRawFile(d->dngSettings->backupOriginalRawFile());
    d->thread->setCompressLossLess(d->dngSettings->compressLossLess());
    d->thread->setPreviewMode(d->dngSettings->previewMode());
    d->thread->processRawFiles(d->listView->imageUrls(true));

    if (!d->thread->isRunning())
    {
        d->thread->start();
    }
}

void DNGConverterDialog::slotThreadFinished()
{
    setBusy(false);
    slotAborted();
}

void DNGConverterDialog::setBusy(bool busy)
{
    d->busy = busy;

    if (d->busy)
    {
        m_buttons->button(QDialogButtonBox::Ok)->setText(i18n("&Abort"));
        m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18n("Abort the conversion of Raw files."));
    }
    else
    {
        m_buttons->button(QDialogButtonBox::Ok)->setText(i18n("Con&vert"));
        m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18n("Start converting the Raw images using the current settings."));
    }

    d->dngSettings->setEnabled(!d->busy);
    d->conflictSettings->setEnabled(!d->busy);
    d->listView->listView()->viewport()->setEnabled(!d->busy);
    d->busy ? setCursor(Qt::WaitCursor) : unsetCursor();
}

void DNGConverterDialog::processed(const QUrl& url, const QString& tmpFile)
{
    DNGConverterListViewItem* const item = dynamic_cast<DNGConverterListViewItem*>(d->listView->listView()->findItem(url));

    if (!item)
    {
        return;
    }

    QString destFile(item->destPath());

    if (d->conflictSettings->conflictRule() != FileSaveConflictBox::OVERWRITE)
    {
        if (!QFile::exists(destFile))
        {
            item->setStatus(i18n("Failed to save image"));
        }
    }

    if (!destFile.isEmpty())
    {
        if (DMetadata::hasSidecar(tmpFile))
        {
            if (!DFileOperations::renameFile(DMetadata::sidecarPath(tmpFile),
                                             DMetadata::sidecarPath(destFile)))
            {
                item->setStatus(i18n("Failed to move sidecar"));
            }
        }

        if (!DFileOperations::renameFile(tmpFile, destFile))
        {
            item->setStatus(i18n("Failed to save image."));
            d->listView->processed(url, false);
        }
        else
        {
            item->setDestFileName(QFileInfo(destFile).fileName());
            d->listView->processed(url, true);
            item->setStatus(i18n("Success"));
        }
    }

    d->progressBar->setValue(d->progressBar->value()+1);
}

void DNGConverterDialog::processingFailed(const QUrl& url, int result)
{
    d->listView->processed(url, false);
    d->progressBar->setValue(d->progressBar->value()+1);

    DNGConverterListViewItem* const item = dynamic_cast<DNGConverterListViewItem*>(d->listView->listView()->findItem(url));

    if (!item)
    {
        return;
    }

    QString status;

    switch (result)
    {
        case DNGWriter::PROCESS_FAILED:
        {
            status = i18n("Process failed");
            break;
        }

        case DNGWriter::PROCESS_CANCELED:
        {
            status = i18n("Process Canceled");
            break;
        }

        case DNGWriter::FILE_NOT_SUPPORTED:
        {
            status = i18n("File not supported");
            break;
        }

        default:
        {
            status = i18n("Internal error");
            break;
        }
    }

    item->setStatus(status);
}

void DNGConverterDialog::slotDNGConverterAction(const DigikamGenericDNGConverterPlugin::DNGConverterActionData& ad)
{
    if (ad.starting)            // Something have been started...
    {
        switch (ad.action)
        {
            case IDENTIFY:
            {
                break;
            }

            case PROCESS:
            {
                setBusy(true);
                d->listView->processing(ad.fileUrl);
                d->progressBar->progressStatusChanged(i18n("Processing %1", ad.fileUrl.fileName()));
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "DigikamGenericDNGConverterPlugin: Unknown action";
                break;
            }
        }
    }
    else
    {
        if (ad.result != DNGWriter::PROCESS_COMPLETE)        // Something is failed...
        {
            switch (ad.action)
            {
                case IDENTIFY:
                {
                    break;
                }

                case PROCESS:
                {
                    processingFailed(ad.fileUrl, ad.result);
                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "DigikamGenericDNGConverterPlugin: Unknown action";
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case IDENTIFY:
                {
                    DNGConverterListViewItem* const item = dynamic_cast<DNGConverterListViewItem*>(d->listView->listView()->findItem(ad.fileUrl));

                    if (item)
                    {
                        item->setIdentity(ad.message);
                    }

                    break;
                }

                case PROCESS:
                {
                    processed(ad.fileUrl, ad.destPath);
                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "DigikamGenericDNGConverterPlugin: Unknown action";
                    break;
                }
            }
        }
    }
}

} // namespace DigikamGenericDNGConverterPlugin

#include "moc_dngconverterdialog.cpp"

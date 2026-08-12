/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a widget to edit EXIF metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exifeditwidget.h"

// Qt includes

#include <QCloseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>
#include <QUrl>
#include <QFileInfo>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "metadataeditdialog.h"
#include "metaenginesettings.h"
#include "exifadjust.h"
#include "exifcaption.h"
#include "exifdatetime.h"
#include "exifdevice.h"
#include "exiflens.h"
#include "exiflight.h"
#include "digikam_debug.h"
#include "dmetadata.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN EXIFEditWidget::Private
{
public:

    Private() = default;

public:

    bool                modified        = false;
    bool                isReadOnly      = false;

    DConfigDlgWdgItem*  page_caption    = nullptr;
    DConfigDlgWdgItem*  page_datetime   = nullptr;
    DConfigDlgWdgItem*  page_lens       = nullptr;
    DConfigDlgWdgItem*  page_device     = nullptr;
    DConfigDlgWdgItem*  page_light      = nullptr;
    DConfigDlgWdgItem*  page_adjust     = nullptr;

    EXIFCaption*        captionPage     = nullptr;
    EXIFDateTime*       datetimePage    = nullptr;
    EXIFLens*           lensPage        = nullptr;
    EXIFDevice*         devicePage      = nullptr;
    EXIFLight*          lightPage       = nullptr;
    EXIFAdjust*         adjustPage      = nullptr;

    MetadataEditDialog* dlg             = nullptr;
};

EXIFEditWidget::EXIFEditWidget(MetadataEditDialog* const parent)
    : DConfigDlgWdg(parent),
      d            (new Private)
{
    d->dlg           = parent;

    d->captionPage   = new EXIFCaption(this);
    d->page_caption  = addPage(d->captionPage, i18nc("@item: image caption", "Caption"));
    d->page_caption->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));

    d->datetimePage  = new EXIFDateTime(this);
    d->page_datetime = addPage(d->datetimePage, i18nc("@item: date properties", "Date & Time"));
    d->page_datetime->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));

    d->lensPage      = new EXIFLens(this);
    d->page_lens     = addPage(d->lensPage, i18nc("@item: lens properties", "Lens"));
    d->page_lens->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));

    d->devicePage    = new EXIFDevice(this);
    d->page_device   = addPage(d->devicePage, i18nc("@item: device properties", "Device"));
    d->page_device->setIcon(QIcon::fromTheme(QLatin1String("scanner")));

    d->lightPage     = new EXIFLight(this);
    d->page_light    = addPage(d->lightPage, i18nc("@item; light properties", "Light"));
    d->page_light->setIcon(QIcon::fromTheme(QLatin1String("view-preview")));

    d->adjustPage    = new EXIFAdjust(this);
    d->page_adjust   = addPage(d->adjustPage, i18nc("@item: picture adjustments", "Adjustments"));
    d->page_adjust->setIcon(QIcon::fromTheme(QLatin1String("fill-color")));

    // ------------------------------------------------------------

    connect(d->captionPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->datetimePage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->lensPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->devicePage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->lightPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->adjustPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    // ------------------------------------------------------------

    readSettings();
    slotItemChanged();
}

EXIFEditWidget::~EXIFEditWidget()
{
    delete d;
}

void EXIFEditWidget::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));
    showPage(group.readEntry(QLatin1String("All EXIF Edit Page"), 0));
    d->captionPage->setCheckedSyncJFIFComment(group.readEntry(QLatin1String("All Sync JFIF Comment"), true));
    d->captionPage->setCheckedSyncXMPCaption(group.readEntry(QLatin1String("All Sync XMP Caption"),   true));
    d->captionPage->setCheckedSyncIPTCCaption(group.readEntry(QLatin1String("All Sync IPTC Caption"), true));
    d->datetimePage->setCheckedSyncXMPDate(group.readEntry(QLatin1String("All Sync XMP Date"),        true));
    d->datetimePage->setCheckedSyncIPTCDate(group.readEntry(QLatin1String("All Sync IPTC Date"),      true));
}

void EXIFEditWidget::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));
    group.writeEntry(QLatin1String("All EXIF Edit Page"),    activePageIndex());
    group.writeEntry(QLatin1String("All Sync JFIF Comment"), d->captionPage->syncJFIFCommentIsChecked());
    group.writeEntry(QLatin1String("All Sync XMP Caption"),  d->captionPage->syncXMPCaptionIsChecked());
    group.writeEntry(QLatin1String("All Sync IPTC Caption"), d->captionPage->syncIPTCCaptionIsChecked());
    group.writeEntry(QLatin1String("All Sync XMP Date"),     d->datetimePage->syncXMPDateIsChecked());
    group.writeEntry(QLatin1String("All Sync IPTC Date"),    d->datetimePage->syncIPTCDateIsChecked());
    config->sync();
}

void EXIFEditWidget::slotItemChanged()
{
    d->page_caption->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Caption Information"))
                                       .arg(i18nc("@title", "Record technical descriptions"))));

    d->page_datetime->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Date and Time Information"))
                                       .arg(i18nc("@title", "Record camera time-stamp properties"))));

    d->page_lens->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Lens Settings"))
                                       .arg(i18nc("@title", "Record lens details used with camera"))));

    d->page_device->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Capture Device Settings"))
                                       .arg(i18nc("@title", "Record shot conditions used by camera"))));

    d->page_light->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Light Source Information"))
                                       .arg(i18nc("@title", "Record ambient condition captured by camera"))));

    d->page_adjust->setHeader(d->dlg->currentItemTitleHeader(
                                   QString::fromUtf8("<qt>%1<br><i>%2</i></qt>")
                                       .arg(i18nc("@title", "Pictures Adjustments"))
                                       .arg(i18nc("@title", "Record technical details used by camera"))));

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->load((*d->dlg->currentItem()).toLocalFile());

    d->captionPage->readMetadata(*meta);
    d->datetimePage->readMetadata(*meta);
    d->lensPage->readMetadata(*meta);
    d->devicePage->readMetadata(*meta);
    d->lightPage->readMetadata(*meta);
    d->adjustPage->readMetadata(*meta);

    d->isReadOnly = (
                     (MetaEngineSettings::instance()->settings().metadataWritingMode == DMetadata::WRITE_TO_FILE_ONLY) &&
                     !QFileInfo((*d->dlg->currentItem()).toLocalFile()).isWritable()
                    );

    Q_EMIT signalSetReadOnly(d->isReadOnly);

    d->page_caption->setEnabled(!d->isReadOnly);
    d->page_datetime->setEnabled(!d->isReadOnly);
    d->page_lens->setEnabled(!d->isReadOnly);
    d->page_device->setEnabled(!d->isReadOnly);
    d->page_light->setEnabled(!d->isReadOnly);
    d->page_adjust->setEnabled(!d->isReadOnly);
}

void EXIFEditWidget::apply()
{
    if (d->modified && !d->isReadOnly)
    {
        QScopedPointer<DMetadata> meta(new DMetadata);
        meta->load((*d->dlg->currentItem()).toLocalFile());

        d->captionPage->applyMetadata(*meta);
        d->datetimePage->applyMetadata(*meta);
        d->lensPage->applyMetadata(*meta);
        d->devicePage->applyMetadata(*meta);
        d->lightPage->applyMetadata(*meta);
        d->adjustPage->applyMetadata(*meta);

        meta->applyChanges();

        d->modified = false;
    }
}

void EXIFEditWidget::slotModified()
{
    if (!d->isReadOnly)
    {
        d->modified = true;

        Q_EMIT signalModified();
    }
}

void EXIFEditWidget::showPage(int page)
{
    switch (page)
    {
        case 0:
        {
            setCurrentPage(d->page_caption);
            break;
        }

        case 1:
        {
            setCurrentPage(d->page_datetime);
            break;
        }

        case 2:
        {
            setCurrentPage(d->page_lens);
            break;
        }

        case 3:
        {
            setCurrentPage(d->page_device);
            break;
        }

        case 4:
        {
            setCurrentPage(d->page_light);
            break;
        }

        case 5:
        {
            setCurrentPage(d->page_adjust);
            break;
        }

        default:
        {
            setCurrentPage(d->page_caption);
            break;
        }
    }
}

int EXIFEditWidget::activePageIndex() const
{
    const DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_caption)  return 0;
    if (cur == d->page_datetime) return 1;
    if (cur == d->page_lens)     return 2;
    if (cur == d->page_device)   return 3;
    if (cur == d->page_light)    return 4;
    if (cur == d->page_adjust)   return 5;

    return 0;
}

bool EXIFEditWidget::isModified() const
{
    return d->modified;
}

} // namespace DigikamGenericMetadataEditPlugin

#include "moc_exifeditwidget.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display item metadata information
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiesmetadatatab.h"

// Qt includes

#include <QFile>
#include <QLabel>
#include <QPixmap>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QScopedPointer>
#include <QElapsedTimer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "metadatapanel.h"
#include "exifwidget.h"
#include "makernotewidget.h"
#include "iptcwidget.h"
#include "xmpwidget.h"
#include "exiftoolwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesMetadataTab::Private
{
public:

    enum MetadataTab
    {
        EXIF       = 0,
        MAKERNOTE,
        IPTC,
        XMP,
        EXIFTOOL
    };

public:

    Private() = default;

    ExifWidget*      exifWidget         = nullptr;
    MakerNoteWidget* makernoteWidget    = nullptr;
    IptcWidget*      iptcWidget         = nullptr;
    XmpWidget*       xmpWidget          = nullptr;
    ExifToolWidget*  exifToolWidget     = nullptr;
};

ItemPropertiesMetadataTab::ItemPropertiesMetadataTab(QWidget* const parent)
    : QTabWidget(parent),
      d         (new Private)
{
    // Exif tab area ---------------------------------------

    d->exifWidget      = new ExifWidget(this);
    insertTab(Private::EXIF, d->exifWidget, i18n("EXIF"));

    // Makernote tab area ----------------------------------

    d->makernoteWidget = new MakerNoteWidget(this);
    insertTab(Private::MAKERNOTE, d->makernoteWidget, i18n("Makernote"));

    // IPTC tab area ---------------------------------------

    d->iptcWidget      = new IptcWidget(this);
    insertTab(Private::IPTC, d->iptcWidget, i18n("IPTC"));

    // XMP tab area ----------------------------------------

    d->xmpWidget       = new XmpWidget(this);

    if (DMetadata::supportXmp())
    {
        insertTab(Private::XMP, d->xmpWidget, i18n("XMP"));
    }
    else
    {
        d->xmpWidget->hide();
    }

    // ExifTool tab area ----------------------------------

    d->exifToolWidget = new ExifToolWidget(this);
    insertTab(Private::EXIFTOOL, d->exifToolWidget, QLatin1String("ExifTool")); // No i18n here.

    connect(d->exifToolWidget, SIGNAL(signalSetupExifTool()),
            this, SIGNAL(signalSetupExifTool()));

    // ---

    connect(d->exifWidget, SIGNAL(signalSetupMetadataFilters()),
            this, SLOT(slotSetupMetadataFilters()));

    connect(d->makernoteWidget, SIGNAL(signalSetupMetadataFilters()),
            this, SLOT(slotSetupMetadataFilters()));

    connect(d->iptcWidget, SIGNAL(signalSetupMetadataFilters()),
            this, SLOT(slotSetupMetadataFilters()));

    connect(d->xmpWidget, SIGNAL(signalSetupMetadataFilters()),
            this, SLOT(slotSetupMetadataFilters()));

    connect(d->exifToolWidget, SIGNAL(signalSetupMetadataFilters()),
            this, SLOT(slotSetupMetadataFilters()));
}

ItemPropertiesMetadataTab::~ItemPropertiesMetadataTab()
{
    delete d;
}

void ItemPropertiesMetadataTab::slotSetupMetadataFilters()
{
    if      (sender() == d->exifWidget)
    {
        Q_EMIT signalSetupMetadataFilters(Private::EXIF);
    }
    else if (sender() == d->makernoteWidget)
    {
        Q_EMIT signalSetupMetadataFilters(Private::MAKERNOTE);
    }
    else if (sender() == d->iptcWidget)
    {
        Q_EMIT signalSetupMetadataFilters(Private::IPTC);
    }
    else if (sender() == d->xmpWidget)
    {
        Q_EMIT signalSetupMetadataFilters(Private::XMP);
    }
    else if (sender() == d->exifToolWidget)
    {
        Q_EMIT signalSetupMetadataFilters(Private::EXIFTOOL);
    }
}

void ItemPropertiesMetadataTab::readSettings(const KConfigGroup& group)
{
    setCurrentIndex(group.readEntry("ImagePropertiesMetaData Tab",
                                    (int)Private::EXIF));
    d->exifWidget->setMode(group.readEntry("EXIF Level",                              (int)ExifWidget::NONE));
    d->makernoteWidget->setMode(group.readEntry("MAKERNOTE Level",                    (int)MakerNoteWidget::NONE));
    d->iptcWidget->setMode(group.readEntry("IPTC Level",                              (int)IptcWidget::NONE));
    d->xmpWidget->setMode(group.readEntry("XMP Level",                                (int)XmpWidget::NONE));
    d->exifToolWidget->setMode(group.readEntry("EXIFTOOL Level",                      (int)ExifToolWidget::NONE));
    d->exifWidget->setCurrentItemByKey(group.readEntry("Current EXIF Item",           QString()));
    d->makernoteWidget->setCurrentItemByKey(group.readEntry("Current MAKERNOTE Item", QString()));
    d->iptcWidget->setCurrentItemByKey(group.readEntry("Current IPTC Item",           QString()));
    d->xmpWidget->setCurrentItemByKey(group.readEntry("Current XMP Item",             QString()));
    d->exifToolWidget->setCurrentItemByKey(group.readEntry("Current ExifTool Item",   QString()));

    loadFilters();
}

void ItemPropertiesMetadataTab::loadFilters()
{
    KConfigGroup grp2 = KSharedConfig::openConfig()->group(QLatin1String("Image Properties SideBar"));
    d->exifWidget->setTagsFilter(grp2.readEntry("EXIF Tags Filter",           QStringList()));
    d->makernoteWidget->setTagsFilter(grp2.readEntry("MAKERNOTE Tags Filter", QStringList()));
    d->iptcWidget->setTagsFilter(grp2.readEntry("IPTC Tags Filter",           QStringList()));
    d->xmpWidget->setTagsFilter(grp2.readEntry("XMP Tags Filter",             QStringList()));
    d->exifToolWidget->setTagsFilter(grp2.readEntry("EXIFTOOL Tags Filter",   QStringList()));
}

void ItemPropertiesMetadataTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry("ImagePropertiesMetaData Tab", currentIndex());
    group.writeEntry("EXIF Level",                  d->exifWidget->getMode());
    group.writeEntry("MAKERNOTE Level",             d->makernoteWidget->getMode());
    group.writeEntry("IPTC Level",                  d->iptcWidget->getMode());
    group.writeEntry("XMP Level",                   d->xmpWidget->getMode());
    group.writeEntry("EXIFTOOL Level",              d->exifToolWidget->getMode());
    group.writeEntry("Current EXIF Item",           d->exifWidget->getCurrentItemKey());
    group.writeEntry("Current MAKERNOTE Item",      d->makernoteWidget->getCurrentItemKey());
    group.writeEntry("Current IPTC Item",           d->iptcWidget->getCurrentItemKey());
    group.writeEntry("Current XMP Item",            d->xmpWidget->getCurrentItemKey());
    group.writeEntry("Current ExifTool Item",       d->exifToolWidget->getCurrentItemKey());
}

void ItemPropertiesMetadataTab::setCurrentUrl(const QUrl& url)
{
    if (url.isEmpty())
    {
        d->exifWidget->loadFromURL(url);
        d->makernoteWidget->loadFromURL(url);
        d->iptcWidget->loadFromURL(url);
        d->xmpWidget->loadFromURL(url);
        d->exifToolWidget->loadFromUrl(url);
        d->exifWidget->setEnabled(false);
        d->makernoteWidget->setEnabled(false);
        d->iptcWidget->setEnabled(false);
        d->xmpWidget->setEnabled(false);
        d->exifToolWidget->setEnabled(false);

        return;
    }

    d->exifWidget->setEnabled(true);
    d->makernoteWidget->setEnabled(true);
    d->iptcWidget->setEnabled(true);
    d->xmpWidget->setEnabled(true);
    d->exifToolWidget->setEnabled(true);

    d->exifToolWidget->loadFromUrl(url);

    QScopedPointer<DMetadata> metadata(new DMetadata);

    QElapsedTimer execTimer;
    execTimer.start();

    bool ret = metadata->load(url.toLocalFile(), true);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Metadata loading with Exiv2 took"
                                 << execTimer.elapsed() << "ms (" << ret << ")";

    d->exifWidget->loadFromData(url.fileName(),      *metadata);
    d->makernoteWidget->loadFromData(url.fileName(), *metadata);
    d->iptcWidget->loadFromData(url.fileName(),      *metadata);
    d->xmpWidget->loadFromData(url.fileName(),       *metadata);
}

void ItemPropertiesMetadataTab::setCurrentData(DMetadata* const metaData, const QUrl& url)
{
    d->exifToolWidget->loadFromUrl(url);
    d->exifToolWidget->setEnabled(true);

    QScopedPointer<DMetadata> data(new DMetadata(metaData->data()));

    if (!data->hasExif() && !data->hasIptc() && !data->hasXmp())
    {
        d->exifWidget->loadFromData(url.fileName(),      *data);
        d->makernoteWidget->loadFromData(url.fileName(), *data);
        d->iptcWidget->loadFromData(url.fileName(),      *data);
        d->xmpWidget->loadFromData(url.fileName(),       *data);
        d->exifWidget->setEnabled(false);
        d->makernoteWidget->setEnabled(false);
        d->iptcWidget->setEnabled(false);
        d->xmpWidget->setEnabled(false);
        return;
    }

    d->exifWidget->setEnabled(true);
    d->makernoteWidget->setEnabled(true);
    d->iptcWidget->setEnabled(true);
    d->xmpWidget->setEnabled(true);

    d->exifWidget->loadFromData(url.fileName(),      *data);
    d->makernoteWidget->loadFromData(url.fileName(), *data);
    d->iptcWidget->loadFromData(url.fileName(),      *data);
    d->xmpWidget->loadFromData(url.fileName(),       *data);
}

} // namespace Digikam

#include "moc_itempropertiesmetadatatab.cpp"

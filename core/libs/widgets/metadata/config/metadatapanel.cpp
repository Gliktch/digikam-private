/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-17
 * Description : Metadata tags selector config panel.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatapanel.h"

// Qt includes

#include <QFrame>
#include <QVBoxLayout>
#include <QList>
#include <QApplication>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "metaengine.h"
#include "metadataselector.h"
#include "dmetadata.h"

namespace Digikam
{

static const char* ExifHumanList[] =
{
    "Make",
    "Model",
    "DateTime",
    "ImageDescription",
    "Copyright",
    "ShutterSpeedValue",
    "ApertureValue",
    "ExposureProgram",
    "ExposureMode",
    "ExposureBiasValue",
    "ExposureTime",
    "WhiteBalance",
    "ISOSpeedRatings",
    "FocalLength",
    "SubjectDistance",
    "MeteringMode",
    "Contrast",
    "Saturation",
    "Sharpness",
    "LightSource",
    "Flash",
    "FNumber",
    "GPSLatitude",
    "GPSLongitude",
    "GPSAltitude",
    "-1"
};

// This list mix different tags name used by camera makers.
static const char* MakerNoteHumanList[] =
{
    "AFFocusPos",
    "AFMode",
    "AFPoint",
    "AutofocusMode",
    "ColorMode",
    "ColorTemperature",
    "Contrast",
    "DigitalZoom",
    "ExposureMode",
    "ExposureProgram",
    "ExposureCompensation",
    "ExposureManualBias",
    "Flash",
    "FlashBias",
    "FlashMode",
    "FlashType",
    "FlashDevice",
    "FNumber",
    "Focus",
    "FocusDistance",
    "FocusMode",
    "FocusSetting",
    "FocusType",
    "Hue",
    "HueAdjustment",
    "ImageStabilizer",
    "ImageStabilization",
    "InternalFlash",
    "ISOSelection",
    "ISOSpeed",
    "Lens",
    "LensType",
    "LensRange",
    "Macro",
    "MacroFocus",
    "MeteringMode",
    "NoiseReduction",
    "OwnerName",
    "Quality",
    "Tone",
    "ToneComp",
    "Saturation",
    "Sharpness",
    "ShootingMode",
    "ShutterSpeedValue",
    "SpotMode",
    "SubjectDistance",
    "WhiteBalance",
    "WhiteBalanceBias",
    "-1"
};

static const char* IptcHumanList[] =
{
    "Caption",
    "City",
    "Contact",
    "Copyright",
    "Credit",
    "DateCreated",
    "Headline",
    "Keywords",
    "ProvinceState",
    "Source",
    "Urgency",
    "Writer",
    "-1"
};

static const char* XmpHumanList[] =
{
    "Description",
    "City",
    "Relation",
    "Rights",
    "Publisher",
    "CreateDate",
    "Title",
    "Identifier",
    "State",
    "Source",
    "Rating",
    "Advisory",
    "-1"
};

class Q_DECL_HIDDEN MetadataPanel::Private
{
public:

    Private()
    {
        setDefaultFilter(ExifHumanList,      defaultExifFilter);
        setDefaultFilter(MakerNoteHumanList, defaultMknoteFilter);
        setDefaultFilter(IptcHumanList,      defaultIptcFilter);
        setDefaultFilter(XmpHumanList,       defaultXmpFilter);

        defaultExifToolFilter = defaultExifFilter   +
                                defaultMknoteFilter +
                                defaultIptcFilter   +
                                defaultXmpFilter;
    };

    void setDefaultFilter(const char** const list, QStringList& filter)
    {
        for (int i = 0 ; QLatin1String(list[i]) != QLatin1String("-1") ; ++i)
        {
            filter << QLatin1String(list[i]);
        }
    };

public:

    QTabWidget*                  tab                    = nullptr;

    QStringList                  defaultExifFilter;
    QStringList                  defaultMknoteFilter;
    QStringList                  defaultIptcFilter;
    QStringList                  defaultXmpFilter;
    QStringList                  defaultExifToolFilter;

    ExifToolParser::ExifToolData parsedData;
    ExifToolParser*              parser                 = nullptr;
    bool                         parserRunning          = false;

    MetadataSelectorView*        exifViewerConfig       = nullptr;
    MetadataSelectorView*        mknoteViewerConfig     = nullptr;
    MetadataSelectorView*        iptcViewerConfig       = nullptr;
    MetadataSelectorView*        xmpViewerConfig        = nullptr;
    MetadataSelectorView*        exifToolViewerConfig   = nullptr;
};

MetadataPanel::MetadataPanel(QTabWidget* const tab)
    : QObject(tab),
      d      (new Private)
{
    d->tab    = tab;
    d->parser = new ExifToolParser(this, true);

    // --------------------------------------------------------

    d->exifViewerConfig     = new MetadataSelectorView(d->tab, MetadataSelectorView::Exiv2Backend);
    d->exifViewerConfig->setDefaultFilter(d->defaultExifFilter);
    d->tab->addTab(d->exifViewerConfig, i18n("EXIF viewer"));

    d->mknoteViewerConfig   = new MetadataSelectorView(d->tab, MetadataSelectorView::Exiv2Backend);
    d->mknoteViewerConfig->setDefaultFilter(d->defaultMknoteFilter);
    d->tab->addTab(d->mknoteViewerConfig, i18n("Makernotes viewer"));

    d->iptcViewerConfig     = new MetadataSelectorView(d->tab, MetadataSelectorView::Exiv2Backend);
    d->iptcViewerConfig->setDefaultFilter(d->defaultIptcFilter);
    d->tab->addTab(d->iptcViewerConfig, i18n("IPTC viewer"));

    d->xmpViewerConfig      = new MetadataSelectorView(d->tab, MetadataSelectorView::Exiv2Backend);
    d->xmpViewerConfig->setDefaultFilter(d->defaultXmpFilter);
    d->tab->addTab(d->xmpViewerConfig, i18n("XMP viewer"));

    d->exifToolViewerConfig = new MetadataSelectorView(d->tab, MetadataSelectorView::ExifToolBackend);
    d->exifToolViewerConfig->setDefaultFilter(d->defaultExifToolFilter);
    d->tab->addTab(d->exifToolViewerConfig, i18n("ExifTool viewer"));

    slotTabChanged(d->tab->currentIndex());

    // --------------------------------------------------------

    connect(d->parser, SIGNAL(signalExifToolAsyncData(ExifToolParser::ExifToolData)),
            this, SLOT(slotExifToolAsyncData(ExifToolParser::ExifToolData)));

    connect(d->tab, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));
}

MetadataPanel::~MetadataPanel()
{
    qApp->restoreOverrideCursor();

    delete d;
}

QStringList MetadataPanel::defaultExifFilter()
{
    Private d;
    return d.defaultExifFilter;
}

QStringList MetadataPanel::defaultMknoteFilter()
{
    Private d;
    return d.defaultMknoteFilter;
}

QStringList MetadataPanel::defaultIptcFilter()
{
    Private d;
    return d.defaultIptcFilter;
}

QStringList MetadataPanel::defaultXmpFilter()
{
    Private d;
    return d.defaultXmpFilter;
}

QStringList MetadataPanel::defaultExifToolFilter()
{
    Private d;
    return d.defaultExifToolFilter;
}

void MetadataPanel::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Image Properties SideBar"));

    if (d->exifViewerConfig->itemsCount())
    {
        group.writeEntry("EXIF Tags Filter", d->exifViewerConfig->checkedTagsList());
    }

    if (d->mknoteViewerConfig->itemsCount())
    {
        group.writeEntry("MAKERNOTE Tags Filter", d->mknoteViewerConfig->checkedTagsList());
    }

    if (d->iptcViewerConfig->itemsCount())
    {
        group.writeEntry("IPTC Tags Filter", d->iptcViewerConfig->checkedTagsList());
    }

    if (d->xmpViewerConfig->itemsCount())
    {
        group.writeEntry("XMP Tags Filter", d->xmpViewerConfig->checkedTagsList());
    }

    if (d->exifToolViewerConfig->itemsCount())
    {
        group.writeEntry("EXIFTOOL Tags Filter", d->exifToolViewerConfig->checkedTagsList());
    }

    config->sync();
}

void MetadataPanel::slotTabChanged(int)
{
    QScopedPointer<DMetadata> meta(new DMetadata);
    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Image Properties SideBar"));
    const QWidget* const tab  = d->tab->currentWidget();

    if      (tab == d->exifViewerConfig)
    {
        if (!d->exifViewerConfig->itemsCount())
        {
            d->exifViewerConfig->setTagsMap(meta->getStdExifTagsList());
            d->exifViewerConfig->setcheckedTagsList(group.readEntry("EXIF Tags Filter",
                                                                    d->exifViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->mknoteViewerConfig)
    {
        if (!d->mknoteViewerConfig->itemsCount())
        {
            d->mknoteViewerConfig->setTagsMap(meta->getMakernoteTagsList());
            d->mknoteViewerConfig->setcheckedTagsList(group.readEntry("MAKERNOTE Tags Filter",
                                                                      d->mknoteViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->iptcViewerConfig)
    {
        if (!d->iptcViewerConfig->itemsCount())
        {
            d->iptcViewerConfig->setTagsMap(meta->getIptcTagsList());
            d->iptcViewerConfig->setcheckedTagsList(group.readEntry("IPTC Tags Filter",
                                                                    d->iptcViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->xmpViewerConfig)
    {
        if (!d->xmpViewerConfig->itemsCount())
        {
            d->xmpViewerConfig->setTagsMap(meta->getXmpTagsList());
            d->xmpViewerConfig->setcheckedTagsList(group.readEntry("XMP Tags Filter",
                                                                   d->xmpViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->exifToolViewerConfig)
    {
        if (!d->exifToolViewerConfig->itemsCount() && d->parser->exifToolAvailable())
        {
            if      (!d->parserRunning)
            {
                d->parserRunning = d->parser->tagsDatabase();

                // Let the waiting cursor continue to run.

                return;
            }
            else if (!d->parsedData.isEmpty())
            {
                d->exifToolViewerConfig->setTagsMap(d->parser->tagsDbToOrderedMap(d->parsedData));
                d->exifToolViewerConfig->setcheckedTagsList(group.readEntry("EXIFTOOL Tags Filter",
                                                                            d->exifToolViewerConfig->defaultFilter()));
                d->parserRunning = false;
            }
        }
    }

    qApp->restoreOverrideCursor();
}

void MetadataPanel::slotExifToolAsyncData(const ExifToolParser::ExifToolData& data)
{
    d->parsedData = data;
    qApp->restoreOverrideCursor();
    slotTabChanged(d->tab->currentIndex());
}

QStringList MetadataPanel::getAllCheckedTags() const
{
    QStringList checkedTags;
    checkedTags
            << d->exifViewerConfig->checkedTagsList()
            << d->iptcViewerConfig->checkedTagsList()
            << d->mknoteViewerConfig->checkedTagsList()
            << d->xmpViewerConfig->checkedTagsList()
            << d->exifToolViewerConfig->checkedTagsList()
            ;

    return checkedTags;
}

QList<MetadataSelectorView*> MetadataPanel::viewers() const
{
    QList<MetadataSelectorView*> viewers;
    viewers
            << d->exifViewerConfig
            << d->iptcViewerConfig
            << d->mknoteViewerConfig
            << d->xmpViewerConfig
            << d->exifToolViewerConfig
            ;

    return viewers;
}

} // namespace Digikam

#include "moc_metadatapanel.cpp"

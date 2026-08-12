/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab widget to display GPS info
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

/**
 * NOTE: Good explanations about GPS (in French) can be found at this url :
 * www.gpspassion.com/forumsen/topic.asp?TOPIC_ID=16593
 */

#include "itempropertiesgpstab.h"

// Qt includes

#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QToolButton>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QComboBox>
#include <QDesktopServices>
#include <QIcon>
#include <QLocale>
#include <QScopedPointer>
#include <QStackedWidget>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "mapwidget.h"
#include "itemmarkertiler.h"
#include "dlayoutbox.h"
#include "itemgpsmodelhelper.h"
#include "dexpanderbox.h"
#include "digikam_config.h"
#include "webbrowserdlg.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesGPSTab::Private
{

public:

    Private() = default;

    QLabel*                    altLabel                     = nullptr;
    QLabel*                    latLabel                     = nullptr;
    QLabel*                    lonLabel                     = nullptr;
    QLabel*                    dateLabel                    = nullptr;

    QStackedWidget*            mapView                      = nullptr;
    QToolButton*               detailsBtn                   = nullptr;
    QToolButton*               gpsCopyBtn                   = nullptr;
    QComboBox*                 detailsCombo                 = nullptr;

    DAdjustableLabel*          altitude                     = nullptr;
    DAdjustableLabel*          latitude                     = nullptr;
    DAdjustableLabel*          longitude                    = nullptr;
    DAdjustableLabel*          date                         = nullptr;

    MapWidget*                 map                          = nullptr;
    ItemMarkerTiler*           itemMarkerTiler              = nullptr;
    GPSItemInfo::List          gpsInfoList;

    QStandardItemModel*        itemModel                    = nullptr;
    ItemGPSModelHelper*        gpsModelHelper               = nullptr;
    GPSItemInfoSorter*         gpsItemInfoSorter            = nullptr;
    bool                       boundariesShouldBeAdjusted   = false;
};

ItemPropertiesGPSTab::ItemPropertiesGPSTab(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    QGridLayout* const layout = new QGridLayout(this);

    // --------------------------------------------------------

    d->mapView                = new QStackedWidget(this);
    d->mapView->setMinimumWidth(100);
    d->mapView->setMinimumHeight(100);
    d->mapView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    d->mapView->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    QWidget* const mapPanel   = new QWidget(this);
    QVBoxLayout* const vlay2  = new QVBoxLayout(mapPanel);
    d->map                    = new MapWidget(mapPanel);
    d->map->setAvailableMouseModes(MouseModePan | MouseModeZoomIntoGroup);
    d->map->setVisibleMouseModes(MouseModePan | MouseModeZoomIntoGroup);
    d->map->setEnabledExtraActions(ExtraActionSticky);
    d->map->setVisibleExtraActions(ExtraActionSticky);
    d->map->setBackend(QLatin1String("marble"));
    d->gpsItemInfoSorter      = new GPSItemInfoSorter(this);
    d->gpsItemInfoSorter->addToMapWidget(d->map);
    vlay2->addWidget(d->map);
    vlay2->setContentsMargins(QMargins());
    vlay2->setSpacing(0);

    d->mapView->insertWidget(0, mapPanel);

    QLabel* const noGPSInfo   = new QLabel(i18n("No Geolocation\nInformation\nAvailable"));
    noGPSInfo->setAlignment(Qt::AlignCenter);
    noGPSInfo->setWordWrap(true);
    d->mapView->insertWidget(1, noGPSInfo);

    // --------------------------------------------------------

    d->itemModel        = new QStandardItemModel(this);
    d->gpsModelHelper   = new ItemGPSModelHelper(d->itemModel, this);
    d->itemMarkerTiler  = new ItemMarkerTiler(d->gpsModelHelper, this);
    d->map->setGroupedModel(d->itemMarkerTiler);

    d->altLabel         = new QLabel(i18n("<b>Altitude</b>:"),  this);
    d->latLabel         = new QLabel(i18n("<b>Latitude</b>:"),  this);
    d->lonLabel         = new QLabel(i18n("<b>Longitude</b>:"), this);
    d->dateLabel        = new QLabel(i18n("<b>Date</b>:"),      this);
    d->altitude         = new DAdjustableLabel(this);
    d->latitude         = new DAdjustableLabel(this);
    d->longitude        = new DAdjustableLabel(this);
    d->date             = new DAdjustableLabel(this);
    d->altitude->setAlignment(Qt::AlignRight);
    d->latitude->setAlignment(Qt::AlignRight);
    d->longitude->setAlignment(Qt::AlignRight);
    d->date->setAlignment(Qt::AlignRight);

    // --------------------------------------------------------

    QWidget* const box            = new DHBox(this);
    QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(box->layout());

    d->gpsCopyBtn                 = new QToolButton(box);
    d->gpsCopyBtn->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    d->gpsCopyBtn->setToolTip(i18n("Copy GPS coordinates to clipboard"));

    if (hBoxLayout)
    {
        hBoxLayout->addStretch();
    }

    d->detailsCombo = new QComboBox(box);
    d->detailsBtn   = new QToolButton(box);
    d->detailsBtn->setIcon(QIcon::fromTheme(QLatin1String("globe")));
    d->detailsBtn->setToolTip(i18n("See more information on the Internet"));
    d->detailsCombo->insertItem(MapQuest,      QLatin1String("MapQuest"));
    d->detailsCombo->insertItem(GoogleMaps,    QLatin1String("Google Maps"));
    d->detailsCombo->insertItem(BingMaps,      QLatin1String("Bing Maps"));
    d->detailsCombo->insertItem(OpenStreetMap, QLatin1String("OpenStreetMap"));
    d->detailsCombo->insertItem(LocAlizeMaps,  QLatin1String("loc.alize.us Maps"));

    // --------------------------------------------------------

    layout->addWidget(d->mapView,                 0, 0, 1, 2);
    layout->addWidget(d->altLabel,                1, 0, 1, 1);
    layout->addWidget(d->altitude,                1, 1, 1, 1);
    layout->addWidget(d->latLabel,                2, 0, 1, 1);
    layout->addWidget(d->latitude,                2, 1, 1, 1);
    layout->addWidget(d->lonLabel,                3, 0, 1, 1);
    layout->addWidget(d->longitude,               3, 1, 1, 1);
    layout->addWidget(d->dateLabel,               4, 0, 1, 1);
    layout->addWidget(d->date,                    4, 1, 1, 1);
    layout->addWidget(d->map->getControlWidget(), 5, 0, 1, 2);
    layout->addWidget(box,                        6, 0, 1, 2);
    layout->setRowStretch(0, 10);
    layout->setColumnStretch(1, 10);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    // --------------------------------------------------------

    connect(d->detailsBtn, SIGNAL(clicked()),
            this, SLOT(slotGPSDetails()));

    connect(d->gpsCopyBtn, SIGNAL(clicked()),
            this, SLOT(slotCopyGPSDetails()));
}

ItemPropertiesGPSTab::~ItemPropertiesGPSTab()
{
    delete d;
}

void ItemPropertiesGPSTab::readSettings(const KConfigGroup& group)
{
    d->gpsItemInfoSorter->setSortOptions(GPSItemInfoSorter::SortOptions(group.readEntry(QLatin1String("Sort Order"),
                                          int(d->gpsItemInfoSorter->getSortOptions()))));
    setWebGPSLocator(group.readEntry(QLatin1String("Web GPS Locator"), getWebGPSLocator()));

    KConfigGroup groupMapWidget = KConfigGroup(&group, QLatin1String("Map Widget"));
    d->map->readSettingsFromGroup(&groupMapWidget);

}
void ItemPropertiesGPSTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("Sort Order"),      int(d->gpsItemInfoSorter->getSortOptions()));
    group.writeEntry(QLatin1String("Web GPS Locator"), getWebGPSLocator());

    KConfigGroup groupMapWidget = KConfigGroup(&group, QLatin1String("Map Widget"));
    d->map->saveSettingsToGroup(&groupMapWidget);
}

int ItemPropertiesGPSTab::getWebGPSLocator() const
{
    return d->detailsCombo->currentIndex();
}

void ItemPropertiesGPSTab::setWebGPSLocator(int locator)
{
    d->detailsCombo->setCurrentIndex(locator);
}

void ItemPropertiesGPSTab::slotGPSDetails()
{
    QString val, url;

    if (d->gpsInfoList.isEmpty())
    {
        return;
    }

    const GPSItemInfo info = d->gpsInfoList.first();

    switch ( getWebGPSLocator() )
    {
        case MapQuest:
        {
            url.append(QLatin1String("https://www.mapquest.com/maps/map.adp?searchtype=address"
                                     "&formtype=address&latlongtype=decimal"));
            url.append(QLatin1String("&latitude="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String("&longitude="));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            break;
        }

        case GoogleMaps:
        {
            url.append(QLatin1String("https://maps.google.com/?q="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String(","));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(QLatin1String("&spn=0.05,0.05&t=h&om=1"));
            break;
        }

        case LocAlizeMaps:
        {
            url.append(QLatin1String("https://loc.alize.us/#/geo:"));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String(","));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(QLatin1String(",15,/"));
            break;
        }

        case BingMaps:
        {
            url.append(QLatin1String("https://www.bing.com/maps/?v=2&where1="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String(","));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            break;
        }

        case OpenStreetMap:
        {
            // lat and lon would also work, but wouldn't show a marker
            url.append(QLatin1String("https://www.openstreetmap.org/?"));
            url.append(QLatin1String("mlat="));
            url.append(val.setNum(info.coordinates.lat(), 'g', 12));
            url.append(QLatin1String("&mlon="));
            url.append(val.setNum(info.coordinates.lon(), 'g', 12));
            url.append(QLatin1String("&zoom=15"));
            break;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << url;

    WebBrowserDlg* const browser = new WebBrowserDlg(QUrl(url), this);
    browser->show();
}

void ItemPropertiesGPSTab::slotCopyGPSDetails()
{
    if (d->gpsInfoList.isEmpty())
    {
        return;
    }

    const GPSItemInfo info = d->gpsInfoList.first();
    QString copyGPSText    = QString::fromLatin1("%1, %2")
                                 .arg(info.coordinates.lat(), 0, 'g', 12)
                                 .arg(info.coordinates.lon(), 0, 'g', 12);

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(copyGPSText);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void ItemPropertiesGPSTab::setCurrentUrl(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearGPSInfo();
        return;
    }

    QScopedPointer<DMetadata> meta(new DMetadata(url.toLocalFile()));

    setMetadata(meta.data(), url);
}

void ItemPropertiesGPSTab::setMetadata(DMetadata* const meta, const QUrl& url)
{
    double lat, lng;
    const bool haveCoordinates = meta->getGPSLatitudeNumber(&lat) && meta->getGPSLongitudeNumber(&lng);

    if (haveCoordinates)
    {
        double alt;
        const bool haveAlt = meta->getGPSAltitude(&alt);

        GeoCoordinates coordinates(lat, lng);

        if (haveAlt)
        {
            coordinates.setAlt(alt);
        }

        GPSItemInfo gpsInfo;
        gpsInfo.coordinates = coordinates;
        gpsInfo.dateTime    = meta->getItemDateTime();
        gpsInfo.rating      = meta->getItemRating();
        gpsInfo.url         = url;
        setGPSInfoList(GPSItemInfo::List() << gpsInfo);
    }
    else
    {
        clearGPSInfo();
    }
}

void ItemPropertiesGPSTab::clearGPSInfo()
{
    d->altitude->setAdjustedText();
    d->latitude->setAdjustedText();
    d->longitude->setAdjustedText();
    d->date->setAdjustedText();
    d->itemModel->clear();
    d->mapView->setCurrentIndex(1);
    setEnabled(false);
}

void ItemPropertiesGPSTab::setGPSInfoList(const GPSItemInfo::List& list)
{
    // Clear info label

    d->altitude->setAdjustedText();
    d->latitude->setAdjustedText();
    d->longitude->setAdjustedText();
    d->date->setAdjustedText();
    d->gpsInfoList.clear();
    d->itemModel->clear();
    d->gpsInfoList = list;

    if (list.isEmpty())
    {
        clearGPSInfo();
        return;
    }

    d->mapView->setCurrentIndex(0);
    setEnabled(true);

    if (list.count() == 1)
    {
        const GPSItemInfo info            = list.first();
        const GeoCoordinates& coordinates = info.coordinates;

        if (!coordinates.hasAltitude())
        {
            d->altitude->setAdjustedText(i18nc("@label: no GPS coordinates", "Undefined"));
        }
        else
        {
            d->altitude->setAdjustedText(QString::fromLatin1("%1 m").arg(QLocale().toString(coordinates.alt(), 'g', 7)));
        }

        d->gpsCopyBtn->setEnabled(true);
        d->latitude->setAdjustedText(QLocale().toString(coordinates.lat(), 'g', 7));
        d->longitude->setAdjustedText(QLocale().toString(coordinates.lon(), 'g', 7));
        d->date->setAdjustedText(asUserDateTime(info.dateTime));
    }
    else
    {
        d->gpsCopyBtn->setEnabled(false);
    }

    for (int i = 0 ; i < d->gpsInfoList.count() ; ++i)
    {
        QStandardItem* const currentItemGPSItem = new QStandardItem();
        currentItemGPSItem->setData(QVariant::fromValue(d->gpsInfoList.at(i)), RoleGPSItemInfo);
        d->itemModel->appendRow(currentItemGPSItem);
    }

    if (!d->map->getStickyModeState())
    {
        if (!d->map->getActiveState())
        {
            d->boundariesShouldBeAdjusted = true;
        }
        else
        {
            d->boundariesShouldBeAdjusted = false;
            d->map->adjustBoundariesToGroupedMarkers();
        }
    }
}

void ItemPropertiesGPSTab::setActive(const bool state)
{
    d->map->setActive(state);

    if (state && d->boundariesShouldBeAdjusted)
    {
        d->boundariesShouldBeAdjusted = false;
        d->map->adjustBoundariesToGroupedMarkers();
    }
}

} // namespace Digikam

#include "moc_itempropertiesgpstab.cpp"

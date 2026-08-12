/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "MarbleWidgetPopupMenu.h"

// Qt includes

#include <QApplication>
#include <QFile>
#include <QMimeData>
#include <QPointer>
#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "AbstractDataPluginItem.h"
#include "AbstractFloatItem.h"
#include "MarbleDirs.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "GeoDataExtendedData.h"
#include "GeoDataFolder.h"
#include "GeoDataPlacemark.h"
#include "GeoDataLookAt.h"
#include "GeoDataData.h"
#include "GeoDataSnippet.h"
#include "GeoDataStyle.h"
#include "GeoDataBalloonStyle.h"
#include "GeoDataIconStyle.h"
#include "GeoDataPoint.h"
#include "GeoDataPhotoOverlay.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "MarbleClock.h"
#include "PopupLayer.h"
#include "Planet.h"
#include "ReverseGeocodingRunnerManager.h"
#include "TemplateDocument.h"
#include "OsmPlacemarkData.h"
#include "StyleBuilder.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN MarbleWidgetPopupMenu::Private
{
public:

    const MarbleModel* const        m_model                 = nullptr;
    MarbleWidget* const             m_widget                = nullptr;

    QVector<const GeoDataFeature*>  m_featurelist;
    QList<AbstractDataPluginItem*>  m_itemList;

    QMenu                           m_lmbMenu;
    QMenu                           m_rmbMenu;

    QAction*                        m_infoDialogAction      = nullptr;

    QAction* const                  m_copyCoordinateAction  = nullptr;

    QAction*                        m_rmbExtensionPoint     = nullptr;

    ReverseGeocodingRunnerManager   m_runnerManager;

    QPoint                          m_mousePosition;

public:

    Private(MarbleWidget* widget, const MarbleModel* model, MarbleWidgetPopupMenu* parent);
    QMenu* createInfoBoxMenu(QWidget* parent);

    /**
      * Returns the geo coordinates of the mouse pointer at the last right button menu.
      * You must not pass 0 as coordinates parameter. The result indicates whether the
      * coordinates are valid, which will be true if the right button menu was opened at least once.
      */
    GeoDataCoordinates mouseCoordinates(QAction* dataContainer) const;

    static QString filterEmptyShortDescription(const QString& description);
    void setupDialogOsm(PopupLayer* popup, const GeoDataPlacemark* placemark);
    void setupDialogSatellite(const GeoDataPlacemark* placemark);
    static void setupDialogCity(PopupLayer* popup, const GeoDataPlacemark* placemark);
    static void setupDialogNation(PopupLayer* popup, const GeoDataPlacemark* placemark);
    static void setupDialogGeoPlaces(PopupLayer* popup, const GeoDataPlacemark* placemark);
    static void setupDialogSkyPlaces(PopupLayer* popup, const GeoDataPlacemark* placemark);
    static void setupDialogPhotoOverlay(PopupLayer* popup, const GeoDataPhotoOverlay* overlay);
};

MarbleWidgetPopupMenu::Private::Private(MarbleWidget* widget, const MarbleModel* model, MarbleWidgetPopupMenu* parent)
    : m_model(model),
      m_widget(widget),
      m_lmbMenu(m_widget),
      m_rmbMenu(m_widget),
      m_copyCoordinateAction(new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("Copy Coordinates"), parent)),
      m_rmbExtensionPoint(nullptr),
      m_runnerManager(model)
{
    // Property actions (Left mouse button)

    m_infoDialogAction = new QAction(parent);
    m_infoDialogAction->setData(0);

    //  Tool actions (Right mouse button)

    QMenu* infoBoxMenu = createInfoBoxMenu(m_widget);

    m_rmbMenu.addSeparator();
    m_rmbMenu.addAction(m_copyCoordinateAction);

    m_rmbMenu.addAction(QIcon::fromTheme(QStringLiteral("addressbook-details")), i18n("&Address Details"),
                        parent, SLOT(startReverseGeocoding()));

    m_rmbMenu.addSeparator();
    m_rmbMenu.addMenu(infoBoxMenu);

    parent->connect(&m_lmbMenu, SIGNAL(aboutToHide()),
                    SLOT(resetMenu()));

    parent->connect(m_copyCoordinateAction, SIGNAL(triggered()),
                    SLOT(slotCopyCoordinates()));

    parent->connect(m_infoDialogAction, SIGNAL(triggered()),
                    SLOT(slotInfoDialog()));

    parent->connect(&m_runnerManager, SIGNAL(reverseGeocodingFinished(GeoDataCoordinates,GeoDataPlacemark)),
                    parent, SLOT(showAddressInformation(GeoDataCoordinates,GeoDataPlacemark)));
}

QString MarbleWidgetPopupMenu::Private::filterEmptyShortDescription(const QString& description)
{
    if (description.isEmpty())
    {
        return i18n("No description available.");
    }

    return description;
}

void MarbleWidgetPopupMenu::Private::setupDialogOsm(PopupLayer* popup, const GeoDataPlacemark* placemark)
{
    const GeoDataCoordinates location = placemark->coordinate();
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/osm.html"));

    if (!descriptionFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QString none           = QStringLiteral("none");
    QString description          = QString::fromUtf8(descriptionFile.readAll());
    const OsmPlacemarkData& data = placemark->osmData();

    if (!data.containsTagKey(QString::fromUtf8("addr:street")) && !data.containsTagKey(QString::fromUtf8("addr:housenumber")))
    {
        description.replace(QStringLiteral("<br> %postcode%"), QStringLiteral("%postcode%"));
    }

    TemplateDocument doc(description);

    doc[QStringLiteral("name")] = data.tagValue(QStringLiteral("name"));
    QString natural             = data.tagValue(QStringLiteral("natural"));

    if (!natural.isEmpty())
    {
        natural[0] = natural[0].toUpper();

        if (natural == QLatin1String("Peak"))
        {
            QString elevation = data.tagValue(QStringLiteral("ele"));

            if (!elevation.isEmpty())
            {
                natural = natural + QLatin1String(" - ") + elevation + QLatin1String(" m");
            }
        }

        doc[QStringLiteral("details")] = natural;
    }

    else
    {
        doc[QStringLiteral("detailsVisibility")] = none;
    }

    QString amenity;
    QString shop = data.tagValue(QStringLiteral("shop"));

    if (!shop.isEmpty())
    {
        shop[0] = shop[0].toUpper();

        if (shop == QLatin1String("Clothes"))
        {
            QString type = data.tagValue(QStringLiteral("clothes"));

            if (type.isEmpty())
            {
                type = data.tagValue(QStringLiteral("designation"));
            }

            if (!type.isEmpty())
            {
                type[0] = type[0].toUpper();
                amenity = QLatin1String("Shop - ") + shop + QLatin1String(" (") + type + QLatin1Char(')');
            }
        }

        if (amenity.isEmpty())
        {
            amenity = QLatin1String("Shop - ") + shop;
        }
    }
    else
    {
        amenity = data.tagValue(QStringLiteral("amenity"));

        if (!amenity.isEmpty())
        {
            amenity[0] = amenity[0].toUpper();
        }
    }

    if (!amenity.isEmpty())
    {
        doc[QStringLiteral("amenity")] = amenity;
    }
    else
    {
        doc[QStringLiteral("amenityVisibility")] = none;
    }

    QString cuisine = data.tagValue(QStringLiteral("cuisine"));

    if (!cuisine.isEmpty())
    {
        cuisine[0] = cuisine[0].toUpper();
        doc[QStringLiteral("cuisine")] = cuisine;
    }
    else
    {
        doc[QStringLiteral("cuisineVisibility")] = none;
    }

    QString openingHours = data.tagValue(QStringLiteral("opening_hours"));

    if (!openingHours.isEmpty())
    {
        doc[QStringLiteral("openinghours")] = openingHours;
    }
    else
    {
        doc[QStringLiteral("openinghoursVisibility")] = none;
    }

    bool hasContactsData = false;

    const QStringList addressItemKeys = QStringList()
                                        << QStringLiteral("street")
                                        << QStringLiteral("housenumber")
                                        << QStringLiteral("postcode")
                                        << QStringLiteral("city");
    bool hasAddressItem = false;
    QStringList addressItems;

    for (const QString& key : addressItemKeys)
    {
        const QString item = data.tagValue(QLatin1String("addr:") + key);

        if (!item.isEmpty())
        {
            hasAddressItem = true;
        }

        addressItems << item;
    }

    if (hasAddressItem)
    {
        hasContactsData = true;

        for (int i = 0 ; i < addressItemKeys.size() ; ++i)
        {
            doc[addressItemKeys[i]] = addressItems[i];
        }
    }
    else
    {
        doc[QStringLiteral("addressVisibility")] = none;
    }

    QString phoneData = data.tagValue(QStringLiteral("phone"));

    if (!phoneData.isEmpty())
    {
        hasContactsData = true;
        doc[QStringLiteral("phone")] = phoneData;
    }
    else
    {
        doc[QStringLiteral("phoneVisibility")] = none;
    }

    QString websiteData;
    auto const tags = QStringList() << QString::fromUtf8("website")
                                    << QString::fromUtf8("contact:website")
                                    << QString::fromUtf8("facebook")
                                    << QString::fromUtf8("contact:facebook")
                                    << QString::fromUtf8("url");

    for (const QString& tag : tags)
    {
        websiteData = data.tagValue(tag);

        if (!websiteData.isEmpty())
        {
            break;
        }
    }

    if (!websiteData.isEmpty())
    {
        hasContactsData = true;
        doc[QStringLiteral("website")] = websiteData;
    }
    else
    {
        doc[QStringLiteral("websiteVisibility")] = none;
    }

    if (!hasContactsData)
    {
        doc[QStringLiteral("contactVisibility")] = none;
    }

    bool hasFacilitiesData   = false;
    const QString wheelchair = data.tagValue(QStringLiteral("wheelchair"));

    if (!wheelchair.isEmpty())
    {
        hasFacilitiesData = true;
        doc[QStringLiteral("wheelchair")] = wheelchair;
    }
    else
    {
        doc[QStringLiteral("wheelchairVisibility")] = none;
    }

    const QString internetAccess = data.tagValue(QStringLiteral("internet_access"));

    if (!internetAccess.isEmpty())
    {
        hasFacilitiesData                     = true;
        doc[QStringLiteral("internetaccess")] = internetAccess;
    }
    else
    {
        doc[QStringLiteral("internetVisibility")] = none;
    }

    const QString smoking = data.tagValue(QStringLiteral("smoking"));

    if (!smoking.isEmpty())
    {
        hasFacilitiesData              = true;
        doc[QStringLiteral("smoking")] = smoking;
    }
    else
    {
        doc[QStringLiteral("smokingVisibility")] = none;
    }

    if (!hasFacilitiesData)
    {
        doc[QStringLiteral("facilitiesVisibility")] = none;
    }

    const QString flagPath = m_widget->styleBuilder()->createStyle(StyleParameters(placemark))->iconStyle().iconPath();
    doc[QString::fromUtf8("flag")] = flagPath;
    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupMenu::Private::setupDialogSatellite(const GeoDataPlacemark* placemark)
{
    PopupLayer* const popup           = m_widget->popupLayer();
    const GeoDataCoordinates location = placemark->coordinate(m_widget->model()->clockDateTime());
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    const QString description = placemark->description();
    TemplateDocument doc(description);
    doc[QString::fromUtf8("altitude")]  = QString::number(location.altitude(), 'f', 2);
    doc[QString::fromUtf8("latitude")]  = location.latToString();
    doc[QString::fromUtf8("longitude")] = location.lonToString();
    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupMenu::Private::setupDialogCity(PopupLayer* popup, const GeoDataPlacemark* placemark)
{
    const GeoDataCoordinates location = placemark->coordinate();
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/city.html"));

    if (!descriptionFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QString description = QString::fromUtf8(descriptionFile.readAll());
    TemplateDocument doc(description);

    doc[QString::fromUtf8("name")] = placemark->name();
    QString  roleString;
    const QString role = placemark->role();

    if      (role == QLatin1String("PPLC"))
    {
        roleString = i18n("National Capital");
    }
    else if (role == QLatin1String("PPL"))
    {
        roleString = i18n("City");
    }
    else if (role == QLatin1String("PPLA"))
    {
        roleString = i18n("State Capital");
    }
    else if (role == QLatin1String("PPLA2"))
    {
        roleString = i18n("County Capital");
    }
    else if (role == QLatin1String("PPLA3") ||
             role == QLatin1String("PPLA4"))
    {
        roleString = i18n("Capital");
    }
    else if (role == QLatin1String("PPLF") ||
             role == QLatin1String("PPLG") ||
             role == QLatin1String("PPLL") ||
             role == QLatin1String("PPLQ") ||
             role == QLatin1String("PPLR") ||
             role == QLatin1String("PPLS") ||
             role == QLatin1String("PPLW"))
    {
        roleString = i18n("Village");
    }

    doc[QString::fromUtf8("category")]         = roleString;
    doc[QString::fromUtf8("shortDescription")] = filterEmptyShortDescription(placemark->description());
    doc[QString::fromUtf8("latitude")]         = location.latToString();
    doc[QString::fromUtf8("longitude")]        = location.lonToString();
    doc[QString::fromUtf8("elevation")]        = QString::number(location.altitude(), 'f', 2);
    doc[QString::fromUtf8("population")]       = QString::number(placemark->population());
    doc[QString::fromUtf8("country")]          = placemark->countryCode();
    doc[QString::fromUtf8("state")]            = placemark->state();

    QString dst = QStringLiteral("%1").arg((placemark->extendedData().value(QStringLiteral("gmt")).value().toInt() +
                                            placemark->extendedData().value(QStringLiteral("dst")).value().toInt()) /
                                            (double) 100, 0, 'f', 1);

    // There is an issue about UTC.
    // It's possible to variants (e.g.):
    // +1.0 and -1.0, but dst does not have + an the start

    if (dst.startsWith(QLatin1Char('-')))
    {
        doc[QString::fromUtf8("timezone")] = dst;
    }
    else
    {
        doc[QString::fromUtf8("timezone")] = QLatin1Char('+') + dst;
    }

    const QString flagPath = MarbleDirs::path(QLatin1String("flags/flag_") +
                                              placemark->countryCode().toLower() +
                                              QLatin1String(".svg"));
    doc[QString::fromUtf8("flag")] = flagPath;

    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupMenu::Private::setupDialogNation(PopupLayer* popup, const GeoDataPlacemark* index)
{
    const GeoDataCoordinates location = index->coordinate();
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/nation.html"));

    if (!descriptionFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QString description = QString::fromUtf8(descriptionFile.readAll());
    TemplateDocument doc(description);

    doc[QString::fromUtf8("name")]             = index->name();
    doc[QString::fromUtf8("shortDescription")] = filterEmptyShortDescription(index->description());
    doc[QString::fromUtf8("latitude")]         = location.latToString();
    doc[QString::fromUtf8("longitude")]        = location.lonToString();
    doc[QString::fromUtf8("elevation")]        = QString::number(location.altitude(), 'f', 2);
    doc[QString::fromUtf8("population")]       = QString::number(index->population());
    doc[QString::fromUtf8("area")]             = QString::number(index->area(), 'f', 2);
    const QString flagPath                     = MarbleDirs::path(QString::fromUtf8("flags/flag_%1.svg").arg(index->countryCode().toLower()));
    doc[QString::fromUtf8("flag")]             = flagPath;

    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupMenu::Private::setupDialogGeoPlaces(PopupLayer* popup, const GeoDataPlacemark* index)
{
    const GeoDataCoordinates location = index->coordinate();
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/geoplace.html"));

    if (!descriptionFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QString description = QString::fromUtf8(descriptionFile.readAll());
    TemplateDocument doc(description);

    doc[QString::fromUtf8("name")]             = index->name();
    doc[QString::fromUtf8("latitude")]         = location.latToString();
    doc[QString::fromUtf8("longitude")]        = location.lonToString();
    doc[QString::fromUtf8("elevation")]        = QString::number(location.altitude(), 'f', 2);
    doc[QString::fromUtf8("shortDescription")] = filterEmptyShortDescription(index->description());

    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupMenu::Private::setupDialogSkyPlaces(PopupLayer* popup, const GeoDataPlacemark* index)
{
    const GeoDataCoordinates location = index->coordinate();
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/skyplace.html"));

    if (!descriptionFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QString description = QString::fromUtf8(descriptionFile.readAll());
    TemplateDocument doc(description);

    doc[QString::fromUtf8("name")]             = index->name();
    doc[QString::fromUtf8("latitude")]         = GeoDataCoordinates::latToString(
                                                 location.latitude(), GeoDataCoordinates::Astro, GeoDataCoordinates::Radian, -1, 'f');
    doc[QString::fromUtf8("longitude")]        = GeoDataCoordinates::lonToString(
                                                 location.longitude(), GeoDataCoordinates::Astro, GeoDataCoordinates::Radian, -1, 'f');
    doc[QString::fromUtf8("shortDescription")] = filterEmptyShortDescription(index->description());

    popup->setContent(doc.finalText());
}

void MarbleWidgetPopupMenu::Private::setupDialogPhotoOverlay(PopupLayer* popup, const GeoDataPhotoOverlay* index)
{
    const GeoDataCoordinates location = index->point().coordinates();
    popup->setCoordinates(location, Qt::AlignRight | Qt::AlignVCenter);

    QFile descriptionFile(QStringLiteral(":/marble/webpopup/photooverlay.html"));

    if (!descriptionFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QString description = QString::fromUtf8(descriptionFile.readAll());
    TemplateDocument doc(description);

    doc[QString::fromUtf8("name")]             = index->name();
    doc[QString::fromUtf8("latitude")]         = location.latToString();
    doc[QString::fromUtf8("longitude")]        = location.lonToString();
    doc[QString::fromUtf8("elevation")]        = QString::number(location.altitude(), 'f', 2);
    doc[QString::fromUtf8("shortDescription")] = filterEmptyShortDescription(index->description());
    doc[QString::fromUtf8("source")]           = index->absoluteIconFile();
    doc[QString::fromUtf8("width")]            = QString::number(200);
    doc[QString::fromUtf8("height")]           = QString::number(100);
    QString const basePath                     = index->resolvePath(QString::fromUtf8("."));
    QUrl const baseUrl                         = (basePath != QLatin1String(".")) ? QUrl::fromLocalFile(basePath + QLatin1Char('/')) : QUrl();

    popup->setContent(doc.finalText(), baseUrl);
}

MarbleWidgetPopupMenu::MarbleWidgetPopupMenu(MarbleWidget* widget,
                                             const MarbleModel* model)
    : QObject(widget),
      d(new Private(widget, model, this))
{
    // nothing to do
}

MarbleWidgetPopupMenu::~MarbleWidgetPopupMenu()
{
    delete d;
}

QMenu* MarbleWidgetPopupMenu::Private::createInfoBoxMenu(QWidget* parent)
{
    QMenu* menu                                         = new QMenu(i18n("&Info Boxes"), parent);
    QList<AbstractFloatItem*> floatItemList             = m_widget->floatItems();
    QList<AbstractFloatItem*>::const_iterator iter      = floatItemList.constBegin();
    QList<AbstractFloatItem*>::const_iterator const end = floatItemList.constEnd();

    for ( ; iter != end ; ++iter)
    {
        menu->addAction((*iter)->action());
    }

    return menu;
}

void MarbleWidgetPopupMenu::showLmbMenu(int xpos, int ypos)
{
    bool const smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;

    if (smallScreen)
    {
        showRmbMenu(xpos, ypos);
        return;
    }

    d->m_mousePosition.setX(xpos);
    d->m_mousePosition.setY(ypos);

    const QPoint curpos = QPoint(xpos, ypos);
    d->m_featurelist    = d->m_widget->whichFeatureAt(curpos);
    int  actionidx      = 1;

    QVector<const GeoDataFeature*>::const_iterator it          = d->m_featurelist.constBegin();
    QVector<const GeoDataFeature*>::const_iterator const itEnd = d->m_featurelist.constEnd();

    for ( ; it != itEnd ; ++it)
    {
        QString name = (*it)->name();
        QPixmap icon = QPixmap::fromImage((*it)->style()->iconStyle().icon());
        d->m_infoDialogAction->setData(actionidx);
        d->m_infoDialogAction->setText(name);
        d->m_infoDialogAction->setIcon(icon);

        // Insert as first action in the menu

        QAction* firstAction = nullptr;

        if (!d->m_lmbMenu.actions().isEmpty())
        {
            firstAction = d->m_lmbMenu.actions().first();
        }

        d->m_lmbMenu.insertAction(firstAction, d->m_infoDialogAction);
        actionidx++;
    }

    d->m_itemList                                               = d->m_widget->whichItemAt(curpos);
    QList<AbstractDataPluginItem*>::const_iterator itW          = d->m_itemList.constBegin();
    QList<AbstractDataPluginItem*>::const_iterator const itWEnd = d->m_itemList.constEnd();

    for ( ; itW != itWEnd ; ++itW)
    {
        const auto acs = (*itW)->actions();

        for (QAction* const action : acs)
        {
            d->m_lmbMenu.addAction(action);
        }
    }

    switch (d->m_lmbMenu.actions().size())
    {
        case 0: // nothing to do, ignore
            break;

        case 1: // one action? perform immediately
            d->m_lmbMenu.actions().first()->activate(QAction::Trigger);
            d->m_lmbMenu.clear();
            break;

        default:
            d->m_lmbMenu.popup(d->m_widget->mapToGlobal(curpos));
    }
}

void MarbleWidgetPopupMenu::showRmbMenu(int xpos, int ypos)
{
    qreal lon, lat;
    const bool visible = d->m_widget->geoCoordinates(xpos, ypos, lon, lat, GeoDataCoordinates::Radian);

    if (!visible)
    {
        return;
    }

    d->m_mousePosition.setX(xpos);
    d->m_mousePosition.setY(ypos);

    QPoint curpos = QPoint(xpos, ypos);
    d->m_copyCoordinateAction->setData(curpos);

    d->m_rmbMenu.popup(d->m_widget->mapToGlobal(curpos));
}

void MarbleWidgetPopupMenu::resetMenu()
{
    d->m_lmbMenu.clear();
}

void MarbleWidgetPopupMenu::slotInfoDialog()
{
    QAction* action = qobject_cast<QAction*>(sender());

    if (action == nullptr)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Warning: slotInfoDialog should be called by a QAction signal";
        return;
    }

    int actionidx = action->data().toInt();

    if (actionidx > 0)
    {
        const GeoDataPlacemark* placemark  = dynamic_cast<const GeoDataPlacemark*>(d->m_featurelist.at(actionidx - 1));
        const GeoDataPhotoOverlay* overlay = dynamic_cast<const GeoDataPhotoOverlay*>(d->m_featurelist.at(actionidx - 1));
        PopupLayer* popup                  = d->m_widget->popupLayer();
        bool isSatellite                   = false;
        bool isCity                        = false;
        bool isNation                      = false;
        const OsmPlacemarkData& data       = placemark->osmData();
        bool hasOsmData                    = false;

        QStringList recognizedTags;
        recognizedTags << QString::fromUtf8("name") << QString::fromUtf8("amenity") << QString::fromUtf8("cuisine") << QString::fromUtf8("opening_hours");
        recognizedTags << QString::fromUtf8("addr:street") << QString::fromUtf8("addr:housenumber") << QString::fromUtf8("addr:postcode");
        recognizedTags << QString::fromUtf8("addr:city") << QString::fromUtf8("phone") << QString::fromUtf8("wheelchair") << QString::fromUtf8("internet_access");
        recognizedTags << QString::fromUtf8("smoking") << QString::fromUtf8("website") << QString::fromUtf8("contact:website") << QString::fromUtf8("facebook");
        recognizedTags << QString::fromUtf8("contact:facebook") << QString::fromUtf8("url");

        for (const QString& tag : std::as_const(recognizedTags))
        {
            if (data.containsTagKey(tag))
            {
                hasOsmData = true;
                break;
            }
        }

        if (placemark)
        {
            isSatellite = (placemark->visualCategory() == GeoDataPlacemark::Satellite);
            isCity      = (placemark->visualCategory() >= GeoDataPlacemark::SmallCity &&
                           placemark->visualCategory() <= GeoDataPlacemark::LargeNationCapital);
            isNation    = (placemark->visualCategory() == GeoDataPlacemark::Nation);
        }

        bool isSky = false;

        if (d->m_widget->model()->mapTheme())
        {
            isSky = d->m_widget->model()->mapTheme()->head()->target() == QLatin1String("sky");
        }

        popup->setSize(QSizeF(420, 420));

        if      (hasOsmData)
        {
            d->setupDialogOsm(popup, placemark);
        }
        else if (isSatellite)
        {
            d->setupDialogSatellite(placemark);
        }
        else if (isCity)
        {
            Private::setupDialogCity(popup, placemark);
        }
        else if (isNation)
        {
            Private::setupDialogNation(popup, placemark);
        }
        else if (isSky)
        {
            Private::setupDialogSkyPlaces(popup, placemark);
        }
        else if (overlay)
        {
            Private::setupDialogPhotoOverlay(popup, overlay);
        }
        else if (placemark && placemark->role().isEmpty())
        {
            popup->setContent(placemark->description());
        }
        else if (placemark)
        {
            Private::setupDialogGeoPlaces(popup, placemark);
        }

        if (placemark)
        {
            if (placemark->style() == nullptr)
            {
                popup->setBackgroundColor(QColor(Qt::white));
                popup->setTextColor(QColor(Qt::black));
                return;
            }

            if (placemark->style()->balloonStyle().displayMode() == GeoDataBalloonStyle::Hide)
            {
                popup->setVisible(false);
                return;
            }

            QString content = placemark->style()->balloonStyle().text();

            if (content.length() > 0)
            {
                content.replace(QStringLiteral("$[name]"), placemark->name(), Qt::CaseInsensitive);
                content.replace(QStringLiteral("$[description]"), placemark->description(), Qt::CaseInsensitive);
                content.replace(QStringLiteral("$[address]"), placemark->address(), Qt::CaseInsensitive);

                // @todo: implement the line calculation, so that snippet().maxLines actually has effect.

                content.replace(QStringLiteral("$[snippet]"), placemark->snippet().text(), Qt::CaseInsensitive);
                content.replace(QStringLiteral("$[id]"), placemark->id(), Qt::CaseInsensitive);
                QString const basePath = placemark->resolvePath(QString::fromUtf8("."));
                QUrl const baseUrl     = (basePath != QLatin1String(".")) ? QUrl::fromLocalFile(basePath + QLatin1Char('/')) : QUrl();

                popup->setContent(content, baseUrl);
            }

            popup->setBackgroundColor(placemark->style()->balloonStyle().backgroundColor());
            popup->setTextColor(placemark->style()->balloonStyle().textColor());
        }

        popup->popup();
    }
}

void MarbleWidgetPopupMenu::slotCopyCoordinates()
{
    const GeoDataCoordinates coordinates = d->mouseCoordinates(d->m_copyCoordinateAction);

    if (coordinates.isValid())
    {
        const qreal longitude_degrees = coordinates.longitude(GeoDataCoordinates::Degree);
        const qreal latitude_degrees  = coordinates.latitude(GeoDataCoordinates::Degree);

        // importing this representation into Marble does not show anything,
        // but Merkaartor shows the point

        const QString kmlRepresentation = QString::fromLatin1(
                                              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                              "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"        // krazy:exclude=insecurenet
                                              "<Document>\n"
                                              " <Placemark>\n"
                                              //    "   <name></name>\n"
                                              "   <Point>\n"
                                              "     <coordinates>%1,%2</coordinates>\n"
                                              "   </Point>\n"
                                              " </Placemark>\n"
                                              "</Document>\n"
                                              "</kml>\n"
                                          ).arg(longitude_degrees, 0, 'f', 10).arg(latitude_degrees, 0, 'f', 10);

        // importing this data into Marble and Merkaartor works

        const QString gpxRepresentation = QString::fromLatin1(
                                              "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
                                              "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"trippy\" version=\"0.1\"\n"                       // krazy:exclude=insecurenet
                                              " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"                                                  // krazy:exclude=insecurenet
                                              " xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"      // krazy:exclude=insecurenet
                                              "  <wpt lat=\"%1\" lon=\"%2\">\n"
                                              //      "   <ele>%3</ele>\n"
                                              //      "   <time></time>\n"
                                              //      "   <name>%4</name>\n"
                                              "  </wpt>\n"
                                              "</gpx>\n"
                                          ).arg(latitude_degrees, 0, 'f', 10).arg(longitude_degrees, 0, 'f', 10);

        QString  positionString = coordinates.toString(GeoDataCoordinates::Simply);

        QMimeData* const myMimeData = new QMimeData();
        myMimeData->setText(positionString);
        myMimeData->setData(QLatin1String("application/vnd.google-earth.kml+xml"), kmlRepresentation.toUtf8());
        myMimeData->setData(QLatin1String("application/gpx+xml"), gpxRepresentation.toUtf8());

        QClipboard* const clipboard = QApplication::clipboard();
        clipboard->setMimeData(myMimeData);
    }
}

void MarbleWidgetPopupMenu::addAction(Qt::MouseButton button, QAction* action)
{
    if (button == Qt::RightButton)
    {
        d->m_rmbMenu.insertAction(d->m_rmbExtensionPoint, action);
    }
    else
    {
        d->m_lmbMenu.addAction(action);
    }
}

GeoDataCoordinates MarbleWidgetPopupMenu::Private::mouseCoordinates(QAction* dataContainer) const
{
    if (!dataContainer)
    {
        return GeoDataCoordinates();
    }

    if (!m_featurelist.isEmpty() && geodata_cast<GeoDataPlacemark>(m_featurelist.first()))
    {
        const GeoDataPlacemark* placemark =  static_cast<const GeoDataPlacemark*>(m_featurelist.first());

        return placemark->coordinate(m_model->clock()->dateTime());
    }
    else
    {
        QPoint p = dataContainer->data().toPoint();
        qreal lat(0.0), lon(0.0);

        const bool valid = m_widget->geoCoordinates(p.x(), p.y(), lon, lat, GeoDataCoordinates::Radian);

        if (valid)
        {
            return GeoDataCoordinates(lon, lat);
        }
    }

    return GeoDataCoordinates();
}

void MarbleWidgetPopupMenu::startReverseGeocoding()
{
    const GeoDataCoordinates coordinates = d->mouseCoordinates(d->m_copyCoordinateAction);

    if (coordinates.isValid())
    {
        d->m_runnerManager.reverseGeocoding(coordinates);
    }
}

void MarbleWidgetPopupMenu::showAddressInformation(const GeoDataCoordinates&, const GeoDataPlacemark& placemark)
{
    QString text = placemark.address();

    if (!text.isEmpty())
    {
        QMessageBox::information(d->m_widget, i18n("Address Details"), text, QMessageBox::Ok);
    }
}

QPoint MarbleWidgetPopupMenu::mousePosition() const
{
    return d->m_mousePosition;
}

} // namespace Marble

#include "moc_MarbleWidgetPopupMenu.cpp"

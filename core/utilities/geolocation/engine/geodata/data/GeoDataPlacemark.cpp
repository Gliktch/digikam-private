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

#include "GeoDataPlacemark.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#endif

#include "GeoDataPlacemark_p.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Qt includes

#include <QDataStream>
#include <QString>
#include <QXmlStreamWriter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GeoDataMultiGeometry.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataData.h"
#include "OsmPlacemarkData.h"
#include "GeoDataTrack.h"
#include "GeoDataModel.h"
#include "digikam_debug.h"

namespace Marble
{

const OsmPlacemarkData GeoDataPlacemarkPrivate::s_nullOsmPlacemarkData = OsmPlacemarkData();
const GeoDataPlacemarkExtendedData GeoDataPlacemarkPrivate::s_nullPlacemarkExtendedData = GeoDataPlacemarkExtendedData();

GeoDataPlacemark::GeoDataPlacemark()
    : GeoDataFeature(new GeoDataPlacemarkPrivate)
{
    Q_D(GeoDataPlacemark);
    d->m_geometry->setParent(this);
}


GeoDataPlacemark::GeoDataPlacemark(const GeoDataPlacemark& other)
    : GeoDataFeature(other, new GeoDataPlacemarkPrivate(*other.d_func()))
{
    Q_D(GeoDataPlacemark);

    if (d->m_geometry)
    {
        d->m_geometry->setParent(this);
    }
}

GeoDataPlacemark::GeoDataPlacemark(const QString& name)
    : GeoDataFeature(new GeoDataPlacemarkPrivate)
{
    Q_D(GeoDataPlacemark);
    d->m_name = name;
    d->m_geometry->setParent(this);
}

GeoDataPlacemark::~GeoDataPlacemark()
{
    // nothing to do
}

GeoDataPlacemark& GeoDataPlacemark::operator=(const GeoDataPlacemark& other)
{
    if (this != &other)
    {
        Q_D(GeoDataPlacemark);
        *d = *other.d_func();

        if (d->m_geometry)
        {
            d->m_geometry->setParent(this);
        }
    }

    return *this;
}

bool GeoDataPlacemark::operator==(const GeoDataPlacemark& other) const
{
    Q_D(const GeoDataPlacemark);
    const GeoDataPlacemarkPrivate* const other_d = other.d_func();

    if (!equals(other) ||
        (d->m_population != other_d->m_population))
    {
        return false;
    }

    if ((d->m_placemarkExtendedData && !other_d->m_placemarkExtendedData) &&
        (*d->m_placemarkExtendedData != GeoDataPlacemarkExtendedData()))
    {
        return false;
    }

    if ((!d->m_placemarkExtendedData && other_d->m_placemarkExtendedData) &&
        (GeoDataPlacemarkExtendedData() != *other_d->m_placemarkExtendedData))
    {
        return false;
    }

    if (d->m_placemarkExtendedData && other_d->m_placemarkExtendedData &&
        !(*d->m_placemarkExtendedData == *other_d->m_placemarkExtendedData))
    {
        return false;
    }

    if (!d->m_geometry && !other_d->m_geometry)
    {
        return true;
    }

    if ((!d->m_geometry && other_d->m_geometry) ||
        (d->m_geometry && !other_d->m_geometry))
    {
        return false;
    }

    if (*d->m_geometry != *other_d->m_geometry)
    {
        return false;
    }

    return true;
}

bool GeoDataPlacemark::operator!=(const GeoDataPlacemark& other) const
{
    return !this->operator==(other);
}

const char* GeoDataPlacemark::nodeType() const
{
    return GeoDataTypes::GeoDataPlacemarkType;
}


GeoDataFeature* GeoDataPlacemark::clone() const
{
    return new GeoDataPlacemark(*this);
}


GeoDataPlacemark::GeoDataVisualCategory GeoDataPlacemark::visualCategory() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_visualCategory;
}

void GeoDataPlacemark::setVisualCategory(GeoDataPlacemark::GeoDataVisualCategory index)
{
    Q_D(GeoDataPlacemark);
    d->m_visualCategory = index;
}

GeoDataGeometry* GeoDataPlacemark::geometry()
{
    Q_D(GeoDataPlacemark);      // cppcheck-suppress constVariablePointer
    return d->m_geometry;
}

const GeoDataGeometry* GeoDataPlacemark::geometry() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_geometry;
}

const OsmPlacemarkData& GeoDataPlacemark::osmData() const
{
    Q_D(const GeoDataPlacemark);
    return d->osmPlacemarkData();
}

void GeoDataPlacemark::setOsmData(const OsmPlacemarkData& osmData)
{
    Q_D(GeoDataPlacemark);
    d->osmPlacemarkData() = osmData;
}

OsmPlacemarkData& GeoDataPlacemark::osmData()
{
    Q_D(GeoDataPlacemark);
    return d->osmPlacemarkData();
}

bool GeoDataPlacemark::hasOsmData() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_osmPlacemarkData != nullptr;
}

void GeoDataPlacemark::clearOsmData()
{
    Q_D(GeoDataPlacemark);
    delete d->m_osmPlacemarkData;
    d->m_osmPlacemarkData = nullptr;
}

const GeoDataLookAt* GeoDataPlacemark::lookAt() const
{
    return dynamic_cast<const GeoDataLookAt*>(abstractViewBase());
}

GeoDataLookAt* GeoDataPlacemark::lookAt()
{
    return dynamic_cast<GeoDataLookAt*>(abstractViewBase());
}

bool GeoDataPlacemark::placemarkLayoutOrderCompare(const GeoDataPlacemark* left, const GeoDataPlacemark* right)
{
    const GeoDataPlacemarkPrivate* const left_d  = left->d_func();
    const GeoDataPlacemarkPrivate* const right_d = right->d_func();

    if (left_d->m_zoomLevel != right_d->m_zoomLevel)
    {
        return (left_d->m_zoomLevel < right_d->m_zoomLevel);    // lower zoom level comes first
    }

    if (left_d->m_popularity != right_d->m_popularity)
    {
        return (left_d->m_popularity > right_d->m_popularity);  // higher popularity comes first
    }

    return left < right;                                        // lower pointer value comes first
}

GeoDataCoordinates GeoDataPlacemark::coordinate(const QDateTime& dateTime, bool* iconAtCoordinates) const
{
    Q_D(const GeoDataPlacemark);

    bool hasIcon = false;
    GeoDataCoordinates coord;

    if (d->m_geometry)
    {
        // Beware: comparison between pointers, not strings.

        if      (geodata_cast<GeoDataPoint>(d->m_geometry)
                 || geodata_cast<GeoDataPolygon>(d->m_geometry)
                 || geodata_cast<GeoDataLinearRing>(d->m_geometry))
        {
            hasIcon = true;
            coord   = d->m_geometry->latLonAltBox().center();
        }
        // cppcheck-suppress constVariablePointer
        else if (const auto multiGeometry = geodata_cast<GeoDataMultiGeometry>(d->m_geometry))
        {
            QVector<GeoDataGeometry*>::ConstIterator it  = multiGeometry->constBegin();
            QVector<GeoDataGeometry*>::ConstIterator end = multiGeometry->constEnd();

            for ( ; it != end ; ++it)
            {
                if (geodata_cast<GeoDataPoint>(*it)
                    || geodata_cast<GeoDataPolygon>(*it)
                    || geodata_cast<GeoDataLinearRing>(*it))
                {
                    hasIcon = true;
                    break;
                }
            }

            coord = d->m_geometry->latLonAltBox().center();
        }
        // cppcheck-suppress constVariablePointer
        else if (const auto track = geodata_cast<GeoDataTrack>(d->m_geometry))
        {
            hasIcon = (track->size() != 0) && (track->firstWhen() <= dateTime);
            coord   = track->coordinatesAt(dateTime);
        }
        else if (const auto lineString = geodata_cast<GeoDataLineString>(d->m_geometry))
        {
            auto const size = lineString->size();

            if      (size == 0)
            {
                return GeoDataCoordinates();
            }
            else if (size < 3)
            {
                // Approximate center if there are just two coordinates

                return lineString->latLonAltBox().center();
            }
            else
            {
                return lineString->at(size / 2);
            }
        }
        else
        {
            coord = d->m_geometry->latLonAltBox().center();
        }
    }

    if (iconAtCoordinates != nullptr)
    {
        *iconAtCoordinates = hasIcon;
    }

    return coord;
}

void GeoDataPlacemark::coordinate(qreal& lon, qreal& lat, qreal& alt) const
{
    coordinate().geoCoordinates(lon, lat, alt);
}

void GeoDataPlacemark::setCoordinate(qreal lon, qreal lat, qreal alt, GeoDataPoint::Unit _unit)
{
    setGeometry(new GeoDataPoint(lon, lat, alt, _unit));
}

void GeoDataPlacemark::setCoordinate(const GeoDataCoordinates& point)
{
    setGeometry(new GeoDataPoint(point));
}

void GeoDataPlacemark::setGeometry(GeoDataGeometry* entry)
{
    Q_D(GeoDataPlacemark);
    delete d->m_geometry;
    d->m_geometry = entry;
    d->m_geometry->setParent(this);
}


QString GeoDataPlacemark::displayName() const
{
    if (hasOsmData())
    {
        OsmPlacemarkData const& data = osmData();
        QStringList const uiLanguages = QLocale::system().uiLanguages();

        for (const QString& uiLanguage : uiLanguages)
        {
            for (auto tagIter = data.tagsBegin(), end = data.tagsEnd(); tagIter != end; ++tagIter)
            {
                if (tagIter.key().startsWith(QLatin1String("name:")))
                {
                    QString const tagLanguage = tagIter.key().mid(5);

                    if (tagLanguage == uiLanguage)
                    {
                        return tagIter.value();
                    }
                }
            }
        }
    }

    return name();
}

QString GeoDataPlacemark::categoryName() const
{
    Q_D(const GeoDataPlacemark);

    switch (d->m_visualCategory)
    {
        case Valley:
            return i18n("Valley");

        case OtherTerrain:
            return i18n("Terrain");

        case Crater:
            return i18n("Crater");

        case Mare:
            return i18n("Sea");

        case MannedLandingSite:
            return i18n("Manned Landing Site");

        case RoboticRover:
            return i18n("Robotic Rover");

        case UnmannedSoftLandingSite:
            return i18n("Unmanned Soft Landing Site");

        case UnmannedHardLandingSite:
            return i18n("Unmanned Hard Landing Site");

        case Mons:
            return i18n("Mountain");

        case SmallCity:
            return i18n("City");

        case SmallCountyCapital:
            return i18n("County Capital");

        case SmallStateCapital:
            return i18n("State Capital");

        case SmallNationCapital:
            return i18n("Nation Capital");

        case MediumCity:
            return i18n("City");

        case MediumCountyCapital:
            return i18n("County Capital");

        case MediumStateCapital:
            return i18n("State Capital");

        case MediumNationCapital:
            return i18n("Nation Capital");

        case BigCity:
            return i18n("City");

        case BigCountyCapital:
            return i18n("County Capital");

        case BigStateCapital:
            return i18n("State Capital");

        case BigNationCapital:
            return i18n("Nation Capital");

        case LargeCity:
            return i18n("City");

        case LargeCountyCapital:
            return i18n("County Capital");

        case LargeStateCapital:
            return i18n("State Capital");

        case LargeNationCapital:
            return i18n("Nation Capital");

        case Nation:
            return i18n("Nation");

        case Mountain:
            return i18n("Mountain");

        case Volcano:
            return i18n("Volcano");

        case Continent:
            return i18n("Continent");

        case Ocean:
            return i18n("Ocean");

        case GeographicPole:
            return i18n("Geographic Pole");

        case MagneticPole:
            return i18n("Magnetic Pole");

        case ShipWreck:
            return i18n("Ship Wreck");

        case AirPort:
            return i18n("Air Port");

        case Observatory:
            return i18n("Observatory");

        case MilitaryDangerArea:
            return i18n("Military Danger Area");

        case OsmSite:
            return i18n("OSM Site");

        case Coordinate:
            return i18n("Coordinate");

        case Satellite:
            return i18n("Satellite");

        // OpenStreetMap categories
        case PlaceCity:
            return i18n("City");

        case PlaceCityCapital:
            return i18n("City Capital");

        case PlaceCityNationalCapital:
            return i18n("National Capital");

        case PlaceSuburb:
            return i18n("Suburb");

        case PlaceHamlet:
            return i18n("Hamlet");

        case PlaceLocality:
            return i18n("Locality");

        case PlaceTown:
            return i18n("Town");

        case PlaceTownCapital:
            return i18n("Town Capital");

        case PlaceTownNationalCapital:
            return i18n("National Capital");

        case PlaceVillage:
            return i18n("Village");

        case PlaceVillageCapital:
            return i18n("Village Capital");

        case PlaceVillageNationalCapital:
            return i18n("National Capital");

        case NaturalWater:
            return i18n("Water");

        case NaturalReef:
            return i18n("Reef");

        case NaturalWood:
            return i18n("Wood");

        case NaturalBeach:
            return i18n("Beach");

        case NaturalWetland:
            return i18n("Wetland");

        case NaturalGlacier:
            return i18n("Glacier");

        case NaturalIceShelf:
            return i18n("Ice Shelf");

        case NaturalScrub:
            return i18n("Scrub");

        case NaturalCliff:
            return i18n("Cliff");

        case NaturalHeath:
            return i18n("Heath");

        case HighwayTrafficSignals:
            return i18n("Traffic Signals");

        case HighwayElevator:
            return i18n("Elevator");

        case HighwaySteps:
            return i18n("Steps");

        case HighwayUnknown:
            return i18n("Unknown Road");

        case HighwayPath:
            return i18n("Path");

        case HighwayFootway:
            return i18n("Footway");

        case HighwayTrack:
            return i18n("Track");

        case HighwayPedestrian:
            return i18n("Footway");

        case HighwayCycleway:
            return i18n("Cycleway");

        case HighwayService:
            return i18n("Service Road");

        case HighwayRoad:
            return i18n("Road");

        case HighwayResidential:
            return i18n("Residential Road");

        case HighwayLivingStreet:
            return i18n("Living Street");

        case HighwayUnclassified:
            return i18n("Unclassified Road");

        case HighwayTertiaryLink:
            return i18n("Tertiary Link Road");

        case HighwayTertiary:
            return i18n("Tertiary Road");

        case HighwaySecondaryLink:
            return i18n("Secondary Link Road");

        case HighwaySecondary:
            return i18n("Secondary Road");

        case HighwayPrimaryLink:
            return i18n("Primary Link Road");

        case HighwayPrimary:
            return i18n("Primary Road");

        case HighwayRaceway:
            return i18n("Raceway");

        case HighwayTrunkLink:
            return i18n("Trunk Link Road");

        case HighwayTrunk:
            return i18n("Trunk Road");

        case HighwayMotorwayLink:
            return i18n("Motorway Link Road");

        case HighwayMotorway:
            return i18n("Motorway");

        case HighwayCorridor:
            return i18n("Corridor");

        case Building:
            return i18n("Building");

        case AccomodationCamping:
            return i18n("Camping");

        case AccomodationHostel:
            return i18n("Hostel");

        case AccomodationHotel:
            return i18n("Hotel");

        case AccomodationMotel:
            return i18n("Motel");

        case AccomodationYouthHostel:
            return i18n("Youth Hostel");

        case AccomodationGuestHouse:
            return i18n("Guest House");

        case AmenityLibrary:
            return i18n("Library");

        case AmenityKindergarten:
            return i18n("Kindergarten");

        case EducationCollege:
            return i18n("College");

        case EducationSchool:
            return i18n("School");

        case EducationUniversity:
            return i18n("University");

        case FoodBar:
            return i18n("Bar");

        case FoodBiergarten:
            return i18n("Biergarten");

        case FoodCafe:
            return i18n("Cafe");

        case FoodFastFood:
            return i18n("Fast Food");

        case FoodPub:
            return i18n("Pub");

        case FoodRestaurant:
            return i18n("Restaurant");

        case HealthDentist:
            return i18n("Dentist");

        case HealthDoctors:
            return i18n("Doctors");

        case HealthHospital:
            return i18n("Hospital");

        case HealthPharmacy:
            return i18n("Pharmacy");

        case HealthVeterinary:
            return i18n("Veterinary");

        case MoneyAtm:
            return i18n("ATM");

        case MoneyBank:
            return i18n("Bank");

        case HistoricArchaeologicalSite:
            return i18n("Archaeological Site");

        case AmenityEmbassy:
            return i18n("Embassy");

        case AmenityEmergencyPhone:
            return i18n("Emergency Phone");

        case AmenityMountainRescue:
            return i18n("Mountain Rescue");

        case LeisureWaterPark:
            return i18n("Water Park");

        case AmenityCommunityCentre:
            return i18n("Community Centre");

        case AmenityFountain:
            return i18n("Fountain");

        case AmenityNightClub:
            return i18n("Night Club");

        case AmenityBench:
            return i18n("Bench");

        case AmenityCourtHouse:
            return i18n("Court House");

        case AmenityFireStation:
            return i18n("Fire Station");

        case AmenityHuntingStand:
            return i18n("Hunting Stand");

        case AmenityPolice:
            return i18n("Police");

        case AmenityPostBox:
            return i18n("Post Box");

        case AmenityPostOffice:
            return i18n("Post Office");

        case AmenityPrison:
            return i18n("Prison");

        case AmenityRecycling:
            return i18n("Recycling");

        case AmenityShelter:
            return i18n("Shelter");

        case AmenityTelephone:
            return i18n("Telephone");

        case AmenityToilets:
            return i18n("Toilets");

        case AmenityTownHall:
            return i18n("Town Hall");

        case AmenityWasteBasket:
            return i18n("Waste Basket");

        case AmenityDrinkingWater:
            return i18n("Drinking Water");

        case AmenityGraveyard:
            return i18n("Graveyard");

        case AmenityChargingStation:
            return i18n("Charging Station");

        case AmenityCarWash:
            return i18n("Car Wash");

        case AmenitySocialFacility:
            return i18n("Social Facility");

        case BarrierCityWall:
            return i18n("City Wall");

        case BarrierGate:
            return i18n("Gate");

        case BarrierLiftGate:
            return i18n("Lift Gate");

        case BarrierWall:
            return i18n("Wall");

        case NaturalVolcano:
            return i18n("Volcano");

        case NaturalPeak:
            return i18n("Peak");

        case NaturalTree:
            return i18n("Tree");

        case NaturalCave:
            return i18n("Cave Entrance");

        case ShopBeverages:
            return i18n("Beverages");

        case ShopHifi:
            return i18n("Hifi");

        case ShopSupermarket:
            return i18n("Supermarket");

        case ShopAlcohol:
            return i18n("Liquor Store");

        case ShopBakery:
            return i18n("Bakery");

        case ShopButcher:
            return i18n("Butcher");

        case ShopConfectionery:
            return i18n("Confectionery");

        case ShopConvenience:
            return i18n("Convenience Shop");

        case ShopGreengrocer:
            return i18n("Greengrocer");

        case ShopSeafood:
            return i18n("Seafood Shop");

        case ShopDepartmentStore:
            return i18n("Department Store");

        case ShopKiosk:
            return i18n("Kiosk");

        case ShopBag:
            return i18n("Bag Shop");

        case ShopClothes:
            return i18n("Clothes Shop");

        case ShopFashion:
            return i18n("Fashion Shop");

        case ShopJewelry:
            return i18n("Jewelry Shop");

        case ShopShoes:
            return i18n("Shoe Shop");

        case ShopVarietyStore:
            return i18n("Variety Store");

        case ShopBeauty:
            return i18n("Beauty Services");

        case ShopChemist:
            return i18n("Chemist");

        case ShopCosmetics:
            return i18n("Cosmetics");

        case ShopHairdresser:
            return i18n("Hairdresser");

        case ShopOptician:
            return i18n("Optician");

        case ShopPerfumery:
            return i18n("Perfumery");

        case ShopDoitYourself:
            return i18n("Hardware Store");

        case ShopFlorist:
            return i18n("Florist");

        case ShopHardware:
            return i18n("Hardware Store");

        case ShopFurniture:
            return i18n("Furniture Store");

        case ShopElectronics:
            return i18n("Electronics Shop");

        case ShopMobilePhone:
            return i18n("Mobile Phone Shop");

        case ShopBicycle:
            return i18n("Bicycle Shop");

        case ShopCar:
            return i18n("Car Dealer");

        case ShopCarRepair:
            return i18n("Car Repair Shop");

        case ShopCarParts:
            return i18n("Car Parts");

        case ShopMotorcycle:
            return i18n("Motorcycle Shop");

        case ShopOutdoor:
            return i18n("Outdoor Shop");

        case ShopSports:
            return i18n("Sports Shop");

        case ShopCopy:
            return i18n("Printing Services");

        case ShopArt:
            return i18n("Art Shop");

        case ShopMusicalInstrument:
            return i18n("Musical Instrument Shop");

        case ShopPhoto:
            return i18n("Photo Shop");

        case ShopBook:
            return i18n("Bookshop");

        case ShopGift:
            return i18n("Gift Shop");

        case ShopStationery:
            return i18n("Stationery");

        case ShopLaundry:
            return i18n("Laundry");

        case ShopPet:
            return i18n("Pet Shop");

        case ShopToys:
            return i18n("Toy Store");

        case ShopTravelAgency:
            return i18n("Travel Agency");

        case ShopDeli:
            return i18n("Deli");

        case ShopTobacco:
            return i18n("Tobacco Shop");

        case ShopTea:
            return i18n("Tea Shop");

        case ShopComputer:
            return i18n("Computer Shop");

        case ShopGardenCentre:
            return i18n("Garden Centre");

        case Shop:
            return i18n("Shop");

        case ManmadeBridge:
            return i18n("Bridge");

        case ManmadeLighthouse:
            return i18n("Lighthouse");

        case ManmadePier:
            return i18n("Pier");

        case ManmadeWaterTower:
            return i18n("Water Tower");

        case ManmadeWindMill:
            return i18n("Wind Mill");

        case ManmadeCommunicationsTower:
            return i18n("Communications Tower");

        case TourismAttraction:
            return i18n("Tourist Attraction");

        case TourismArtwork:
            return i18n("Artwork");

        case HistoricCastle:
            return i18n("Castle");

        case AmenityCinema:
            return i18n("Cinema");

        case TourismInformation:
            return i18n("Information");

        case HistoricMonument:
            return i18n("Monument");

        case TourismMuseum:
            return i18n("Museum");

        case HistoricRuins:
            return i18n("Ruin");

        case AmenityTheatre:
            return i18n("Theatre");

        case TourismThemePark:
            return i18n("Theme Park");

        case TourismViewPoint:
            return i18n("View Point");

        case TourismZoo:
            return i18n("Zoo");

        case TourismAlpineHut:
            return i18n("Alpine Hut");

        case TourismWildernessHut:
            return i18n("Wilderness Hut");

        case HistoricMemorial:
            return i18n("Memorial");

        case TransportAerodrome:
            return i18n("Aerodrome");

        case TransportHelipad:
            return i18n("Helipad");

        case TransportAirportGate:
            return i18n("Airport Gate");

        case TransportAirportRunway:
            return i18n("Airport Runway");

        case TransportAirportApron:
            return i18n("Airport Apron");

        case TransportAirportTaxiway:
            return i18n("Airport Taxiway");

        case TransportAirportTerminal:
            return i18n("Airport Terminal");

        case TransportBusStation:
            return i18n("Bus Station");

        case TransportBusStop:
            return i18n("Bus Stop");

        case TransportCarShare:
            return i18n("Car Sharing");

        case TransportFuel:
            return i18n("Gas Station");

        case TransportParking:
            return i18n("Parking");

        case TransportParkingSpace:
            return i18n("Parking Space");

        case TransportPlatform:
            return i18n("Platform");

        case TransportRentalBicycle:
            return i18n("Bicycle Rental");

        case TransportRentalCar:
            return i18n("Car Rental");

        case TransportRentalSki:
            return i18n("Ski Rental");

        case TransportTaxiRank:
            return i18n("Taxi Rank");

        case TransportTrainStation:
            return i18n("Train Station");

        case TransportTramStop:
            return i18n("Tram Stop");

        case TransportBicycleParking:
            return i18n("Bicycle Parking");

        case TransportMotorcycleParking:
            return i18n("Motorcycle Parking");

        case TransportSubwayEntrance:
            return i18n("Subway Entrance");

        case TransportSpeedCamera:
            return i18n("Speed Camera");

        case ReligionPlaceOfWorship:
            return i18n("Place Of Worship");

        case ReligionBahai:
            return i18n("Bahai");

        case ReligionBuddhist:
            return i18n("Buddhist");

        case ReligionChristian:
            return i18n("Christian");

        case ReligionMuslim:
            return i18n("Muslim");

        case ReligionHindu:
            return i18n("Hindu");

        case ReligionJain:
            return i18n("Jain");

        case ReligionJewish:
            return i18n("Jewish");

        case ReligionShinto:
            return i18n("Shinto");

        case ReligionSikh:
            return i18n("Sikh");

        case ReligionTaoist:
            return i18n("Taoist");

        case LeisureGolfCourse:
            return i18n("Golf Course");

        case LeisureMarina:
            return i18n("Marina");

        case LeisurePark:
            return i18n("Park");

        case LeisurePlayground:
            return i18n("Playground");

        case LeisurePitch:
            return i18n("Pitch");

        case LeisureSportsCentre:
            return i18n("Sports Centre");

        case LeisureStadium:
            return i18n("Stadium");

        case LeisureTrack:
            return i18n("Track");

        case LeisureSwimmingPool:
            return i18n("Swimming Pool");

        case LeisureMinigolfCourse:
            return i18n("Miniature Golf Course");

        case LanduseAllotments:
            return i18n("Allotments");

        case LanduseBasin:
            return i18n("Basin");

        case LanduseCemetery:
            return i18n("Cemetery");

        case LanduseCommercial:
            return i18n("Commercial");

        case LanduseConstruction:
            return i18n("Construction");

        case LanduseFarmland:
            return i18n("Farmland");

        case LanduseFarmyard:
            return i18n("Farmyard");

        case LanduseGarages:
            return i18n("Garages");

        case LanduseGrass:
            return i18n("Grass");

        case LanduseIndustrial:
            return i18n("Industrial");

        case LanduseLandfill:
            return i18n("Landfill");

        case LanduseMeadow:
            return i18n("Meadow");

        case LanduseMilitary:
            return i18n("Military");

        case LanduseQuarry:
            return i18n("Quarry");

        case LanduseRailway:
            return i18n("Railway");

        case LanduseReservoir:
            return i18n("Reservoir");

        case LanduseResidential:
            return i18n("Residential");

        case LanduseRetail:
            return i18n("Retail");

        case LanduseOrchard:
            return i18n("Orchard");

        case LanduseVineyard:
            return i18n("Vineyard");

        case RailwayRail:
            return i18n("Rail");

        case RailwayNarrowGauge:
            return i18n("Narrow Gauge");

        case RailwayTram:
            return i18n("Tram");

        case RailwayLightRail:
            return i18n("Light Rail");

        case RailwayAbandoned:
            return i18n("Abandoned Railway");

        case RailwaySubway:
            return i18n("Subway");

        case RailwayPreserved:
            return i18n("Preserved Railway");

        case RailwayMiniature:
            return i18n("Miniature Railway");

        case RailwayConstruction:
            return i18n("Railway Construction");

        case RailwayMonorail:
            return i18n("Monorail");

        case RailwayFunicular:
            return i18n("Funicular Railway");

        case PowerTower:
            return i18n("Power Tower");

        case AerialwayStation:
            return i18n("Aerialway Station");

        case AerialwayPylon:
            return i18nc("A pylon supporting the aerialway cable e.g. on a ski lift", "Pylon");

        case AerialwayCableCar:
            return i18n("Cable Car");

        case AerialwayGondola:
            return i18n("Gondola");

        case AerialwayChairLift:
            return i18n("Chair Lift");

        case AerialwayMixedLift:
            return i18n("Mixed Lift");

        case AerialwayDragLift:
            return i18n("Drag Lift");

        case AerialwayTBar:
            return i18n("T-Bar");

        case AerialwayJBar:
            return i18n("J-Bar");

        case AerialwayPlatter:
            return i18n("Platter");

        case AerialwayRopeTow:
            return i18n("Rope Tow");

        case AerialwayMagicCarpet:
            return i18n("Magic Carpet");

        case AerialwayZipLine:
            return i18n("Zip Line");

        case AerialwayGoods:
            return i18n("Goods");

        case PisteDownhill:
            return i18n("Downhill Piste");

        case PisteNordic:
            return i18n("Nordic Piste");

        case PisteSkitour:
            return i18n("Skitour");

        case PisteSled:
            return i18n("Sled Piste");

        case PisteHike:
            return i18n("Winter Hike");

        case PisteSleigh:
            return i18n("Sleigh Piste");

        case PisteIceSkate:
            return i18n("Ice Skate");

        case PisteSnowPark:
            return i18n("Snow Park");

        case PistePlayground:
            return i18n("Ski Playground");

        case PisteSkiJump:
            return i18n("Ski Jump");

        case AdminLevel1:
            return i18n("Admin Boundary (Level 1)");

        case AdminLevel2:
            return i18n("Admin Boundary (Level 2)");

        case AdminLevel3:
            return i18n("Admin Boundary (Level 3)");

        case AdminLevel4:
            return i18n("Admin Boundary (Level 4)");

        case AdminLevel5:
            return i18n("Admin Boundary (Level 5)");

        case AdminLevel6:
            return i18n("Admin Boundary (Level 6)");

        case AdminLevel7:
            return i18n("Admin Boundary (Level 7)");

        case AdminLevel8:
            return i18n("Admin Boundary (Level 8)");

        case AdminLevel9:
            return i18n("Admin Boundary (Level 9)");

        case AdminLevel10:
            return i18n("Admin Boundary (Level 10)");

        case AdminLevel11:
            return i18n("Admin Boundary (Level 11)");

        case BoundaryMaritime:
            return i18n("Boundary (Maritime)");

        case Landmass:
            return i18n("Land Mass");

        case UrbanArea:
            return i18n("Urban Area");

        case InternationalDateLine:
            return i18n("International Date Line");

        case Bathymetry:
            return i18n("Bathymetry");

        case WaterwayCanal:
            return i18n("Canal");

        case WaterwayDrain:
            return i18n("Drain");

        case WaterwayDitch:
            return i18n("Ditch");

        case WaterwayStream:
            return i18n("Stream");

        case WaterwayRiver:
            return i18n("River");

        case WaterwayWeir:
            return i18n("Weir");

        case CrossingSignals:
            return i18n("Crosswalk");

        case CrossingIsland:
            return i18n("Crosswalk");

        case CrossingZebra:
            return i18n("Crosswalk");

        case CrossingRailway:
            return i18n("Railway Crossing");

        case IndoorDoor:
            return i18n("Door");

        case IndoorWall:
            return i18n("Wall");

        case IndoorRoom:
            return i18n("Room");

        case Default:
        case Unknown:
        case None:
        case LastIndex:
            return QString();
    }

    return QString();
}

qreal GeoDataPlacemark::area() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_placemarkExtendedData ? d->m_placemarkExtendedData->m_area : -1.0;
}

void GeoDataPlacemark::setArea(qreal area)
{
    if (area == -1.0 && !d_func()->m_placemarkExtendedData)
    {
        return; // nothing to do
    }

    Q_D(GeoDataPlacemark);
    d->placemarkExtendedData().m_area = area;
}

qint64 GeoDataPlacemark::population() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_population;
}

void GeoDataPlacemark::setPopulation(qint64 population)
{
    Q_D(GeoDataPlacemark);
    d->m_population = population;
}

const QString GeoDataPlacemark::state() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_placemarkExtendedData ? d->m_placemarkExtendedData->m_state : QString();
}

void GeoDataPlacemark::setState(const QString& state)
{
    if (state.isEmpty() && !d_func()->m_placemarkExtendedData)
    {
        return; // nothing to do
    }

    Q_D(GeoDataPlacemark);
    d->placemarkExtendedData().m_state = state;
}

const QString GeoDataPlacemark::countryCode() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_placemarkExtendedData ? d->m_placemarkExtendedData->m_countrycode : QString();
}

void GeoDataPlacemark::setCountryCode(const QString& countrycode)
{
    if (countrycode.isEmpty() && !d_func()->m_placemarkExtendedData)
    {
        return; // nothing to do
    }

    Q_D(GeoDataPlacemark);
    d->placemarkExtendedData().m_countrycode = countrycode;
}

bool GeoDataPlacemark::isBalloonVisible() const
{
    Q_D(const GeoDataPlacemark);
    return d->m_placemarkExtendedData ? d->m_placemarkExtendedData->m_isBalloonVisible : false;
}

void GeoDataPlacemark::setBalloonVisible(bool visible)
{
    if (!visible && !d_func()->m_placemarkExtendedData)
    {
        return; // nothing to do
    }

    Q_D(GeoDataPlacemark);
    d->placemarkExtendedData().m_isBalloonVisible = visible;
}

void GeoDataPlacemark::pack(QDataStream& stream) const
{
    Q_D(const GeoDataPlacemark);
    GeoDataFeature::pack(stream);

    stream << d->placemarkExtendedData().m_countrycode;
    stream << d->placemarkExtendedData().m_area;
    stream << d->m_population;

    if (d->m_geometry)
    {
        stream << d->m_geometry->geometryId();
        d->m_geometry->pack(stream);
    }

    else
    {
        stream << InvalidGeometryId;
    }
}

QXmlStreamWriter& GeoDataPlacemark::pack(QXmlStreamWriter& stream) const
{
    stream.writeStartElement(QLatin1String("placemark"));

    stream.writeEndElement();
    return stream;
}

QXmlStreamWriter& GeoDataPlacemark::operator <<(QXmlStreamWriter& stream) const
{
    pack(stream);
    return stream;
}

void GeoDataPlacemark::unpack(QDataStream& stream)
{
    Q_D(GeoDataPlacemark);
    GeoDataFeature::unpack(stream);

    stream >> d->placemarkExtendedData().m_countrycode;
    stream >> d->placemarkExtendedData().m_area;
    stream >> d->m_population;
    int geometryId;
    stream >> geometryId;
    GeoDataGeometry* geometry = nullptr;

    switch (geometryId)
    {
        case InvalidGeometryId:
            break;

        case GeoDataPointId:
        {
            GeoDataPoint* point = new GeoDataPoint;
            point->unpack(stream);
            geometry = point;
        }
        break;

        case GeoDataLineStringId:
        {
            GeoDataLineString* lineString = new GeoDataLineString;
            lineString->unpack(stream);
            geometry = lineString;
        }
        break;

        case GeoDataLinearRingId:
        {
            GeoDataLinearRing* linearRing = new GeoDataLinearRing;
            linearRing->unpack(stream);
            geometry = linearRing;
        }
        break;

        case GeoDataPolygonId:
        {
            GeoDataPolygon* polygon = new GeoDataPolygon;
            polygon->unpack(stream);
            geometry = polygon;
        }
        break;

        case GeoDataMultiGeometryId:
        {
            GeoDataMultiGeometry* multiGeometry = new GeoDataMultiGeometry;
            multiGeometry->unpack(stream);
            geometry = multiGeometry;
        }
        break;

        case GeoDataModelId:
            break;

        default:
            break;
    };

    if (geometry)
    {
        delete d->m_geometry;
        d->m_geometry = geometry;
        d->m_geometry->setParent(this);
    }
}

} // namespace Marble

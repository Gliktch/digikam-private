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

#include "StyleBuilder.h"

// Qt includes

#include <QApplication>
#include <QFont>
#include <QImage>
#include <QDate>
#include <QSet>
#include <QScreen>

// Local includes

#include "MarbleDirs.h"
#include "OsmPlacemarkData.h"
#include "OsmcSymbol.h"
#include "GeoDataGeometry.h"
#include "GeoDataLinearRing.h"
#include "GeoDataLineString.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPoint.h"
#include "GeoDataPolygon.h"
#include "GeoDataIconStyle.h"
#include "GeoDataLabelStyle.h"
#include "GeoDataLineStyle.h"
#include "GeoDataPolyStyle.h"
#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN StyleBuilder::Private
{
public:

    Private();

public:

    GeoDataStyle::ConstPtr presetStyle(GeoDataPlacemark::GeoDataVisualCategory visualCategory) const;

    static GeoDataStyle::Ptr createStyle(qreal width, qreal realWidth, const QColor& color,
                                         const QColor& outlineColor, bool fill, bool outline,
                                         Qt::BrushStyle brushStyle, Qt::PenStyle penStyle,
                                         Qt::PenCapStyle capStyle, bool lineBackground,
                                         const QVector< qreal >& dashPattern,
                                         const QFont& font, const QColor& fontColor = Qt::black,
                                         const QString& texturePath = QString());

    static GeoDataStyle::Ptr createPOIStyle(const QFont& font, const QString& bitmap,
                                            const QColor& textColor = Qt::black,
                                            const QColor& color = QColor(0xBE, 0xAD, 0xAD),
                                            const QColor& outline = QColor(0xBE, 0xAD, 0xAD).darker(),
                                            bool fill = true, bool renderOutline = true);

    static GeoDataStyle::Ptr createOsmPOIStyle(const QFont& font, const QString& bitmap,
                                               const QColor& textColor = Qt::black,
                                               const QColor& color = QColor(0xBE, 0xAD, 0xAD),
                                               const QColor& outline = QColor(0xBE, 0xAD, 0xAD).darker());

    static GeoDataStyle::Ptr createOsmPOIRingStyle(const QFont& font, const QString& bitmap,
                                                   const QColor& textColor = Qt::black,
                                                   const QColor& color = QColor(0xBE, 0xAD, 0xAD),
                                                   const QColor& outline = QColor(0xBE, 0xAD, 0xAD).darker());

    static GeoDataStyle::Ptr createOsmPOIAreaStyle(const QFont& font, const QString& bitmap,
                                                   const QColor& textColor = Qt::black,
                                                   const QColor& color = QColor(0xBE, 0xAD, 0xAD),
                                                   const QColor& outline = QColor(0xBE, 0xAD, 0xAD).darker());

    static GeoDataStyle::Ptr createHighwayStyle(const QColor& color, const QColor& outlineColor, const QFont& font,
                                                const QColor& fontColor = Qt::black,
                                                qreal width = 1, qreal realWidth = 0.0,
                                                Qt::PenStyle penStyle = Qt::SolidLine,
                                                Qt::PenCapStyle capStyle = Qt::RoundCap,
                                                bool lineBackground = false);

    GeoDataStyle::Ptr createWayStyle(const QColor& color, const QColor& outlineColor,
                                     bool fill = true, bool outline = true,
                                     Qt::BrushStyle brushStyle = Qt::SolidPattern,
                                     const QString& texturePath = QString()) const;

    static GeoDataStyle::Ptr createIconWayStyle(const QColor& color, const QFont& font, const QColor& textColor, double lineWidth = 1.0, const QString& iconPath = QString());

    GeoDataStyle::ConstPtr createRelationStyle(const StyleParameters& parameters);

    GeoDataStyle::ConstPtr createPlacemarkStyle(const StyleParameters& parameters);

    GeoDataStyle::ConstPtr adjustPisteStyle(const StyleParameters& parameters, const GeoDataStyle::ConstPtr& style);

    static void adjustWayWidth(const StyleParameters& parameters, GeoDataLineStyle& lineStyle);

    // Having an outline with the same color as the fill results in degraded
    // performance and degraded display quality for no good reason
    // Q_ASSERT( !(outline && color == outlineColor && brushStyle == Qt::SolidPattern) );

    void initializeDefaultStyles();

    static QColor effectColor(const QColor& color);

    static QString createPaintLayerItem(const QString& itemType, GeoDataPlacemark::GeoDataVisualCategory visualCategory, const QString& subType = QString());

    static void initializeOsmVisualCategories();
    static void initializeMinimumZoomLevels();

public:

    int                                                               m_maximumZoomLevel;
    QColor                                                            m_defaultLabelColor;
    QFont                                                             m_defaultFont;
    GeoDataStyle::Ptr                                                 m_defaultStyle[GeoDataPlacemark::LastIndex];
    GeoDataStyle::Ptr                                                 m_styleTreeAutumn;
    GeoDataStyle::Ptr                                                 m_styleTreeWinter;
    bool                                                              m_defaultStyleInitialized;

    QHash<QString, GeoDataStyle::Ptr>                                 m_styleCache;
    QHash<GeoDataPlacemark::GeoDataVisualCategory, GeoDataStyle::Ptr> m_buildingStyles;
    QSet<QLocale::Country>                                            m_oceanianCountries;

public:

    /**
     * @brief s_visualCategories contains osm tag mappings to GeoDataVisualCategories
     */
    static QHash<OsmTag, GeoDataPlacemark::GeoDataVisualCategory> s_visualCategories;
    static int s_defaultMinZoomLevels[GeoDataPlacemark::LastIndex];
    static bool s_defaultMinZoomLevelsInitialized;
    static QHash<GeoDataPlacemark::GeoDataVisualCategory, qint64> s_popularities;
    static StyleEffect s_styleEffect;
};

QHash<StyleBuilder::OsmTag, GeoDataPlacemark::GeoDataVisualCategory> StyleBuilder::Private::s_visualCategories;
int                                                                  StyleBuilder::Private::s_defaultMinZoomLevels[GeoDataPlacemark::LastIndex];
bool                                                                 StyleBuilder::Private::s_defaultMinZoomLevelsInitialized = false;
QHash<GeoDataPlacemark::GeoDataVisualCategory, qint64>               StyleBuilder::Private::s_popularities;
StyleEffect                                                          StyleBuilder::Private::s_styleEffect = NoEffect;

StyleBuilder::Private::Private()
    : m_maximumZoomLevel(15),
      m_defaultLabelColor(Qt::black),
      m_defaultFont(QStringLiteral("Sans Serif")),
      m_defaultStyle(),
      m_defaultStyleInitialized(false),
      m_oceanianCountries(
{
    QLocale::Australia, QLocale::NewZealand, QLocale::Fiji,
    QLocale::PapuaNewGuinea, QLocale::NewCaledonia, QLocale::SolomonIslands,
    QLocale::Samoa, QLocale::Vanuatu, QLocale::Guam,
    QLocale::FrenchPolynesia, QLocale::Tonga, QLocale::Palau,
    QLocale::Kiribati, QLocale::CookIslands, QLocale::Micronesia,
    QLocale::MarshallIslands, QLocale::NauruCountry,
    QLocale::AmericanSamoa, QLocale::Niue, QLocale::Pitcairn,
    QLocale::WallisAndFutunaIslands, QLocale::NorfolkIsland
})
{
    m_oceanianCountries << QLocale::TuvaluCountry << QLocale::OutlyingOceania;
    initializeMinimumZoomLevels();

    for (int i = 0; i < GeoDataPlacemark::LastIndex; ++i)
    {
        m_maximumZoomLevel = qMax(m_maximumZoomLevel, s_defaultMinZoomLevels[i]);
    }
}

GeoDataStyle::Ptr StyleBuilder::Private::createPOIStyle(const QFont& font, const QString& path,
                                                        const QColor& textColor, const QColor& color, const QColor& outlineColor, bool fill, bool renderOutline)
{
    GeoDataStyle::Ptr style =  createStyle(1, 0, effectColor(color), effectColor(outlineColor), fill, renderOutline, Qt::SolidPattern, Qt::SolidLine, Qt::RoundCap, false, QVector<qreal>(), font);
    style->setIconStyle(GeoDataIconStyle(path));
    auto const screen = QApplication::screens().first();
    double const physicalSize = 6.0; // mm
    int const pixelSize = qRound(physicalSize * screen->physicalDotsPerInch() / (IN2M * M2MM));
    style->iconStyle().setSize(QSize(pixelSize, pixelSize));
    style->setLabelStyle(GeoDataLabelStyle(font, effectColor(textColor)));
    style->labelStyle().setAlignment(GeoDataLabelStyle::Center);
    return style;
}

GeoDataStyle::Ptr StyleBuilder::Private::createOsmPOIStyle(const QFont& font, const QString& imagePath,
                                                           const QColor& textColor, const QColor& color, const QColor& outlineColor)
{
    QString const path = MarbleDirs::path(QLatin1String("svg/osmcarto/svg/") + imagePath + QLatin1String(".svg"));
    return createPOIStyle(font, path, textColor, color, outlineColor, false, false);
}

GeoDataStyle::Ptr StyleBuilder::Private::createOsmPOIRingStyle(const QFont& font, const QString& imagePath,
                                                               const QColor& textColor, const QColor& color, const QColor& outlineColor)
{
    QString const path = MarbleDirs::path(QLatin1String("svg/osmcarto/svg/") + imagePath + QLatin1String(".svg"));
    return createPOIStyle(font, path, textColor, color, outlineColor, false, true);
}

GeoDataStyle::Ptr StyleBuilder::Private::createOsmPOIAreaStyle(const QFont& font, const QString& imagePath,
                                                               const QColor& textColor, const QColor& color, const QColor& outlineColor)
{
    QString const path = MarbleDirs::path(QLatin1String("svg/osmcarto/svg/") + imagePath + QLatin1String(".svg"));
    return createPOIStyle(font, path, textColor, color, outlineColor, true, false);
}


GeoDataStyle::Ptr StyleBuilder::Private::createHighwayStyle(const QColor& color, const QColor& outlineColor, const QFont& font,
                                                            const QColor& fontColor, qreal width, qreal realWidth, Qt::PenStyle penStyle,
                                                            Qt::PenCapStyle capStyle, bool lineBackground)
{
    return createStyle(width, realWidth, color, outlineColor, true, true, Qt::SolidPattern, penStyle, capStyle, lineBackground, QVector< qreal >(), font, fontColor);
}

GeoDataStyle::Ptr StyleBuilder::Private::createWayStyle(const QColor& color, const QColor& outlineColor,
                                                        bool fill, bool outline, Qt::BrushStyle brushStyle, const QString& texturePath) const
{
    return createStyle(1, 0, color, outlineColor, fill, outline, brushStyle, Qt::SolidLine, Qt::RoundCap, false, QVector<qreal>(), m_defaultFont, Qt::black, texturePath);
}

GeoDataStyle::Ptr StyleBuilder::Private::createIconWayStyle(const QColor& color, const QFont& font, const QColor& textColor, double lineWidth, const QString& iconPath)
{
    auto const path = iconPath.isEmpty() ? iconPath : MarbleDirs::path(iconPath);
    auto style = createPOIStyle(font, path, textColor, color, color, true, true);
    style->lineStyle().setWidth(float(lineWidth));
    return style;
}

GeoDataStyle::ConstPtr StyleBuilder::Private::createRelationStyle(const StyleParameters& parameters)
{
    Q_ASSERT(parameters.relation);
    const GeoDataPlacemark* const placemark = parameters.placemark;
    auto const visualCategory = placemark->visualCategory();
    bool const isHighway = visualCategory >= GeoDataPlacemark::HighwaySteps && visualCategory <= GeoDataPlacemark::HighwayMotorway;
    bool const isRailway = visualCategory >= GeoDataPlacemark::RailwayRail && visualCategory <= GeoDataPlacemark::RailwayFunicular;

    if (isHighway || isRailway)
    {
        if (parameters.relation->relationType() == GeoDataRelation::RouteHiking &&
            parameters.relation->osmData().containsTagKey(QStringLiteral("osmc:symbol")))
        {
            QString const osmcSymbolValue = parameters.relation->osmData().tagValue(QStringLiteral("osmc:symbol"));
            // Take cached Style instance if possible
            QString const cacheKey = QStringLiteral("/route/hiking/%1").arg(osmcSymbolValue);

            if (m_styleCache.contains(cacheKey))
            {
                return m_styleCache[cacheKey];
            }

            auto style = presetStyle(visualCategory);
            auto lineStyle = style->lineStyle();

            if (isHighway)
            {
                adjustWayWidth(parameters, lineStyle);
            }

            auto iconStyle = style->iconStyle();
            GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));
            OsmcSymbol symbol = OsmcSymbol(osmcSymbolValue);
            lineStyle.setColor(symbol.wayColor());
            iconStyle.setIcon(symbol.icon());
            newStyle->setLineStyle(lineStyle);
            newStyle->setIconStyle(iconStyle);
            style = newStyle;
            m_styleCache.insert(cacheKey, newStyle);
            return style;
        }

        if (parameters.relation->relationType() >= GeoDataRelation::RouteRoad &&
            parameters.relation->relationType() <= GeoDataRelation::RouteInlineSkates)
        {
            auto const colorValue = parameters.relation->osmData().tagValue(QStringLiteral("colour"));
            QString color = colorValue;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

            if (!QColor::isValidColorName(colorValue))

#else

            if (!QColor::isValidColor(colorValue))

#endif
            {
                switch (parameters.relation->relationType())
                {
                    case GeoDataRelation::RouteTrain:
                        color = QStringLiteral("navy");
                        break;

                    case GeoDataRelation::RouteSubway:
                        color = QStringLiteral("cornflowerblue");
                        break;

                    case GeoDataRelation::RouteTram:
                        color = QStringLiteral("steelblue");
                        break;

                    case GeoDataRelation::RouteBus:
                    case GeoDataRelation::RouteTrolleyBus:
                        color = QStringLiteral("tomato");
                        break;

                    case GeoDataRelation::RouteBicycle:
                    case GeoDataRelation::RouteMountainbike:
                    case GeoDataRelation::RouteFoot:
                    case GeoDataRelation::RouteHiking:
                    case GeoDataRelation::RouteHorse:
                    case GeoDataRelation::RouteInlineSkates:
                        color = QStringLiteral("paleturquoise");
                        break;

                    case GeoDataRelation::UnknownType:
                    case GeoDataRelation:: RouteRoad:
                    case GeoDataRelation::RouteDetour:
                    case GeoDataRelation::RouteFerry:
                    case GeoDataRelation::RouteSkiDownhill:
                    case GeoDataRelation::RouteSkiNordic:
                    case GeoDataRelation::RouteSkitour:
                    case GeoDataRelation::RouteSled:
                        color = QString();
                        break;
                }
            }

            // Take cached Style instance if possible

            QString const cacheKey = QStringLiteral("/route/%1/%2").arg(parameters.relation->relationType()).arg(color);

            if (m_styleCache.contains(cacheKey))
            {
                return m_styleCache[cacheKey];
            }

            auto style = presetStyle(visualCategory);
            auto lineStyle = style->lineStyle();

            if (isHighway)
            {
                adjustWayWidth(parameters, lineStyle);
            }

            GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));

            if (!color.isEmpty())
            {
                lineStyle.setColor(QColor(color));
                auto labelStyle = style->labelStyle();
                labelStyle.setColor(GeoDataColorStyle::contrastColor(color));
                newStyle->setLabelStyle(labelStyle);
            }

            newStyle->setLineStyle(lineStyle);
            style = newStyle;
            m_styleCache.insert(cacheKey, newStyle);
            return style;
        }
    }

    return GeoDataStyle::ConstPtr();
}

GeoDataStyle::ConstPtr StyleBuilder::Private::createPlacemarkStyle(const StyleParameters& parameters)
{
    const GeoDataPlacemark* const placemark = parameters.placemark;
    QString styleCacheKey;

    OsmPlacemarkData const& osmData         = placemark->osmData();
    auto const visualCategory               = placemark->visualCategory();

    if (visualCategory == GeoDataPlacemark::Building)
    {
        auto const tagMap      = osmTagMapping();
        auto const buildingTag = QStringLiteral("building");

        for (auto iter = osmData.tagsBegin(), end = osmData.tagsEnd(); iter != end; ++iter)
        {
            auto const osmTag = StyleBuilder::OsmTag(iter.key(), iter.value());

            if (iter.key() != buildingTag && tagMap.contains(osmTag))
            {
                return m_buildingStyles.value(tagMap.value(osmTag), m_defaultStyle[visualCategory]);
            }
        }
    }

    GeoDataStyle::ConstPtr style = presetStyle(visualCategory);

    if      (geodata_cast<GeoDataPoint>(placemark->geometry()))
    {
        if (visualCategory == GeoDataPlacemark::NaturalTree)
        {
            GeoDataCoordinates const coordinates = placemark->coordinate();
            qreal const lat = coordinates.latitude(GeoDataCoordinates::Degree);

            if (qAbs(lat) > 15)
            {
                /** @todo Should maybe auto-adjust to MarbleClock at some point */
                int const month = QDate::currentDate().month();
                bool const southernHemisphere = lat < 0;

                if (southernHemisphere)
                {
                    if      ((month >= 3) && (month <= 5))
                    {
                        style = m_styleTreeAutumn;
                    }

                    else if ((month >= 6) && (month <= 8))
                    {
                        style = m_styleTreeWinter;
                    }
                }

                else
                {
                    if      ((month >= 9) && (month <= 11))
                    {
                        style = m_styleTreeAutumn;
                    }

                    else if ((month == 12) || (month == 1) || (month == 2))
                    {
                        style = m_styleTreeWinter;
                    }
                }
            }
        }
    }

    else if (geodata_cast<GeoDataLinearRing>(placemark->geometry()))
    {
        bool adjustStyle = false;

        GeoDataPolyStyle polyStyle = style->polyStyle();
        GeoDataLineStyle lineStyle = style->lineStyle();

        if (visualCategory == GeoDataPlacemark::NaturalWater)
        {
            if (osmData.containsTag(QStringLiteral("salt"), QStringLiteral("yes")))
            {
                polyStyle.setColor(effectColor("#ffff80"));
                lineStyle.setPenStyle(Qt::DashLine);
                lineStyle.setWidth(2);
                adjustStyle = true;
            }
        }

        else if (visualCategory == GeoDataPlacemark::Bathymetry)
        {
            auto tagIter = osmData.findTag(QStringLiteral("ele"));

            if (tagIter != osmData.tagsEnd())
            {
                const QString& elevation = tagIter.value();

                if (elevation == QLatin1String("4000"))
                {
                    polyStyle.setColor(effectColor("#94c2c2"));
                    lineStyle.setColor(effectColor("#94c2c2"));
                    adjustStyle = true;
                }
            }
        }

        else if (visualCategory == GeoDataPlacemark::AmenityGraveyard || visualCategory == GeoDataPlacemark::LanduseCemetery)
        {
            auto tagIter = osmData.findTag(QStringLiteral("religion"));

            if (tagIter != osmData.tagsEnd())
            {
                const QString& religion = tagIter.value();

                if (religion == QLatin1String("jewish"))
                {
                    polyStyle.setTexturePath(MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/grave_yard_jewish.png")));
                    adjustStyle = true;
                }

                else if (religion == QLatin1String("christian"))
                {
                    polyStyle.setTexturePath(MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/grave_yard_christian.png")));
                    adjustStyle = true;
                }

                else if (religion == QLatin1String("INT-generic"))
                {
                    polyStyle.setTexturePath(MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/grave_yard_generic.png")));
                    adjustStyle = true;
                }
            }
        }

        else if (visualCategory == GeoDataPlacemark::PisteDownhill)
        {
            return adjustPisteStyle(parameters, style);
        }

        if (adjustStyle)
        {
            GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));
            newStyle->setPolyStyle(polyStyle);
            newStyle->setLineStyle(lineStyle);
            style = newStyle;
        }

        if (style->iconStyle().iconPath().isEmpty())
        {
            const GeoDataPlacemark::GeoDataVisualCategory category = determineVisualCategory(osmData);
            const GeoDataStyle::ConstPtr categoryStyle = presetStyle(category);

            if (category != GeoDataPlacemark::None && !categoryStyle->iconStyle().scaledIcon().isNull())
            {
                GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));
                newStyle->setIconStyle(categoryStyle->iconStyle());
                style = newStyle;
            }
        }
    }

    else if (geodata_cast<GeoDataLineString>(placemark->geometry()))
    {
        GeoDataPolyStyle polyStyle = style->polyStyle();
        GeoDataLineStyle lineStyle = style->lineStyle();
        GeoDataLabelStyle labelStyle = style->labelStyle();
        GeoDataIconStyle iconStyle = style->iconStyle();
        lineStyle.setCosmeticOutline(true);

        bool adjustStyle = false;

        if (visualCategory == GeoDataPlacemark::AdminLevel2)
        {
            if (osmData.containsTag(QStringLiteral("maritime"), QStringLiteral("yes")))
            {
                lineStyle.setColor(effectColor("#88b3bf"));
                polyStyle.setColor(effectColor("#88b3bf"));

                if (osmData.containsTag(QStringLiteral("marble:disputed"), QStringLiteral("yes")))
                {
                    lineStyle.setPenStyle(Qt::DashLine);
                }

                adjustStyle = true;
            }
        }

        else if ((visualCategory >= GeoDataPlacemark::HighwayService &&
                  visualCategory <= GeoDataPlacemark::HighwayMotorway) ||
                 visualCategory == GeoDataPlacemark::TransportAirportRunway)
        {
            // Take cached Style instance if possible

            styleCacheKey = QStringLiteral("%1/%2").arg(parameters.tileLevel).arg(visualCategory);

            if (m_styleCache.contains(styleCacheKey))
            {
                style = m_styleCache[styleCacheKey];
                return style;
            }

            adjustStyle = true;
            styleCacheKey = QStringLiteral("%1/%2").arg(parameters.tileLevel).arg(visualCategory);
            adjustWayWidth(parameters, lineStyle);

            QString const accessValue = osmData.tagValue(QStringLiteral("access"));

            if (accessValue == QLatin1String("private") ||
                accessValue == QLatin1String("no") ||
                accessValue == QLatin1String("agricultural") ||
                accessValue == QLatin1String("delivery") ||
                accessValue == QLatin1String("forestry"))
            {
                QColor polyColor = polyStyle.color();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                float hue, sat, val;

#else

                qreal hue, sat, val;

#endif
                polyColor.getHsvF(&hue, &sat, &val);
                polyColor.setHsvF(0.98, qMin(1.0, 0.2 + sat), val);
                polyStyle.setColor(effectColor(polyColor));
                lineStyle.setColor(effectColor(lineStyle.color().darker(150)));
            }

            if (osmData.containsTag(QString::fromUtf8("tunnel"), QString::fromUtf8("yes")))
            {
                QColor polyColor = polyStyle.color();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                float hue, sat, val;

#else

                qreal hue, sat, val;

#endif
                polyColor.getHsvF(&hue, &sat, &val);
                polyColor.setHsvF(hue, 0.25 * sat, 0.95 * val);
                polyStyle.setColor(effectColor(polyColor));
                lineStyle.setColor(effectColor(lineStyle.color().lighter(115)));
            }

        }

        else if (visualCategory >= GeoDataPlacemark::WaterwayCanal && visualCategory <= GeoDataPlacemark::WaterwayStream)
        {

            adjustStyle = true;

            // Take cached Style instance if possible
            styleCacheKey = QStringLiteral("%1/%2").arg(parameters.tileLevel).arg(visualCategory);

            if (m_styleCache.contains(styleCacheKey))
            {
                style = m_styleCache[styleCacheKey];
                return style;
            }


            if (parameters.tileLevel <= 3)
            {
                lineStyle.setWidth(1);
                lineStyle.setPhysicalWidth(0.0);
                styleCacheKey = QStringLiteral("%1/%2").arg(parameters.tileLevel).arg(visualCategory);
            }

            else if (parameters.tileLevel <= 7)
            {
                lineStyle.setWidth(2);
                lineStyle.setPhysicalWidth(0.0);
                styleCacheKey = QStringLiteral("%1/%2").arg(parameters.tileLevel).arg(visualCategory);
            }

            else
            {
                QString const widthValue = osmData.tagValue(QStringLiteral("width")).remove(QStringLiteral(" meters")).remove(QStringLiteral(" m"));
                bool ok;
                float const width = widthValue.toFloat(&ok);
                lineStyle.setPhysicalWidth(ok ? qBound(0.1f, width, 200.0f) : 0.0f);
            }
        }

        else if (visualCategory == GeoDataPlacemark::PisteDownhill)
        {
            return adjustPisteStyle(parameters, style);
        }

        if (adjustStyle)
        {
            GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));
            newStyle->setPolyStyle(polyStyle);
            newStyle->setLineStyle(lineStyle);
            newStyle->setLabelStyle(labelStyle);
            newStyle->setIconStyle(iconStyle);
            style = newStyle;

            if (!styleCacheKey.isEmpty())
            {
                m_styleCache.insert(styleCacheKey, newStyle);
            }
        }

    }

    else if (geodata_cast<GeoDataPolygon>(placemark->geometry()))
    {
        GeoDataPolyStyle polyStyle = style->polyStyle();
        GeoDataLineStyle lineStyle = style->lineStyle();
        bool adjustStyle = false;

        if (visualCategory == GeoDataPlacemark::Bathymetry)
        {
            auto tagIter = osmData.findTag(QStringLiteral("ele"));

            if (tagIter != osmData.tagsEnd())
            {
                const QString& elevation = tagIter.value();

                if (elevation == QLatin1String("4000"))
                {
                    polyStyle.setColor(effectColor("#a5c9c9"));
                    lineStyle.setColor(effectColor("#a5c9c9"));
                    adjustStyle = true;
                }
            }
        }

        else if (visualCategory == GeoDataPlacemark::PisteDownhill)
        {
            return adjustPisteStyle(parameters, style);
        }

        if (adjustStyle)
        {
            GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));
            newStyle->setPolyStyle(polyStyle);
            newStyle->setLineStyle(lineStyle);
            style = newStyle;
        }

    }

    return style;
}

GeoDataStyle::ConstPtr StyleBuilder::Private::adjustPisteStyle(const StyleParameters& parameters,
                                                               const GeoDataStyle::ConstPtr& style)
{
    // Take cached Style instance if possible

    auto const& osmData       = parameters.placemark->osmData();
    auto const visualCategory = parameters.placemark->visualCategory();
    auto const difficulty     = osmData.tagValue(QString::fromUtf8("piste:difficulty"));
    QString styleCacheKey     = QStringLiteral("piste/%1/%2").arg(visualCategory).arg(difficulty);

    if (m_styleCache.contains(styleCacheKey))
    {
        return m_styleCache[styleCacheKey];
    }

    GeoDataLineStyle lineStyle = style->lineStyle();

    auto green    = QColor("#006600");
    auto red      = QColor("#cc0000");
    auto black    = QColor("#151515");
    auto yellow   = Qt::yellow;
    auto blue     = QColor("#000099");
    auto orange   = QColor(255, 165, 0);
    auto fallBack = Qt::lightGray;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

    auto country = QLocale::system().territory();

#else

    auto country = QLocale::system().country();

#endif

    if (country == QLocale::Japan)
    {
        if      (difficulty == QString::fromUtf8("easy"))
        {
            lineStyle.setColor(green);
        }
        else if (difficulty == QString::fromUtf8("intermediate"))
        {
            lineStyle.setColor(red);
        }
        else if (difficulty == QString::fromUtf8("advanced"))
        {
            lineStyle.setColor(black);
        }
        else
        {
            lineStyle.setColor(fallBack);
        }
    }
    else if (country == QLocale::UnitedStates ||
             country == QLocale::UnitedStatesMinorOutlyingIslands ||
             country == QLocale::Canada ||
             m_oceanianCountries.contains(country))
    {
        if      (difficulty == QString::fromUtf8("easy"))
        {
            lineStyle.setColor(green);
        }
        else if (difficulty == QString::fromUtf8("intermediate"))
        {
            lineStyle.setColor(blue);
        }
        else if (difficulty == QString::fromUtf8("advanced") || difficulty == QString::fromUtf8("expert"))
        {
            lineStyle.setColor(black);
        }
        else
        {
            lineStyle.setColor(fallBack);
        }

        // fallback on Europe
    }
    else
    {
        if (difficulty == QString::fromUtf8("novice"))
        {
            lineStyle.setColor(green);
        }

        else if (difficulty == QString::fromUtf8("easy"))
        {
            lineStyle.setColor(blue);
        }

        else if (difficulty == QString::fromUtf8("intermediate"))
        {
            lineStyle.setColor(red);
        }
        else if (difficulty == QString::fromUtf8("advanced"))
        {
            lineStyle.setColor(black);
        }
        else if (difficulty == QString::fromUtf8("expert"))
        {
            // scandinavian countries have different colors then the rest of Europe
            if (country == QLocale::Denmark ||
                country == QLocale::Norway ||
                country == QLocale::Sweden)
            {
                lineStyle.setColor(black);
            }

            else
            {
                lineStyle.setColor(orange);
            }
        }
        else if (difficulty == QString::fromUtf8("freeride"))
        {
            lineStyle.setColor(yellow);
        }
        else
        {
            lineStyle.setColor(fallBack);
        }
    }

    GeoDataPolyStyle polyStyle = style->polyStyle();
    polyStyle.setColor(effectColor(lineStyle.color()));
    GeoDataStyle::Ptr newStyle(new GeoDataStyle(*style));
    newStyle->setPolyStyle(polyStyle);
    newStyle->setLineStyle(lineStyle);
    m_styleCache.insert(styleCacheKey, newStyle);

    return newStyle;
}

void StyleBuilder::Private::adjustWayWidth(const StyleParameters& parameters, GeoDataLineStyle& lineStyle)
{
    auto const& osmData = parameters.placemark->osmData();
    auto const visualCategory = parameters.placemark->visualCategory();

    if (parameters.tileLevel <= 8)
    {
        lineStyle.setPhysicalWidth(0.0);
        lineStyle.setWidth(2.0);
    }

    else if (parameters.tileLevel <= 10)
    {
        lineStyle.setPhysicalWidth(0.0);
        lineStyle.setWidth(3.0);
    }

    else if (parameters.tileLevel <= 12)
    {
        lineStyle.setPhysicalWidth(0.0);
        lineStyle.setWidth(4.0);
    }

    else
    {
        auto tagIter = osmData.findTag(QStringLiteral("width"));

        if (tagIter != osmData.tagsEnd())
        {
            QString const widthValue = QString(tagIter.value()).remove(QStringLiteral(" meters")).remove(QStringLiteral(" m"));
            bool ok;
            float const width = widthValue.toFloat(&ok);
            lineStyle.setPhysicalWidth(ok ? qBound(0.1f, width, 200.0f) : 0.0f);
        }

        else
        {
            bool const isOneWay = osmData.containsTag(QStringLiteral("oneway"), QStringLiteral("yes")) ||
                                  osmData.containsTag(QStringLiteral("oneway"), QStringLiteral("-1"));
            int const lanes = isOneWay ? 1 : 2; // also for motorway which implicitly is one way, but has two lanes and each direction has its own highway
            double const laneWidth = 3.0;
            double const margins = visualCategory == GeoDataPlacemark::HighwayMotorway ? 2.0 : (isOneWay ? 1.0 : 0.0);
            double const physicalWidth = margins + lanes * laneWidth;
            lineStyle.setPhysicalWidth(physicalWidth);
        }
    }
}

GeoDataStyle::Ptr StyleBuilder::Private::createStyle(qreal width, qreal realWidth, const QColor& color,
                                                     const QColor& outlineColor, bool fill, bool outline, Qt::BrushStyle brushStyle, Qt::PenStyle penStyle,
                                                     Qt::PenCapStyle capStyle, bool lineBackground, const QVector< qreal >& dashPattern,
                                                     const QFont& font, const QColor& fontColor, const QString& texturePath)
{
    GeoDataStyle* const style = new GeoDataStyle;
    GeoDataLineStyle lineStyle(effectColor(outlineColor));
    lineStyle.setCapStyle(capStyle);
    lineStyle.setPenStyle(penStyle);
    lineStyle.setWidth(width);
    lineStyle.setPhysicalWidth(realWidth);
    lineStyle.setBackground(lineBackground);
    lineStyle.setDashPattern(dashPattern);
    GeoDataPolyStyle polyStyle(effectColor(color));
    polyStyle.setOutline(outline);
    polyStyle.setFill(fill);
    polyStyle.setBrushStyle(brushStyle);
    polyStyle.setTexturePath(texturePath);
    GeoDataLabelStyle labelStyle(font, effectColor(fontColor));
    style->setLineStyle(lineStyle);
    style->setPolyStyle(polyStyle);
    style->setLabelStyle(labelStyle);

    return GeoDataStyle::Ptr(style);
}

void StyleBuilder::Private::initializeDefaultStyles()
{
    // We need to do this similar to the way KCmdLineOptions works in
    // the future: Having a PlacemarkStyleProperty properties[] would
    // help here greatly.

    if (m_defaultStyleInitialized)
    {
        return;
    }

    m_defaultStyleInitialized = true;
    QString defaultFamily     = m_defaultFont.family();

#ifdef Q_OS_MACOS

    int defaultSize = 10;

#else

    int defaultSize = 8;

#endif

    QColor const defaultLabelColor = effectColor(m_defaultLabelColor);

    m_defaultStyle[GeoDataPlacemark::None]
        = GeoDataStyle::Ptr(new GeoDataStyle(QString(),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Default]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/default_location.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Unknown]
        = GeoDataStyle::Ptr(new GeoDataStyle(QString(),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::SmallCity]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_4_white.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::SmallCountyCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_4_yellow.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::SmallStateCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_4_orange.png")),
                                             QFont(defaultFamily, defaultSize, 50, true), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::SmallNationCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_4_red.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::MediumCity]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_3_white.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::MediumCountyCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_3_yellow.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::MediumStateCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_3_orange.png")),
                                             QFont(defaultFamily, defaultSize, 50, true), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::MediumNationCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_3_red.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::BigCity]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_2_white.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::BigCountyCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_2_yellow.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::BigStateCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_2_orange.png")),
                                             QFont(defaultFamily, defaultSize, 50, true), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::BigNationCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_2_red.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::LargeCity]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_1_white.png")),
                                             QFont(defaultFamily, defaultSize, 75, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::LargeCountyCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_1_yellow.png")),
                                             QFont(defaultFamily, defaultSize, 75, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::LargeStateCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_1_orange.png")),
                                             QFont(defaultFamily, defaultSize, 75, true), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::LargeNationCapital]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/city_1_red.png")),
                                             QFont(defaultFamily, defaultSize, 75, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Nation]
        = GeoDataStyle::Ptr(new GeoDataStyle(QString(),
                                             QFont(defaultFamily, int(defaultSize * 1.5), 75, false), QColor("#404040")));
    // Align area labels centered

    m_defaultStyle[GeoDataPlacemark::Nation]->labelStyle().setAlignment(GeoDataLabelStyle::Center);

    QFont osmCityFont                                             = QFont(defaultFamily, int(defaultSize * 1.5), 75, false);
    m_defaultStyle[GeoDataPlacemark::PlaceCity]                   = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-6"), QColor("#202020"));
    m_defaultStyle[GeoDataPlacemark::PlaceCityCapital]            = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-capital-6"), QColor("#202020"));
    m_defaultStyle[GeoDataPlacemark::PlaceCityNationalCapital]    = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-capital-adminlevel2"), QColor("#202020"));
    m_defaultStyle[GeoDataPlacemark::PlaceSuburb]                 = createOsmPOIStyle(osmCityFont, QString(), QColor("#707070"));
    m_defaultStyle[GeoDataPlacemark::PlaceHamlet]                 = createOsmPOIStyle(osmCityFont, QString(), QColor("#707070"));
    QFont localityFont                                            = osmCityFont;
    localityFont.setPointSize(defaultSize);
    m_defaultStyle[GeoDataPlacemark::PlaceLocality]               = createOsmPOIStyle(localityFont, QString(), QColor("#707070"));
    m_defaultStyle[GeoDataPlacemark::PlaceTown]                   = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-6"), QColor("#404040"));
    m_defaultStyle[GeoDataPlacemark::PlaceTownCapital]            = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-capital-6"), QColor("#404040"));
    m_defaultStyle[GeoDataPlacemark::PlaceTownNationalCapital]    = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-capital-adminlevel2"), QColor("#404040"));
    m_defaultStyle[GeoDataPlacemark::PlaceVillage]                = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-6"), QColor("#505050"));
    m_defaultStyle[GeoDataPlacemark::PlaceVillageCapital]         = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-capital-6"), QColor("#505050"));
    m_defaultStyle[GeoDataPlacemark::PlaceVillageNationalCapital] = createOsmPOIStyle(osmCityFont, QString::fromUtf8("place/place-capital-adminlevel2"), QColor("#505050"));

    for (int i = GeoDataPlacemark::PlaceCity ; i <= GeoDataPlacemark::PlaceVillageCapital ; ++i)
    {
        m_defaultStyle[GeoDataPlacemark::GeoDataVisualCategory(i)]->polyStyle().setFill(false);
        m_defaultStyle[GeoDataPlacemark::GeoDataVisualCategory(i)]->polyStyle().setOutline(false);
        m_defaultStyle[GeoDataPlacemark::GeoDataVisualCategory(i)]->labelStyle().setAlignment(GeoDataLabelStyle::Center);

        if (
            (i == GeoDataPlacemark::PlaceCityNationalCapital)    ||
            (i == GeoDataPlacemark::PlaceTownNationalCapital)    ||
            (i == GeoDataPlacemark::PlaceVillageNationalCapital) // cppcheck-suppress knownConditionTrueFalse
           )
        {
            m_defaultStyle[GeoDataPlacemark::GeoDataVisualCategory(i)]->iconStyle().setScale(0.55f);
        }
        else
        {
            m_defaultStyle[GeoDataPlacemark::GeoDataVisualCategory(i)]->iconStyle().setScale(0.25);
        }
    }

    m_defaultStyle[GeoDataPlacemark::Mountain]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/mountain_1.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.9), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Volcano]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/volcano_1.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.9), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Mons]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/mountain_1.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.9), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Valley]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/valley.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.9), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Continent]
        = GeoDataStyle::Ptr(new GeoDataStyle(QString(),
                                             QFont(defaultFamily, int(defaultSize * 1.7), 50, false), QColor("#bf0303")));
    // Align area labels centered

    m_defaultStyle[GeoDataPlacemark::Continent]->labelStyle().setAlignment(GeoDataLabelStyle::Center);

    m_defaultStyle[GeoDataPlacemark::Ocean]
        = GeoDataStyle::Ptr(new GeoDataStyle(QString(),
                                             QFont(defaultFamily, int(defaultSize * 1.7), 50, true), QColor("#2c72c7")));
    // Align area labels centered

    m_defaultStyle[GeoDataPlacemark::Ocean]->labelStyle().setAlignment(GeoDataLabelStyle::Center);

    m_defaultStyle[GeoDataPlacemark::OtherTerrain]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/other.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.9), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Crater]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/crater.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.9), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Mare]
        = GeoDataStyle::Ptr(new GeoDataStyle(QString(),
                                             QFont(defaultFamily, int(defaultSize * 1.7), 50, false), QColor("#bf0303")));
    // Align area labels centered

    m_defaultStyle[GeoDataPlacemark::Mare]->labelStyle().setAlignment(GeoDataLabelStyle::Center);

    m_defaultStyle[GeoDataPlacemark::GeographicPole]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/pole_1.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::MagneticPole]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/pole_2.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::ShipWreck]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/shipwreck.png")),
                                             QFont(defaultFamily, int(defaultSize * 0.8), 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::AirPort]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/airport.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Observatory]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/observatory.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::OsmSite]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/osm.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::Coordinate]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/coordinate.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));


    m_defaultStyle[GeoDataPlacemark::MannedLandingSite]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/manned_landing.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::RoboticRover]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/robotic_rover.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::UnmannedSoftLandingSite]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/unmanned_soft_landing.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    m_defaultStyle[GeoDataPlacemark::UnmannedHardLandingSite]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/unmanned_hard_landing.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    QColor const shopColor("#ac39ac");
    QColor const transportationColor("#0092da");
    QColor const amenityColor("#734a08");
    QColor const healthColor("#da0092");
    QColor const airTransportColor("#8461C4");
    QColor const educationalAreasAndHospital("#f0f0d8");
    QColor const buildingColor("#beadad");
    QColor const waterColor("#b5d0d0");

    // Allows to visualize multiple repaints of buildings
    // QColor const buildingColor(0, 255, 0, 64);

    QFont const osmFont(defaultFamily, 10, 50, false);
    m_defaultStyle[GeoDataPlacemark::AccomodationCamping]        = createOsmPOIRingStyle(osmFont, QString::fromUtf8("transportation/camping.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AccomodationHostel]         = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/hostel.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AccomodationHotel]          = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/hotel.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AccomodationMotel]          = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/motel.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AccomodationYouthHostel]    = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/hostel.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AccomodationGuestHouse]     = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/guest_house.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AmenityLibrary]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/library.20"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityKindergarten]        = createOsmPOIAreaStyle(osmFont, QString(), amenityColor, educationalAreasAndHospital, amenityColor);
    m_defaultStyle[GeoDataPlacemark::EducationCollege]           = createOsmPOIAreaStyle(osmFont, QString(), amenityColor, educationalAreasAndHospital, amenityColor);
    m_defaultStyle[GeoDataPlacemark::EducationSchool]            = createOsmPOIAreaStyle(osmFont, QString(), amenityColor, educationalAreasAndHospital, amenityColor);
    m_defaultStyle[GeoDataPlacemark::EducationUniversity]        = createOsmPOIAreaStyle(osmFont, QString(), amenityColor, educationalAreasAndHospital, amenityColor);
    m_defaultStyle[GeoDataPlacemark::FoodBar]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/bar.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::FoodBiergarten]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/biergarten.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::FoodCafe]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/cafe.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::FoodFastFood]               = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/fast_food.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::FoodPub]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/pub.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::FoodRestaurant]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/restaurant.16"), amenityColor);

    m_defaultStyle[GeoDataPlacemark::HealthDentist]              = createOsmPOIStyle(osmFont, QString::fromUtf8("health/dentist.16"), healthColor);
    m_defaultStyle[GeoDataPlacemark::HealthDoctors]              = createOsmPOIStyle(osmFont, QString::fromUtf8("health/doctors.16"), healthColor);
    m_defaultStyle[GeoDataPlacemark::HealthHospital]             = createOsmPOIStyle(osmFont, QString::fromUtf8("health/hospital.16"), healthColor, educationalAreasAndHospital, amenityColor);
    m_defaultStyle[GeoDataPlacemark::HealthPharmacy]             = createOsmPOIStyle(osmFont, QString::fromUtf8("health/pharmacy.16"), healthColor);
    m_defaultStyle[GeoDataPlacemark::HealthVeterinary]           = createOsmPOIStyle(osmFont, QString::fromUtf8("health/veterinary-14"), healthColor);

    m_defaultStyle[GeoDataPlacemark::MoneyAtm]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/atm.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::MoneyBank]                  = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/bank.16"), amenityColor);

    m_defaultStyle[GeoDataPlacemark::HistoricArchaeologicalSite] = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("amenity/archaeological_site.16"), amenityColor, Qt::transparent);
    m_defaultStyle[GeoDataPlacemark::AmenityEmbassy]             = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/embassy.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AmenityEmergencyPhone]      = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/emergency_phone.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityMountainRescue]      = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/mountain_rescue.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::LeisureWaterPark]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/water_park.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityCommunityCentre]     = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/community_centre-14"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityFountain]            = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/fountain-14"), QColor("#7989de"), waterColor, waterColor.darker(150));
    m_defaultStyle[GeoDataPlacemark::AmenityNightClub]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/nightclub.18"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityBench]               = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/bench.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityCourtHouse]          = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/courthouse-16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityFireStation]         = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/firestation.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityHuntingStand]        = createOsmPOIStyle(osmFont, QString::fromUtf8("manmade/hunting-stand.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityPolice]              = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/police.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityPostBox]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/post_box-12"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityPostOffice]          = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/post_office-14"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityPrison]              = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/prison.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityRecycling]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/recycling.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityShelter]             = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/shelter"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AmenityTelephone]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/telephone.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityToilets]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/toilets.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityTownHall]            = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/town_hall.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityWasteBasket]         = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/waste_basket.10"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityWasteBasket]->iconStyle().setScale(0.75);
    m_defaultStyle[GeoDataPlacemark::AmenityChargingStation]     = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/charging_station"));
    m_defaultStyle[GeoDataPlacemark::AmenityCarWash]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/car_wash"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenitySocialFacility]      = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/social_facility"), amenityColor);

    m_defaultStyle[GeoDataPlacemark::AmenityDrinkingWater]       = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/drinking_water.16"), amenityColor);

    m_defaultStyle[GeoDataPlacemark::NaturalPeak]                = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/peak"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::NaturalPeak]->iconStyle().setScale(0.33f);
    m_defaultStyle[GeoDataPlacemark::NaturalVolcano]             = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/volcano"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::NaturalVolcano]->iconStyle().setScale(0.33f);
    m_defaultStyle[GeoDataPlacemark::NaturalTree]                = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/tree-29"), amenityColor); // tree-16 provides the official icon
    m_styleTreeAutumn                                            = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/tree-29-autumn"), amenityColor);
    m_styleTreeWinter                                            = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/tree-29-winter"), amenityColor);
    qreal const treeIconScale                                    = 0.75;
    m_defaultStyle[GeoDataPlacemark::NaturalTree]->iconStyle().setScale(treeIconScale);
    m_styleTreeAutumn->iconStyle().setScale(treeIconScale);
    m_styleTreeWinter->iconStyle().setScale(treeIconScale);

    m_defaultStyle[GeoDataPlacemark::ShopBeverages]              = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/beverages-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopHifi]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/hifi-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopSupermarket]            = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_supermarket"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopAlcohol]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_alcohol.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopBakery]                 = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_bakery.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopButcher]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/butcher-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopConfectionery]          = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/confectionery.14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopConvenience]            = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_convenience"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopGreengrocer]            = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/greengrocer-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopSeafood]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/seafood-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopDepartmentStore]        = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/department_store-16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopKiosk]                  = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/kiosk-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopBag]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/bag-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopClothes]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_clothes.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopFashion]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_clothes.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopJewelry]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_jewelry.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopShoes]                  = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_shoes.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopVarietyStore]           = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/variety_store-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopBeauty]                 = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/beauty-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopChemist]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/chemist-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopCosmetics]              = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/perfumery-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopHairdresser]            = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_hairdresser.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopOptician]               = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_optician.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopPerfumery]              = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/perfumery-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopDoitYourself]           = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_diy.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopFlorist]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/florist.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopHardware]               = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_diy.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopFurniture]              = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_furniture.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopElectronics]            = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_electronics.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopMobilePhone]            = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_mobile_phone.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopBicycle]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_bicycle.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopCar]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_car"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopCarRepair]              = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shopping_car_repair.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopCarParts]               = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/car_parts-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopMotorcycle]             = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/motorcycle-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopOutdoor]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/outdoor-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopSports]                 = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/sports"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopCopy]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/copyshop"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopArt]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/art"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopMusicalInstrument]      = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/musical_instrument-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopPhoto]                  = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/photo-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopBook]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_books.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopGift]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_gift.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopStationery]             = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/stationery-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopLaundry]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/laundry-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopPet]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop_pet.16"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopToys]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/toys-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopTravelAgency]           = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/travel_agency-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopDeli]                   = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/deli"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopTobacco]                = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/tobacco"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopTea]                    = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/tea"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopComputer]               = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/computer-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::ShopGardenCentre]           = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/garden_centre-14"), shopColor);
    m_defaultStyle[GeoDataPlacemark::Shop]                       = createOsmPOIStyle(osmFont, QString::fromUtf8("shop/shop-14"), shopColor);

    m_defaultStyle[GeoDataPlacemark::ManmadeBridge]              = createWayStyle(QColor("#b8b8b8"), Qt::transparent, true, true);
    m_defaultStyle[GeoDataPlacemark::ManmadeLighthouse]          = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/lighthouse.16"), transportationColor, "#f2efe9", QColor("#f2efe9").darker());
    m_defaultStyle[GeoDataPlacemark::ManmadePier]                = createStyle(0.0, 3.0, "#f2efe9", "#f2efe9", true, false, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont);
    m_defaultStyle[GeoDataPlacemark::ManmadeWaterTower]          = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/water_tower.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::ManmadeWindMill]            = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/windmill.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::ManmadeCommunicationsTower] = createOsmPOIStyle(osmFont, QString::fromUtf8("manmade/communications.16"), amenityColor);

    m_defaultStyle[GeoDataPlacemark::HistoricCastle]             = createOsmPOIRingStyle(osmFont, QString::fromUtf8("amenity/cinema.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::HistoricMemorial]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/tourist_memorial.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::HistoricMonument]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/monument.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::HistoricRuins]              = createOsmPOIRingStyle(osmFont, QString(), amenityColor);

    m_defaultStyle[GeoDataPlacemark::TourismAttraction]          = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/tourist_memorial.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::TourismArtwork]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/artwork"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityCinema]              = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/cinema.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::TourismInformation]         = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/information.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::TourismMuseum]              = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/museum.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::AmenityTheatre]             = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/theatre.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::TourismThemePark]           = createOsmPOIStyle(osmFont, QString(), amenityColor);
    m_defaultStyle[GeoDataPlacemark::TourismViewPoint]           = createOsmPOIStyle(osmFont, QString::fromUtf8("amenity/viewpoint.16"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::TourismZoo]                 = createOsmPOIRingStyle(osmFont, QString(), amenityColor, Qt::transparent);
    m_defaultStyle[GeoDataPlacemark::TourismAlpineHut]           = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/alpinehut.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TourismWildernessHut]       = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/wilderness_hut"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportAerodrome]         = createOsmPOIStyle(osmFont, QString::fromUtf8("airtransport/aerodrome"), airTransportColor);
    m_defaultStyle[GeoDataPlacemark::TransportHelipad]           = createOsmPOIStyle(osmFont, QString::fromUtf8("airtransport/helipad"), airTransportColor);
    m_defaultStyle[GeoDataPlacemark::TransportAirportTerminal]   = createOsmPOIAreaStyle(osmFont, QString(), airTransportColor);
    m_defaultStyle[GeoDataPlacemark::TransportAirportGate]       = createOsmPOIAreaStyle(osmFont, QString(), airTransportColor);
    m_defaultStyle[GeoDataPlacemark::TransportBusStation]        = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/bus_station.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportBusStop]           = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/bus_stop.12"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportCarShare]          = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/car_share.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportFuel]              = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/fuel.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportParking]           = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("transportation/parking"), transportationColor, "#F6EEB6", QColor("#F6EEB6").darker());
    m_defaultStyle[GeoDataPlacemark::TransportBicycleParking]    = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("transportation/bicycle_parking.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportMotorcycleParking] = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("transportation/motorcycle_parking.16"), transportationColor);
    qreal const parkingIconScale                                 = 0.75;
    m_defaultStyle[GeoDataPlacemark::TransportParking]->iconStyle().setScale(parkingIconScale);
    m_defaultStyle[GeoDataPlacemark::TransportBicycleParking]->iconStyle().setScale(parkingIconScale);
    m_defaultStyle[GeoDataPlacemark::TransportMotorcycleParking]->iconStyle().setScale(parkingIconScale);
    m_defaultStyle[GeoDataPlacemark::TransportParkingSpace]      = createWayStyle("#F6EEB6", QColor("#F6EEB6").darker(), true, true);
    m_defaultStyle[GeoDataPlacemark::TransportPlatform]          = createWayStyle("#bbbbbb", Qt::transparent, true, false);
    m_defaultStyle[GeoDataPlacemark::TransportTrainStation]      = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/railway_station"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportTrainStation]->iconStyle().setScale(0.5);
    m_defaultStyle[GeoDataPlacemark::TransportTramStop]          = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/railway_station"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportTramStop]->iconStyle().setScale(0.33f);
    m_defaultStyle[GeoDataPlacemark::TransportRentalBicycle]     = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/rental_bicycle.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportRentalCar]         = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/rental_car.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportRentalSki]         = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/rental_ski.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportTaxiRank]          = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/taxi.16"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::TransportSubwayEntrance]    = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/subway_entrance"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::ReligionPlaceOfWorship]     = createOsmPOIStyle(osmFont, QString::fromUtf8("black/place_of_worship.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionBahai]              = createOsmPOIStyle(osmFont, QString());
    m_defaultStyle[GeoDataPlacemark::ReligionBuddhist]           = createOsmPOIStyle(osmFont, QString::fromUtf8("black/buddhist.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionChristian]          = createOsmPOIStyle(osmFont, QString::fromUtf8("black/christian.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionMuslim]             = createOsmPOIStyle(osmFont, QString::fromUtf8("black/muslim.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionHindu]              = createOsmPOIStyle(osmFont, QString::fromUtf8("black/hinduist.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionJain]               = createOsmPOIStyle(osmFont, QString());
    m_defaultStyle[GeoDataPlacemark::ReligionJewish]             = createOsmPOIStyle(osmFont, QString::fromUtf8("black/jewish.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionShinto]             = createOsmPOIStyle(osmFont, QString::fromUtf8("black/shintoist.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionSikh]               = createOsmPOIStyle(osmFont, QString::fromUtf8("black/sikhist.16"));
    m_defaultStyle[GeoDataPlacemark::ReligionTaoist]             = createOsmPOIStyle(osmFont, QString::fromUtf8("black/taoist.16"));

    m_defaultStyle[GeoDataPlacemark::IndoorDoor]                 = createOsmPOIRingStyle(osmFont, QString::fromUtf8("indoor/door_b.16"));
    m_defaultStyle[GeoDataPlacemark::IndoorWall]                 = createOsmPOIStyle(osmFont, QString());
    m_defaultStyle[GeoDataPlacemark::IndoorRoom]                 = createWayStyle(buildingColor.darker(), QColor(Qt::black));

    m_defaultStyle[GeoDataPlacemark::HighwayTrafficSignals]      = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/traffic_light"));
    m_defaultStyle[GeoDataPlacemark::HighwayTrafficSignals]->iconStyle().setScale(0.75);

    m_defaultStyle[GeoDataPlacemark::HighwayElevator]            = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/elevator-12"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::HighwayElevator]->iconStyle().setScale(0.75);

    m_defaultStyle[GeoDataPlacemark::PowerTower]                 = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/power_tower"), QColor("#888888"));
    m_defaultStyle[GeoDataPlacemark::PowerTower]->iconStyle().setScale(0.6f);

    m_defaultStyle[GeoDataPlacemark::BarrierCityWall]            = createStyle(6.0, 3.0, "#787878", Qt::transparent, true, false, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, Qt::transparent);
    m_defaultStyle[GeoDataPlacemark::BarrierGate]                = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/gate"));
    m_defaultStyle[GeoDataPlacemark::BarrierGate]->iconStyle().setScale(0.75);
    m_defaultStyle[GeoDataPlacemark::BarrierLiftGate]            = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/liftgate"));
    m_defaultStyle[GeoDataPlacemark::BarrierLiftGate]->iconStyle().setScale(0.75);
    m_defaultStyle[GeoDataPlacemark::BarrierWall]                = createStyle(2.0, 0.0, "#787878", Qt::transparent, true, false, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, Qt::transparent);

    m_defaultStyle[GeoDataPlacemark::HighwaySteps]               = createStyle(0.0, 2.0, "#fa8072", QColor(Qt::white), true, true, Qt::SolidPattern, Qt::CustomDashLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::HighwayUnknown]             = createHighwayStyle("#dddddd", "#bbbbbb", osmFont, "000000", 0, 1);
    m_defaultStyle[GeoDataPlacemark::HighwayPath]                = createHighwayStyle("#dddde8", "#999999", osmFont, "000000", 0, 1, Qt::DotLine, Qt::SquareCap, true);
    m_defaultStyle[GeoDataPlacemark::HighwayTrack]               = createHighwayStyle("#996600", QColor(Qt::white), osmFont, QColor(Qt::transparent), 1, 1, Qt::DashLine, Qt::SquareCap, true);
    m_defaultStyle[GeoDataPlacemark::HighwayPedestrian]          = createHighwayStyle("#dddde8", "#999999", osmFont, "000000", 0, 2);
    m_defaultStyle[GeoDataPlacemark::HighwayPedestrian]->polyStyle().setOutline(false);
    m_defaultStyle[GeoDataPlacemark::HighwayFootway]             = createHighwayStyle("#fa8072", QColor(Qt::white), osmFont, "000000", 0, 2, Qt::DotLine, Qt::SquareCap, true);
    m_defaultStyle[GeoDataPlacemark::HighwayCycleway]            = createHighwayStyle(QColor(Qt::blue), QColor(Qt::white), osmFont, "000000", 0, 2, Qt::DotLine, Qt::SquareCap, true);
    m_defaultStyle[GeoDataPlacemark::HighwayService]             = createHighwayStyle("#ffffff", "#bbbbbb", osmFont, "000000", 1, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayResidential]         = createHighwayStyle("#ffffff", "#bbbbbb", osmFont, "000000", 3, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayLivingStreet]        = createHighwayStyle("#ffffff", "#bbbbbb", osmFont, "000000", 3, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayRoad]                = createHighwayStyle("#dddddd", "#bbbbbb", osmFont, "000000", 3, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayUnclassified]        = createHighwayStyle("#ffffff", "#bbbbbb", osmFont, "000000", 3, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayTertiary]            = createHighwayStyle("#ffffff", "#8f8f8f", osmFont, "000000", 6, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayTertiaryLink]        = createHighwayStyle("#ffffff", "#8f8f8f", osmFont, "000000", 6, 6);
    m_defaultStyle[GeoDataPlacemark::HighwaySecondary]           = createHighwayStyle("#f7fabf", "#707d05", osmFont, "000000", 6, 6);
    m_defaultStyle[GeoDataPlacemark::HighwaySecondaryLink]       = createHighwayStyle("#f7fabf", "#707d05", osmFont, "000000", 6, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayPrimary]             = createHighwayStyle("#fcd6a4", "#a06b00", osmFont, "000000", 9, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayPrimaryLink]         = createHighwayStyle("#fcd6a4", "#a06b00", osmFont, "000000", 6, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayRaceway]             = createHighwayStyle("#ffc0cb", "#ffc0cb", osmFont, "000000", 1, 5);
    m_defaultStyle[GeoDataPlacemark::HighwayTrunk]               = createHighwayStyle("#f9b29c", "#c84e2f", osmFont, "000000", 9, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayTrunkLink]           = createHighwayStyle("#f9b29c", "#c84e2f", osmFont, "000000", 9, 6);
    m_defaultStyle[GeoDataPlacemark::HighwayMotorway]            = createHighwayStyle("#e892a2", "#dc2a67", osmFont, "000000", 9, 10);
    m_defaultStyle[GeoDataPlacemark::HighwayMotorwayLink]        = createHighwayStyle("#e892a2", "#dc2a67", osmFont, "000000", 9, 10);
    m_defaultStyle[GeoDataPlacemark::HighwayCorridor]            = createHighwayStyle("#ffffff", "#bbbbbb", osmFont, "000000", 1, 3);
    m_defaultStyle[GeoDataPlacemark::TransportAirportRunway]     = createHighwayStyle("#bbbbcc", "#bbbbcc", osmFont, "000000", 0, 1, Qt::NoPen);
    m_defaultStyle[GeoDataPlacemark::TransportAirportTaxiway]    = createHighwayStyle("#bbbbcc", "#bbbbcc", osmFont, "000000", 0, 1, Qt::NoPen);
    m_defaultStyle[GeoDataPlacemark::TransportAirportApron]      = createWayStyle("#e9d1ff", Qt::transparent, true, false);
    m_defaultStyle[GeoDataPlacemark::TransportSpeedCamera]       = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/speedcamera"));

    m_defaultStyle[GeoDataPlacemark::NaturalWater]               = createStyle(4, 0, waterColor, waterColor, true, false,
                                                                               Qt::SolidPattern, Qt::SolidLine, Qt::RoundCap, false, QVector< qreal >(),
                                                                               osmFont, waterColor.darker(150));
    m_defaultStyle[GeoDataPlacemark::WaterwayRiver]              = createStyle(4, 0, waterColor, waterColor, true, false,
                                                                               Qt::SolidPattern, Qt::SolidLine, Qt::RoundCap, false, QVector< qreal >(),
                                                                               osmFont, waterColor.darker(150));
    m_defaultStyle[GeoDataPlacemark::WaterwayCanal]              = m_defaultStyle[GeoDataPlacemark::WaterwayRiver];
    m_defaultStyle[GeoDataPlacemark::WaterwayDitch]              = m_defaultStyle[GeoDataPlacemark::WaterwayRiver];
    m_defaultStyle[GeoDataPlacemark::WaterwayDrain]              = m_defaultStyle[GeoDataPlacemark::WaterwayRiver];
    m_defaultStyle[GeoDataPlacemark::WaterwayStream]             = m_defaultStyle[GeoDataPlacemark::WaterwayRiver];
    m_defaultStyle[GeoDataPlacemark::WaterwayWeir]               = createStyle(4, 0, "#ffffff", "#87939b", true, false,
                                                                              Qt::SolidPattern, Qt::DotLine, Qt::RoundCap, true, QVector< qreal >(),
                                                                              osmFont, waterColor.darker(150));

    m_defaultStyle[GeoDataPlacemark::CrossingIsland]             = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/zebra_crossing"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::CrossingIsland]->iconStyle().setScale(0.75);
    m_defaultStyle[GeoDataPlacemark::CrossingRailway]            = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/railway_crossing"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::CrossingRailway]->iconStyle().setScale(0.5);
    m_defaultStyle[GeoDataPlacemark::CrossingSignals]            = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/traffic_light_crossing"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::CrossingSignals]->iconStyle().setScale(0.75);
    m_defaultStyle[GeoDataPlacemark::CrossingZebra]              = createOsmPOIStyle(osmFont, QString::fromUtf8("transportation/zebra_crossing"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::CrossingZebra]->iconStyle().setScale(0.75);

    m_defaultStyle[GeoDataPlacemark::NaturalReef]                = createStyle(5.5, 0, "#36677c", "#36677c", true, false,
                                                                               Qt::Dense7Pattern, Qt::DotLine, Qt::RoundCap, false, QVector< qreal >(),
                                                                               osmFont, waterColor.darker(150));

    m_defaultStyle[GeoDataPlacemark::AmenityGraveyard]           = createWayStyle("#AACBAF", "#AACBAF", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/grave_yard_generic.png")));

    m_defaultStyle[GeoDataPlacemark::NaturalWood]                = createWayStyle("#8DC46C", "#8DC46C", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/forest.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalBeach]               = createWayStyle("#FFF1BA", "#FFF1BA", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/beach.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalWetland]             = createWayStyle("#DDECEC", "#DDECEC", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/wetland.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalGlacier]             = createWayStyle("#DDECEC", "#DDECEC", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/glacier.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalIceShelf]            = createWayStyle("#8ebebe", "#8ebebe", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/glacier.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalScrub]               = createWayStyle("#B5E3B5", "#B5E3B5", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/scrub.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalCliff]               = createWayStyle(Qt::transparent, Qt::transparent, true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/cliff2.png")));
    m_defaultStyle[GeoDataPlacemark::NaturalCave]                = createOsmPOIStyle(osmFont, QString::fromUtf8("amenities/cave"), amenityColor);
    m_defaultStyle[GeoDataPlacemark::NaturalHeath]               = createWayStyle("#d6d99f", QColor("#d6d99f").darker(150), true, false);

    m_defaultStyle[GeoDataPlacemark::LeisureGolfCourse]          = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("leisure/golf"), "#39ac39", "#b5e3b5", QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::LeisureMinigolfCourse]      = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("leisure/miniature_golf"), "#39ac39", "#b5e3b5", QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::LeisureMarina]              = createOsmPOIStyle(osmFont, QString(), QColor("#95abd5"), QColor("#aec8d1"), QColor("#95abd5").darker(150));
    m_defaultStyle[GeoDataPlacemark::LeisurePark]                = createWayStyle(QColor("#c8facc"), QColor("#c8facc").darker(150), true, true);
    m_defaultStyle[GeoDataPlacemark::LeisurePlayground]          = createOsmPOIAreaStyle(osmFont, QString::fromUtf8("amenity/playground.16"), amenityColor, "#CCFFF1", "#BDFFED");
    m_defaultStyle[GeoDataPlacemark::LeisurePitch]               = createWayStyle("#8ad3af", QColor("#8ad3af").darker(150), true, true);
    m_defaultStyle[GeoDataPlacemark::LeisureSportsCentre]        = createWayStyle("#33cc99", QColor("#33cc99").darker(150), true, true);
    m_defaultStyle[GeoDataPlacemark::LeisureStadium]             = createWayStyle("#33cc99", QColor("#33cc99").darker(150), true, true);
    m_defaultStyle[GeoDataPlacemark::LeisureTrack]               = createWayStyle("#74dcba", QColor("#74dcba").darker(150), true, true);
    m_defaultStyle[GeoDataPlacemark::LeisureSwimmingPool]        = createWayStyle(waterColor, waterColor.darker(150), true, true);

    m_defaultStyle[GeoDataPlacemark::LanduseAllotments]          = createWayStyle("#E4C6AA", "#E4C6AA", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/allotments.png")));
    m_defaultStyle[GeoDataPlacemark::LanduseBasin]               = createWayStyle(QColor(0xB5, 0xD0, 0xD0, 0x80), QColor(0xB5, 0xD0, 0xD0));
    m_defaultStyle[GeoDataPlacemark::LanduseCemetery]            = createWayStyle("#AACBAF", "#AACBAF", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/grave_yard_generic.png")));
    m_defaultStyle[GeoDataPlacemark::LanduseCommercial]          = createWayStyle("#F2DAD9", "#D1B2B0", true, true);
    m_defaultStyle[GeoDataPlacemark::LanduseConstruction]        = createWayStyle("#b6b592", "#b6b592", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseFarmland]            = createWayStyle("#EDDDC9", "#C8B69E", true, true);
    m_defaultStyle[GeoDataPlacemark::LanduseFarmyard]            = createWayStyle("#EFD6B5", "#D1B48C", true, true);
    m_defaultStyle[GeoDataPlacemark::LanduseGarages]             = createWayStyle("#E0DDCD", "#E0DDCD", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseGrass]               = createWayStyle("#A8C8A5", "#A8C8A5", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseIndustrial]          = createWayStyle("#DED0D5", "#DED0D5", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseLandfill]            = createWayStyle("#b6b592", "#b6b592", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseMeadow]              = createWayStyle("#cdebb0", "#cdebb0", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseMilitary]            = createWayStyle("#F3D8D2", "#F3D8D2", true, true, Qt::BDiagPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/military_red_hatch.png")));
    m_defaultStyle[GeoDataPlacemark::LanduseQuarry]              = createWayStyle("#C4C2C2", "#C4C2C2", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/quarry.png")));
    m_defaultStyle[GeoDataPlacemark::LanduseRailway]             = createWayStyle("#DED0D5", "#DED0D5", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseReservoir]           = createWayStyle(waterColor, waterColor, true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseResidential]         = createWayStyle("#DCDCDC", "#DCDCDC", true, false);
    m_defaultStyle[GeoDataPlacemark::LanduseRetail]              = createWayStyle("#FFD6D1", "#D99C95", true, true);
    m_defaultStyle[GeoDataPlacemark::LanduseOrchard]             = createWayStyle("#AEDFA3", "#AEDFA3", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/orchard.png")));
    m_defaultStyle[GeoDataPlacemark::LanduseVineyard]            = createWayStyle("#AEDFA3", "#AEDFA3", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/vineyard.png")));

    m_defaultStyle[GeoDataPlacemark::MilitaryDangerArea]         = createWayStyle("#FFC0CB", "#FFC0CB", true, false, Qt::SolidPattern, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/danger.png")));

    m_defaultStyle[GeoDataPlacemark::RailwayRail]                = createStyle(2.0, 1.435, "#706E70", "#EEEEEE", true, true, Qt::SolidPattern, Qt::CustomDashLine, Qt::FlatCap, true, QVector< qreal >() << 2 << 3, osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayNarrowGauge]         = createStyle(2.0, 1.0, "#706E70", "#EEEEEE", true, true, Qt::SolidPattern, Qt::CustomDashLine, Qt::FlatCap, true, QVector< qreal >() << 2 << 3, osmFont, QColor(Qt::transparent));

    // FIXME: the tram is currently being rendered as a polygon.

    m_defaultStyle[GeoDataPlacemark::RailwayTram]                = createStyle(2.0, 1.435, "#706E70", "#B7B6B7", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayLightRail]           = createStyle(2.0, 1.435, "#706E70", "#706E70", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayAbandoned]           = createStyle(2.0, 1.435, Qt::transparent, "#706E70", false, false, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwaySubway]              = createStyle(2.0, 1.435, "#706E70", "#EEEEEE", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayPreserved]           = createStyle(2.0, 1.435, "#EEEEEE", "#706E70", true, true, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, true, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayMiniature]           = createStyle(2.0, 1.435, "#706E70", "#EEEEEE", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayConstruction]        = createStyle(2.0, 1.435, "#EEEEEE", "#706E70", true, true, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, true, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayMonorail]            = createStyle(2.0, 1.435, "#706E70", "#EEEEEE", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));
    m_defaultStyle[GeoDataPlacemark::RailwayFunicular]           = createStyle(2.0, 1.435, "#706E70", "#EEEEEE", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont, QColor(Qt::transparent));

    m_defaultStyle[GeoDataPlacemark::Landmass]                   = createWayStyle("#F1EEE8", "#F1EEE8", true, false);
    m_defaultStyle[GeoDataPlacemark::UrbanArea]                  = createWayStyle("#E6E3DD", "#E6E3DD", true, false);
    m_defaultStyle[GeoDataPlacemark::InternationalDateLine]      = createStyle(1.0, 0.0, "#000000", "#000000", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont);
    m_defaultStyle[GeoDataPlacemark::Bathymetry]                 = createWayStyle("#a5c9c9", "#a5c9c9", true, false);

    m_defaultStyle[GeoDataPlacemark::AerialwayStation]           = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/railway_station"), transportationColor);
    m_defaultStyle[GeoDataPlacemark::AerialwayStation]->iconStyle().setScale(0.33f);
    m_defaultStyle[GeoDataPlacemark::AerialwayPylon]             = createOsmPOIStyle(osmFont, QString::fromUtf8("individual/pylon"), QColor("#dddddd"));
    m_defaultStyle[GeoDataPlacemark::AerialwayPylon]->iconStyle().setScale(0.33f);
    m_defaultStyle[GeoDataPlacemark::AerialwayCableCar]          = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-583813-cable-car.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayGondola]           = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-21636-gondola.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayChairLift]         = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-231-chair-lift.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayMixedLift]         = createIconWayStyle("#bbbbbb", osmFont, transportationColor);
    m_defaultStyle[GeoDataPlacemark::AerialwayDragLift]          = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-8803-platter-lift.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayTBar]              = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-8803-platter-lift.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayJBar]              = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-8803-platter-lift.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayPlatter]           = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/transportation-8803-platter-lift.svg"));
    m_defaultStyle[GeoDataPlacemark::AerialwayRopeTow]           = createIconWayStyle("#bbbbbb", osmFont, transportationColor);
    m_defaultStyle[GeoDataPlacemark::AerialwayMagicCarpet]       = createIconWayStyle("#bbbbbb", osmFont, transportationColor);
    m_defaultStyle[GeoDataPlacemark::AerialwayZipLine]           = createIconWayStyle("#bbbbbb", osmFont, transportationColor);
    m_defaultStyle[GeoDataPlacemark::AerialwayGoods]             = createIconWayStyle("#bbbbbb", osmFont, transportationColor);

    m_defaultStyle[GeoDataPlacemark::PisteDownhill]              = createStyle(9, 0.0, "#dddddd", Qt::transparent, true, false, Qt::SolidPattern, Qt::SolidLine, Qt::RoundCap, false, QVector< qreal >(), osmFont, Qt::transparent);
    m_defaultStyle[GeoDataPlacemark::PisteNordic]                = createStyle(3, 0.0, "#fffafa", "#006666", true, false, Qt::SolidPattern, Qt::SolidLine, Qt::RoundCap, false, QVector< qreal >(), osmFont, Qt::transparent, MarbleDirs::path(QString::fromUtf8("bitmaps/osmcarto/patterns/ice.png")));
    m_defaultStyle[GeoDataPlacemark::PisteSkitour]               = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PisteSled]                  = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PisteHike]                  = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PisteSleigh]                = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PisteIceSkate]              = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PisteSnowPark]              = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PistePlayground]            = m_defaultStyle[GeoDataPlacemark::PisteNordic];
    m_defaultStyle[GeoDataPlacemark::PisteSkiJump]               = createIconWayStyle("#bbbbbb", osmFont, transportationColor, 1.0, QStringLiteral("svg/thenounproject/sports-245-ski-jump.svg"));

    m_defaultStyle[GeoDataPlacemark::AdminLevel1]                = createStyle(0.0, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::CustomDashLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel2]                = createStyle(2.0, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel3]                = createStyle(1.8, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DashLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel4]                = createStyle(1.5, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel5]                = createStyle(1.25, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DashDotDotLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel6]                = createStyle(1, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DashDotLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel7]                = createStyle(1, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DashLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel8]                = createStyle(1, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DashLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel9]                = createStyle(1.5, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel10]               = createStyle(1.5, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);
    m_defaultStyle[GeoDataPlacemark::AdminLevel11]               = createStyle(1.5, 0.0, "#DF9CCF", "#DF9CCF", false, true, Qt::SolidPattern, Qt::DotLine, Qt::FlatCap, false, QVector< qreal >() << 0.3 << 0.3, osmFont);

    m_defaultStyle[GeoDataPlacemark::BoundaryMaritime]           = createStyle(2.0, 0.0, "#88b3bf", "#88b3bf", false, true, Qt::SolidPattern, Qt::SolidLine, Qt::FlatCap, false, QVector<qreal>(), osmFont);

    m_defaultStyle[GeoDataPlacemark::Satellite]
        = GeoDataStyle::Ptr(new GeoDataStyle(MarbleDirs::path(QString::fromUtf8("bitmaps/satellite.png")),
                                             QFont(defaultFamily, defaultSize, 50, false), defaultLabelColor));

    QFont tmp;

    // Fonts for areas ...

    tmp = m_defaultStyle[GeoDataPlacemark::Continent]->labelStyle().font();
    tmp.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    tmp.setCapitalization(QFont::AllUppercase);
    tmp.setBold(true);
    m_defaultStyle[GeoDataPlacemark::Continent]->labelStyle().setFont(tmp);

    // Fonts for areas ...

    tmp = m_defaultStyle[GeoDataPlacemark::Mare]->labelStyle().font();
    tmp.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    tmp.setCapitalization(QFont::AllUppercase);
    tmp.setBold(true);
    m_defaultStyle[GeoDataPlacemark::Mare]->labelStyle().setFont(tmp);

    // Now we need to underline the capitals ...

    tmp = m_defaultStyle[GeoDataPlacemark::SmallNationCapital]->labelStyle().font();
    tmp.setUnderline(true);
    m_defaultStyle[GeoDataPlacemark::SmallNationCapital]->labelStyle().setFont(tmp);

    tmp = m_defaultStyle[GeoDataPlacemark::MediumNationCapital]->labelStyle().font();
    tmp.setUnderline(true);
    m_defaultStyle[GeoDataPlacemark::MediumNationCapital]->labelStyle().setFont(tmp);

    tmp = m_defaultStyle[GeoDataPlacemark::BigNationCapital]->labelStyle().font();
    tmp.setUnderline(true);
    m_defaultStyle[GeoDataPlacemark::BigNationCapital]->labelStyle().setFont(tmp);

    tmp = m_defaultStyle[GeoDataPlacemark::LargeNationCapital]->labelStyle().font();
    tmp.setUnderline(true);
    m_defaultStyle[GeoDataPlacemark::LargeNationCapital]->labelStyle().setFont(tmp);

    // Buildings

    m_defaultStyle[GeoDataPlacemark::Building] = createStyle(1, 0, buildingColor, buildingColor.darker(),
                                                             true, true, Qt::SolidPattern, Qt::SolidLine, Qt::RoundCap, false, QVector<qreal>(), osmFont);

    for (int i = 0 ; i < GeoDataPlacemark::LastIndex ; ++i)
    {
        if (m_defaultStyle[i] && !m_defaultStyle[i]->iconStyle().iconPath().isEmpty())
        {
            auto const category                      = GeoDataPlacemark::GeoDataVisualCategory(i);
            m_buildingStyles[category]               = GeoDataStyle::Ptr(new GeoDataStyle(*m_defaultStyle[GeoDataPlacemark::Building]));
            m_buildingStyles[category]->iconStyle()  = m_defaultStyle[i]->iconStyle();
            m_buildingStyles[category]->labelStyle() = m_defaultStyle[i]->labelStyle();
        }
    }

}

QColor StyleBuilder::Private::effectColor(const QColor& color)
{
    int gray;

    switch (s_styleEffect)
    {
        case InvertedEffect:
            return QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());

        case GrayscaleEffect:
            gray = qMin(255, static_cast<int>(7 * qGray(color.darker(800).rgb())));
            return QColor(gray, gray, gray);

        case RedModeEffect:
            gray = qMin(255, static_cast<int>(7 * qGray(color.darker(800).rgb())));
            return QColor(gray, 0, 0);

        case NoEffect:
            return color;
    }

    return color;
}

QString StyleBuilder::Private::createPaintLayerItem(const QString& itemType, GeoDataPlacemark::GeoDataVisualCategory visualCategory, const QString& subType)
{
    QString const category = visualCategoryName(visualCategory);

    if (subType.isEmpty())
    {
        return itemType + QLatin1Char('/') + category;
    }

    else
    {
        return itemType + QLatin1Char('/') + category + QLatin1Char('/') + subType;
    }
}

void StyleBuilder::Private::initializeOsmVisualCategories()
{
    // Only initialize the map once
    if (!s_visualCategories.isEmpty())
    {
        return;
    }

    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("1"))]              = GeoDataPlacemark::AdminLevel1;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("2"))]              = GeoDataPlacemark::AdminLevel2;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("3"))]              = GeoDataPlacemark::AdminLevel3;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("4"))]              = GeoDataPlacemark::AdminLevel4;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("5"))]              = GeoDataPlacemark::AdminLevel5;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("6"))]              = GeoDataPlacemark::AdminLevel6;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("7"))]              = GeoDataPlacemark::AdminLevel7;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("8"))]              = GeoDataPlacemark::AdminLevel8;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("9"))]              = GeoDataPlacemark::AdminLevel9;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("10"))]             = GeoDataPlacemark::AdminLevel10;
    s_visualCategories[OsmTag(QString::fromUtf8("admin_level"), QString::fromUtf8("11"))]             = GeoDataPlacemark::AdminLevel11;

    s_visualCategories[OsmTag(QString::fromUtf8("boundary"), QString::fromUtf8("maritime"))]          = GeoDataPlacemark::BoundaryMaritime;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("restaurant"))]         = GeoDataPlacemark::FoodRestaurant;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("fast_food"))]          = GeoDataPlacemark::FoodFastFood;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("pub"))]                = GeoDataPlacemark::FoodPub;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("bar"))]                = GeoDataPlacemark::FoodBar;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("cafe"))]               = GeoDataPlacemark::FoodCafe;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("biergarten"))]         = GeoDataPlacemark::FoodBiergarten;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("college"))]            = GeoDataPlacemark::EducationCollege;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("school"))]             = GeoDataPlacemark::EducationSchool;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("university"))]         = GeoDataPlacemark::EducationUniversity;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("childcare"))]          = GeoDataPlacemark::AmenityKindergarten;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("kindergarten"))]       = GeoDataPlacemark::AmenityKindergarten;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("library"))]            = GeoDataPlacemark::AmenityLibrary;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("bus_station"))]        = GeoDataPlacemark::TransportBusStation;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("car_sharing"))]        = GeoDataPlacemark::TransportCarShare;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("fuel"))]               = GeoDataPlacemark::TransportFuel;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("parking"))]            = GeoDataPlacemark::TransportParking;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("parking_space"))]      = GeoDataPlacemark::TransportParkingSpace;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("atm"))]                = GeoDataPlacemark::MoneyAtm;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("bank"))]               = GeoDataPlacemark::MoneyBank;

    s_visualCategories[OsmTag(QString::fromUtf8("historic"), QString::fromUtf8("archaeological_site"))] = GeoDataPlacemark::HistoricArchaeologicalSite;
    s_visualCategories[OsmTag(QString::fromUtf8("historic"), QString::fromUtf8("castle"))]            = GeoDataPlacemark::HistoricCastle;
    s_visualCategories[OsmTag(QString::fromUtf8("historic"), QString::fromUtf8("fort"))]              = GeoDataPlacemark::HistoricCastle;
    s_visualCategories[OsmTag(QString::fromUtf8("historic"), QString::fromUtf8("memorial"))]          = GeoDataPlacemark::HistoricMemorial;
    s_visualCategories[OsmTag(QString::fromUtf8("historic"), QString::fromUtf8("monument"))]          = GeoDataPlacemark::HistoricMonument;
    s_visualCategories[OsmTag(QString::fromUtf8("historic"), QString::fromUtf8("ruins"))]             = GeoDataPlacemark::HistoricRuins;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("bench"))]              = GeoDataPlacemark::AmenityBench;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("car_wash"))]           = GeoDataPlacemark::AmenityCarWash;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("charging_station"))]   = GeoDataPlacemark::AmenityChargingStation;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("cinema"))]             = GeoDataPlacemark::AmenityCinema;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("community_centre"))]   = GeoDataPlacemark::AmenityCommunityCentre;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("courthouse"))]         = GeoDataPlacemark::AmenityCourtHouse;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("drinking_water"))]     = GeoDataPlacemark::AmenityDrinkingWater;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("embassy"))]            = GeoDataPlacemark::AmenityEmbassy;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("fire_station"))]       = GeoDataPlacemark::AmenityFireStation;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("fountain"))]           = GeoDataPlacemark::AmenityFountain;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("graveyard"))]          = GeoDataPlacemark::AmenityGraveyard;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("hunting_stand"))]      = GeoDataPlacemark::AmenityHuntingStand;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("nightclub"))]          = GeoDataPlacemark::AmenityNightClub;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("police"))]             = GeoDataPlacemark::AmenityPolice;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("post_box"))]           = GeoDataPlacemark::AmenityPostBox;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("post_office"))]        = GeoDataPlacemark::AmenityPostOffice;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("prison"))]             = GeoDataPlacemark::AmenityPrison;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("recycling"))]          = GeoDataPlacemark::AmenityRecycling;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("shelter"))]            = GeoDataPlacemark::AmenityShelter;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("social_facility"))]    = GeoDataPlacemark::AmenitySocialFacility;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("telephone"))]          = GeoDataPlacemark::AmenityTelephone;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("theatre"))]            = GeoDataPlacemark::AmenityTheatre;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("toilets"))]            = GeoDataPlacemark::AmenityToilets;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("townhall"))]           = GeoDataPlacemark::AmenityTownHall;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("waste_basket"))]       = GeoDataPlacemark::AmenityWasteBasket;
    s_visualCategories[OsmTag(QString::fromUtf8("emergency"), QString::fromUtf8("phone"))]            = GeoDataPlacemark::AmenityEmergencyPhone;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("mountain_rescue"))]    = GeoDataPlacemark::AmenityMountainRescue;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("dentist"))]            = GeoDataPlacemark::HealthDentist;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("doctors"))]            = GeoDataPlacemark::HealthDoctors;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("hospital"))]           = GeoDataPlacemark::HealthHospital;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("pharmacy"))]           = GeoDataPlacemark::HealthPharmacy;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("veterinary"))]         = GeoDataPlacemark::HealthVeterinary;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("place_of_worship"))]   = GeoDataPlacemark::ReligionPlaceOfWorship;

    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("information"))]        = GeoDataPlacemark::TourismInformation;

    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("cave_entrance"))]      = GeoDataPlacemark::NaturalCave;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("peak"))]               = GeoDataPlacemark::NaturalPeak;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("tree"))]               = GeoDataPlacemark::NaturalTree;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("volcano"))]            = GeoDataPlacemark::NaturalVolcano;

    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("alcohol"))]               = GeoDataPlacemark::ShopAlcohol;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("art"))]                   = GeoDataPlacemark::ShopArt;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("bag"))]                   = GeoDataPlacemark::ShopBag;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("bakery"))]                = GeoDataPlacemark::ShopBakery;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("beauty"))]                = GeoDataPlacemark::ShopBeauty;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("beverages"))]             = GeoDataPlacemark::ShopBeverages;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("bicycle"))]               = GeoDataPlacemark::ShopBicycle;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("books"))]                 = GeoDataPlacemark::ShopBook;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("butcher"))]               = GeoDataPlacemark::ShopButcher;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("car"))]                   = GeoDataPlacemark::ShopCar;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("car_parts"))]             = GeoDataPlacemark::ShopCarParts;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("car_repair"))]            = GeoDataPlacemark::ShopCarRepair;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("chemist"))]               = GeoDataPlacemark::ShopChemist;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("clothes"))]               = GeoDataPlacemark::ShopClothes;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("confectionery"))]         = GeoDataPlacemark::ShopConfectionery;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("convenience"))]           = GeoDataPlacemark::ShopConvenience;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("copy"))]                  = GeoDataPlacemark::ShopCopy;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("cosmetics"))]             = GeoDataPlacemark::ShopCosmetics;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("deli"))]                  = GeoDataPlacemark::ShopDeli;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("department_store"))]      = GeoDataPlacemark::ShopDepartmentStore;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("doityourself"))]          = GeoDataPlacemark::ShopDoitYourself;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("electronics"))]           = GeoDataPlacemark::ShopElectronics;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("fashion"))]               = GeoDataPlacemark::ShopFashion;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("florist"))]               = GeoDataPlacemark::ShopFlorist;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("furniture"))]             = GeoDataPlacemark::ShopFurniture;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("gift"))]                  = GeoDataPlacemark::ShopGift;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("greengrocer"))]           = GeoDataPlacemark::ShopGreengrocer;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("hairdresser"))]           = GeoDataPlacemark::ShopHairdresser;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("hardware"))]              = GeoDataPlacemark::ShopHardware;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("hifi"))]                  = GeoDataPlacemark::ShopHifi;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("jewelry"))]               = GeoDataPlacemark::ShopJewelry;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("kiosk"))]                 = GeoDataPlacemark::ShopKiosk;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("laundry"))]               = GeoDataPlacemark::ShopLaundry;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("mobile_phone"))]          = GeoDataPlacemark::ShopMobilePhone;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("motorcycle"))]            = GeoDataPlacemark::ShopMotorcycle;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("musical_instrument"))]    = GeoDataPlacemark::ShopMusicalInstrument;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("optician"))]              = GeoDataPlacemark::ShopOptician;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("outdoor"))]               = GeoDataPlacemark::ShopOutdoor;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("perfumery"))]             = GeoDataPlacemark::ShopPerfumery;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("pet"))]                   = GeoDataPlacemark::ShopPet;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("photo"))]                 = GeoDataPlacemark::ShopPhoto;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("seafood"))]               = GeoDataPlacemark::ShopSeafood;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("shoes"))]                 = GeoDataPlacemark::ShopShoes;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("sports"))]                = GeoDataPlacemark::ShopSports;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("stationery"))]            = GeoDataPlacemark::ShopStationery;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("supermarket"))]           = GeoDataPlacemark::ShopSupermarket;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("tea"))]                   = GeoDataPlacemark::ShopTea;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("computer"))]              = GeoDataPlacemark::ShopComputer;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("garden_centre"))]         = GeoDataPlacemark::ShopGardenCentre;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("tobacco"))]               = GeoDataPlacemark::ShopTobacco;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("toys"))]                  = GeoDataPlacemark::ShopToys;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("travel_agency"))]         = GeoDataPlacemark::ShopTravelAgency;
    s_visualCategories[OsmTag(QString::fromUtf8("shop"), QString::fromUtf8("variety_store"))]         = GeoDataPlacemark::ShopVarietyStore;


    // Default for all other shops

    const auto vals = shopValues();

    for (const QString& value : vals)
    {
        s_visualCategories[OsmTag(QString::fromUtf8("shop"), value)]               = GeoDataPlacemark::Shop;
    }

    s_visualCategories[OsmTag(QString::fromUtf8("man_made"), QString::fromUtf8("bridge"))]            = GeoDataPlacemark::ManmadeBridge;
    s_visualCategories[OsmTag(QString::fromUtf8("man_made"), QString::fromUtf8("lighthouse"))]        = GeoDataPlacemark::ManmadeLighthouse;
    s_visualCategories[OsmTag(QString::fromUtf8("man_made"), QString::fromUtf8("pier"))]              = GeoDataPlacemark::ManmadePier;
    s_visualCategories[OsmTag(QString::fromUtf8("man_made"), QString::fromUtf8("water_tower"))]       = GeoDataPlacemark::ManmadeWaterTower;
    s_visualCategories[OsmTag(QString::fromUtf8("man_made"), QString::fromUtf8("windmill"))]          = GeoDataPlacemark::ManmadeWindMill;
    s_visualCategories[OsmTag(QString::fromUtf8("man_made"), QString::fromUtf8("communications_tower"))] = GeoDataPlacemark::ManmadeCommunicationsTower;
    s_visualCategories[OsmTag(QString::fromUtf8("tower:type"), QString::fromUtf8("communication"))]   = GeoDataPlacemark::ManmadeCommunicationsTower;

    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8(""))]                  = GeoDataPlacemark::ReligionPlaceOfWorship;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("bahai"))]             = GeoDataPlacemark::ReligionBahai;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("buddhist"))]          = GeoDataPlacemark::ReligionBuddhist;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("christian"))]         = GeoDataPlacemark::ReligionChristian;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("hindu"))]             = GeoDataPlacemark::ReligionHindu;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("jain"))]              = GeoDataPlacemark::ReligionJain;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("jewish"))]            = GeoDataPlacemark::ReligionJewish;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("muslim"))]            = GeoDataPlacemark::ReligionMuslim;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("shinto"))]            = GeoDataPlacemark::ReligionShinto;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("sikh"))]              = GeoDataPlacemark::ReligionSikh;
    s_visualCategories[OsmTag(QString::fromUtf8("religion"), QString::fromUtf8("taoist"))]            = GeoDataPlacemark::ReligionTaoist;

    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("camp_site"))]          = GeoDataPlacemark::AccomodationCamping;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("guest_house"))]        = GeoDataPlacemark::AccomodationGuestHouse;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("hostel"))]             = GeoDataPlacemark::AccomodationHostel;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("hotel"))]              = GeoDataPlacemark::AccomodationHotel;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("motel"))]              = GeoDataPlacemark::AccomodationMotel;

    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("alpine_hut"))]         = GeoDataPlacemark::TourismAlpineHut;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("artwork"))]            = GeoDataPlacemark::TourismArtwork;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("attraction"))]         = GeoDataPlacemark::TourismAttraction;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("museum"))]             = GeoDataPlacemark::TourismMuseum;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("theme_park"))]         = GeoDataPlacemark::TourismThemePark;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("viewpoint"))]          = GeoDataPlacemark::TourismViewPoint;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("wilderness_hut"))]     = GeoDataPlacemark::TourismWildernessHut;
    s_visualCategories[OsmTag(QString::fromUtf8("tourism"), QString::fromUtf8("zoo"))]                = GeoDataPlacemark::TourismZoo;

    s_visualCategories[OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("city_wall"))]          = GeoDataPlacemark::BarrierCityWall;
    s_visualCategories[OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("gate"))]               = GeoDataPlacemark::BarrierGate;
    s_visualCategories[OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("lift_gate"))]          = GeoDataPlacemark::BarrierLiftGate;
    s_visualCategories[OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("wall"))]               = GeoDataPlacemark::BarrierWall;

    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("traffic_signals"))]    = GeoDataPlacemark::HighwayTrafficSignals;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("elevator"))]           = GeoDataPlacemark::HighwayElevator;

    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("cycleway"))]           = GeoDataPlacemark::HighwayCycleway;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("footway"))]            = GeoDataPlacemark::HighwayFootway;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("living_street"))]      = GeoDataPlacemark::HighwayLivingStreet;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("motorway"))]           = GeoDataPlacemark::HighwayMotorway;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("motorway_link"))]      = GeoDataPlacemark::HighwayMotorwayLink;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("path"))]               = GeoDataPlacemark::HighwayPath;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("pedestrian"))]         = GeoDataPlacemark::HighwayPedestrian;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("primary"))]            = GeoDataPlacemark::HighwayPrimary;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("primary_link"))]       = GeoDataPlacemark::HighwayPrimaryLink;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("raceway"))]            = GeoDataPlacemark::HighwayRaceway;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("residential"))]        = GeoDataPlacemark::HighwayResidential;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("road"))]               = GeoDataPlacemark::HighwayRoad;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("secondary"))]          = GeoDataPlacemark::HighwaySecondary;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("secondary_link"))]     = GeoDataPlacemark::HighwaySecondaryLink;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("service"))]            = GeoDataPlacemark::HighwayService;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("steps"))]              = GeoDataPlacemark::HighwaySteps;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("tertiary"))]           = GeoDataPlacemark::HighwayTertiary;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("tertiary_link"))]      = GeoDataPlacemark::HighwayTertiaryLink;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("track"))]              = GeoDataPlacemark::HighwayTrack;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("trunk"))]              = GeoDataPlacemark::HighwayTrunk;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("trunk_link"))]         = GeoDataPlacemark::HighwayTrunkLink;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("unclassified"))]       = GeoDataPlacemark::HighwayUnclassified;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("unknown"))]            = GeoDataPlacemark::HighwayUnknown;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("corridor"))]           = GeoDataPlacemark::HighwayCorridor;

    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("bay"))]                = GeoDataPlacemark::NaturalWater;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("coastline"))]          = GeoDataPlacemark::NaturalWater;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("reef"))]               = GeoDataPlacemark::NaturalReef;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("water"))]              = GeoDataPlacemark::NaturalWater;

    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("canal"))]             = GeoDataPlacemark::WaterwayCanal;
    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("ditch"))]             = GeoDataPlacemark::WaterwayDitch;
    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("drain"))]             = GeoDataPlacemark::WaterwayDrain;
    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("river"))]             = GeoDataPlacemark::WaterwayRiver;
    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("riverbank"))]         = GeoDataPlacemark::NaturalWater;
    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("weir"))]              = GeoDataPlacemark::WaterwayWeir;
    s_visualCategories[OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("stream"))]            = GeoDataPlacemark::WaterwayStream;

    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("beach"))]              = GeoDataPlacemark::NaturalBeach;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("cliff"))]              = GeoDataPlacemark::NaturalCliff;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("glacier"))]            = GeoDataPlacemark::NaturalGlacier;
    s_visualCategories[OsmTag(QString::fromUtf8("glacier:type"), QString::fromUtf8("shelf"))]         = GeoDataPlacemark::NaturalIceShelf;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("scrub"))]              = GeoDataPlacemark::NaturalScrub;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("wetland"))]            = GeoDataPlacemark::NaturalWetland;
    s_visualCategories[OsmTag(QString::fromUtf8("natural"), QString::fromUtf8("wood"))]               = GeoDataPlacemark::NaturalWood;

    s_visualCategories[OsmTag(QString::fromUtf8("military"), QString::fromUtf8("danger_area"))]       = GeoDataPlacemark::MilitaryDangerArea;

    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("allotments"))]         = GeoDataPlacemark::LanduseAllotments;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("basin"))]              = GeoDataPlacemark::LanduseBasin;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("brownfield"))]         = GeoDataPlacemark::LanduseConstruction;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("cemetery"))]           = GeoDataPlacemark::LanduseCemetery;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("commercial"))]         = GeoDataPlacemark::LanduseCommercial;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("construction"))]       = GeoDataPlacemark::LanduseConstruction;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("farm"))]               = GeoDataPlacemark::LanduseFarmland;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("farmland"))]           = GeoDataPlacemark::LanduseFarmland;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("farmyard"))]           = GeoDataPlacemark::LanduseFarmland;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("forest"))]             = GeoDataPlacemark::NaturalWood;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("garages"))]            = GeoDataPlacemark::LanduseGarages;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("grass"))]              = GeoDataPlacemark::LanduseGrass;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("greenfield"))]         = GeoDataPlacemark::LanduseConstruction;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("greenhouse_horticulture"))] = GeoDataPlacemark::LanduseFarmland;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("industrial"))]         = GeoDataPlacemark::LanduseIndustrial;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("landfill"))]           = GeoDataPlacemark::LanduseLandfill;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("meadow"))]             = GeoDataPlacemark::LanduseMeadow;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("military"))]           = GeoDataPlacemark::LanduseMilitary;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("orchard"))]            = GeoDataPlacemark::LanduseFarmland;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("orchard"))]            = GeoDataPlacemark::LanduseOrchard;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("quarry"))]             = GeoDataPlacemark::LanduseQuarry;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("railway"))]            = GeoDataPlacemark::LanduseRailway;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("recreation_ground"))]  = GeoDataPlacemark::LeisurePark;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("reservoir"))]          = GeoDataPlacemark::LanduseReservoir;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("residential"))]        = GeoDataPlacemark::LanduseResidential;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("retail"))]             = GeoDataPlacemark::LanduseRetail;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("village_green"))]      = GeoDataPlacemark::LanduseGrass;
    s_visualCategories[OsmTag(QString::fromUtf8("landuse"), QString::fromUtf8("vineyard"))]           = GeoDataPlacemark::LanduseVineyard;

    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("common"))]             = GeoDataPlacemark::LanduseGrass;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("garden"))]             = GeoDataPlacemark::LanduseGrass;

    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("golf_course"))]        = GeoDataPlacemark::LeisureGolfCourse;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("marina"))]             = GeoDataPlacemark::LeisureMarina;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("miniature_golf"))]     = GeoDataPlacemark::LeisureMinigolfCourse;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("park"))]               = GeoDataPlacemark::LeisurePark;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("pitch"))]              = GeoDataPlacemark::LeisurePitch;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("playground"))]         = GeoDataPlacemark::LeisurePlayground;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("sports_centre"))]      = GeoDataPlacemark::LeisureSportsCentre;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("stadium"))]            = GeoDataPlacemark::LeisureStadium;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("swimming_pool"))]      = GeoDataPlacemark::LeisureSwimmingPool;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("track"))]              = GeoDataPlacemark::LeisureTrack;
    s_visualCategories[OsmTag(QString::fromUtf8("leisure"), QString::fromUtf8("water_park"))]         = GeoDataPlacemark::LeisureWaterPark;

    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("abandoned"))]          = GeoDataPlacemark::RailwayAbandoned;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("construction"))]       = GeoDataPlacemark::RailwayConstruction;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("disused"))]            = GeoDataPlacemark::RailwayAbandoned;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("funicular"))]          = GeoDataPlacemark::RailwayFunicular;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("halt"))]               = GeoDataPlacemark::TransportTrainStation;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("light_rail"))]         = GeoDataPlacemark::RailwayLightRail;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("miniature"))]          = GeoDataPlacemark::RailwayMiniature;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("monorail"))]           = GeoDataPlacemark::RailwayMonorail;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("narrow_gauge"))]       = GeoDataPlacemark::RailwayNarrowGauge;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("platform"))]           = GeoDataPlacemark::TransportPlatform;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("preserved"))]          = GeoDataPlacemark::RailwayPreserved;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("rail"))]               = GeoDataPlacemark::RailwayRail;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("razed"))]              = GeoDataPlacemark::RailwayAbandoned;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("station"))]            = GeoDataPlacemark::TransportTrainStation;
    s_visualCategories[OsmTag(QString::fromUtf8("public_transport"), QString::fromUtf8("station"))]   = GeoDataPlacemark::TransportTrainStation;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("subway"))]             = GeoDataPlacemark::RailwaySubway;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("tram"))]               = GeoDataPlacemark::RailwayTram;

    s_visualCategories[OsmTag(QString::fromUtf8("power"), QString::fromUtf8("tower"))]                = GeoDataPlacemark::PowerTower;

    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("aerodrome"))]          = GeoDataPlacemark::TransportAerodrome;
    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("apron"))]              = GeoDataPlacemark::TransportAirportApron;
    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("gate"))]               = GeoDataPlacemark::TransportAirportGate;
    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("helipad"))]            = GeoDataPlacemark::TransportHelipad;
    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("runway"))]             = GeoDataPlacemark::TransportAirportRunway;
    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("taxiway"))]            = GeoDataPlacemark::TransportAirportTaxiway;
    s_visualCategories[OsmTag(QString::fromUtf8("aeroway"), QString::fromUtf8("terminal"))]           = GeoDataPlacemark::TransportAirportTerminal;

    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("downhill"))]        = GeoDataPlacemark::PisteDownhill;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("nordic"))]          = GeoDataPlacemark::PisteNordic;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("skitour"))]         = GeoDataPlacemark::PisteSkitour;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("sled"))]            = GeoDataPlacemark::PisteSled;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("hike"))]            = GeoDataPlacemark::PisteHike;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("sleigh"))]          = GeoDataPlacemark::PisteSleigh;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("ice_skate"))]       = GeoDataPlacemark::PisteIceSkate;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("snow_park"))]       = GeoDataPlacemark::PisteSnowPark;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("playground"))]      = GeoDataPlacemark::PistePlayground;
    s_visualCategories[OsmTag(QString::fromUtf8("piste:type"), QString::fromUtf8("ski_jump"))]        = GeoDataPlacemark::PisteSkiJump;

    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("bicycle_parking"))]    = GeoDataPlacemark::TransportBicycleParking;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("bicycle_rental"))]     = GeoDataPlacemark::TransportRentalBicycle;
    s_visualCategories[OsmTag(QString::fromUtf8("rental"), QString::fromUtf8("bicycle"))]             = GeoDataPlacemark::TransportRentalBicycle;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("car_rental"))]         = GeoDataPlacemark::TransportRentalCar;
    s_visualCategories[OsmTag(QString::fromUtf8("rental"), QString::fromUtf8("car"))]                 = GeoDataPlacemark::TransportRentalCar;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("ski_rental"))]         = GeoDataPlacemark::TransportRentalSki;
    s_visualCategories[OsmTag(QString::fromUtf8("rental"), QString::fromUtf8("ski"))]                 = GeoDataPlacemark::TransportRentalSki;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("motorcycle_parking"))] = GeoDataPlacemark::TransportMotorcycleParking;
    s_visualCategories[OsmTag(QString::fromUtf8("amenity"), QString::fromUtf8("taxi"))]               = GeoDataPlacemark::TransportTaxiRank;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("bus_stop"))]           = GeoDataPlacemark::TransportBusStop;
    s_visualCategories[OsmTag(QString::fromUtf8("highway"), QString::fromUtf8("speed_camera"))]       = GeoDataPlacemark::TransportSpeedCamera;
    s_visualCategories[OsmTag(QString::fromUtf8("public_transport"), QString::fromUtf8("platform"))]  = GeoDataPlacemark::TransportPlatform;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("subway_entrance"))]    = GeoDataPlacemark::TransportSubwayEntrance;
    s_visualCategories[OsmTag(QString::fromUtf8("railway"), QString::fromUtf8("tram_stop"))]          = GeoDataPlacemark::TransportTramStop;

    s_visualCategories[OsmTag(QString::fromUtf8("place"), QString::fromUtf8("city"))]                 = GeoDataPlacemark::PlaceCity;
    s_visualCategories[OsmTag(QString::fromUtf8("place"), QString::fromUtf8("hamlet"))]               = GeoDataPlacemark::PlaceHamlet;
    s_visualCategories[OsmTag(QString::fromUtf8("place"), QString::fromUtf8("locality"))]             = GeoDataPlacemark::PlaceLocality;
    s_visualCategories[OsmTag(QString::fromUtf8("place"), QString::fromUtf8("suburb"))]               = GeoDataPlacemark::PlaceSuburb;
    s_visualCategories[OsmTag(QString::fromUtf8("place"), QString::fromUtf8("town"))]                 = GeoDataPlacemark::PlaceTown;
    s_visualCategories[OsmTag(QString::fromUtf8("place"), QString::fromUtf8("village"))]              = GeoDataPlacemark::PlaceVillage;

    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("station"))]          = GeoDataPlacemark::AerialwayStation;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("pylon"))]            = GeoDataPlacemark::AerialwayPylon;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("cable_car"))]        = GeoDataPlacemark::AerialwayCableCar;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("gondola"))]          = GeoDataPlacemark::AerialwayGondola;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("chair_lift"))]       = GeoDataPlacemark::AerialwayChairLift;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("mixed_lift"))]       = GeoDataPlacemark::AerialwayMixedLift;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("drag_lift"))]        = GeoDataPlacemark::AerialwayDragLift;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("t-bar"))]            = GeoDataPlacemark::AerialwayTBar;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("j-bar"))]            = GeoDataPlacemark::AerialwayJBar;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("platter"))]          = GeoDataPlacemark::AerialwayPlatter;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("rope_tow"))]         = GeoDataPlacemark::AerialwayRopeTow;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("magic_carpet"))]     = GeoDataPlacemark::AerialwayMagicCarpet;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("zip_line"))]         = GeoDataPlacemark::AerialwayZipLine;
    s_visualCategories[OsmTag(QString::fromUtf8("aerialway"), QString::fromUtf8("goods"))]            = GeoDataPlacemark::AerialwayGoods;

    s_visualCategories[OsmTag(QString::fromUtf8("indoor"), QString::fromUtf8("door"))]                = GeoDataPlacemark::IndoorDoor;
    s_visualCategories[OsmTag(QString::fromUtf8("indoor"), QString::fromUtf8("wall"))]                = GeoDataPlacemark::IndoorWall;
    s_visualCategories[OsmTag(QString::fromUtf8("indoor"), QString::fromUtf8("room"))]                = GeoDataPlacemark::IndoorRoom;

    //Custom Marble OSM Tags
    s_visualCategories[OsmTag(QString::fromUtf8("marble_land"), QString::fromUtf8("landmass"))]       = GeoDataPlacemark::Landmass;
    s_visualCategories[OsmTag(QString::fromUtf8("settlement"), QString::fromUtf8("yes"))]             = GeoDataPlacemark::UrbanArea;
    s_visualCategories[OsmTag(QString::fromUtf8("marble_line"), QString::fromUtf8("date"))]           = GeoDataPlacemark::InternationalDateLine;
    s_visualCategories[OsmTag(QString::fromUtf8("marble:feature"), QString::fromUtf8("bathymetry"))]  = GeoDataPlacemark::Bathymetry;

    // Default for buildings
    for (const auto& tag : buildingTags())
    {
        s_visualCategories[tag]                                 = GeoDataPlacemark::Building;
    }
}

void StyleBuilder::Private::initializeMinimumZoomLevels()
{
    if (s_defaultMinZoomLevelsInitialized)
    {
        return;
    }

    s_defaultMinZoomLevelsInitialized = true;

    for (int i = 0; i < GeoDataPlacemark::LastIndex; i++)
    {
        s_defaultMinZoomLevels[i] = -1;
    }

    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel10] = 8;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel11] = 8;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel1] = 0;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel2] = 1;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel3] = 1;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel4] = 2;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel5] = 4;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel6] = 5;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel7] = 5;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel8] = 7;
    s_defaultMinZoomLevels[GeoDataPlacemark::AdminLevel9] = 7;

    s_defaultMinZoomLevels[GeoDataPlacemark::HistoricArchaeologicalSite] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityBench] = 19;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityFountain]     = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityGraveyard]    = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityTelephone]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityKindergarten]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityLibrary]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityWasteBasket]  = 19;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityToilets] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityTownHall] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureWaterPark]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityDrinkingWater] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityEmbassy] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityEmergencyPhone] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityMountainRescue] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityCommunityCentre] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityFountain] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityNightClub] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityCourtHouse] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityFireStation] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityHuntingStand] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityPolice] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityPostBox] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityPostOffice] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityPrison] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityRecycling] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityShelter] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityChargingStation] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityCarWash] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenitySocialFacility] = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::BarrierCityWall] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::BarrierGate]     = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::BarrierLiftGate] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::BarrierWall]     = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::Bathymetry]  = 1;

    s_defaultMinZoomLevels[GeoDataPlacemark::BoundaryMaritime]    = 1;

    s_defaultMinZoomLevels[GeoDataPlacemark::Building]    = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::Default]     = 1;

    s_defaultMinZoomLevels[GeoDataPlacemark::EducationCollege]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::EducationSchool]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::EducationUniversity]  = 15;

    s_defaultMinZoomLevels[GeoDataPlacemark::FoodBar]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::FoodBiergarten]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::FoodCafe]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::FoodFastFood]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::FoodPub]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::FoodRestaurant]  = 16;

    s_defaultMinZoomLevels[GeoDataPlacemark::HealthHospital]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::HealthPharmacy]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::HealthDentist]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::HealthDoctors]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::HealthVeterinary]  = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::HistoricMemorial]    = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayCycleway]     = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayFootway]      = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayLivingStreet] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayMotorwayLink] = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayMotorway]     = 6;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayPath] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayPedestrian]   = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayPrimaryLink]  = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayPrimary]      = 8;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayRaceway]      = 12;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayResidential]  = 14;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayRoad] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwaySecondaryLink] = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwaySecondary]    = 9;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayService]      = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwaySteps] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayTertiaryLink] = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayTertiary]     = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayTrack] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayTrunkLink]    = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayTrunk] = 7;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayUnknown]      = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayUnclassified] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayTrafficSignals]      = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayCorridor]     = 18;
    s_defaultMinZoomLevels[GeoDataPlacemark::HighwayElevator] = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::AccomodationCamping] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AccomodationHostel] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AccomodationHotel] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AccomodationMotel] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AccomodationYouthHostel] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AccomodationGuestHouse] = 16;

    s_defaultMinZoomLevels[GeoDataPlacemark::InternationalDateLine]      = 1;

    s_defaultMinZoomLevels[GeoDataPlacemark::Landmass]    = 0;

    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseAllotments]   = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseBasin] = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseCemetery]     = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseCommercial]   = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseConstruction] = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseFarmland]     = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseFarmyard]     = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseGarages]      = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseGrass] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseIndustrial]   = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseLandfill]     = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseMeadow]       = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseMilitary]     = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseOrchard]      = 14;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseQuarry]       = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseRailway]      = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseReservoir]    = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseResidential]  = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseRetail]       = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LanduseVineyard]     = 14;

    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureGolfCourse]   = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureMarina]       = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisurePark] = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisurePlayground]   = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisurePitch]   = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureStadium]   = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureSwimmingPool]   = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureSportsCentre]   = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureTrack]   = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::LeisureMinigolfCourse] = 16;

    s_defaultMinZoomLevels[GeoDataPlacemark::ManmadeBridge]       = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::ManmadeLighthouse]       = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::ManmadePier]       = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::ManmadeWaterTower]       = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::ManmadeWindMill]       = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::ManmadeCommunicationsTower]       = 15;

    s_defaultMinZoomLevels[GeoDataPlacemark::MilitaryDangerArea]  = 11;

    s_defaultMinZoomLevels[GeoDataPlacemark::MoneyAtm]    = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::MoneyBank]    = 16;

    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalBeach] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalCliff] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalGlacier]      = 3;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalHeath]      = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalIceShelf]     = 3;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalVolcano]      = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalPeak] = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalReef] = 3;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalScrub] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalTree] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalCave] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalWater] = 3;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalWetland]      = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::NaturalWood] = 8;

    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceCityNationalCapital] = 9;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceCityCapital]    = 9;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceCity]   = 9;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceHamlet] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceLocality]       = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceSuburb] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceTownNationalCapital] = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceTownCapital]    = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceTown]   = 11;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceVillageNationalCapital] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceVillageCapital] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::PlaceVillage] = 13;

    s_defaultMinZoomLevels[GeoDataPlacemark::PowerTower] = 18;

    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayAbandoned]    = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayConstruction] = 10;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayFunicular]    = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayLightRail]    = 12;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayMiniature]    = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayMonorail]     = 12;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayNarrowGauge]  = 6;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayPreserved]    = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayRail] = 6;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwaySubway]       = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::RailwayTram] = 14;

    s_defaultMinZoomLevels[GeoDataPlacemark::Satellite]   = 0;

    for (int shop = GeoDataPlacemark::ShopBeverages; shop <= GeoDataPlacemark::Shop; ++shop)
    {
        s_defaultMinZoomLevels[shop] = 17;
    }

    s_defaultMinZoomLevels[GeoDataPlacemark::ShopSupermarket] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::ShopDepartmentStore] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::ShopDoitYourself] = 16;

    s_defaultMinZoomLevels[GeoDataPlacemark::TourismAlpineHut]  = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismWildernessHut] = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismAttraction]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismArtwork] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::HistoricCastle]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityCinema]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismMuseum]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::HistoricRuins]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AmenityTheatre]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismThemePark]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismViewPoint]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismZoo]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::HistoricMonument]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TourismInformation]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportAerodrome] = 9;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportAirportApron] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportAirportRunway] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportAirportTaxiway] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportBusStation]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportCarShare]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportFuel]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportHelipad] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportAirportTerminal] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportAirportGate] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportPlatform]   = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportSpeedCamera] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportRentalCar] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportRentalBicycle] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportRentalSki] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportTaxiRank]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportParking]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportBusStop]  = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportTrainStation]  = 13;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportTramStop]  = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportParkingSpace]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportBicycleParking]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportMotorcycleParking]  = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::TransportSubwayEntrance]  = 17;

    for (int religion = GeoDataPlacemark::ReligionPlaceOfWorship; religion <= GeoDataPlacemark::ReligionTaoist; ++religion)
    {
        s_defaultMinZoomLevels[religion] = 17;
    }

    s_defaultMinZoomLevels[GeoDataPlacemark::UrbanArea]   = 3;

    s_defaultMinZoomLevels[GeoDataPlacemark::WaterwayCanal] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::WaterwayDitch] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::WaterwayDrain] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::WaterwayStream] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::WaterwayRiver] = 3;
    s_defaultMinZoomLevels[GeoDataPlacemark::WaterwayWeir] = 17;

    s_defaultMinZoomLevels[GeoDataPlacemark::CrossingIsland] = 18;
    s_defaultMinZoomLevels[GeoDataPlacemark::CrossingRailway] = 18;
    s_defaultMinZoomLevels[GeoDataPlacemark::CrossingSignals] = 18;
    s_defaultMinZoomLevels[GeoDataPlacemark::CrossingZebra] = 18;

    s_defaultMinZoomLevels[GeoDataPlacemark::PisteDownhill] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteNordic] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteSkitour] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteSled] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteHike] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteSleigh] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteIceSkate] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteSnowPark] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PistePlayground] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::PisteSkiJump] = 15;

    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayStation] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayPylon] = 16;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayCableCar] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayGondola] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayChairLift] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayMixedLift] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayDragLift] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayTBar] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayJBar] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayPlatter] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayRopeTow] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayMagicCarpet] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayZipLine] = 15;
    s_defaultMinZoomLevels[GeoDataPlacemark::AerialwayGoods] = 15;

    s_defaultMinZoomLevels[GeoDataPlacemark::IndoorDoor] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::IndoorWall] = 17;
    s_defaultMinZoomLevels[GeoDataPlacemark::IndoorRoom] = 17;

    for (int i = GeoDataPlacemark::PlaceCity; i < GeoDataPlacemark::LastIndex; i++)
    {
        if (s_defaultMinZoomLevels[i] < 0)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Missing default min zoom level for GeoDataPlacemark::GeoDataVisualCategory " << i;
            Q_ASSERT(false && "StyleBuilder::Private::initializeMinimumZoomLevels is incomplete");
            s_defaultMinZoomLevels[i] = 15;
        }
    }

}

StyleBuilder::StyleBuilder() :
    d(new Private)
{
    // nothing to do
}

StyleBuilder::~StyleBuilder()
{
    delete d;
}

QFont StyleBuilder::defaultFont() const
{
    return d->m_defaultFont;
}

void StyleBuilder::setDefaultFont(const QFont& font)
{
    d->m_defaultFont = font;
    reset();
}

QColor StyleBuilder::defaultLabelColor() const
{
    return d->m_defaultLabelColor;
}

void StyleBuilder::setDefaultLabelColor(const QColor& color)
{
    d->m_defaultLabelColor = color;
    reset();
}

GeoDataStyle::ConstPtr StyleBuilder::createStyle(const StyleParameters& parameters) const
{
    const GeoDataPlacemark* const placemark = parameters.placemark;

    if (!placemark)
    {
        Q_ASSERT(false && "Must not pass a null placemark to StyleBuilder::createStyle");
        return GeoDataStyle::Ptr();
    }

    if (placemark->customStyle())
    {
        return placemark->customStyle();
    }

    if (parameters.relation)
    {
        auto style = d->createRelationStyle(parameters);

        if (style)
        {
            return style;
        }
    }

    return d->createPlacemarkStyle(parameters);
}

GeoDataStyle::ConstPtr StyleBuilder::Private::presetStyle(GeoDataPlacemark::GeoDataVisualCategory visualCategory) const
{
    if (!m_defaultStyleInitialized)
    {
        const_cast<StyleBuilder::Private*>(this)->initializeDefaultStyles();  // const cast due to lazy initialization
    }

    if (m_defaultStyle[visualCategory])
    {
        return m_defaultStyle[visualCategory];
    }

    else
    {
        return m_defaultStyle[GeoDataPlacemark::Default];
    }
}


QStringList StyleBuilder::renderOrder() const
{
    static QStringList paintLayerOrder;

    if (paintLayerOrder.isEmpty())
    {
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::Landmass);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::UrbanArea);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseResidential);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseAllotments);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseBasin);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseCemetery);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseCommercial);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseConstruction);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseFarmland);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseFarmyard);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseGarages);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseIndustrial);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseLandfill);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseMeadow);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseMilitary);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseQuarry);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseRailway);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseReservoir);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseRetail);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseOrchard);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseVineyard);

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::Bathymetry);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureGolfCourse);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureMinigolfCourse);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalBeach);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalWetland);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalGlacier);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalIceShelf);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalVolcano);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalCliff);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalPeak);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::MilitaryDangerArea);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisurePark);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisurePitch);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureSportsCentre);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureStadium);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalWood);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LanduseGrass);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::HighwayPedestrian);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisurePlayground);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalScrub);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureTrack);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::TransportParking);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::TransportParkingSpace);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::ManmadeBridge);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::BarrierCityWall);

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::AmenityGraveyard);

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::AmenityKindergarten);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::EducationCollege);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::EducationSchool);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::EducationUniversity);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::HealthHospital);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureSwimmingPool);

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::Landmass);

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::NaturalWater);

        for (int i = GeoDataPlacemark::WaterwayCanal; i <= GeoDataPlacemark::WaterwayStream; ++i)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("outline"));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("inline"));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("label"));
        }

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::NaturalReef, QString::fromUtf8("outline"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::NaturalReef, QString::fromUtf8("inline"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::NaturalReef, QString::fromUtf8("label"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::LeisureMarina);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::ManmadePier);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::ManmadePier, QString::fromUtf8("outline"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::ManmadePier, QString::fromUtf8("inline"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::ManmadePier, QString::fromUtf8("label"));

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::TransportAirportApron);

        for (int i = GeoDataPlacemark::HighwaySteps; i <= GeoDataPlacemark::HighwayMotorway; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("outline"));
        }

        for (int i = GeoDataPlacemark::HighwaySteps; i <= GeoDataPlacemark::HighwayMotorway; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("inline"));
        }

        for (int i = GeoDataPlacemark::RailwayRail; i <= GeoDataPlacemark::RailwayFunicular; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("outline"));
        }

        for (int i = GeoDataPlacemark::RailwayRail; i <= GeoDataPlacemark::RailwayFunicular; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("inline"));
        }

        // Highway labels shall appear on top of railways, hence here and not already above
        for (int i = GeoDataPlacemark::HighwaySteps; i <= GeoDataPlacemark::HighwayMotorway; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("label"));
        }

        for (int i = GeoDataPlacemark::RailwayRail; i <= GeoDataPlacemark::RailwayFunicular; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("label"));
        }

        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::TransportPlatform);
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::TransportPlatform, QString::fromUtf8("outline"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::TransportPlatform, QString::fromUtf8("inline"));
        paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::TransportPlatform, QString::fromUtf8("label"));

        for (int i = GeoDataPlacemark::PisteDownhill; i <= GeoDataPlacemark::PisteSkiJump; ++i)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("Polygon"), GeoDataPlacemark::GeoDataVisualCategory(i));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("outline"));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("inline"));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("label"));
        }

        for (int i = GeoDataPlacemark::AerialwayCableCar; i <= GeoDataPlacemark::AerialwayGoods; ++i)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("outline"));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("inline"));
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("label"));
        }

        for (int i = GeoDataPlacemark::AdminLevel1; i <= GeoDataPlacemark::AdminLevel11; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("outline"));
        }

        for (int i = GeoDataPlacemark::AdminLevel1; i <= GeoDataPlacemark::AdminLevel11; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("inline"));
        }

        for (int i = GeoDataPlacemark::AdminLevel1; i <= GeoDataPlacemark::AdminLevel11; i++)
        {
            paintLayerOrder << Private::createPaintLayerItem(QString::fromUtf8("LineString"), GeoDataPlacemark::GeoDataVisualCategory(i), QString::fromUtf8("label"));
        }

        paintLayerOrder << QStringLiteral("Polygon/Building/frame");
        paintLayerOrder << QStringLiteral("Polygon/Building/roof");

        paintLayerOrder << QStringLiteral("Photo");

        // This assert checks that all the values in paintLayerOrder are unique.
        Q_ASSERT(QSet<QString>(paintLayerOrder.constBegin(), paintLayerOrder.constEnd()).size() == paintLayerOrder.size());
    }

    return paintLayerOrder;
}

void StyleBuilder::reset()
{
    d->m_defaultStyleInitialized = false;
}

int StyleBuilder::minimumZoomLevel(const GeoDataPlacemark& placemark) const
{
    Q_ASSERT(Private::s_defaultMinZoomLevelsInitialized);
    return Private::s_defaultMinZoomLevels[placemark.visualCategory()];
}

int StyleBuilder::minimumZoomLevel(const GeoDataPlacemark::GeoDataVisualCategory& visualCategory)
{
    Private::initializeMinimumZoomLevels();
    return Private::s_defaultMinZoomLevels[visualCategory];
}

qint64 StyleBuilder::popularity(const GeoDataPlacemark* placemark)
{
    qint64 const defaultValue = 100;
    int const offset = 10;

    if (StyleBuilder::Private::s_popularities.isEmpty())
    {
        QVector<GeoDataPlacemark::GeoDataVisualCategory> popularities;
        popularities << GeoDataPlacemark::PlaceCityNationalCapital;
        popularities << GeoDataPlacemark::PlaceTownNationalCapital;
        popularities << GeoDataPlacemark::PlaceCityCapital;
        popularities << GeoDataPlacemark::PlaceTownCapital;
        popularities << GeoDataPlacemark::PlaceCity;
        popularities << GeoDataPlacemark::PlaceTown;
        popularities << GeoDataPlacemark::PlaceSuburb;
        popularities << GeoDataPlacemark::PlaceVillageNationalCapital;
        popularities << GeoDataPlacemark::PlaceVillageCapital;
        popularities << GeoDataPlacemark::PlaceVillage;
        popularities << GeoDataPlacemark::PlaceHamlet;
        popularities << GeoDataPlacemark::PlaceLocality;

        popularities << GeoDataPlacemark::AmenityEmergencyPhone;
        popularities << GeoDataPlacemark::AmenityMountainRescue;
        popularities << GeoDataPlacemark::HealthHospital;
        popularities << GeoDataPlacemark::AmenityToilets;
        popularities << GeoDataPlacemark::MoneyAtm;
        popularities << GeoDataPlacemark::TransportSpeedCamera;

        popularities << GeoDataPlacemark::NaturalPeak;
        popularities << GeoDataPlacemark::NaturalVolcano;

        popularities << GeoDataPlacemark::AccomodationHotel;
        popularities << GeoDataPlacemark::AccomodationMotel;
        popularities << GeoDataPlacemark::AccomodationGuestHouse;
        popularities << GeoDataPlacemark::AccomodationYouthHostel;
        popularities << GeoDataPlacemark::AccomodationHostel;
        popularities << GeoDataPlacemark::AccomodationCamping;

        popularities << GeoDataPlacemark::HealthDentist;
        popularities << GeoDataPlacemark::HealthDoctors;
        popularities << GeoDataPlacemark::HealthPharmacy;
        popularities << GeoDataPlacemark::HealthVeterinary;

        popularities << GeoDataPlacemark::AmenityLibrary;
        popularities << GeoDataPlacemark::EducationCollege;
        popularities << GeoDataPlacemark::EducationSchool;
        popularities << GeoDataPlacemark::EducationUniversity;

        popularities << GeoDataPlacemark::FoodBar;
        popularities << GeoDataPlacemark::FoodBiergarten;
        popularities << GeoDataPlacemark::FoodCafe;
        popularities << GeoDataPlacemark::FoodFastFood;
        popularities << GeoDataPlacemark::FoodPub;
        popularities << GeoDataPlacemark::FoodRestaurant;

        popularities << GeoDataPlacemark::MoneyBank;

        popularities << GeoDataPlacemark::HistoricArchaeologicalSite;
        popularities << GeoDataPlacemark::AmenityCarWash;
        popularities << GeoDataPlacemark::AmenityEmbassy;
        popularities << GeoDataPlacemark::LeisureWaterPark;
        popularities << GeoDataPlacemark::AmenityCommunityCentre;
        popularities << GeoDataPlacemark::AmenityFountain;
        popularities << GeoDataPlacemark::AmenityNightClub;
        popularities << GeoDataPlacemark::AmenityCourtHouse;
        popularities << GeoDataPlacemark::AmenityFireStation;
        popularities << GeoDataPlacemark::AmenityShelter;
        popularities << GeoDataPlacemark::AmenityHuntingStand;
        popularities << GeoDataPlacemark::AmenityPolice;
        popularities << GeoDataPlacemark::AmenityPostBox;
        popularities << GeoDataPlacemark::AmenityPostOffice;
        popularities << GeoDataPlacemark::AmenityPrison;
        popularities << GeoDataPlacemark::AmenityRecycling;
        popularities << GeoDataPlacemark::AmenitySocialFacility;
        popularities << GeoDataPlacemark::AmenityTelephone;
        popularities << GeoDataPlacemark::AmenityTownHall;
        popularities << GeoDataPlacemark::AmenityDrinkingWater;
        popularities << GeoDataPlacemark::AmenityGraveyard;

        popularities << GeoDataPlacemark::ManmadeBridge;
        popularities << GeoDataPlacemark::ManmadeLighthouse;
        popularities << GeoDataPlacemark::ManmadePier;
        popularities << GeoDataPlacemark::ManmadeWaterTower;
        popularities << GeoDataPlacemark::ManmadeWindMill;
        popularities << GeoDataPlacemark::ManmadeCommunicationsTower;

        popularities << GeoDataPlacemark::TourismAttraction;
        popularities << GeoDataPlacemark::TourismArtwork;
        popularities << GeoDataPlacemark::HistoricCastle;
        popularities << GeoDataPlacemark::AmenityCinema;
        popularities << GeoDataPlacemark::TourismInformation;
        popularities << GeoDataPlacemark::HistoricMonument;
        popularities << GeoDataPlacemark::TourismMuseum;
        popularities << GeoDataPlacemark::HistoricRuins;
        popularities << GeoDataPlacemark::AmenityTheatre;
        popularities << GeoDataPlacemark::TourismThemePark;
        popularities << GeoDataPlacemark::TourismViewPoint;
        popularities << GeoDataPlacemark::TourismZoo;
        popularities << GeoDataPlacemark::TourismAlpineHut;
        popularities << GeoDataPlacemark::TourismWildernessHut;

        popularities << GeoDataPlacemark::HistoricMemorial;

        popularities << GeoDataPlacemark::TransportAerodrome;
        popularities << GeoDataPlacemark::TransportHelipad;
        popularities << GeoDataPlacemark::TransportAirportTerminal;
        popularities << GeoDataPlacemark::TransportBusStation;
        popularities << GeoDataPlacemark::TransportBusStop;
        popularities << GeoDataPlacemark::TransportCarShare;
        popularities << GeoDataPlacemark::TransportFuel;
        popularities << GeoDataPlacemark::TransportParking;
        popularities << GeoDataPlacemark::TransportParkingSpace;
        popularities << GeoDataPlacemark::TransportPlatform;
        popularities << GeoDataPlacemark::TransportRentalBicycle;
        popularities << GeoDataPlacemark::TransportRentalCar;
        popularities << GeoDataPlacemark::TransportRentalSki;
        popularities << GeoDataPlacemark::TransportTaxiRank;
        popularities << GeoDataPlacemark::TransportTrainStation;
        popularities << GeoDataPlacemark::TransportTramStop;
        popularities << GeoDataPlacemark::TransportBicycleParking;
        popularities << GeoDataPlacemark::TransportMotorcycleParking;
        popularities << GeoDataPlacemark::TransportSubwayEntrance;
        popularities << GeoDataPlacemark::AerialwayStation;

        popularities << GeoDataPlacemark::ShopBeverages;
        popularities << GeoDataPlacemark::ShopHifi;
        popularities << GeoDataPlacemark::ShopSupermarket;
        popularities << GeoDataPlacemark::ShopAlcohol;
        popularities << GeoDataPlacemark::ShopBakery;
        popularities << GeoDataPlacemark::ShopButcher;
        popularities << GeoDataPlacemark::ShopConfectionery;
        popularities << GeoDataPlacemark::ShopConvenience;
        popularities << GeoDataPlacemark::ShopGreengrocer;
        popularities << GeoDataPlacemark::ShopSeafood;
        popularities << GeoDataPlacemark::ShopDepartmentStore;
        popularities << GeoDataPlacemark::ShopKiosk;
        popularities << GeoDataPlacemark::ShopBag;
        popularities << GeoDataPlacemark::ShopClothes;
        popularities << GeoDataPlacemark::ShopFashion;
        popularities << GeoDataPlacemark::ShopJewelry;
        popularities << GeoDataPlacemark::ShopShoes;
        popularities << GeoDataPlacemark::ShopVarietyStore;
        popularities << GeoDataPlacemark::ShopBeauty;
        popularities << GeoDataPlacemark::ShopChemist;
        popularities << GeoDataPlacemark::ShopCosmetics;
        popularities << GeoDataPlacemark::ShopHairdresser;
        popularities << GeoDataPlacemark::ShopOptician;
        popularities << GeoDataPlacemark::ShopPerfumery;
        popularities << GeoDataPlacemark::ShopDoitYourself;
        popularities << GeoDataPlacemark::ShopFlorist;
        popularities << GeoDataPlacemark::ShopHardware;
        popularities << GeoDataPlacemark::ShopFurniture;
        popularities << GeoDataPlacemark::ShopElectronics;
        popularities << GeoDataPlacemark::ShopMobilePhone;
        popularities << GeoDataPlacemark::ShopBicycle;
        popularities << GeoDataPlacemark::ShopCar;
        popularities << GeoDataPlacemark::ShopCarRepair;
        popularities << GeoDataPlacemark::ShopCarParts;
        popularities << GeoDataPlacemark::ShopMotorcycle;
        popularities << GeoDataPlacemark::ShopOutdoor;
        popularities << GeoDataPlacemark::ShopSports;
        popularities << GeoDataPlacemark::ShopCopy;
        popularities << GeoDataPlacemark::ShopArt;
        popularities << GeoDataPlacemark::ShopMusicalInstrument;
        popularities << GeoDataPlacemark::ShopPhoto;
        popularities << GeoDataPlacemark::ShopBook;
        popularities << GeoDataPlacemark::ShopGift;
        popularities << GeoDataPlacemark::ShopStationery;
        popularities << GeoDataPlacemark::ShopLaundry;
        popularities << GeoDataPlacemark::ShopPet;
        popularities << GeoDataPlacemark::ShopToys;
        popularities << GeoDataPlacemark::ShopTravelAgency;
        popularities << GeoDataPlacemark::ShopDeli;
        popularities << GeoDataPlacemark::ShopTobacco;
        popularities << GeoDataPlacemark::ShopTea;
        popularities << GeoDataPlacemark::ShopComputer;
        popularities << GeoDataPlacemark::ShopGardenCentre;
        popularities << GeoDataPlacemark::Shop;

        popularities << GeoDataPlacemark::LeisureGolfCourse;
        popularities << GeoDataPlacemark::LeisureMinigolfCourse;
        popularities << GeoDataPlacemark::LeisurePark;
        popularities << GeoDataPlacemark::LeisurePlayground;
        popularities << GeoDataPlacemark::LeisurePitch;
        popularities << GeoDataPlacemark::LeisureSportsCentre;
        popularities << GeoDataPlacemark::LeisureStadium;
        popularities << GeoDataPlacemark::LeisureTrack;
        popularities << GeoDataPlacemark::LeisureSwimmingPool;

        popularities << GeoDataPlacemark::CrossingIsland;
        popularities << GeoDataPlacemark::CrossingRailway;
        popularities << GeoDataPlacemark::CrossingSignals;
        popularities << GeoDataPlacemark::CrossingZebra;
        popularities << GeoDataPlacemark::HighwayTrafficSignals;
        popularities << GeoDataPlacemark::HighwayElevator;

        popularities << GeoDataPlacemark::BarrierGate;
        popularities << GeoDataPlacemark::BarrierLiftGate;
        popularities << GeoDataPlacemark::AmenityBench;
        popularities << GeoDataPlacemark::NaturalTree;
        popularities << GeoDataPlacemark::NaturalCave;
        popularities << GeoDataPlacemark::AmenityWasteBasket;
        popularities << GeoDataPlacemark::AerialwayPylon;
        popularities << GeoDataPlacemark::PowerTower;

        int value = defaultValue + offset * popularities.size();

        for (auto popularity : popularities)
        {
            StyleBuilder::Private::s_popularities[popularity] = value;
            value -= offset;
        }
    }

    bool const isPrivate = placemark->osmData().containsTag(QStringLiteral("access"), QStringLiteral("private"));
    int const base = defaultValue + (isPrivate ? 0 : offset * StyleBuilder::Private::s_popularities.size());
    return base + StyleBuilder::Private::s_popularities.value(placemark->visualCategory(), defaultValue);
}

int StyleBuilder::maximumZoomLevel() const
{
    return d->m_maximumZoomLevel;
}

QString StyleBuilder::visualCategoryName(GeoDataPlacemark::GeoDataVisualCategory category)
{
    static QHash<GeoDataPlacemark::GeoDataVisualCategory, QString> visualCategoryNames;

    if (visualCategoryNames.isEmpty())
    {
        visualCategoryNames[GeoDataPlacemark::None] = QString::fromUtf8("None");
        visualCategoryNames[GeoDataPlacemark::Default] = QString::fromUtf8("Default");
        visualCategoryNames[GeoDataPlacemark::Unknown] = QString::fromUtf8("Unknown");
        visualCategoryNames[GeoDataPlacemark::SmallCity] = QString::fromUtf8("SmallCity");
        visualCategoryNames[GeoDataPlacemark::SmallCountyCapital] = QString::fromUtf8("SmallCountyCapital");
        visualCategoryNames[GeoDataPlacemark::SmallStateCapital] = QString::fromUtf8("SmallStateCapital");
        visualCategoryNames[GeoDataPlacemark::SmallNationCapital] = QString::fromUtf8("SmallNationCapital");
        visualCategoryNames[GeoDataPlacemark::MediumCity] = QString::fromUtf8("MediumCity");
        visualCategoryNames[GeoDataPlacemark::MediumCountyCapital] = QString::fromUtf8("MediumCountyCapital");
        visualCategoryNames[GeoDataPlacemark::MediumStateCapital] = QString::fromUtf8("MediumStateCapital");
        visualCategoryNames[GeoDataPlacemark::MediumNationCapital] = QString::fromUtf8("MediumNationCapital");
        visualCategoryNames[GeoDataPlacemark::BigCity] = QString::fromUtf8("BigCity");
        visualCategoryNames[GeoDataPlacemark::BigCountyCapital] = QString::fromUtf8("BigCountyCapital");
        visualCategoryNames[GeoDataPlacemark::BigStateCapital] = QString::fromUtf8("BigStateCapital");
        visualCategoryNames[GeoDataPlacemark::BigNationCapital] = QString::fromUtf8("BigNationCapital");
        visualCategoryNames[GeoDataPlacemark::LargeCity] = QString::fromUtf8("LargeCity");
        visualCategoryNames[GeoDataPlacemark::LargeCountyCapital] = QString::fromUtf8("LargeCountyCapital");
        visualCategoryNames[GeoDataPlacemark::LargeStateCapital] = QString::fromUtf8("LargeStateCapital");
        visualCategoryNames[GeoDataPlacemark::LargeNationCapital] = QString::fromUtf8("LargeNationCapital");
        visualCategoryNames[GeoDataPlacemark::Nation] = QString::fromUtf8("Nation");
        visualCategoryNames[GeoDataPlacemark::PlaceCity] = QString::fromUtf8("PlaceCity");
        visualCategoryNames[GeoDataPlacemark::PlaceCityCapital] = QString::fromUtf8("PlaceCityCapital");
        visualCategoryNames[GeoDataPlacemark::PlaceCityNationalCapital] = QString::fromUtf8("PlaceCityNationalCapital");
        visualCategoryNames[GeoDataPlacemark::PlaceSuburb] = QString::fromUtf8("PlaceSuburb");
        visualCategoryNames[GeoDataPlacemark::PlaceHamlet] = QString::fromUtf8("PlaceHamlet");
        visualCategoryNames[GeoDataPlacemark::PlaceLocality] = QString::fromUtf8("PlaceLocality");
        visualCategoryNames[GeoDataPlacemark::PlaceTown] = QString::fromUtf8("PlaceTown");
        visualCategoryNames[GeoDataPlacemark::PlaceTownCapital] = QString::fromUtf8("PlaceTownCapital");
        visualCategoryNames[GeoDataPlacemark::PlaceTownNationalCapital] = QString::fromUtf8("PlaceTownNationalCapital");
        visualCategoryNames[GeoDataPlacemark::PlaceVillage] = QString::fromUtf8("PlaceVillage");
        visualCategoryNames[GeoDataPlacemark::PlaceVillageCapital] = QString::fromUtf8("PlaceVillageCapital");
        visualCategoryNames[GeoDataPlacemark::PlaceVillageNationalCapital] = QString::fromUtf8("PlaceVillageNationalCapital");
        visualCategoryNames[GeoDataPlacemark::Mountain] = QString::fromUtf8("Mountain");
        visualCategoryNames[GeoDataPlacemark::Volcano] = QString::fromUtf8("Volcano");
        visualCategoryNames[GeoDataPlacemark::Mons] = QString::fromUtf8("Mons");
        visualCategoryNames[GeoDataPlacemark::Valley] = QString::fromUtf8("Valley");
        visualCategoryNames[GeoDataPlacemark::Continent] = QString::fromUtf8("Continent");
        visualCategoryNames[GeoDataPlacemark::Ocean] = QString::fromUtf8("Ocean");
        visualCategoryNames[GeoDataPlacemark::OtherTerrain] = QString::fromUtf8("OtherTerrain");
        visualCategoryNames[GeoDataPlacemark::Crater] = QString::fromUtf8("Crater");
        visualCategoryNames[GeoDataPlacemark::Mare] = QString::fromUtf8("Mare");
        visualCategoryNames[GeoDataPlacemark::GeographicPole] = QString::fromUtf8("GeographicPole");
        visualCategoryNames[GeoDataPlacemark::MagneticPole] = QString::fromUtf8("MagneticPole");
        visualCategoryNames[GeoDataPlacemark::ShipWreck] = QString::fromUtf8("ShipWreck");
        visualCategoryNames[GeoDataPlacemark::AirPort] = QString::fromUtf8("AirPort");
        visualCategoryNames[GeoDataPlacemark::Observatory] = QString::fromUtf8("Observatory");
        visualCategoryNames[GeoDataPlacemark::MilitaryDangerArea] = QString::fromUtf8("MilitaryDangerArea");
        visualCategoryNames[GeoDataPlacemark::OsmSite] = QString::fromUtf8("OsmSite");
        visualCategoryNames[GeoDataPlacemark::Coordinate] = QString::fromUtf8("Coordinate");
        visualCategoryNames[GeoDataPlacemark::MannedLandingSite] = QString::fromUtf8("MannedLandingSite");
        visualCategoryNames[GeoDataPlacemark::RoboticRover] = QString::fromUtf8("RoboticRover");
        visualCategoryNames[GeoDataPlacemark::UnmannedSoftLandingSite] = QString::fromUtf8("UnmannedSoftLandingSite");
        visualCategoryNames[GeoDataPlacemark::UnmannedHardLandingSite] = QString::fromUtf8("UnmannedHardLandingSite");
        visualCategoryNames[GeoDataPlacemark::NaturalWater] = QString::fromUtf8("NaturalWater");
        visualCategoryNames[GeoDataPlacemark::NaturalReef] = QString::fromUtf8("NaturalReef");
        visualCategoryNames[GeoDataPlacemark::NaturalWood] = QString::fromUtf8("NaturalWood");
        visualCategoryNames[GeoDataPlacemark::NaturalBeach] = QString::fromUtf8("NaturalBeach");
        visualCategoryNames[GeoDataPlacemark::NaturalWetland] = QString::fromUtf8("NaturalWetland");
        visualCategoryNames[GeoDataPlacemark::NaturalGlacier] = QString::fromUtf8("NaturalGlacier");
        visualCategoryNames[GeoDataPlacemark::NaturalIceShelf] = QString::fromUtf8("NaturalIceShelf");
        visualCategoryNames[GeoDataPlacemark::NaturalScrub] = QString::fromUtf8("NaturalScrub");
        visualCategoryNames[GeoDataPlacemark::NaturalCliff] = QString::fromUtf8("NaturalCliff");
        visualCategoryNames[GeoDataPlacemark::NaturalHeath] = QString::fromUtf8("NaturalHeath");
        visualCategoryNames[GeoDataPlacemark::HighwayTrafficSignals] = QString::fromUtf8("HighwayTrafficSignals");
        visualCategoryNames[GeoDataPlacemark::HighwaySteps] = QString::fromUtf8("HighwaySteps");
        visualCategoryNames[GeoDataPlacemark::HighwayUnknown] = QString::fromUtf8("HighwayUnknown");
        visualCategoryNames[GeoDataPlacemark::HighwayPath] = QString::fromUtf8("HighwayPath");
        visualCategoryNames[GeoDataPlacemark::HighwayFootway] = QString::fromUtf8("HighwayFootway");
        visualCategoryNames[GeoDataPlacemark::HighwayTrack] = QString::fromUtf8("HighwayTrack");
        visualCategoryNames[GeoDataPlacemark::HighwayPedestrian] = QString::fromUtf8("HighwayPedestrian");
        visualCategoryNames[GeoDataPlacemark::HighwayCycleway] = QString::fromUtf8("HighwayCycleway");
        visualCategoryNames[GeoDataPlacemark::HighwayService] = QString::fromUtf8("HighwayService");
        visualCategoryNames[GeoDataPlacemark::HighwayRoad] = QString::fromUtf8("HighwayRoad");
        visualCategoryNames[GeoDataPlacemark::HighwayResidential] = QString::fromUtf8("HighwayResidential");
        visualCategoryNames[GeoDataPlacemark::HighwayLivingStreet] = QString::fromUtf8("HighwayLivingStreet");
        visualCategoryNames[GeoDataPlacemark::HighwayUnclassified] = QString::fromUtf8("HighwayUnclassified");
        visualCategoryNames[GeoDataPlacemark::HighwayTertiaryLink] = QString::fromUtf8("HighwayTertiaryLink");
        visualCategoryNames[GeoDataPlacemark::HighwayTertiary] = QString::fromUtf8("HighwayTertiary");
        visualCategoryNames[GeoDataPlacemark::HighwaySecondaryLink] = QString::fromUtf8("HighwaySecondaryLink");
        visualCategoryNames[GeoDataPlacemark::HighwaySecondary] = QString::fromUtf8("HighwaySecondary");
        visualCategoryNames[GeoDataPlacemark::HighwayPrimaryLink] = QString::fromUtf8("HighwayPrimaryLink");
        visualCategoryNames[GeoDataPlacemark::HighwayPrimary] = QString::fromUtf8("HighwayPrimary");
        visualCategoryNames[GeoDataPlacemark::HighwayRaceway] = QString::fromUtf8("HighwayRaceway");
        visualCategoryNames[GeoDataPlacemark::HighwayTrunkLink] = QString::fromUtf8("HighwayTrunkLink");
        visualCategoryNames[GeoDataPlacemark::HighwayTrunk] = QString::fromUtf8("HighwayTrunk");
        visualCategoryNames[GeoDataPlacemark::HighwayMotorwayLink] = QString::fromUtf8("HighwayMotorwayLink");
        visualCategoryNames[GeoDataPlacemark::HighwayMotorway] = QString::fromUtf8("HighwayMotorway");
        visualCategoryNames[GeoDataPlacemark::HighwayCorridor] = QString::fromUtf8("HighwayCorridor");
        visualCategoryNames[GeoDataPlacemark::HighwayElevator] = QString::fromUtf8("HighwayElevator");
        visualCategoryNames[GeoDataPlacemark::Building] = QString::fromUtf8("Building");
        visualCategoryNames[GeoDataPlacemark::AccomodationCamping] = QString::fromUtf8("AccomodationCamping");
        visualCategoryNames[GeoDataPlacemark::AccomodationHostel] = QString::fromUtf8("AccomodationHostel");
        visualCategoryNames[GeoDataPlacemark::AccomodationHotel] = QString::fromUtf8("AccomodationHotel");
        visualCategoryNames[GeoDataPlacemark::AccomodationMotel] = QString::fromUtf8("AccomodationMotel");
        visualCategoryNames[GeoDataPlacemark::AccomodationYouthHostel] = QString::fromUtf8("AccomodationYouthHostel");
        visualCategoryNames[GeoDataPlacemark::AccomodationGuestHouse] = QString::fromUtf8("AccomodationGuestHouse");
        visualCategoryNames[GeoDataPlacemark::AmenityLibrary] = QString::fromUtf8("AmenityLibrary");
        visualCategoryNames[GeoDataPlacemark::AmenityKindergarten] = QString::fromUtf8("AmenityKindergarten");
        visualCategoryNames[GeoDataPlacemark::EducationCollege] = QString::fromUtf8("EducationCollege");
        visualCategoryNames[GeoDataPlacemark::EducationSchool] = QString::fromUtf8("EducationSchool");
        visualCategoryNames[GeoDataPlacemark::EducationUniversity] = QString::fromUtf8("EducationUniversity");
        visualCategoryNames[GeoDataPlacemark::FoodBar] = QString::fromUtf8("FoodBar");
        visualCategoryNames[GeoDataPlacemark::FoodBiergarten] = QString::fromUtf8("FoodBiergarten");
        visualCategoryNames[GeoDataPlacemark::FoodCafe] = QString::fromUtf8("FoodCafe");
        visualCategoryNames[GeoDataPlacemark::FoodFastFood] = QString::fromUtf8("FoodFastFood");
        visualCategoryNames[GeoDataPlacemark::FoodPub] = QString::fromUtf8("FoodPub");
        visualCategoryNames[GeoDataPlacemark::FoodRestaurant] = QString::fromUtf8("FoodRestaurant");
        visualCategoryNames[GeoDataPlacemark::HealthDentist] = QString::fromUtf8("HealthDentist");
        visualCategoryNames[GeoDataPlacemark::HealthDoctors] = QString::fromUtf8("HealthDoctors");
        visualCategoryNames[GeoDataPlacemark::HealthHospital] = QString::fromUtf8("HealthHospital");
        visualCategoryNames[GeoDataPlacemark::HealthPharmacy] = QString::fromUtf8("HealthPharmacy");
        visualCategoryNames[GeoDataPlacemark::HealthVeterinary] = QString::fromUtf8("HealthVeterinary");
        visualCategoryNames[GeoDataPlacemark::MoneyAtm] = QString::fromUtf8("MoneyAtm");
        visualCategoryNames[GeoDataPlacemark::MoneyBank] = QString::fromUtf8("MoneyBank");
        visualCategoryNames[GeoDataPlacemark::AmenityEmbassy] = QString::fromUtf8("AmenityEmbassy");
        visualCategoryNames[GeoDataPlacemark::AmenityEmergencyPhone] = QString::fromUtf8("AmenityEmergencyPhone");
        visualCategoryNames[GeoDataPlacemark::AmenityMountainRescue] = QString::fromUtf8("AmenityMountainRescue");
        visualCategoryNames[GeoDataPlacemark::LeisureWaterPark] = QString::fromUtf8("LeisureWaterPark");
        visualCategoryNames[GeoDataPlacemark::AmenityCommunityCentre] = QString::fromUtf8("AmenityCommunityCentre");
        visualCategoryNames[GeoDataPlacemark::AmenityFountain] = QString::fromUtf8("AmenityFountain");
        visualCategoryNames[GeoDataPlacemark::AmenityNightClub] = QString::fromUtf8("AmenityNightClub");
        visualCategoryNames[GeoDataPlacemark::AmenityBench] = QString::fromUtf8("AmenityBench");
        visualCategoryNames[GeoDataPlacemark::AmenityCourtHouse] = QString::fromUtf8("AmenityCourtHouse");
        visualCategoryNames[GeoDataPlacemark::AmenityFireStation] = QString::fromUtf8("AmenityFireStation");
        visualCategoryNames[GeoDataPlacemark::AmenityHuntingStand] = QString::fromUtf8("AmenityHuntingStand");
        visualCategoryNames[GeoDataPlacemark::AmenityPolice] = QString::fromUtf8("AmenityPolice");
        visualCategoryNames[GeoDataPlacemark::AmenityPostBox] = QString::fromUtf8("AmenityPostBox");
        visualCategoryNames[GeoDataPlacemark::AmenityPostOffice] = QString::fromUtf8("AmenityPostOffice");
        visualCategoryNames[GeoDataPlacemark::AmenityPrison] = QString::fromUtf8("AmenityPrison");
        visualCategoryNames[GeoDataPlacemark::AmenityRecycling] = QString::fromUtf8("AmenityRecycling");
        visualCategoryNames[GeoDataPlacemark::AmenityShelter] = QString::fromUtf8("AmenityShelter");
        visualCategoryNames[GeoDataPlacemark::AmenityTelephone] = QString::fromUtf8("AmenityTelephone");
        visualCategoryNames[GeoDataPlacemark::AmenityToilets] = QString::fromUtf8("AmenityToilets");
        visualCategoryNames[GeoDataPlacemark::AmenityTownHall] = QString::fromUtf8("AmenityTownHall");
        visualCategoryNames[GeoDataPlacemark::AmenityWasteBasket] = QString::fromUtf8("AmenityWasteBasket");
        visualCategoryNames[GeoDataPlacemark::AmenityDrinkingWater] = QString::fromUtf8("AmenityDrinkingWater");
        visualCategoryNames[GeoDataPlacemark::AmenityGraveyard] = QString::fromUtf8("AmenityGraveyard");
        visualCategoryNames[GeoDataPlacemark::AmenityChargingStation] = QString::fromUtf8("ChargingStation");
        visualCategoryNames[GeoDataPlacemark::AmenityCarWash] = QString::fromUtf8("CarWash");
        visualCategoryNames[GeoDataPlacemark::AmenitySocialFacility] = QString::fromUtf8("SocialFacility");
        visualCategoryNames[GeoDataPlacemark::BarrierCityWall] = QString::fromUtf8("BarrierCityWall");
        visualCategoryNames[GeoDataPlacemark::BarrierGate] = QString::fromUtf8("BarrierGate");
        visualCategoryNames[GeoDataPlacemark::BarrierLiftGate] = QString::fromUtf8("BarrierLiftGate");
        visualCategoryNames[GeoDataPlacemark::BarrierWall] = QString::fromUtf8("BarrierWall");
        visualCategoryNames[GeoDataPlacemark::NaturalVolcano] = QString::fromUtf8("NaturalVolcano");
        visualCategoryNames[GeoDataPlacemark::NaturalPeak] = QString::fromUtf8("NaturalPeak");
        visualCategoryNames[GeoDataPlacemark::NaturalTree] = QString::fromUtf8("NaturalTree");
        visualCategoryNames[GeoDataPlacemark::NaturalCave] = QString::fromUtf8("NaturalCave");
        visualCategoryNames[GeoDataPlacemark::ShopBeverages] = QString::fromUtf8("ShopBeverages");
        visualCategoryNames[GeoDataPlacemark::ShopHifi] = QString::fromUtf8("ShopHifi");
        visualCategoryNames[GeoDataPlacemark::ShopSupermarket] = QString::fromUtf8("ShopSupermarket");
        visualCategoryNames[GeoDataPlacemark::ShopAlcohol] = QString::fromUtf8("ShopAlcohol");
        visualCategoryNames[GeoDataPlacemark::ShopBakery] = QString::fromUtf8("ShopBakery");
        visualCategoryNames[GeoDataPlacemark::ShopButcher] = QString::fromUtf8("ShopButcher");
        visualCategoryNames[GeoDataPlacemark::ShopConfectionery] = QString::fromUtf8("ShopConfectionery");
        visualCategoryNames[GeoDataPlacemark::ShopConvenience] = QString::fromUtf8("ShopConvenience");
        visualCategoryNames[GeoDataPlacemark::ShopGreengrocer] = QString::fromUtf8("ShopGreengrocer");
        visualCategoryNames[GeoDataPlacemark::ShopSeafood] = QString::fromUtf8("ShopSeafood");
        visualCategoryNames[GeoDataPlacemark::ShopDepartmentStore] = QString::fromUtf8("ShopDepartmentStore");
        visualCategoryNames[GeoDataPlacemark::ShopKiosk] = QString::fromUtf8("ShopKiosk");
        visualCategoryNames[GeoDataPlacemark::ShopBag] = QString::fromUtf8("ShopBag");
        visualCategoryNames[GeoDataPlacemark::ShopClothes] = QString::fromUtf8("ShopClothes");
        visualCategoryNames[GeoDataPlacemark::ShopFashion] = QString::fromUtf8("ShopFashion");
        visualCategoryNames[GeoDataPlacemark::ShopJewelry] = QString::fromUtf8("ShopJewelry");
        visualCategoryNames[GeoDataPlacemark::ShopShoes] = QString::fromUtf8("ShopShoes");
        visualCategoryNames[GeoDataPlacemark::ShopVarietyStore] = QString::fromUtf8("ShopVarietyStore");
        visualCategoryNames[GeoDataPlacemark::ShopBeauty] = QString::fromUtf8("ShopBeauty");
        visualCategoryNames[GeoDataPlacemark::ShopChemist] = QString::fromUtf8("ShopChemist");
        visualCategoryNames[GeoDataPlacemark::ShopCosmetics] = QString::fromUtf8("ShopCosmetics");
        visualCategoryNames[GeoDataPlacemark::ShopHairdresser] = QString::fromUtf8("ShopHairdresser");
        visualCategoryNames[GeoDataPlacemark::ShopOptician] = QString::fromUtf8("ShopOptician");
        visualCategoryNames[GeoDataPlacemark::ShopPerfumery] = QString::fromUtf8("ShopPerfumery");
        visualCategoryNames[GeoDataPlacemark::ShopDoitYourself] = QString::fromUtf8("ShopDoitYourself");
        visualCategoryNames[GeoDataPlacemark::ShopFlorist] = QString::fromUtf8("ShopFlorist");
        visualCategoryNames[GeoDataPlacemark::ShopHardware] = QString::fromUtf8("ShopHardware");
        visualCategoryNames[GeoDataPlacemark::ShopFurniture] = QString::fromUtf8("ShopFurniture");
        visualCategoryNames[GeoDataPlacemark::ShopElectronics] = QString::fromUtf8("ShopElectronics");
        visualCategoryNames[GeoDataPlacemark::ShopMobilePhone] = QString::fromUtf8("ShopMobilePhone");
        visualCategoryNames[GeoDataPlacemark::ShopBicycle] = QString::fromUtf8("ShopBicycle");
        visualCategoryNames[GeoDataPlacemark::ShopCar] = QString::fromUtf8("ShopCar");
        visualCategoryNames[GeoDataPlacemark::ShopCarRepair] = QString::fromUtf8("ShopCarRepair");
        visualCategoryNames[GeoDataPlacemark::ShopCarParts] = QString::fromUtf8("ShopCarParts");
        visualCategoryNames[GeoDataPlacemark::ShopMotorcycle] = QString::fromUtf8("ShopMotorcycle");
        visualCategoryNames[GeoDataPlacemark::ShopOutdoor] = QString::fromUtf8("ShopOutdoor");
        visualCategoryNames[GeoDataPlacemark::ShopSports] = QString::fromUtf8("ShopSports");
        visualCategoryNames[GeoDataPlacemark::ShopCopy] = QString::fromUtf8("ShopCopy");
        visualCategoryNames[GeoDataPlacemark::ShopArt] = QString::fromUtf8("ShopArt");
        visualCategoryNames[GeoDataPlacemark::ShopMusicalInstrument] = QString::fromUtf8("ShopMusicalInstrument");
        visualCategoryNames[GeoDataPlacemark::ShopPhoto] = QString::fromUtf8("ShopPhoto");
        visualCategoryNames[GeoDataPlacemark::ShopBook] = QString::fromUtf8("ShopBook");
        visualCategoryNames[GeoDataPlacemark::ShopGift] = QString::fromUtf8("ShopGift");
        visualCategoryNames[GeoDataPlacemark::ShopStationery] = QString::fromUtf8("ShopStationery");
        visualCategoryNames[GeoDataPlacemark::ShopLaundry] = QString::fromUtf8("ShopLaundry");
        visualCategoryNames[GeoDataPlacemark::ShopPet] = QString::fromUtf8("ShopPet");
        visualCategoryNames[GeoDataPlacemark::ShopToys] = QString::fromUtf8("ShopToys");
        visualCategoryNames[GeoDataPlacemark::ShopTravelAgency] = QString::fromUtf8("ShopTravelAgency");
        visualCategoryNames[GeoDataPlacemark::ShopDeli] = QString::fromUtf8("ShopDeli");
        visualCategoryNames[GeoDataPlacemark::ShopTobacco] = QString::fromUtf8("ShopTobacco");
        visualCategoryNames[GeoDataPlacemark::ShopTea] = QString::fromUtf8("ShopTea");
        visualCategoryNames[GeoDataPlacemark::ShopComputer] = QString::fromUtf8("ShopComputer");
        visualCategoryNames[GeoDataPlacemark::ShopGardenCentre] = QString::fromUtf8("ShopGardenCentre");
        visualCategoryNames[GeoDataPlacemark::Shop] = QString::fromUtf8("Shop");
        visualCategoryNames[GeoDataPlacemark::ManmadeBridge] = QString::fromUtf8("ManmadeBridge");
        visualCategoryNames[GeoDataPlacemark::ManmadeLighthouse] = QString::fromUtf8("ManmadeLighthouse");
        visualCategoryNames[GeoDataPlacemark::ManmadePier] = QString::fromUtf8("ManmadePier");
        visualCategoryNames[GeoDataPlacemark::ManmadeWaterTower] = QString::fromUtf8("ManmadeWaterTower");
        visualCategoryNames[GeoDataPlacemark::ManmadeWindMill] = QString::fromUtf8("ManmadeWindMill");
        visualCategoryNames[GeoDataPlacemark::ManmadeCommunicationsTower] = QString::fromUtf8("ManmadeCommunicationsTower");
        visualCategoryNames[GeoDataPlacemark::TourismAttraction] = QString::fromUtf8("TouristAttraction");
        visualCategoryNames[GeoDataPlacemark::TourismArtwork] = QString::fromUtf8("TouristArtwork");
        visualCategoryNames[GeoDataPlacemark::HistoricArchaeologicalSite] = QString::fromUtf8("HistoricArchaeologicalSite");
        visualCategoryNames[GeoDataPlacemark::HistoricCastle] = QString::fromUtf8("HistoricCastle");
        visualCategoryNames[GeoDataPlacemark::HistoricMemorial] = QString::fromUtf8("HistoricMemorial");
        visualCategoryNames[GeoDataPlacemark::HistoricMonument] = QString::fromUtf8("HistoricMonument");
        visualCategoryNames[GeoDataPlacemark::AmenityCinema] = QString::fromUtf8("TouristCinema");
        visualCategoryNames[GeoDataPlacemark::TourismInformation] = QString::fromUtf8("TouristInformation");
        visualCategoryNames[GeoDataPlacemark::TourismMuseum] = QString::fromUtf8("TouristMuseum");
        visualCategoryNames[GeoDataPlacemark::HistoricRuins] = QString::fromUtf8("TouristRuin");
        visualCategoryNames[GeoDataPlacemark::AmenityTheatre] = QString::fromUtf8("TouristTheatre");
        visualCategoryNames[GeoDataPlacemark::TourismThemePark] = QString::fromUtf8("TouristThemePark");
        visualCategoryNames[GeoDataPlacemark::TourismViewPoint] = QString::fromUtf8("TouristViewPoint");
        visualCategoryNames[GeoDataPlacemark::TourismZoo] = QString::fromUtf8("TouristZoo");
        visualCategoryNames[GeoDataPlacemark::TourismAlpineHut] = QString::fromUtf8("TouristAlpineHut");
        visualCategoryNames[GeoDataPlacemark::TourismWildernessHut] = QString::fromUtf8("TouristWildernessHut");
        visualCategoryNames[GeoDataPlacemark::TransportAerodrome] = QString::fromUtf8("TransportAerodrome");
        visualCategoryNames[GeoDataPlacemark::TransportHelipad] = QString::fromUtf8("TransportHelipad");
        visualCategoryNames[GeoDataPlacemark::TransportAirportTerminal] = QString::fromUtf8("TransportAirportTerminal");
        visualCategoryNames[GeoDataPlacemark::TransportAirportGate] = QString::fromUtf8("TransportAirportGate");
        visualCategoryNames[GeoDataPlacemark::TransportAirportRunway] = QString::fromUtf8("TransportAirportRunway");
        visualCategoryNames[GeoDataPlacemark::TransportAirportTaxiway] = QString::fromUtf8("TransportAirportTaxiway");
        visualCategoryNames[GeoDataPlacemark::TransportAirportApron] = QString::fromUtf8("TransportAirportApron");
        visualCategoryNames[GeoDataPlacemark::TransportBusStation] = QString::fromUtf8("TransportBusStation");
        visualCategoryNames[GeoDataPlacemark::TransportBusStop] = QString::fromUtf8("TransportBusStop");
        visualCategoryNames[GeoDataPlacemark::TransportCarShare] = QString::fromUtf8("TransportCarShare");
        visualCategoryNames[GeoDataPlacemark::TransportFuel] = QString::fromUtf8("TransportFuel");
        visualCategoryNames[GeoDataPlacemark::TransportParking] = QString::fromUtf8("TransportParking");
        visualCategoryNames[GeoDataPlacemark::TransportParkingSpace] = QString::fromUtf8("TransportParkingSpace");
        visualCategoryNames[GeoDataPlacemark::TransportPlatform] = QString::fromUtf8("TransportPlatform");
        visualCategoryNames[GeoDataPlacemark::TransportRentalBicycle] = QString::fromUtf8("TransportRentalBicycle");
        visualCategoryNames[GeoDataPlacemark::TransportRentalCar] = QString::fromUtf8("TransportRentalCar");
        visualCategoryNames[GeoDataPlacemark::TransportRentalSki] = QString::fromUtf8("TransportRentalSki");
        visualCategoryNames[GeoDataPlacemark::TransportTaxiRank] = QString::fromUtf8("TransportTaxiRank");
        visualCategoryNames[GeoDataPlacemark::TransportTrainStation] = QString::fromUtf8("TransportTrainStation");
        visualCategoryNames[GeoDataPlacemark::TransportTramStop] = QString::fromUtf8("TransportTramStop");
        visualCategoryNames[GeoDataPlacemark::TransportSpeedCamera] = QString::fromUtf8("TransportSpeedCamera");
        visualCategoryNames[GeoDataPlacemark::TransportBicycleParking] = QString::fromUtf8("TransportBicycleParking");
        visualCategoryNames[GeoDataPlacemark::TransportMotorcycleParking] = QString::fromUtf8("TransportMotorcycleParking");
        visualCategoryNames[GeoDataPlacemark::TransportSubwayEntrance] = QString::fromUtf8("TransportSubwayEntrance");
        visualCategoryNames[GeoDataPlacemark::ReligionPlaceOfWorship] = QString::fromUtf8("ReligionPlaceOfWorship");
        visualCategoryNames[GeoDataPlacemark::ReligionBahai] = QString::fromUtf8("ReligionBahai");
        visualCategoryNames[GeoDataPlacemark::ReligionBuddhist] = QString::fromUtf8("ReligionBuddhist");
        visualCategoryNames[GeoDataPlacemark::ReligionChristian] = QString::fromUtf8("ReligionChristian");
        visualCategoryNames[GeoDataPlacemark::ReligionMuslim] = QString::fromUtf8("ReligionMuslim");
        visualCategoryNames[GeoDataPlacemark::ReligionHindu] = QString::fromUtf8("ReligionHindu");
        visualCategoryNames[GeoDataPlacemark::ReligionJain] = QString::fromUtf8("ReligionJain");
        visualCategoryNames[GeoDataPlacemark::ReligionJewish] = QString::fromUtf8("ReligionJewish");
        visualCategoryNames[GeoDataPlacemark::ReligionShinto] = QString::fromUtf8("ReligionShinto");
        visualCategoryNames[GeoDataPlacemark::ReligionSikh] = QString::fromUtf8("ReligionSikh");
        visualCategoryNames[GeoDataPlacemark::ReligionTaoist] = QString::fromUtf8("ReligionTaoist");
        visualCategoryNames[GeoDataPlacemark::LeisureGolfCourse] = QString::fromUtf8("LeisureGolfCourse");
        visualCategoryNames[GeoDataPlacemark::LeisureMarina] = QString::fromUtf8("LeisureMarina");
        visualCategoryNames[GeoDataPlacemark::LeisurePark] = QString::fromUtf8("LeisurePark");
        visualCategoryNames[GeoDataPlacemark::LeisurePlayground] = QString::fromUtf8("LeisurePlayground");
        visualCategoryNames[GeoDataPlacemark::LeisurePitch] = QString::fromUtf8("LeisurePitch");
        visualCategoryNames[GeoDataPlacemark::LeisureSportsCentre] = QString::fromUtf8("LeisureSportsCentre");
        visualCategoryNames[GeoDataPlacemark::LeisureStadium] = QString::fromUtf8("LeisureStadium");
        visualCategoryNames[GeoDataPlacemark::LeisureTrack] = QString::fromUtf8("LeisureTrack");
        visualCategoryNames[GeoDataPlacemark::LeisureSwimmingPool] = QString::fromUtf8("LeisureSwimmingPool");
        visualCategoryNames[GeoDataPlacemark::LeisureMinigolfCourse] = QString::fromUtf8("LeisureMinigolfCourse");
        visualCategoryNames[GeoDataPlacemark::LanduseAllotments] = QString::fromUtf8("LanduseAllotments");
        visualCategoryNames[GeoDataPlacemark::LanduseBasin] = QString::fromUtf8("LanduseBasin");
        visualCategoryNames[GeoDataPlacemark::LanduseCemetery] = QString::fromUtf8("LanduseCemetery");
        visualCategoryNames[GeoDataPlacemark::LanduseCommercial] = QString::fromUtf8("LanduseCommercial");
        visualCategoryNames[GeoDataPlacemark::LanduseConstruction] = QString::fromUtf8("LanduseConstruction");
        visualCategoryNames[GeoDataPlacemark::LanduseFarmland] = QString::fromUtf8("LanduseFarmland");
        visualCategoryNames[GeoDataPlacemark::LanduseFarmyard] = QString::fromUtf8("LanduseFarmyard");
        visualCategoryNames[GeoDataPlacemark::LanduseGarages] = QString::fromUtf8("LanduseGarages");
        visualCategoryNames[GeoDataPlacemark::LanduseGrass] = QString::fromUtf8("LanduseGrass");
        visualCategoryNames[GeoDataPlacemark::LanduseIndustrial] = QString::fromUtf8("LanduseIndustrial");
        visualCategoryNames[GeoDataPlacemark::LanduseLandfill] = QString::fromUtf8("LanduseLandfill");
        visualCategoryNames[GeoDataPlacemark::LanduseMeadow] = QString::fromUtf8("LanduseMeadow");
        visualCategoryNames[GeoDataPlacemark::LanduseMilitary] = QString::fromUtf8("LanduseMilitary");
        visualCategoryNames[GeoDataPlacemark::LanduseQuarry] = QString::fromUtf8("LanduseQuarry");
        visualCategoryNames[GeoDataPlacemark::LanduseRailway] = QString::fromUtf8("LanduseRailway");
        visualCategoryNames[GeoDataPlacemark::LanduseReservoir] = QString::fromUtf8("LanduseReservoir");
        visualCategoryNames[GeoDataPlacemark::LanduseResidential] = QString::fromUtf8("LanduseResidential");
        visualCategoryNames[GeoDataPlacemark::LanduseRetail] = QString::fromUtf8("LanduseRetail");
        visualCategoryNames[GeoDataPlacemark::LanduseOrchard] = QString::fromUtf8("LanduseOrchard");
        visualCategoryNames[GeoDataPlacemark::LanduseVineyard] = QString::fromUtf8("LanduseVineyard");
        visualCategoryNames[GeoDataPlacemark::RailwayRail] = QString::fromUtf8("RailwayRail");
        visualCategoryNames[GeoDataPlacemark::RailwayNarrowGauge] = QString::fromUtf8("RailwayNarrowGauge");
        visualCategoryNames[GeoDataPlacemark::RailwayTram] = QString::fromUtf8("RailwayTram");
        visualCategoryNames[GeoDataPlacemark::RailwayLightRail] = QString::fromUtf8("RailwayLightRail");
        visualCategoryNames[GeoDataPlacemark::RailwayAbandoned] = QString::fromUtf8("RailwayAbandoned");
        visualCategoryNames[GeoDataPlacemark::RailwaySubway] = QString::fromUtf8("RailwaySubway");
        visualCategoryNames[GeoDataPlacemark::RailwayPreserved] = QString::fromUtf8("RailwayPreserved");
        visualCategoryNames[GeoDataPlacemark::RailwayMiniature] = QString::fromUtf8("RailwayMiniature");
        visualCategoryNames[GeoDataPlacemark::RailwayConstruction] = QString::fromUtf8("RailwayConstruction");
        visualCategoryNames[GeoDataPlacemark::RailwayMonorail] = QString::fromUtf8("RailwayMonorail");
        visualCategoryNames[GeoDataPlacemark::RailwayFunicular] = QString::fromUtf8("RailwayFunicular");
        visualCategoryNames[GeoDataPlacemark::PowerTower] = QString::fromUtf8("PowerTower");
        visualCategoryNames[GeoDataPlacemark::AerialwayStation] = QString::fromUtf8("AerialwayStation");
        visualCategoryNames[GeoDataPlacemark::AerialwayPylon] = QString::fromUtf8("AerialwayPylon");
        visualCategoryNames[GeoDataPlacemark::AerialwayCableCar] = QString::fromUtf8("AerialwayCableCar");
        visualCategoryNames[GeoDataPlacemark::AerialwayGondola] = QString::fromUtf8("AerialwayGondola");
        visualCategoryNames[GeoDataPlacemark::AerialwayChairLift] = QString::fromUtf8("AerialwayChairLift");
        visualCategoryNames[GeoDataPlacemark::AerialwayMixedLift] = QString::fromUtf8("AerialwayMixedLift");
        visualCategoryNames[GeoDataPlacemark::AerialwayDragLift] = QString::fromUtf8("AerialwayDragLift");
        visualCategoryNames[GeoDataPlacemark::AerialwayTBar] = QString::fromUtf8("AerialwayTBar");
        visualCategoryNames[GeoDataPlacemark::AerialwayJBar] = QString::fromUtf8("AerialwayJBar");
        visualCategoryNames[GeoDataPlacemark::AerialwayPlatter] = QString::fromUtf8("AerialwayPlatter");
        visualCategoryNames[GeoDataPlacemark::AerialwayRopeTow] = QString::fromUtf8("AerialwayRopeTow");
        visualCategoryNames[GeoDataPlacemark::AerialwayMagicCarpet] = QString::fromUtf8("AerialwayMagicCarpet");
        visualCategoryNames[GeoDataPlacemark::AerialwayZipLine] = QString::fromUtf8("AerialwayZipLine");
        visualCategoryNames[GeoDataPlacemark::AerialwayGoods] = QString::fromUtf8("AerialwayGoods");
        visualCategoryNames[GeoDataPlacemark::PisteDownhill] = QString::fromUtf8("PisteDownhill");
        visualCategoryNames[GeoDataPlacemark::PisteNordic] = QString::fromUtf8("PisteNordic");
        visualCategoryNames[GeoDataPlacemark::PisteSkitour] = QString::fromUtf8("PisteSkitour");
        visualCategoryNames[GeoDataPlacemark::PisteSled] = QString::fromUtf8("PisteSled");
        visualCategoryNames[GeoDataPlacemark::PisteHike] = QString::fromUtf8("PisteHike");
        visualCategoryNames[GeoDataPlacemark::PisteSleigh] = QString::fromUtf8("PisteSleigh");
        visualCategoryNames[GeoDataPlacemark::PisteIceSkate] = QString::fromUtf8("PisteIceSkate");
        visualCategoryNames[GeoDataPlacemark::PisteSnowPark] = QString::fromUtf8("PisteSnowPark");
        visualCategoryNames[GeoDataPlacemark::PistePlayground] = QString::fromUtf8("PistePlayground");
        visualCategoryNames[GeoDataPlacemark::PisteSkiJump] = QString::fromUtf8("PisteSkiJump");
        visualCategoryNames[GeoDataPlacemark::Satellite] = QString::fromUtf8("Satellite");
        visualCategoryNames[GeoDataPlacemark::Landmass] = QString::fromUtf8("Landmass");
        visualCategoryNames[GeoDataPlacemark::UrbanArea] = QString::fromUtf8("UrbanArea");
        visualCategoryNames[GeoDataPlacemark::InternationalDateLine] = QString::fromUtf8("InternationalDateLine");
        visualCategoryNames[GeoDataPlacemark::Bathymetry] = QString::fromUtf8("Bathymetry");
        visualCategoryNames[GeoDataPlacemark::AdminLevel1] = QString::fromUtf8("AdminLevel1");
        visualCategoryNames[GeoDataPlacemark::AdminLevel2] = QString::fromUtf8("AdminLevel2");
        visualCategoryNames[GeoDataPlacemark::AdminLevel3] = QString::fromUtf8("AdminLevel3");
        visualCategoryNames[GeoDataPlacemark::AdminLevel4] = QString::fromUtf8("AdminLevel4");
        visualCategoryNames[GeoDataPlacemark::AdminLevel5] = QString::fromUtf8("AdminLevel5");
        visualCategoryNames[GeoDataPlacemark::AdminLevel6] = QString::fromUtf8("AdminLevel6");
        visualCategoryNames[GeoDataPlacemark::AdminLevel7] = QString::fromUtf8("AdminLevel7");
        visualCategoryNames[GeoDataPlacemark::AdminLevel8] = QString::fromUtf8("AdminLevel8");
        visualCategoryNames[GeoDataPlacemark::AdminLevel9] = QString::fromUtf8("AdminLevel9");
        visualCategoryNames[GeoDataPlacemark::AdminLevel10] = QString::fromUtf8("AdminLevel10");
        visualCategoryNames[GeoDataPlacemark::AdminLevel11] = QString::fromUtf8("AdminLevel11");
        visualCategoryNames[GeoDataPlacemark::BoundaryMaritime] = QString::fromUtf8("BoundaryMaritime");
        visualCategoryNames[GeoDataPlacemark::WaterwayCanal] = QString::fromUtf8("WaterwayCanal");
        visualCategoryNames[GeoDataPlacemark::WaterwayDitch] = QString::fromUtf8("WaterwayDitch");
        visualCategoryNames[GeoDataPlacemark::WaterwayDrain] = QString::fromUtf8("WaterwayDrain");
        visualCategoryNames[GeoDataPlacemark::WaterwayStream] = QString::fromUtf8("WaterwayStream");
        visualCategoryNames[GeoDataPlacemark::WaterwayRiver] = QString::fromUtf8("WaterwayRiver");
        visualCategoryNames[GeoDataPlacemark::WaterwayWeir] = QString::fromUtf8("WaterwayWeir");
        visualCategoryNames[GeoDataPlacemark::CrossingIsland] = QString::fromUtf8("CrossingIsland");
        visualCategoryNames[GeoDataPlacemark::CrossingRailway] = QString::fromUtf8("CrossingRailway");
        visualCategoryNames[GeoDataPlacemark::CrossingSignals] = QString::fromUtf8("CrossingSignals");
        visualCategoryNames[GeoDataPlacemark::CrossingZebra] = QString::fromUtf8("CrossingZebra");
        visualCategoryNames[GeoDataPlacemark::IndoorDoor] = QString::fromUtf8("IndoorDoor");
        visualCategoryNames[GeoDataPlacemark::IndoorWall] = QString::fromUtf8("IndoorWall");
        visualCategoryNames[GeoDataPlacemark::IndoorRoom] = QString::fromUtf8("IndoorRoom");
        visualCategoryNames[GeoDataPlacemark::LastIndex] = QString::fromUtf8("LastIndex");
    }

    Q_ASSERT(visualCategoryNames.contains(category));
    return visualCategoryNames[category];
}

QColor StyleBuilder::effectColor(const QColor& color)
{
    return Private::effectColor(color);
}

StyleEffect StyleBuilder::styleEffect()
{
    return Private::s_styleEffect;
}

void StyleBuilder::setStyleEffect(StyleEffect effect)
{
    Private::s_styleEffect = effect;
}

QHash<StyleBuilder::OsmTag, GeoDataPlacemark::GeoDataVisualCategory> StyleBuilder::osmTagMapping()
{
    Private::initializeOsmVisualCategories();
    return Private::s_visualCategories;
}

QStringList StyleBuilder::shopValues()
{
    // from https://taginfo.openstreetmap.org/keys/building#values
    static const QStringList osmShopValues = QStringList()
                                             << QString::fromUtf8("cheese")
                                             << QString::fromUtf8("chocolate")
                                             << QString::fromUtf8("coffee")
                                             << QString::fromUtf8("dairy")
                                             << QString::fromUtf8("farm")
                                             << QString::fromUtf8("pasta")
                                             << QString::fromUtf8("pastry")
                                             << QString::fromUtf8("wine")
                                             << QString::fromUtf8("general")
                                             << QString::fromUtf8("mall")
                                             << QString::fromUtf8("baby_goods")
                                             << QString::fromUtf8("boutique")
                                             << QString::fromUtf8("fabric")
                                             << QString::fromUtf8("leather")
                                             << QString::fromUtf8("tailor")
                                             << QString::fromUtf8("watches")
                                             << QString::fromUtf8("charity")
                                             << QString::fromUtf8("second_hand")
                                             << QString::fromUtf8("erotic")
                                             << QString::fromUtf8("hearing_aids")
                                             << QString::fromUtf8("herbalist")
                                             << QString::fromUtf8("massage")
                                             << QString::fromUtf8("medical_supply")
                                             << QString::fromUtf8("tattoo")
                                             << QString::fromUtf8("bathroom_furnishing")
                                             << QString::fromUtf8("electrical")
                                             << QString::fromUtf8("energy")
                                             << QString::fromUtf8("furnace")
                                             << QString::fromUtf8("garden_furniture")
                                             << QString::fromUtf8("gas")
                                             << QString::fromUtf8("glaziery")
                                             << QString::fromUtf8("houseware")
                                             << QString::fromUtf8("locksmith")
                                             << QString::fromUtf8("paint")
                                             << QString::fromUtf8("trade")
                                             << QString::fromUtf8("antiques")
                                             << QString::fromUtf8("bed")
                                             << QString::fromUtf8("candles")
                                             << QString::fromUtf8("carpet")
                                             << QString::fromUtf8("curtain")
                                             << QString::fromUtf8("interior_decoration")
                                             << QString::fromUtf8("kitchen")
                                             << QString::fromUtf8("lamps")
                                             << QString::fromUtf8("window_blind")
                                             << QString::fromUtf8("radiotechnics")
                                             << QString::fromUtf8("vacuum_cleaner")
                                             << QString::fromUtf8("fishing")
                                             << QString::fromUtf8("free_flying")
                                             << QString::fromUtf8("hunting")
                                             << QString::fromUtf8("outdoor")
                                             << QString::fromUtf8("scuba_diving")
                                             << QString::fromUtf8("sports")
                                             << QString::fromUtf8("tyres")
                                             << QString::fromUtf8("swimming_pool")
                                             << QString::fromUtf8("art")
                                             << QString::fromUtf8("craft")
                                             << QString::fromUtf8("frame")
                                             << QString::fromUtf8("games")
                                             << QString::fromUtf8("model")
                                             << QString::fromUtf8("music")
                                             << QString::fromUtf8("trophy")
                                             << QString::fromUtf8("video")
                                             << QString::fromUtf8("video_games")
                                             << QString::fromUtf8("anime")
                                             << QString::fromUtf8("ticket")
                                             << QString::fromUtf8("copyshop")
                                             << QString::fromUtf8("dry_cleaning")
                                             << QString::fromUtf8("e-cigarette")
                                             << QString::fromUtf8("funeral_directors")
                                             << QString::fromUtf8("money_lender")
                                             << QString::fromUtf8("pawnbroker")
                                             << QString::fromUtf8("pyrotechnics")
                                             << QString::fromUtf8("religion")
                                             << QString::fromUtf8("storage_rental")
                                             << QString::fromUtf8("weapons")
                                             << QString::fromUtf8("user defined");

    return osmShopValues;
}

QSet<StyleBuilder::OsmTag> StyleBuilder::buildingTags()
{
    static const QString building = QStringLiteral("building");
    // from https://taginfo.openstreetmap.org/keys/building#values
    static const QSet<OsmTag> osmBuildingTags = QSet<OsmTag>()
                                                << OsmTag(building, QString::fromUtf8("yes"))
                                                << OsmTag(building, QString::fromUtf8("house"))
                                                << OsmTag(building, QString::fromUtf8("residential"))
                                                << OsmTag(building, QString::fromUtf8("garage"))
                                                << OsmTag(building, QString::fromUtf8("apartments"))
                                                << OsmTag(building, QString::fromUtf8("hut"))
                                                << OsmTag(building, QString::fromUtf8("industrial"))
                                                << OsmTag(building, QString::fromUtf8("detached"))
                                                << OsmTag(building, QString::fromUtf8("roof"))
                                                << OsmTag(building, QString::fromUtf8("garages"))
                                                << OsmTag(building, QString::fromUtf8("commercial"))
                                                << OsmTag(building, QString::fromUtf8("terrace"))
                                                << OsmTag(building, QString::fromUtf8("shed"))
                                                << OsmTag(building, QString::fromUtf8("school"))
                                                << OsmTag(building, QString::fromUtf8("retail"))
                                                << OsmTag(building, QString::fromUtf8("farm_auxiliary"))
                                                << OsmTag(building, QString::fromUtf8("church"))
                                                << OsmTag(building, QString::fromUtf8("cathedral"))
                                                << OsmTag(building, QString::fromUtf8("greenhouse"))
                                                << OsmTag(building, QString::fromUtf8("barn"))
                                                << OsmTag(building, QString::fromUtf8("service"))
                                                << OsmTag(building, QString::fromUtf8("manufacture"))
                                                << OsmTag(building, QString::fromUtf8("construction"))
                                                << OsmTag(building, QString::fromUtf8("cabin"))
                                                << OsmTag(building, QString::fromUtf8("farm"))
                                                << OsmTag(building, QString::fromUtf8("warehouse"))
                                                << OsmTag(building, QString::fromUtf8("House"))
                                                << OsmTag(building, QString::fromUtf8("office"))
                                                << OsmTag(building, QString::fromUtf8("civic"))
                                                << OsmTag(building, QString::fromUtf8("Residential"))
                                                << OsmTag(building, QString::fromUtf8("hangar"))
                                                << OsmTag(building, QString::fromUtf8("public"))
                                                << OsmTag(building, QString::fromUtf8("university"))
                                                << OsmTag(building, QString::fromUtf8("hospital"))
                                                << OsmTag(building, QString::fromUtf8("chapel"))
                                                << OsmTag(building, QString::fromUtf8("hotel"))
                                                << OsmTag(building, QString::fromUtf8("train_station"))
                                                << OsmTag(building, QString::fromUtf8("dormitory"))
                                                << OsmTag(building, QString::fromUtf8("kindergarten"))
                                                << OsmTag(building, QString::fromUtf8("stable"))
                                                << OsmTag(building, QString::fromUtf8("storage_tank"))
                                                << OsmTag(building, QString::fromUtf8("shop"))
                                                << OsmTag(building, QString::fromUtf8("college"))
                                                << OsmTag(building, QString::fromUtf8("supermarket"))
                                                << OsmTag(building, QString::fromUtf8("factory"))
                                                << OsmTag(building, QString::fromUtf8("bungalow"))
                                                << OsmTag(building, QString::fromUtf8("tower"))
                                                << OsmTag(building, QString::fromUtf8("silo"))
                                                << OsmTag(building, QString::fromUtf8("storage"))
                                                << OsmTag(building, QString::fromUtf8("station"))
                                                << OsmTag(building, QString::fromUtf8("education"))
                                                << OsmTag(building, QString::fromUtf8("carport"))
                                                << OsmTag(building, QString::fromUtf8("houseboat"))
                                                << OsmTag(building, QString::fromUtf8("castle"))
                                                << OsmTag(building, QString::fromUtf8("social_facility"))
                                                << OsmTag(building, QString::fromUtf8("water_tower"))
                                                << OsmTag(building, QString::fromUtf8("container"))
                                                << OsmTag(building, QString::fromUtf8("exhibition_hall"))
                                                << OsmTag(building, QString::fromUtf8("monastery"))
                                                << OsmTag(building, QString::fromUtf8("bunker"))
                                                << OsmTag(building, QString::fromUtf8("shelter"))
                                                << OsmTag(QString::fromUtf8("building:part"), QString::fromUtf8("yes"))
                                                << OsmTag(QString::fromUtf8("building:part"), QString::fromUtf8("antenna"));

    return osmBuildingTags;
}

GeoDataPlacemark::GeoDataVisualCategory StyleBuilder::determineVisualCategory(const OsmPlacemarkData& osmData)
{
    QString const yes(QStringLiteral("yes"));

    if (osmData.containsTagKey(QStringLiteral("area:highway")) ||              // Not supported yet
        osmData.containsTag(QStringLiteral("boundary"), QStringLiteral("protected_area")) ||   // Not relevant for the default map
        osmData.containsTag(QStringLiteral("boundary"), QStringLiteral("postal_code")) ||
        osmData.containsTag(QStringLiteral("boundary"), QStringLiteral("aerial_views")) ||     // Created by OSM editor(s) application for digitalization
        osmData.containsTagKey(QStringLiteral("closed:highway")) ||
        osmData.containsTagKey(QStringLiteral("abandoned:highway")) ||
        osmData.containsTagKey(QStringLiteral("abandoned:natural")) ||
        osmData.containsTagKey(QStringLiteral("abandoned:building")) ||
        osmData.containsTagKey(QStringLiteral("abandoned:leisure")) ||
        osmData.containsTagKey(QStringLiteral("disused:highway")) ||
        osmData.containsTag(QStringLiteral("highway"), QStringLiteral("razed")) ||
        osmData.containsTag(QStringLiteral("piste:abandoned"), yes))
    {
        return GeoDataPlacemark::None;
    }

    if (osmData.containsTag(QStringLiteral("building"), yes))
    {
        return GeoDataPlacemark::Building;
    }

    if (osmData.containsTag(QStringLiteral("historic"), QStringLiteral("castle")) && osmData.containsTag(QStringLiteral("castle_type"), QStringLiteral("kremlin")))
    {
        return GeoDataPlacemark::None;
    }

    if (osmData.containsTag(QStringLiteral("natural"), QStringLiteral("glacier")) && osmData.containsTag(QStringLiteral("glacier:type"), QStringLiteral("shelf")))
    {
        return GeoDataPlacemark::NaturalIceShelf;
    }

    if (osmData.containsTag(QStringLiteral("highway"), QStringLiteral("crossing")))
    {
        QStringList const crossings = osmData.tagValue(QStringLiteral("crossing")).split(QLatin1Char(';'));
        QString const crossingRef = osmData.tagValue(QStringLiteral("crossing_ref"));

        if (crossingRef == QStringLiteral("zebra") ||
            crossingRef == QStringLiteral("tiger") ||
            crossings.contains(QStringLiteral("zebra")) ||
            crossings.contains(QStringLiteral("tiger")))
        {
            return GeoDataPlacemark::CrossingZebra;
        }

        else if (crossingRef == QStringLiteral("toucan") ||
                 crossingRef == QStringLiteral("pelican") ||
                 crossings.contains(QStringLiteral("traffic_signals")) ||
                 crossings.contains(QStringLiteral("toucan")) ||
                 crossings.contains(QStringLiteral("pelican")))
        {
            return GeoDataPlacemark::CrossingSignals;
        }

        else if (crossings.contains(QStringLiteral("island")))
        {
            return GeoDataPlacemark::CrossingIsland;
        }
    }

    if (osmData.containsTag(QStringLiteral("railway"), QStringLiteral("crossing")) ||
        osmData.containsTag(QStringLiteral("railway"), QStringLiteral("level_crossing")))
    {
        return GeoDataPlacemark::CrossingRailway;
    }

    Private::initializeOsmVisualCategories();

    auto const pisteType = osmData.tagValue(QStringLiteral("piste:type"));

    if (!pisteType.isEmpty())
    {
        auto const tag = OsmTag(QStringLiteral("piste:type"), pisteType);
        auto category = Private::s_visualCategories.value(tag, GeoDataPlacemark::None);

        if (category != GeoDataPlacemark::None)
        {
            return category;
        }
    }

    QString const capital(QStringLiteral("capital"));
    QString const admin_level(QStringLiteral("admin_level"));
    // National capitals have admin_level=2
    // More at https://wiki.openstreetmap.org/wiki/Key:capital#Using_relations_for_capitals
    QString const national_level(QStringLiteral("2"));

    for (auto iter = osmData.tagsBegin(), end = osmData.tagsEnd(); iter != end; ++iter)
    {
        const auto tag = OsmTag(iter.key(), iter.value());
        GeoDataPlacemark::GeoDataVisualCategory category = Private::s_visualCategories.value(tag, GeoDataPlacemark::None);

        if (category != GeoDataPlacemark::None)
        {
            if (category == GeoDataPlacemark::PlaceCity && osmData.containsTag(admin_level, national_level))
            {
                category = GeoDataPlacemark::PlaceCityNationalCapital;
            }

            else if (category == GeoDataPlacemark::PlaceCity && osmData.containsTag(capital, yes))
            {
                category = GeoDataPlacemark::PlaceCityCapital;
            }

            else if (category == GeoDataPlacemark::PlaceTown && osmData.containsTag(admin_level, national_level))
            {
                category = GeoDataPlacemark::PlaceTownNationalCapital;
            }

            else if (category == GeoDataPlacemark::PlaceTown && osmData.containsTag(capital, yes))
            {
                category = GeoDataPlacemark::PlaceTownCapital;
            }

            else if (category == GeoDataPlacemark::PlaceVillage && osmData.containsTag(admin_level, national_level))
            {
                category = GeoDataPlacemark::PlaceVillageNationalCapital;
            }

            else if (category == GeoDataPlacemark::PlaceVillage && osmData.containsTag(capital, yes))
            {
                category = GeoDataPlacemark::PlaceVillageCapital;
            }
        }

        if (category != GeoDataPlacemark::None)
        {
            return category;
        }
    }

    return GeoDataPlacemark::None;
}

StyleParameters::StyleParameters(const GeoDataPlacemark* placemark_, int tileLevel_) :
    placemark(placemark_),
    tileLevel(tileLevel_),
    relation(nullptr)
{
    // nothing to do
}

} // namespace Marble

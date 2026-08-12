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

#include "OsmTagEditorWidget_p.h"
#include "OsmTagEditorWidget.h"

// Qt includes

#include <QTreeWidget>
#include <QObject>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GeoDataLineString.h"
#include "GeoDataPolygon.h"
#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"
#include "GeoDataIconStyle.h"
#include "OsmPlacemarkData.h"
#include "GeoDataExtendedData.h"
#include "GeoDataData.h"
#include "GeoDataGeometry.h"
#include "GeoDataPoint.h"
#include "StyleBuilder.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Marble
{

const QString OsmTagEditorWidgetPrivate::m_customTagAdderText = QString::fromUtf8(I18N_NOOP("Add custom tag..."));

OsmTagEditorWidgetPrivate::OsmTagEditorWidgetPrivate()
{
    // nothing to do
}

OsmTagEditorWidgetPrivate::~OsmTagEditorWidgetPrivate()
{
    // nothing to do
}

void OsmTagEditorWidgetPrivate::populateCurrentTagsList()
{
    // Name tag
    if (!m_placemark->name().isEmpty())
    {
        QStringList itemText;

        // "name" is a standard OSM tag, don't translate
        itemText << QString::fromUtf8("name") << m_placemark->name();
        QTreeWidgetItem* nameTag = new QTreeWidgetItem(itemText);
        nameTag->setDisabled(true);
        m_currentTagsList->addTopLevelItem(nameTag);
    }

    // Multipolygon type tag
    if (geodata_cast<GeoDataPolygon>(m_placemark->geometry()))
    {
        QStringList itemText;
        // "type" is a standard OSM tag, don't translate
        itemText << QString::fromUtf8("type") << QString::fromUtf8("multipolygon");
        QTreeWidgetItem* typeTag = new QTreeWidgetItem(itemText);
        typeTag->setDisabled(true);
        m_currentTagsList->addTopLevelItem(typeTag);
    }

    // Other tags
    if (m_placemark->hasOsmData())
    {
        const OsmPlacemarkData& osmData = m_placemark->osmData();
        QHash< QString, QString>::const_iterator it = osmData.tagsBegin();
        QHash< QString, QString>::const_iterator end = osmData.tagsEnd();

        for (; it != end; ++it)
        {
            QTreeWidgetItem* tagItem = tagWidgetItem(OsmTag(it.key(), it.value()));
            m_currentTagsList->addTopLevelItem(tagItem);
        }
    }

    // Custom tag adder item
    QTreeWidgetItem* adderItem = new QTreeWidgetItem();
    adderItem->setText(0, m_customTagAdderText);
    adderItem->setForeground(0, Qt::gray);
    adderItem->setIcon(0, QIcon::fromTheme(QLatin1String("list-add")));
    adderItem->setFlags(adderItem->flags() | Qt::ItemIsEditable);
    m_currentTagsList->addTopLevelItem(adderItem);
    m_currentTagsList->resizeColumnToContents(0);
    m_currentTagsList->resizeColumnToContents(1);
}

void OsmTagEditorWidgetPrivate::populatePresetTagsList()
{
    QList<OsmTag> tags = recommendedTags();

    for (const OsmTag& tag : tags)
    {
        QTreeWidgetItem* tagItem = tagWidgetItem(tag);
        m_recommendedTagsList->addTopLevelItem(tagItem);
    }
}

QTreeWidgetItem* OsmTagEditorWidgetPrivate::tagWidgetItem(const OsmTag& tag)
{
    QStringList itemText;

    itemText << tag.first;
    itemText << (tag.second.isEmpty() ? QLatin1Char('<') + i18n("value") + QLatin1Char('>') : tag.second);

    QTreeWidgetItem* tagItem = new QTreeWidgetItem(itemText);

    return tagItem;
}

QList<OsmTagEditorWidgetPrivate::OsmTag> OsmTagEditorWidgetPrivate::recommendedTags() const
{
    static const QVector<OsmTag> additionalOsmTags = createAdditionalOsmTags();

    QList<OsmTag> recommendedTags;

    QStringList filter = generateTagFilter();

    auto const osmTagMapping = StyleBuilder::osmTagMapping();

    for (auto iter = osmTagMapping.begin(), end = osmTagMapping.end() ; iter != end; ++iter)
    {
        if (filter.contains(iter.key().first))
        {
            recommendedTags += iter.key();
        }
    }

    for (const auto& additionalOsmTag : additionalOsmTags)
    {
        if (filter.contains(additionalOsmTag.first))
        {
            recommendedTags += additionalOsmTag;
        }
    }

    return recommendedTags;
}


QStringList OsmTagEditorWidgetPrivate::generateTagFilter() const
{
    // TO DO: implement more dynamic criteria for the filter
    // based on https://taginfo.openstreetmap.org/ and https://wiki.openstreetmap.org/wiki/

    // Contains all keys that should pass through the filter ( eg. { "amenity", "landuse", etc.. } )
    QStringList filter;

    QStringList tags, tagsAux;
    OsmPlacemarkData osmData;

    if (m_placemark->hasOsmData())
    {
        osmData = m_placemark->osmData();
    }

    else
    {
        osmData = OsmPlacemarkData();
    }

    // Patterns in order of usefulness


    // If the placemark is a node, and it doesn't already have any node-specific tags, recommend all node-specific tags
    tags      = QStringList() << QString::fromUtf8("amenity=*") << QString::fromUtf8("shop=*") << QString::fromUtf8("transport=*") << QString::fromUtf8("tourism=*") << QString::fromUtf8("historic=*") << QString::fromUtf8("power=*") << QString::fromUtf8("barrier=*");

    if (geodata_cast<GeoDataPoint>(m_placemark->geometry()) && !containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tags);
    }



    // If the placemark is a way, and it doesn't already have any way-specific tags, recommend all way-specific tags
    tags      = QStringList() << QString::fromUtf8("highway=*") << QString::fromUtf8("waterway=*") << QString::fromUtf8("railway=*");

    if (geodata_cast<GeoDataLineString>(m_placemark->geometry()) && !containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tags);
    }



    // If the placemark is a polygon, and it doesn't already have any polygon-specific tags, recommend all polygon-specific tags
    tags      = QStringList() << QString::fromUtf8("landuse=*") << QString::fromUtf8("leisure=*");

    if (geodata_cast<GeoDataPolygon>(m_placemark->geometry()) && !containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tags);
    }



    // If the placemark is a relation, recommend type=*
    tags      = QStringList() << QString::fromUtf8("type=*");

    if (m_placemark->extendedData().value(QStringLiteral("osmRelation")).value().toString() == QLatin1String("yes"))
    {
        addPattern(filter, osmData, tags);
    }



    // If the placemark has type=route, recommend route=*, network=*, ref=*, operator=*
    tags      = QStringList() << QString::fromUtf8("type=route");
    tagsAux   = QStringList() << QString::fromUtf8("route=*") << QString::fromUtf8("network=*") << QString::fromUtf8("ref=*") << QString::fromUtf8("operator=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has type=route_master, recommend route_master=*,
    tags      = QStringList() << QString::fromUtf8("type=route_master");
    tagsAux   = QStringList() << QString::fromUtf8("route_master=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has type=public_transport, recommend public_transport=*,
    tags      = QStringList() << QString::fromUtf8("type=public_transport");
    tagsAux   = QStringList() << QString::fromUtf8("public_transport=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has type=waterway, recommend waterway=*,
    tags      = QStringList() << QString::fromUtf8("type=waterway");
    tagsAux   = QStringList() << QString::fromUtf8("waterway=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has type=enforcement, recommend enforcement=*,
    tags      = QStringList() << QString::fromUtf8("type=enforcement");
    tagsAux   = QStringList() << QString::fromUtf8("enforcement=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has amenity=place_of_worship, recommend religion=*
    tags      = QStringList() << QString::fromUtf8("amenity=place_of_worship");
    tagsAux   = QStringList() << QString::fromUtf8("religion=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has amenity=toilets, recommend drinking_water=*, indoor=*
    tags      = QStringList() << QString::fromUtf8("amenity=toilets");
    tagsAux   = QStringList() << QString::fromUtf8("drinking_water=*") << QString::fromUtf8("indoor=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has  tourism=hostel, tourism=hotel or tourism=motel, recommend rooms=*, beds=*, wheelchair=*
    tags      = QStringList() << QString::fromUtf8("tourism=hotel") << QString::fromUtf8("tourism=hostel") << QString::fromUtf8("tourism=motel");
    tagsAux   = QStringList() << QString::fromUtf8("rooms=*") << QString::fromUtf8("beds=*") << QString::fromUtf8("wheelchair=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has  tourism=*, shop=*, amenity=*, recommend website=*, email=*, fee=*
    tags      = QStringList() << QString::fromUtf8("tourism=*") << QString::fromUtf8("shop=*") << QString::fromUtf8("amenity=*");
    tagsAux   = QStringList() << QString::fromUtf8("website=*") << QString::fromUtf8("email=*") << QString::fromUtf8("fee=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has amenity=* shop=*, recommend building=*
    tags      = QStringList() << QString::fromUtf8("amenity=*") << QString::fromUtf8("shop=*");
    tagsAux   = QStringList() << QString::fromUtf8("building=*");

    if (containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark has highway=*, recommend "lanes=*", "maxspeed=*", "oneway=*", "service=*", "bridge=*", "tunnel=*"
    tags      = QStringList() << QString::fromUtf8("highway=*");
    tagsAux   = QStringList() << QString::fromUtf8("lanes=*") << QString::fromUtf8("maxspeed=*") << QString::fromUtf8("maxheight=*") << QString::fromUtf8("maxweight=*") << QString::fromUtf8("abutters=*") << QString::fromUtf8("oneway=*") << QString::fromUtf8("service=*") << QString::fromUtf8("bridge=*") << QString::fromUtf8("tunnel=*");

    if (geodata_cast<GeoDataLineString>(m_placemark->geometry()) && containsAny(osmData, tags))
    {
        addPattern(filter, osmData, tagsAux);
    }



    // If the placemark is a polygon, recommend "surface=*"
    tags      = QStringList() << QString::fromUtf8("surface=*");

    if (geodata_cast<GeoDataPolygon>(m_placemark->geometry()))
    {
        addPattern(filter, osmData, tags);
    }



    // Always recommend these:
    tags      = QStringList() << QString::fromUtf8("addr:street=*") << QString::fromUtf8("addr:housenumber=*") << QString::fromUtf8("addr:postcode=*") << QString::fromUtf8("addr:country=*") << QString::fromUtf8("access=*");
    addPattern(filter, osmData, tags);


    return filter;
}

bool OsmTagEditorWidgetPrivate::containsAny(const OsmPlacemarkData& osmData, const QStringList& tags)
{
    for (const QString& tag : tags)
    {
        const QStringList tagSplit = tag.split(QLatin1Char('='));

        // Only "key=value" mappings should be checked
        Q_ASSERT(tagSplit.size() == 2);

        QString key = tagSplit.at(0);
        QString value = tagSplit.at(1);

        if (value == QLatin1String("*") && osmData.containsTagKey(key))
        {
            return true;
        }

        else if (value != QLatin1String("*") && osmData.containsTag(key, value))
        {
            return true;
        }
    }

    return false;
}

void OsmTagEditorWidgetPrivate::addPattern(QStringList& filter, const OsmPlacemarkData& osmData, const QStringList& tags)
{
    for (const QString& tag : tags)
    {
        const QStringList tagSplit = tag.split(QLatin1Char('='));
        QString key = tagSplit.at(0);

        if (!osmData.containsTagKey(key))
        {
            filter << key;
        }
    }
}

QVector<OsmTagEditorWidgetPrivate::OsmTag> OsmTagEditorWidgetPrivate::createAdditionalOsmTags()
{
    const QVector<OsmTag> additionalOsmTags = QVector<OsmTag>()

                                              // Recommended for nodes
                                              << OsmTag(QString::fromUtf8("power"), QString::fromUtf8("pole"))
                                              << OsmTag(QString::fromUtf8("power"), QString::fromUtf8("generator"))
                                              << OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("fence"))
                                              << OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("wall"))
                                              << OsmTag(QString::fromUtf8("barrier"), QString::fromUtf8("gate"))

                                              // Recommended for ways
                                              << OsmTag(QString::fromUtf8("lanes"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("maxspeed"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("maxheight"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("maxweight"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("oneway"), QString::fromUtf8("yes"))
                                              << OsmTag(QString::fromUtf8("service"), QString::fromUtf8("driveway"))
                                              << OsmTag(QString::fromUtf8("service"), QString::fromUtf8("parking_aisle"))
                                              << OsmTag(QString::fromUtf8("service"), QString::fromUtf8("alley"))
                                              << OsmTag(QString::fromUtf8("tunnel"), QString::fromUtf8("yes"))
                                              << OsmTag(QString::fromUtf8("abutters"), QString::fromUtf8("commercial"))
                                              << OsmTag(QString::fromUtf8("abutters"), QString::fromUtf8("industrial"))
                                              << OsmTag(QString::fromUtf8("abutters"), QString::fromUtf8("mixed"))
                                              << OsmTag(QString::fromUtf8("abutters"), QString::fromUtf8("residential"))

                                              // Recommended for areas
                                              << OsmTag(QString::fromUtf8("surface"), QString::fromUtf8("unpaved"))
                                              << OsmTag(QString::fromUtf8("surface"), QString::fromUtf8("paved"))
                                              << OsmTag(QString::fromUtf8("surface"), QString::fromUtf8("gravel"))
                                              << OsmTag(QString::fromUtf8("surface"), QString::fromUtf8("dirt"))
                                              << OsmTag(QString::fromUtf8("surface"), QString::fromUtf8("grass"))

                                              // Relations
                                              << OsmTag(QString::fromUtf8("type"), QString::fromUtf8("route"))
                                              << OsmTag(QString::fromUtf8("type"), QString::fromUtf8("route_master"))
                                              << OsmTag(QString::fromUtf8("type"), QString::fromUtf8("public_transport"))
                                              << OsmTag(QString::fromUtf8("type"), QString::fromUtf8("destination_sign"))
                                              << OsmTag(QString::fromUtf8("type"), QString::fromUtf8("waterway"))
                                              << OsmTag(QString::fromUtf8("type"), QString::fromUtf8("enforcement"))

                                              // Relations: route
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("road"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("bicycle"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("foot"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("hiking"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("bus"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("trolleybus"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("ferry"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("detour"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("train"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("tram"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("mtb"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("horse"))
                                              << OsmTag(QString::fromUtf8("route"), QString::fromUtf8("ski"))
                                              << OsmTag(QString::fromUtf8("roundtrip"), QString::fromUtf8("yes"))
                                              << OsmTag(QString::fromUtf8("network"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("ref"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("operator"), QString::fromUtf8(""))

                                              // Relations: route_master
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("train"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("subway"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("monorail"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("tram"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("bus"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("trolleybus"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("ferry"))
                                              << OsmTag(QString::fromUtf8("route_master"), QString::fromUtf8("bicycle"))

                                              // Relations: public_transport
                                              << OsmTag(QString::fromUtf8("public_transport"), QString::fromUtf8("stop_area"))
                                              << OsmTag(QString::fromUtf8("public_transport"), QString::fromUtf8("stop_area_group"))

                                              // Relations: waterway
                                              << OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("river"))
                                              << OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("stream"))
                                              << OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("canal"))
                                              << OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("drain"))
                                              << OsmTag(QString::fromUtf8("waterway"), QString::fromUtf8("ditch"))

                                              // Relations: enforcement
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("maxheight"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("maxweight"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("maxspeed"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("mindistance"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("traffic_signals"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("check"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("access"))
                                              << OsmTag(QString::fromUtf8("enforcement"), QString::fromUtf8("toll"))

                                              // Others
                                              << OsmTag(QString::fromUtf8("height"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("rooms"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("beds"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("wheelchair"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("website"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("email"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("fee"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("destination"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("indoor"), QString::fromUtf8("yes"))

                                              // Recommended for all
                                              << OsmTag(QString::fromUtf8("addr:street"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("addr:housenumber"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("addr:postcode"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("addr:country"), QString::fromUtf8(""))
                                              << OsmTag(QString::fromUtf8("access"), QString::fromUtf8("private"))
                                              << OsmTag(QString::fromUtf8("access"), QString::fromUtf8("permissive"));

    return additionalOsmTags;
}

} // namespace Marble

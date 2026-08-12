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

#include "KmlIconStyleTagWriter.h"

// Local includes

#include "GeoDataIconStyle.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerIconStyle(
    GeoTagWriter::QualifiedName(QString::fromUtf8(GeoDataTypes::GeoDataIconStyleType),
                                QString::fromUtf8(kml::kmlTag_nameSpaceOgc22)),
    new KmlIconStyleTagWriter);

KmlIconStyleTagWriter::KmlIconStyleTagWriter() : KmlColorStyleTagWriter(QString::fromUtf8(kml::kmlTag_IconStyle))
{
    // nothing to do
}

bool KmlIconStyleTagWriter::writeMid(const GeoNode* node, GeoWriter& writer) const
{
    const GeoDataIconStyle* style = static_cast<const GeoDataIconStyle*>(node);

    if (style->scale() != 1.0)
    {
        writer.writeElement(QString::fromUtf8(kml::kmlTag_scale), QString::number(style->scale(), 'f'));
    }

    if (!style->size().isEmpty())
    {
        writer.writeNamespace(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QStringLiteral("mx"));
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_nameSpaceMx), QString::fromUtf8(kml::kmlTag_size));
        writer.writeAttribute(QString::fromUtf8(kml::kmlTag_width), QString::number(style->size().width()));
        writer.writeAttribute(QString::fromUtf8(kml::kmlTag_height), QString::number(style->size().height()));
        writer.writeEndElement();
    }

    if (!style->iconPath().isEmpty())
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_Icon));
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_href));
        writer.writeCharacters(style->iconPath());
        writer.writeEndElement();
        writer.writeEndElement();
    }

    GeoDataHotSpot::Units xunits, yunits;
    QPointF const hotSpot = style->hotSpot(xunits, yunits);
    bool const emptyHotSpot = hotSpot.x() == 0.5 && hotSpot.y() == 0.5 &&
                              xunits == GeoDataHotSpot::Fraction && yunits == GeoDataHotSpot::Fraction;

    if (!emptyHotSpot)
    {
        writer.writeStartElement(QString::fromUtf8(kml::kmlTag_hotSpot));

        if (hotSpot.x() != 0.5 || xunits != GeoDataHotSpot::Fraction)
        {
            writer.writeAttribute(QString::fromUtf8("x"), QString::number(hotSpot.x(), 'f'));
        }

        if (hotSpot.y() != 0.5 || yunits != GeoDataHotSpot::Fraction)
        {
            writer.writeAttribute(QString::fromUtf8("y"), QString::number(hotSpot.y(), 'f'));
        }

        if (xunits != GeoDataHotSpot::Fraction)
        {
            writer.writeAttribute(QString::fromUtf8("xunits"), unitString(xunits));
        }

        if (yunits != GeoDataHotSpot::Fraction)
        {
            writer.writeAttribute(QString::fromUtf8("yunits"), unitString(yunits));
        }

        writer.writeEndElement();
    }

    return true;
}

bool KmlIconStyleTagWriter::isEmpty(const GeoNode* node) const
{
    const GeoDataIconStyle* style = static_cast<const GeoDataIconStyle*>(node);
    GeoDataHotSpot::Units xunits, yunits;
    QPointF const hotSpot = style->hotSpot(xunits, yunits);
    return style->iconPath().isEmpty() &&
           hotSpot.x() == 0.5 &&
           hotSpot.y() == 0.5 &&
           xunits == GeoDataHotSpot::Fraction &&
           yunits == GeoDataHotSpot::Fraction;
}

QString KmlIconStyleTagWriter::unitString(GeoDataHotSpot::Units unit)
{
    switch (unit)
    {
        case GeoDataHotSpot::Pixels:
            return QString::fromUtf8("pixels");

        case GeoDataHotSpot::InsetPixels:
            return QString::fromUtf8("insetPixels");

        case GeoDataHotSpot::Fraction:
            return QString::fromUtf8("fraction");
    }

    return QString::fromUtf8("fraction");
}

} // namespace Marble

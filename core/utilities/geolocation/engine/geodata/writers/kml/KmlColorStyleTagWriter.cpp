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

#include "KmlColorStyleTagWriter.h"

// Local includes

#include "GeoDataTypes.h"
#include "GeoDataColorStyle.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

KmlColorStyleTagWriter::KmlColorStyleTagWriter(const QString& elementName)
    : m_elementName(elementName)
{
    // nothing to do
}

bool KmlColorStyleTagWriter::write(const Marble::GeoNode* node, GeoWriter& writer) const
{
    GeoDataColorStyle const* colorStyle = static_cast<const GeoDataColorStyle*>(node);

    if (colorStyle->id().isEmpty() &&
        colorStyle->targetId().isEmpty() &&
        colorStyle->color() == defaultColor() &&
        colorStyle->colorMode() == GeoDataColorStyle::Normal &&
        isEmpty(node))
    {
        return true;
    }

    writer.writeStartElement(m_elementName);

    KmlObjectTagWriter::writeIdentifiers(writer, colorStyle);
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_color), formatColor(colorStyle->color()), formatColor(defaultColor()));
    QString const colorMode = colorStyle->colorMode() == GeoDataColorStyle::Random ? QString::fromUtf8("random") : QString::fromUtf8("normal");
    writer.writeOptionalElement(QString::fromUtf8(kml::kmlTag_colorMode), colorMode, QString::fromUtf8("normal"));

    bool const result = writeMid(node, writer);
    writer.writeEndElement();
    return result;
}

QString KmlColorStyleTagWriter::formatColor(const QColor& color)
{
    QChar const fill = QLatin1Char('0');
    return QString::fromUtf8("%1%2%3%4")
           .arg(color.alpha(), 2, 16, fill)
           .arg(color.blue(), 2, 16, fill)
           .arg(color.green(), 2, 16, fill)
           .arg(color.red(), 2, 16, fill);
}

QColor KmlColorStyleTagWriter::defaultColor() const
{
    return QColor(Qt::white);
}

} // namespace Marble

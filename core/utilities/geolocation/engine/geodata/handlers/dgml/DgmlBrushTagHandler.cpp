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

#include "DgmlBrushTagHandler.h"

// Qt includes

#include <QBrush>
#include <QColor>
#include <QList>
#include <QString>

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "GeoParser.h"
#include "GeoSceneGeodata.h"

namespace Marble
{

namespace dgml
{

DGML_DEFINE_TAG_HANDLER(Brush)

GeoNode* DgmlBrushTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Brush)));

    QString color = parser.attribute(dgmlAttr_color).trimmed();
    QString colorMap = parser.attribute(dgmlAttr_colorMap).trimmed();
    qreal alpha = parser.attribute(dgmlAttr_alpha).isEmpty() ? 1.0 : parser.attribute(dgmlAttr_alpha).toDouble();

    QBrush brush;

    if (!color.isEmpty() && QColor(color).isValid())
    {
        QColor brushColor(color);

        if (color.contains(QLatin1String("transparent")))
        {
            brushColor.setAlphaF(0.0);
        }

        else
        {
            brushColor.setAlphaF(alpha);
        }

        brush.setColor(brushColor);
    }

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Vector)
        || parentItem.represents(dgmlTag_Geodata))
    {
        GeoSceneGeodata* geodata = parentItem.nodeAs<GeoSceneGeodata>();
        geodata->setBrush(brush);

        if (!colorMap.isEmpty())
        {
            const QStringList colorString = colorMap.split(QLatin1Char(','));

            QVector<QColor> colorList;
            colorList.reserve(colorString.size());

            for (const QString& colorName : colorString)
            {
                colorList.append(QColor(colorName));
            }

            geodata->setColors(colorList);
        }

        geodata->setAlpha(alpha);
    }

    return nullptr;
}

} // namespace dgml

} // namespace Marble

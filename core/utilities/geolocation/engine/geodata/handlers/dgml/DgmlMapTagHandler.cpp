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

#include "DgmlMapTagHandler.h"

// Qt includes

#include <QColor>

// Local includes

#include "DgmlElementDictionary.h"
#include "DgmlAttributeDictionary.h"
#include "GeoParser.h"
#include "GeoSceneDocument.h"
#include "GeoSceneMap.h"

namespace Marble
{
namespace dgml
{
DGML_DEFINE_TAG_HANDLER(Map)

GeoNode* DgmlMapTagHandler::parse(GeoParser& parser) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Map)));

    QColor  labelColor  = parser.attribute(dgmlAttr_labelColor).trimmed();

    if (!labelColor.isValid())
    {
        labelColor = Qt::black;
    }

    QColor highlightBrushColor = QColor(parser.attribute(dgmlAttr_highlightBrush).trimmed());
    QColor highlightPenColor = QColor(parser.attribute(dgmlAttr_highlightPen).trimmed());

    GeoSceneMap* map = nullptr;

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Document))
    {
        map = parentItem.nodeAs<GeoSceneDocument>()->map();
        map->setBackgroundColor(QColor(parser.attribute(dgmlAttr_bgcolor).trimmed()));
        map->setLabelColor(labelColor);
        map->setHighlightBrushColor(highlightBrushColor);
        map->setHighlightPenColor(highlightPenColor);
    }

    return map;
}

} // namespace dgml

} // namespace Marble

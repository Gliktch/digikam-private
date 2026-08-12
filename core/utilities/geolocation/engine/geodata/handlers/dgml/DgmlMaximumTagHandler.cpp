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

#include "DgmlMaximumTagHandler.h"

// Local includes

#include "DgmlElementDictionary.h"
#include "GeoParser.h"
#include "GeoSceneZoom.h"
#include <klocalizedstring.h>

namespace Marble
{
namespace dgml
{
DGML_DEFINE_TAG_HANDLER(Maximum)

GeoNode* DgmlMaximumTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_Maximum)));

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (parentItem.represents(dgmlTag_Zoom))
    {
        QString parsedText = parser.readElementText();

        bool ok = false;
        int parsedInt = parsedText.toInt(&ok);

        if (ok)
        {
            parentItem.nodeAs<GeoSceneZoom>()->setMaximum(parsedInt);
        }

        else
        {
            parser.raiseWarning(i18n("Could not convert <maximum> child text content to integer. Was: '%1'", parsedText));
        }
    }

    return nullptr;
}

} // namespace dgml

} // namespace Marble

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

#include "DgmlDownloadPolicyTagHandler.h"

// Local includes

#include "DgmlAttributeDictionary.h"
#include "DgmlElementDictionary.h"
#include "GeoParser.h"
#include "GeoSceneTileDataset.h"
#include "MarbleGlobal.h"
#include "digikam_debug.h"

namespace Marble
{
namespace dgml
{
static GeoTagHandlerRegistrar handler(GeoParser::QualifiedName(QString::fromUtf8(dgmlTag_DownloadPolicy),
                                                               QString::fromUtf8(dgmlTag_nameSpace20)),
                                      new DgmlDownloadPolicyTagHandler);

// Error handling:
// Here it is not possible to return an error code or throw an exception
// so in case of an error we just ignore the element.

GeoNode* DgmlDownloadPolicyTagHandler::parse(GeoParser& parser) const
{
    // Check whether the tag is valid
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(dgmlTag_DownloadPolicy)));

    // Checking for parent item
    GeoStackItem parentItem = parser.parentElement();

    if (!parentItem.represents(dgmlTag_Texture) && !parentItem.represents(dgmlTag_Vectortile))
    {
        qCCritical(DIGIKAM_GEOENGINE_LOG) << "Parse error: parent element is not 'texture' or 'vectortile'";
        return nullptr;
    }

    // Attribute usage
    DownloadUsage usage;
    const QString usageStr = parser.attribute(dgmlAttr_usage).trimmed();

    if (usageStr == QLatin1String("Browse"))
    {
        usage = DownloadBrowse;
    }

    else if (usageStr == QLatin1String("Bulk"))
    {
        usage = DownloadBulk;
    }

    else
    {
        qCCritical(DIGIKAM_GEOENGINE_LOG) << "Parse error: invalid attribute downloadPolicy/@usage";
        return nullptr;
    }

    // Attribute maximumConnections
    const QString maximumConnectionsStr = parser.attribute(dgmlAttr_maximumConnections).trimmed();
    bool ok;
    const int maximumConnections = maximumConnectionsStr.toInt(&ok);

    if (!ok)
    {
        qCCritical(DIGIKAM_GEOENGINE_LOG) << "Parse error: invalid attribute downloadPolicy/@maximumConnections";
        return nullptr;
    }

    parentItem.nodeAs<GeoSceneTileDataset>()->addDownloadPolicy(usage, maximumConnections);
    return nullptr;
}

} // namespace dgml

} // namespace Marble

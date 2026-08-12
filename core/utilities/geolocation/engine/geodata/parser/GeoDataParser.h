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

#pragma once

// Local includes

#include "GeoParser.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDocument;
class GeoDataDocument;

enum GeoDataSourceType
{
    GeoData_UNKNOWN = -1,
    GeoData_KML     = 1,
    GeoData_GeoRSS  = 2
};

class DIGIKAM_EXPORT GeoDataParser : public GeoParser
{
public:

    explicit GeoDataParser(GeoDataSourceType source);
    ~GeoDataParser() override;

private:

    bool isValidElement(const QString& tagName) const override;
    bool isValidRootElement() override;

    GeoDocument* createDocument() const override;
};

// Global helper function for the tag handlers
DIGIKAM_EXPORT GeoDataDocument* geoDataDoc(GeoParser& parser);

} // namespace Marble

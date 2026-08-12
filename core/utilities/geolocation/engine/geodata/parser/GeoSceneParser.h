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
class GeoSceneDocument;

enum GeoSceneSourceType
{
    GeoScene_DGML   = 0
};

class DIGIKAM_EXPORT GeoSceneParser : public GeoParser
{
public:

    explicit GeoSceneParser(GeoSceneSourceType source);
    ~GeoSceneParser() override;

private:

    bool isValidElement(const QString& tagName) const override;
    bool isValidRootElement() override;

    GeoDocument* createDocument() const override;
};

// Global helper function for the tag handlers
GeoSceneDocument* geoSceneDoc(GeoParser& parser);

} // namespace Marble

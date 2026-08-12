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

#include "GeoDataObject.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataImagePyramidPrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataImagePyramid : public GeoDataObject
{
public:

    GeoDataImagePyramid();

    GeoDataImagePyramid(const GeoDataImagePyramid& other);

    GeoDataImagePyramid& operator=(const GeoDataImagePyramid& other);
    bool operator==(const GeoDataImagePyramid& other) const;
    bool operator!=(const GeoDataImagePyramid& other) const;

    ~GeoDataImagePyramid() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    enum GridOrigin
    {
        LowerLeft,
        UpperLeft
    };

    int tileSize() const;
    void setTileSize(int tileSize);

    int maxWidth() const;
    void setMaxWidth(int maxWidth);

    int maxHeight() const;
    void setMaxHeight(int maxHeight);

    GridOrigin gridOrigin() const;
    void setGridOrigin(GridOrigin gridOrigin);

private:

    GeoDataImagePyramidPrivate* const d = nullptr;
};

} // namespace Marble

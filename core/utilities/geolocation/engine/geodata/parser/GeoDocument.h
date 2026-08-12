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

#include "digikam_export.h"

namespace Marble
{

/**
 * @short A shared base class between GeoDataDocument/GeoSourceDocument
 */
class DIGIKAM_EXPORT GeoDocument
{
protected:

    GeoDocument();

public:

    virtual ~GeoDocument();

    virtual bool isGeoDataDocument() const;
    virtual bool isGeoSceneDocument() const;
};

/**
 * @short A shared base class for all classes that are mapped to a specific tag (ie. GeoFolder)
 */
class DIGIKAM_EXPORT GeoNode
{
public:

    GeoNode();
    virtual ~GeoNode();

    /// Provides type information for downcasting a GeoNode
    virtual const char* nodeType() const = 0;
};

} // namespace Marble

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

#include "GeoDataContainer.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataCreatePrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataCreate : public GeoDataContainer
{
public:

    GeoDataCreate();

    GeoDataCreate(const GeoDataCreate& other);

    ~GeoDataCreate() override;

    GeoDataCreate& operator=(const GeoDataCreate& other);

    bool operator==(const GeoDataCreate& other) const;
    bool operator!=(const GeoDataCreate& other) const;

    GeoDataFeature* clone() const override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

private:

    Q_DECLARE_PRIVATE(GeoDataCreate)
};

} // namespace Marble

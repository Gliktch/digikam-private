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

#include "GeoDocument.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataAlias;
class GeoDataResourceMapPrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataResourceMap: public GeoNode
{
public:

    GeoDataResourceMap();

    GeoDataResourceMap(const GeoDataResourceMap& other);

    GeoDataResourceMap& operator=(const GeoDataResourceMap& other);

    bool operator==(const GeoDataResourceMap& other) const;
    bool operator!=(const GeoDataResourceMap& other) const;

    ~GeoDataResourceMap() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    const GeoDataAlias& alias() const;

    GeoDataAlias& alias();

    void setAlias(const GeoDataAlias& alias);

    QString sourceHref() const;

    void setSourceHref(const QString& sourceHref);

    QString targetHref() const;

    void setTargetHref(const QString& targetHref);

private:

    GeoDataResourceMapPrivate* const d = nullptr;
};

} // namespace Marble

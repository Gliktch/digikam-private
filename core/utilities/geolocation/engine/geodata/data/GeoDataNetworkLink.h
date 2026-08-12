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

#include "GeoDataFeature.h"
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataLink;
class GeoDataNetworkLinkPrivate;

class DIGIKAM_EXPORT GeoDataNetworkLink: public GeoDataFeature
{
public:

    GeoDataNetworkLink();

    GeoDataNetworkLink(const GeoDataNetworkLink& other);

    GeoDataNetworkLink& operator=(const GeoDataNetworkLink& other);

    bool operator==(const GeoDataNetworkLink& other) const;
    bool operator!=(const GeoDataNetworkLink& other) const;

    ~GeoDataNetworkLink() override;

    GeoDataFeature* clone() const override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    bool refreshVisibility() const;

    void setRefreshVisibility(bool refreshVisibility);

    bool flyToView() const;

    void setFlyToView(bool flyToView);

    GeoDataLink& link();

    const GeoDataLink& link() const;

    void setLink(const GeoDataLink& link);

private:

    GeoDataNetworkLinkPrivate* const d = nullptr;
};

} // namespace Marble

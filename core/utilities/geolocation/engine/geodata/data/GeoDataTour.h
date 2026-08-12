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
#include "digikam_export.h"

namespace Marble
{

class GeoDataTourPrivate;
class GeoDataPlaylist;
class GeoDataSoundCue;

/**
 */
class DIGIKAM_EXPORT GeoDataTour : public GeoDataFeature
{
public:

    GeoDataTour();
    GeoDataTour(const GeoDataTour& other);
    ~GeoDataTour()                                            override;

    GeoDataTour& operator=(const GeoDataTour& other);

    bool operator==(const GeoDataTour& other)           const;
    bool operator!=(const GeoDataTour& other)           const;

    GeoDataFeature* clone()                             const override;

    GeoDataPlaylist* playlist();
    const GeoDataPlaylist* playlist()                   const;
    void setPlaylist(GeoDataPlaylist* playlist);

    const char* nodeType()                              const override;

    static const GeoDataTour null;

private:

    Q_DECLARE_PRIVATE(GeoDataTour)
};

} // namespace Marble

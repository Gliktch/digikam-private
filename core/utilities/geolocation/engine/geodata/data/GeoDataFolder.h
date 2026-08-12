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

class GeoDataFolderPrivate;

/**
 * @short A container that is used to arrange other GeoDataFeatures.
 *
 * A GeoDataFolder is used to arrange other GeoDataFeatures
 * hierarchically (Folders, Placemarks, NetworkLinks, or Overlays). A
 * GeoDataFeature is visible only if it and all its ancestors are
 * visible.
 *
 * @see GeoDataFeature
 * @see GeoDataContainer
 */
class DIGIKAM_EXPORT GeoDataFolder : public GeoDataContainer
{
public:

    GeoDataFolder();
    GeoDataFolder(const GeoDataFolder& other);
    ~GeoDataFolder() override;

    GeoDataFolder& operator=(const GeoDataFolder& other);

    bool operator==(const GeoDataFolder& other) const;
    bool operator!=(const GeoDataFolder& other) const;

    const char* nodeType() const override;

    GeoDataFeature* clone() const override;

private:

    Q_DECLARE_PRIVATE(GeoDataFolder)
};

} // namespace Marble

Q_DECLARE_METATYPE(Marble::GeoDataFolder*)

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Merges tiles into groups
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "geoifacecommon.h"

namespace Digikam
{
class MapBackend;

class DIGIKAM_EXPORT TileGrouper : public QObject
{
    Q_OBJECT

public:

    explicit TileGrouper(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData,
                         QObject* const parent);
    ~TileGrouper() override;

    void setClustersDirty();
    bool getClustersDirty() const;
    void updateClusters();
    void setCurrentBackend(MapBackend* const backend);

private:

    bool currentBackendReady();

private:

    class Private;
    const QScopedPointer<Private>                          d;

    const QExplicitlySharedDataPointer<GeoIfaceSharedData> s;
};

} // namespace Digikam

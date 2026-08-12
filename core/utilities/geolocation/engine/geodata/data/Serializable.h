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

class QDataStream;

namespace Marble
{

class Serializable
{
public:

    virtual ~Serializable() {}
    virtual void pack(QDataStream& stream) const = 0;
    virtual void unpack(QDataStream& stream) = 0;
};

/**
 * The next two id's are needed to get unpacking working - this cannot be
 * achieved without a special Id
 */
enum EnumFeatureId
{
    InvalidFeatureId  = -1,
    GeoDataDocumentId = 1,
    GeoDataFolderId,
    GeoDataPlacemarkId,
    GeoDataNetworkLinkId,
    GeoDataScreenOverlayId,
    GeoDataGroundOverlayId
};

enum EnumGeometryId
{
    InvalidGeometryId = -1,
    GeoDataPointId    = 1,
    GeoDataLineStringId,
    GeoDataLinearRingId,
    GeoDataPolygonId,
    GeoDataMultiGeometryId,
    GeoDataMultiTrackId,
    GeoDataModelId,
    GeoDataTrackId,
    GeoDataBuildingId
};

} // namespace Marble

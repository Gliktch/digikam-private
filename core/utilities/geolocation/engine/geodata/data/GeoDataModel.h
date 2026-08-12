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

#include "GeoDataGeometry.h"
#include "MarbleGlobal.h"
#include "GeoDataLink.h"

namespace Marble
{

class GeoDataCoordinates;
class GeoDataLink;
class GeoDataLocation;
class GeoDataOrientation;
class GeoDataResourceMap;
class GeoDataScale;
class GeoDataModelPrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataModel: public GeoDataGeometry
{
public:

    GeoDataModel();

    GeoDataModel(const GeoDataModel& other);

    GeoDataModel& operator=(const GeoDataModel& other);

    const char* nodeType() const override;

    EnumGeometryId geometryId() const override;

    GeoDataGeometry* copy() const override;

    bool operator==(const GeoDataModel& other) const;
    bool operator!=(const GeoDataModel& other) const;

    ~GeoDataModel() override;

    const GeoDataLink& link() const;
    GeoDataLink& link();
    void setLink(const GeoDataLink& link);

    const GeoDataCoordinates& coordinates() const;
    GeoDataCoordinates& coordinates();
    void setCoordinates(const GeoDataCoordinates& coordinates);

    const GeoDataScale& scale() const;
    GeoDataScale& scale();
    void setScale(const GeoDataScale& scale);

    const GeoDataOrientation& orientation() const;
    GeoDataOrientation& orientation();
    void setOrientation(const GeoDataOrientation& orientation);

    const GeoDataLocation& location() const;
    GeoDataLocation& location();
    void setLocation(const GeoDataLocation& location);

    const GeoDataResourceMap& resourceMap() const;
    GeoDataResourceMap& resourceMap();
    void setResourceMap(const GeoDataResourceMap& map);

    QString sourceHref() const;
    QString targetHref() const;
    void setSourceHref(const QString& sourceHref);
    void setTargetHref(const QString& targetHref);

private:

    Q_DECLARE_PRIVATE(GeoDataModel)
};

} // namespace Marble

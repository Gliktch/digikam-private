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

class GeoDataChange;
class GeoDataCreate;
class GeoDataDelete;
class GeoDataUpdatePrivate;

/**
 */
class DIGIKAM_EXPORT GeoDataUpdate : public GeoDataObject
{
public:
    GeoDataUpdate();

    GeoDataUpdate(const GeoDataUpdate& other);

    GeoDataUpdate& operator=(const GeoDataUpdate& other);
    bool operator==(const GeoDataUpdate& other) const;
    bool operator!=(const GeoDataUpdate& other) const;

    ~GeoDataUpdate() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    QString targetHref() const;
    void setTargetHref(const QString& targetHref);

    const GeoDataChange* change() const;
    GeoDataChange* change();
    void setChange(GeoDataChange* change);

    const GeoDataCreate* create() const;
    GeoDataCreate* create();
    void setCreate(GeoDataCreate* create);

    const GeoDataDelete* getDelete() const;
    GeoDataDelete* getDelete();
    void setDelete(GeoDataDelete* dataDelete);

private:

    GeoDataUpdatePrivate* const d = nullptr;
};

} // namespace Marble

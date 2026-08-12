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

class GeoDataAliasPrivate;

class DIGIKAM_EXPORT GeoDataAlias: public GeoNode
{
public:

    GeoDataAlias();

    GeoDataAlias(const GeoDataAlias& other);

    GeoDataAlias& operator=(const GeoDataAlias& other);

    bool operator==(const GeoDataAlias& other) const;

    bool operator!=(const GeoDataAlias& other) const;

    ~GeoDataAlias() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    QString sourceHref() const;

    void setSourceHref(const QString& sourceHref);

    QString targetHref() const;

    void setTargetHref(const QString& targetHref);

private:

    GeoDataAliasPrivate* const d = nullptr;
};

} // namespace Marble

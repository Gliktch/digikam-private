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
#include "MarbleGlobal.h"

namespace Marble
{

class GeoDataLinkPrivate;

class DIGIKAM_EXPORT GeoDataLink: public GeoDataObject
{
public:

    enum RefreshMode
    {
        OnChange,
        OnInterval,
        OnExpire
    };

    enum ViewRefreshMode
    {
        Never,
        OnStop,
        OnRequest,
        OnRegion
    };

    GeoDataLink();

    GeoDataLink(const GeoDataLink& other);

    GeoDataLink& operator=(const GeoDataLink& other);
    bool operator==(const GeoDataLink& other) const;
    bool operator!=(const GeoDataLink& other) const;

    ~GeoDataLink() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    QString href() const;

    void setHref(const QString& href);

    RefreshMode refreshMode() const;

    void setRefreshMode(RefreshMode refreshMode);

    qreal refreshInterval() const;

    void setRefreshInterval(qreal refreshInterval);

    ViewRefreshMode viewRefreshMode() const;

    void setViewRefreshMode(ViewRefreshMode refreshMode);

    qreal viewRefreshTime() const;

    void setViewRefreshTime(qreal viewRefreshTime);

    qreal viewBoundScale() const;

    void setViewBoundScale(qreal viewBoundScale);

    QString viewFormat() const;

    void setViewFormat(const QString& viewFormat);

    QString httpQuery() const;

    void setHttpQuery(const QString& httpQuery);

private:

    GeoDataLinkPrivate* const d = nullptr;
};

} // namespace Marble

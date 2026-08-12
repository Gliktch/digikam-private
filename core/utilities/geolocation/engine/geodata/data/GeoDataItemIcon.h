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

// Qt includes

#include <QFlags>

// Local includes

#include "GeoDataObject.h"

class QImage;

namespace Marble
{

class GeoDataItemIconPrivate;

class DIGIKAM_EXPORT GeoDataItemIcon : public GeoDataObject
{
    Q_FLAGS(ItemIconState ItemIconStates)

public:

    GeoDataItemIcon();

    GeoDataItemIcon(const GeoDataItemIcon& other);

    GeoDataItemIcon& operator=(const GeoDataItemIcon& other);
    bool operator==(const GeoDataItemIcon& other) const;
    bool operator!=(const GeoDataItemIcon& other) const;

    ~GeoDataItemIcon() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    enum ItemIconState
    {
        Open      = 1 << 0,
        Closed    = 1 << 1,
        Error     = 1 << 2,
        Fetching0 = 1 << 3,
        Fetching1 = 1 << 4,
        Fetching2 = 1 << 5
    };
    Q_DECLARE_FLAGS(ItemIconStates, ItemIconState)

    ItemIconStates state() const;
    void setState(ItemIconStates state);

    QString iconPath() const;
    void setIconPath(const QString& path);

    QImage icon() const;
    void setIcon(const QImage& icon);

private:

    GeoDataItemIconPrivate* const d = nullptr;
};

} // namespace Marble

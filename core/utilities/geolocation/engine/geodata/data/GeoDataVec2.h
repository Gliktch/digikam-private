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

#include <QPointF>

// Local includes

#include "digikam_export.h"

class QString;

namespace Marble
{

class GeoDataVec2Private;

class DIGIKAM_EXPORT GeoDataVec2 : public QPointF
{
public:

    enum Unit {Fraction, Pixels, InsetPixels};

    GeoDataVec2();

    GeoDataVec2(qreal x, qreal y, const QString& xunit, const QString& yunit);

    GeoDataVec2(const GeoDataVec2& other);

    GeoDataVec2& operator=(const GeoDataVec2& other);
    bool operator==(const GeoDataVec2& other) const;
    bool operator!=(const GeoDataVec2& other) const;
    ~GeoDataVec2();

    Unit xunit() const;
    void setXunits(Unit xunit);

    Unit yunit() const;
    void setYunits(Unit yunit);

private:

    GeoDataVec2Private* const d = nullptr;
};

} // namespace Marble

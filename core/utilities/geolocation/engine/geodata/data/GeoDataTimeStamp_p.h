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

#include <QDateTime>

// Local includes

#include "GeoDataTypes.h"
#include "GeoDataTimeStamp.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataTimeStampPrivate
{
public:

    QDateTime m_when;

    GeoDataTimeStamp::TimeResolution m_resolution;

    GeoDataTimeStampPrivate();
};

GeoDataTimeStampPrivate::GeoDataTimeStampPrivate()
    : m_resolution(GeoDataTimeStamp::SecondResolution)
{
    // nothing to do
}

} // namespace Marble


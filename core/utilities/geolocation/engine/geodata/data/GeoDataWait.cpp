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

#include "GeoDataWait.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

GeoDataWait::GeoDataWait()
    : m_duration(0.0)
{
}

GeoDataWait::~GeoDataWait()
{
}

bool GeoDataWait::operator==(const GeoDataWait& other) const
{
    return this->m_duration == other.m_duration;
}

bool GeoDataWait::operator!=(const GeoDataWait& other) const
{
    return !this->operator==(other);
}

const char* GeoDataWait::nodeType() const
{
    return GeoDataTypes::GeoDataWaitType;
}

double GeoDataWait::duration() const
{
    return m_duration;
}

void GeoDataWait::setDuration(double duration)
{
    m_duration = duration;
}

} // namespace Marble

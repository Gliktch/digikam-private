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

#include "GeoDataTourControl.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

GeoDataTourControl::GeoDataTourControl()
    : m_playMode(Play)
{
}

GeoDataTourControl::~GeoDataTourControl()
{
}

bool GeoDataTourControl::operator==(const GeoDataTourControl& other) const
{
    return this->m_playMode == other.m_playMode;
}

bool GeoDataTourControl::operator!=(const GeoDataTourControl& other) const
{
    return !this->operator==(other);
}

const char* GeoDataTourControl::nodeType() const
{
    return GeoDataTypes::GeoDataTourControlType;
}

GeoDataTourControl::PlayMode GeoDataTourControl::playMode() const
{
    return m_playMode;
}

void GeoDataTourControl::setPlayMode(PlayMode mode)
{
    m_playMode = mode;
}

} // namespace Marble

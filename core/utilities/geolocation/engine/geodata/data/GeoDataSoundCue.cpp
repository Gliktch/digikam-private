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

#include "GeoDataSoundCue.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

GeoDataSoundCue::GeoDataSoundCue()
    : m_delayedStart(0)
{
}

bool GeoDataSoundCue::operator==(const GeoDataSoundCue& other) const
{
    return this->m_delayedStart == other.m_delayedStart && this->m_href == other.m_href;
}

bool GeoDataSoundCue::operator!=(const GeoDataSoundCue& other) const
{
    return !this->operator==(other);
}

GeoDataSoundCue::~GeoDataSoundCue()
{
}

const char* GeoDataSoundCue::nodeType() const
{
    return GeoDataTypes::GeoDataSoundCueType;
}

QString GeoDataSoundCue::href() const
{
    return m_href;
}

void GeoDataSoundCue::setHref(const QString& url)
{
    m_href = url;
}

double GeoDataSoundCue::delayedStart() const
{
    return m_delayedStart;
}

void GeoDataSoundCue::setDelayedStart(double pause)
{
    m_delayedStart = pause;
}

} // namespace Marble

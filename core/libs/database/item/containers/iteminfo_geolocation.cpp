/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data - Geolocation
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfo_p.h"

namespace Digikam
{

ItemPosition ItemInfo::imagePosition() const
{
    if (!m_data)
    {
        return ItemPosition();
    }

    ItemPosition pos(m_data->id);

    if (!m_data->positionsCached)
    {
        ItemInfoWriteLocker lock;
        m_data.data()->longitude       = pos.longitudeNumber();
        m_data.data()->latitude        = pos.latitudeNumber();
        m_data.data()->altitude        = pos.altitude();
        m_data.data()->hasCoordinates  = pos.hasCoordinates();
        m_data.data()->hasAltitude     = pos.hasAltitude();
        m_data.data()->positionsCached = true;
    }

    return pos;
}

double ItemInfo::longitudeNumber() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->longitude;
}

double ItemInfo::latitudeNumber() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->latitude;
}

double ItemInfo::altitudeNumber() const
{
    if (!m_data)
    {
        return 0;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->altitude;
}

bool ItemInfo::hasCoordinates() const
{
    if (!m_data)
    {
        return false;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->hasCoordinates;
}

bool ItemInfo::hasAltitude() const
{
    if (!m_data)
    {
        return false;
    }

    if (!m_data->positionsCached)
    {
        imagePosition();
    }

    return m_data->hasAltitude;
}

} // namespace Digikam

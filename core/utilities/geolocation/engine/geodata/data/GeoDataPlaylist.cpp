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

#include "GeoDataPlaylist.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

bool GeoDataPlaylist::operator==(const GeoDataPlaylist& other) const
{
    if (this->m_primitives.size() != other.m_primitives.size())
    {
        return false;
    }
    else
    {
        int index = 0;

        for (const GeoDataTourPrimitive* const m_primitive : m_primitives)
        {
            if (*m_primitive != *other.m_primitives.at(index))
            {
                return false;
            }

            index++;
        }

        return true;
    }
}

bool GeoDataPlaylist::operator!=(const GeoDataPlaylist& other) const
{
    return !this->operator==(other);
}

const char* GeoDataPlaylist::nodeType() const
{
    return GeoDataTypes::GeoDataPlaylistType;
}

GeoDataTourPrimitive* GeoDataPlaylist::primitive(int id)
{
    if ((size() <= id) || (id < 0))
    {
        return nullptr;
    }

    return m_primitives.at(id);
}

const GeoDataTourPrimitive* GeoDataPlaylist::primitive(int id) const
{
    if ((size() <= id) || (id < 0))
    {
        return nullptr;
    }

    return m_primitives.at(id);
}

void GeoDataPlaylist::addPrimitive(GeoDataTourPrimitive* primitive)
{
    primitive->setParent(this);
    m_primitives.push_back(primitive);
}

void GeoDataPlaylist::insertPrimitive(int position, GeoDataTourPrimitive* primitive)
{
    primitive->setParent(this);
    int const index = qBound(0, position, m_primitives.size());
    m_primitives.insert(index, primitive);
}

void GeoDataPlaylist::removePrimitiveAt(int position)
{
    m_primitives.removeAt(position);
}

void GeoDataPlaylist::swapPrimitives(int positionA, int positionB)
{
    if ((qMin(positionA, positionB) >= 0) && (qMax(positionA, positionB) < m_primitives.size()))
    {
        m_primitives.swapItemsAt(positionA, positionB);
    }
}

int GeoDataPlaylist::size() const
{
    return m_primitives.size();
}

} // namespace Marble

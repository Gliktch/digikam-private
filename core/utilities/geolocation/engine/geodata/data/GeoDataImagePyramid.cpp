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

#include "GeoDataImagePyramid.h"

// Local includes

#include "GeoDataTypes.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataImagePyramidPrivate
{
public:

    GeoDataImagePyramidPrivate();

    int m_tileSize;
    int m_maxWidth;
    int m_maxHeight;
    GeoDataImagePyramid::GridOrigin m_gridOrigin;
};

GeoDataImagePyramidPrivate::GeoDataImagePyramidPrivate() :
    m_tileSize(256),
    m_maxWidth(),
    m_maxHeight(),
    m_gridOrigin()
{
    // nothing to do
}

GeoDataImagePyramid::GeoDataImagePyramid() : d(new GeoDataImagePyramidPrivate)
{
    // nothing to do
}

GeoDataImagePyramid::GeoDataImagePyramid(const Marble::GeoDataImagePyramid& other) :
    GeoDataObject(), d(new GeoDataImagePyramidPrivate(*other.d))
{
    // nothing to do
}

GeoDataImagePyramid& GeoDataImagePyramid::operator=(const GeoDataImagePyramid& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;
    return *this;
}

bool GeoDataImagePyramid::operator==(const GeoDataImagePyramid& other) const
{
    return equals(other) &&
           d->m_tileSize == other.d->m_tileSize &&
           d->m_maxWidth == other.d->m_maxWidth &&
           d->m_maxHeight == other.d->m_maxHeight &&
           d->m_gridOrigin == other.d->m_gridOrigin;
}

bool GeoDataImagePyramid::operator!=(const GeoDataImagePyramid& other) const
{
    return !this->operator==(other);
}

GeoDataImagePyramid::~GeoDataImagePyramid()
{
    delete d;
}

const char* GeoDataImagePyramid::nodeType() const
{
    return GeoDataTypes::GeoDataImagePyramidType;
}

int GeoDataImagePyramid::tileSize() const
{
    return d->m_tileSize;
}

void GeoDataImagePyramid::setTileSize(int tileSize)
{
    d->m_tileSize = tileSize;
}

int GeoDataImagePyramid::maxWidth() const
{
    return d->m_maxWidth;
}

void GeoDataImagePyramid::setMaxWidth(int maxWidth)
{
    d->m_maxWidth = maxWidth;
}

int GeoDataImagePyramid::maxHeight() const
{
    return d->m_maxHeight;
}

void GeoDataImagePyramid::setMaxHeight(int maxHeight)
{
    d->m_maxHeight = maxHeight;
}

GeoDataImagePyramid::GridOrigin GeoDataImagePyramid::gridOrigin() const
{
    return d->m_gridOrigin;
}

void GeoDataImagePyramid::setGridOrigin(GridOrigin gridOrigin)
{
    d->m_gridOrigin = gridOrigin;
}

} // namespace Marble

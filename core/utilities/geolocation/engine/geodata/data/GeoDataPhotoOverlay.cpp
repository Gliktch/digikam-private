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

#include "GeoDataPhotoOverlay.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#endif

#include "GeoDataPhotoOverlay_p.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

namespace Marble
{

GeoDataPhotoOverlay::GeoDataPhotoOverlay()
    : GeoDataOverlay(new GeoDataPhotoOverlayPrivate)
{
    // nothing to do
}

GeoDataPhotoOverlay::GeoDataPhotoOverlay(const GeoDataPhotoOverlay& other)
    : GeoDataOverlay(other, new GeoDataPhotoOverlayPrivate(*other.d_func()))
{
    // nothing to do
}

GeoDataPhotoOverlay::~GeoDataPhotoOverlay()
{
}

GeoDataPhotoOverlay& GeoDataPhotoOverlay::operator=(const GeoDataPhotoOverlay& other)
{
    if (this != &other)
    {
        Q_D(GeoDataPhotoOverlay);
        *d = *other.d_func();
    }

    return *this;
}

bool GeoDataPhotoOverlay::operator==(const GeoDataPhotoOverlay& other) const
{
    Q_D(const GeoDataPhotoOverlay);
    const GeoDataPhotoOverlayPrivate* const other_d = other.d_func();

    return equals(other) &&
           d->m_rotation == other_d->m_rotation &&
           d->m_shape == other_d->m_shape &&
           d->m_imagePyramid == other_d->m_imagePyramid &&
           d->m_point == other_d->m_point &&
           d->m_viewVolume == other_d->m_viewVolume;
}

bool GeoDataPhotoOverlay::operator!=(const GeoDataPhotoOverlay& other) const
{
    return (! this->operator == (other));
}

GeoDataFeature* GeoDataPhotoOverlay::clone() const
{
    return (new GeoDataPhotoOverlay(*this));
}

const char* GeoDataPhotoOverlay::nodeType() const
{
    Q_D(const GeoDataPhotoOverlay);
    Q_UNUSED(d);
    return GeoDataTypes::GeoDataPhotoOverlayType;
}

qreal GeoDataPhotoOverlay::rotation() const
{
    Q_D(const GeoDataPhotoOverlay);
    return d->m_rotation;
}

void GeoDataPhotoOverlay::setRotation(const qreal rotation)
{
    Q_D(GeoDataPhotoOverlay);
    d->m_rotation = rotation;
}

GeoDataViewVolume& GeoDataPhotoOverlay::viewVolume()
{
    Q_D(GeoDataPhotoOverlay);
    return d->m_viewVolume;
}

const GeoDataViewVolume& GeoDataPhotoOverlay::viewVolume() const
{
    Q_D(const GeoDataPhotoOverlay);
    return d->m_viewVolume;
}

void GeoDataPhotoOverlay::setViewVolume(const GeoDataViewVolume& viewVolume)
{
    Q_D(GeoDataPhotoOverlay);
    d->m_viewVolume = viewVolume;
}

GeoDataImagePyramid& GeoDataPhotoOverlay::imagePyramid()
{
    Q_D(GeoDataPhotoOverlay);
    return d->m_imagePyramid;
}

const GeoDataImagePyramid& GeoDataPhotoOverlay::imagePyramid() const
{
    Q_D(const GeoDataPhotoOverlay);
    return d->m_imagePyramid;
}

void GeoDataPhotoOverlay::setImagePyramid(const GeoDataImagePyramid& imagePyramid)
{
    Q_D(GeoDataPhotoOverlay);
    d->m_imagePyramid = imagePyramid;
}

GeoDataPoint& GeoDataPhotoOverlay::point()
{
    Q_D(GeoDataPhotoOverlay);
    return d->m_point;
}

const GeoDataPoint& GeoDataPhotoOverlay::point() const
{
    Q_D(const GeoDataPhotoOverlay);
    return d->m_point;
}

void GeoDataPhotoOverlay::setPoint(const GeoDataPoint& point)
{
    Q_D(GeoDataPhotoOverlay);
    d->m_point = point;
}

GeoDataPhotoOverlay::Shape GeoDataPhotoOverlay::shape() const
{
    Q_D(const GeoDataPhotoOverlay);
    return d->m_shape;
}

void GeoDataPhotoOverlay::setShape(Shape shape)
{
    Q_D(GeoDataPhotoOverlay);
    d->m_shape = shape;
}

} // namespace Marble

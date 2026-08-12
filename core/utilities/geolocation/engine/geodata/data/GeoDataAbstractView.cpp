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

#include "GeoDataAbstractView.h"

// Local includes

#include "GeoDataCamera.h"
#include "GeoDataLookAt.h"
#include "GeoDataTypes.h"
#include "GeoDataTimeSpan.h"
#include "GeoDataTimeStamp.h"

namespace Marble
{

class Q_DECL_HIDDEN GeoDataAbstractViewPrivate
{
public:

    GeoDataAbstractViewPrivate();

public:

    GeoDataTimeSpan  m_timeSpan;
    GeoDataTimeStamp m_timeStamp;
    AltitudeMode     m_altitudeMode = ClampToGround;
};

GeoDataAbstractViewPrivate::GeoDataAbstractViewPrivate()
    : m_timeSpan      (),
      m_timeStamp     (),
      m_altitudeMode  (ClampToGround)
{
    // do nothing
}

GeoDataAbstractView::GeoDataAbstractView()
    : d(new GeoDataAbstractViewPrivate())
{
    // do nothing
}

GeoDataAbstractView::~GeoDataAbstractView()
{
    delete d;
}

GeoDataAbstractView::GeoDataAbstractView(const GeoDataAbstractView& other)
    : GeoDataObject(other),
      d            (new GeoDataAbstractViewPrivate(*other.d))
{
    // nothing to do
}

GeoDataAbstractView& GeoDataAbstractView::operator =(const GeoDataAbstractView& other)
{
    GeoDataObject::operator=(other);
    *d = *other.d;

    return *this;
}

bool GeoDataAbstractView::operator==(const GeoDataAbstractView& other) const
{
    if (nodeType() != other.nodeType())
    {
        return false;
    }

    if      (nodeType() == GeoDataTypes::GeoDataCameraType)
    {
        const GeoDataCamera& thisCam  = static_cast<const GeoDataCamera&>(*this);
        const GeoDataCamera& otherCam = static_cast<const GeoDataCamera&>(other);

        return (thisCam == otherCam);
    }
    else if (nodeType() == GeoDataTypes::GeoDataLookAtType)
    {
        const GeoDataLookAt& thisLookAt  = static_cast<const GeoDataLookAt&>(*this);
        const GeoDataLookAt& otherLookAt = static_cast<const GeoDataLookAt&>(other);

        return (thisLookAt == otherLookAt);
    }

    return false;
}

GeoDataCoordinates GeoDataAbstractView::coordinates() const
{
    if      (nodeType() == GeoDataTypes::GeoDataLookAtType)
    {
        const GeoDataLookAt* lookAt = dynamic_cast<const GeoDataLookAt*>(this);

        if (lookAt)
        {
            return lookAt->coordinates();
        }
    }
    else if (nodeType() == GeoDataTypes::GeoDataCameraType)
    {
        const GeoDataCamera* camera = dynamic_cast<const GeoDataCamera*>(this);

        if (camera)
        {
            return camera->coordinates();
        }
    }

    return GeoDataCoordinates();
}

bool GeoDataAbstractView::equals(const GeoDataAbstractView& other) const
{
    return (
            GeoDataObject::equals(other)                    &&
            (d->m_timeSpan      == other.d->m_timeSpan)     &&
            (d->m_timeStamp     == other.d->m_timeStamp)    &&
            (d->m_altitudeMode  == other.d->m_altitudeMode)
           );
}

const GeoDataTimeSpan& GeoDataAbstractView::timeSpan() const
{
    return d->m_timeSpan;
}

GeoDataTimeSpan& GeoDataAbstractView::timeSpan()
{
    return d->m_timeSpan;
}

void GeoDataAbstractView::setTimeSpan(const GeoDataTimeSpan& _timeSpan)
{
    d->m_timeSpan = _timeSpan;
}

GeoDataTimeStamp& GeoDataAbstractView::timeStamp()
{
    return d->m_timeStamp;
}

const GeoDataTimeStamp& GeoDataAbstractView::timeStamp() const
{
    return d->m_timeStamp;
}

void GeoDataAbstractView::setTimeStamp(const GeoDataTimeStamp& _timeStamp)
{
    d->m_timeStamp = _timeStamp;
}

AltitudeMode GeoDataAbstractView::altitudeMode() const
{
    return d->m_altitudeMode;
}

void GeoDataAbstractView::setAltitudeMode(const AltitudeMode _altitudeMode)
{
    d->m_altitudeMode = _altitudeMode;
}

} // namespace Marble

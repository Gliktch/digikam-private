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

#include "AzimuthalEquidistantProjection.h"
#include "AbstractProjection_p.h"

// Qt includes

#include <QIcon>
#include <qmath.h>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ViewportParams.h"
#include "GeoDataPoint.h"
#include "GeoDataLineString.h"
#include "GeoDataCoordinates.h"
#include "MarbleGlobal.h"
#include "AzimuthalProjection_p.h"
#include "digikam_debug.h"

#define SAFE_DISTANCE

namespace Marble
{

class Q_DECL_HIDDEN AzimuthalEquidistantProjectionPrivate : public AzimuthalProjectionPrivate
{
public:

    explicit AzimuthalEquidistantProjectionPrivate(AzimuthalEquidistantProjection* parent);

    Q_DECLARE_PUBLIC(AzimuthalEquidistantProjection)
};

AzimuthalEquidistantProjection::AzimuthalEquidistantProjection()
    : AzimuthalProjection(new AzimuthalEquidistantProjectionPrivate(this))
{
    setMinLat(minValidLat());
    setMaxLat(maxValidLat());
}

AzimuthalEquidistantProjection::AzimuthalEquidistantProjection(AzimuthalEquidistantProjectionPrivate* dd)
    : AzimuthalProjection(dd)
{
    setMinLat(minValidLat());
    setMaxLat(maxValidLat());
}

AzimuthalEquidistantProjection::~AzimuthalEquidistantProjection()
{
}

QString AzimuthalEquidistantProjection::name() const
{
    return i18n("Azimuthal Equidistant");
}

QString AzimuthalEquidistantProjection::description() const
{
    return i18n("<p><b>Azimuthal Equidistant Projection</b> (\"fish eye\")</p><p>Applications: Display of seismic and radio data and for use in digital planetariums.</p>");
}

QIcon AzimuthalEquidistantProjection::icon() const
{
    return QIcon::fromTheme(QStringLiteral("map-globe"));
}

AzimuthalEquidistantProjectionPrivate::AzimuthalEquidistantProjectionPrivate(AzimuthalEquidistantProjection* parent)
    : AzimuthalProjectionPrivate(parent)
{
}

qreal AzimuthalEquidistantProjection::clippingRadius() const
{
    return 1;
}

bool AzimuthalEquidistantProjection::screenCoordinates(const GeoDataCoordinates& coordinates,
                                                       const ViewportParams* viewport,
                                                       qreal& x, qreal& y, bool& globeHidesPoint) const
{
    const qreal lambda = coordinates.longitude();
    const qreal phi = coordinates.latitude();
    const qreal lambdaPrime = viewport->centerLongitude();
    const qreal phi1 = viewport->centerLatitude();

    qreal cosC = qSin(phi1) * qSin(phi) + qCos(phi1) * qCos(phi) * qCos(lambda - lambdaPrime);

    // Prevent division by zero
    if (cosC <= 0)
    {
        globeHidesPoint = true;
        return false;
    }

    qreal c = qAcos(cosC);

    qreal k = cosC == 1 ? 1 : c / qSin(c);

    // Let (x, y) be the position on the screen of the placemark..
    x = (qCos(phi) * qSin(lambda - lambdaPrime)) * k;
    y = (qCos(phi1) * qSin(phi) - qSin(phi1) * qCos(phi) * qCos(lambda - lambdaPrime)) * k;

    x *= 2 * viewport->radius() / M_PI;
    y *= 2 * viewport->radius() / M_PI;

    const qint64  radius  = clippingRadius() * viewport->radius();

    if (x * x + y * y > radius * radius)
    {
        globeHidesPoint = true;
        return false;
    }

    globeHidesPoint = false;

    x += viewport->width() / 2;
    y = viewport->height() / 2 - y;

    // Skip placemarks that are outside the screen area
    return !(x < 0 || x >= viewport->width() || y < 0 || y >= viewport->height());
}

bool AzimuthalEquidistantProjection::screenCoordinates(const GeoDataCoordinates& coordinates,
                                                       const ViewportParams* viewport,
                                                       qreal* x, qreal& y,
                                                       int& pointRepeatNum,
                                                       const QSizeF& size,
                                                       bool& globeHidesPoint) const
{
    pointRepeatNum = 0;
    globeHidesPoint = false;

    bool visible = screenCoordinates(coordinates, viewport, *x, y, globeHidesPoint);

    // Skip placemarks that are outside the screen area
    if (*x + size.width() / 2.0 < 0.0 || *x >= viewport->width() + size.width() / 2.0
        || y + size.height() / 2.0 < 0.0 || y >= viewport->height() + size.height() / 2.0)
    {
        return false;
    }

    // This projection doesn't have any repetitions,
    // so the number of screen points referring to the geopoint is one.
    pointRepeatNum = 1;
    return visible;
}


bool AzimuthalEquidistantProjection::geoCoordinates(const int x, const int y,
                                                    const ViewportParams* viewport,
                                                    qreal& lon, qreal& lat,
                                                    GeoDataCoordinates::Unit unit) const
{
    const qint64  radius  = viewport->radius();
    // Calculate how many degrees are being represented per pixel.
    const qreal rad2Pixel = (2 * radius) / M_PI;
    const qreal centerLon = viewport->centerLongitude();
    const qreal centerLat = viewport->centerLatitude();
    const qreal rx = (- viewport->width()  / 2 + x) / rad2Pixel;
    const qreal ry = (viewport->height() / 2 - y) / rad2Pixel;
    const qreal c = qMax(qSqrt(rx * rx + ry * ry), qreal(0.0001)); // ensure we don't divide by zero
    const qreal sinc = qSin(c);

    lon = centerLon + qAtan2(rx * sinc, (c * qCos(centerLat) * qCos(c) - ry * qSin(centerLat) * sinc));

    while (lon < -M_PI)
    {
        lon += 2 * M_PI;
    }

    while (lon >  M_PI)
    {
        lon -= 2 * M_PI;
    }

    lat = qAsin(qCos(c) * qSin(centerLat) + (ry * sinc * qCos(centerLat)) / c);

    if (unit == GeoDataCoordinates::Degree)
    {
        lon *= RAD2DEG;
        lat *= RAD2DEG;
    }

    return true;
}

} // namespace Marble

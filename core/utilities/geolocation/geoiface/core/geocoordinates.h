/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-16
 * Description : GeoCoordinates class
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFlags>
#include <QDebug>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"

#ifdef HAVE_GEOLOCATION

/// @note forward declaration only to declare interoperability operators
namespace Marble
{
    class GeoDataCoordinates;
}

#endif // HAVE_GEOLOCATION

namespace Digikam
{

class DIGIKAM_EXPORT GeoCoordinates
{
public:

    enum HasFlag
    {
        HasNothing     = 0,
        HasLatitude    = 1,
        HasLongitude   = 2,
        HasCoordinates = 3,
        HasAltitude    = 4
    };

    Q_DECLARE_FLAGS(HasFlags, HasFlag)

    typedef QList<GeoCoordinates>                 List;
    typedef QPair<GeoCoordinates, GeoCoordinates> Pair;
    typedef QList<GeoCoordinates::Pair>           PairList;

public:

    GeoCoordinates()  = default;
    GeoCoordinates(const double inLat, const double inLon);
    GeoCoordinates(const double inLat, const double inLon, const double inAlt);
    ~GeoCoordinates() = default;

    bool operator==(const GeoCoordinates& other)        const;

    double lat()                                        const;
    double lon()                                        const;
    double alt()                                        const;

    bool hasCoordinates()                               const;
    bool hasLatitude()                                  const;
    bool hasLongitude()                                 const;

    void setLatLon(const double inLat, const double inLon);

    bool hasAltitude()                                  const;
    HasFlags hasFlags()                                 const;

    void setAlt(const double inAlt);

    void clearAlt();
    void clear();

    QString altString()                                 const;
    QString latString()                                 const;
    QString lonString()                                 const;
    QString geoUrl()                                    const;

    bool sameLonLatAs(const GeoCoordinates& other)      const;

    static GeoCoordinates fromGeoUrl(const QString& url, bool* const parsedOkay = nullptr);
    static Pair makePair(const qreal lat1, const qreal lon1, const qreal lat2, const qreal lon2);

#ifdef HAVE_GEOLOCATION

    Marble::GeoDataCoordinates toMarbleCoordinates()    const;
    static GeoCoordinates fromMarbleCoordinates(const Marble::GeoDataCoordinates& marbleCoordinates);

#endif // HAVE_GEOLOCATION

private:

    double   m_lat      = 0.0;
    double   m_lon      = 0.0;
    double   m_alt      = 0.0;
    HasFlags m_hasFlags = HasNothing;
};

} // namespace Digikam

// cppcheck-suppress unknownMacro
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoCoordinates::HasFlags)

Q_DECLARE_TYPEINFO(Digikam::GeoCoordinates, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(Digikam::GeoCoordinates)
Q_DECLARE_METATYPE(Digikam::GeoCoordinates::Pair)
Q_DECLARE_METATYPE(Digikam::GeoCoordinates::PairList)

DIGIKAM_EXPORT QDebug operator<<(QDebug debug, const Digikam::GeoCoordinates& coordinate);

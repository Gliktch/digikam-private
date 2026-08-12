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

// Quaternions provides a class that deals with quaternion operations.

// krazy:excludeall=dpointer,inline

#pragma once

// C++ includes

#include <cmath>

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Marble
{

enum
{
    Q_X = 0,
    Q_Y = 1,
    Q_Z = 2,
    Q_W = 3
};


typedef qreal    xmmfloat[4];
typedef xmmfloat  matrix[3];


class DIGIKAM_EXPORT Quaternion
{
public:

    Quaternion();
    Quaternion(qreal w, qreal x, qreal y, qreal z);

    /*!@brief used to generate Quaternion from longitude and latitude
     *
     * \param lon longitude
     * \param lat latitude
     */
    static Quaternion   fromSpherical(qreal lon, qreal lat);
    static Quaternion   fromEuler(qreal pitch, qreal yaw, qreal roll);

    static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, qreal t);
    static Quaternion nlerp(const Quaternion& q1, const Quaternion& q2, qreal t);

    // Operators
    Quaternion  operator*(const Quaternion& q) const;
    Quaternion  operator+(const Quaternion& q) const;
    Quaternion  operator*(qreal factor) const;
    bool        operator==(const Quaternion& q) const;
    Quaternion& operator*=(const Quaternion& q);
    Quaternion& operator*=(qreal);

    void        getSpherical(qreal& lon, qreal& lat) const;

    void        normalize();

    qreal       length() const;

    Quaternion  inverse() const;
    Quaternion  log() const;
    Quaternion  exp() const;

    qreal       pitch() const;
    qreal       yaw() const;
    qreal       roll() const;


    void        rotateAroundAxis(const Quaternion& q);

    void        toMatrix(matrix& m) const;
    void        rotateAroundAxis(const matrix& m);

    xmmfloat    v;
};

} // Namespace Marble

#ifndef QT_NO_DEBUG_STREAM
DIGIKAM_EXPORT QDebug operator<<(QDebug, const Marble::Quaternion&);
#endif

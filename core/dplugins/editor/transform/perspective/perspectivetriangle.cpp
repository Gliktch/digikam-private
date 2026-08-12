/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-01-18
 * Description : triangle geometry calculation class.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#define RADIAN2DEGREE(a) (a * 57.295779513082)

#include "perspectivetriangle.h"

// C++ includes

#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace DigikamEditorPerspectiveToolPlugin
{

PerspectiveTriangle::PerspectiveTriangle(const QPoint& A, const QPoint& B, const QPoint& C)
{
    m_a = distanceP2P(B, C);
    m_b = distanceP2P(A, C);
    m_c = distanceP2P(A, B);
}

float PerspectiveTriangle::angleABC() const
{
    return (RADIAN2DEGREE( acos( (m_b*m_b - m_a*m_a - m_c*m_c ) / (-2*m_a*m_c ) ) ));
}

float PerspectiveTriangle::angleACB() const
{
    return (RADIAN2DEGREE( acos( (m_c*m_c - m_a*m_a - m_b*m_b ) / (-2*m_a*m_b ) ) ));
}

float PerspectiveTriangle::angleBAC() const
{
    return (RADIAN2DEGREE( acos( (m_a*m_a - m_b*m_b - m_c*m_c ) / (-2*m_b*m_c ) ) ));
}

float PerspectiveTriangle::distanceP2P(const QPoint& p1, const QPoint& p2) const
{
    return (
            sqrt(abs( p2.x()-p1.x() ) * abs( p2.x()-p1.x() ) +
                 abs( p2.y()-p1.y() ) * abs( p2.y()-p1.y() ) )
           );
}

} // namespace DigikamEditorPerspectiveToolPlugin

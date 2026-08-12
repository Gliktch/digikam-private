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

#pragma once

// Qt includes

#include <QPoint>

namespace DigikamEditorPerspectiveToolPlugin
{

class PerspectiveTriangle
{

public:

    PerspectiveTriangle(const QPoint& A, const QPoint& B, const QPoint& C);
    ~PerspectiveTriangle() = default;

    float angleABC() const;
    float angleACB() const;
    float angleBAC() const;

private:

    float distanceP2P(const QPoint& p1, const QPoint& p2) const;

private:

    float m_a = 0.0F;
    float m_b = 0.0F;
    float m_c = 0.0F;
};

} // namespace DigikamEditorPerspectiveToolPlugin

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

#pragma once

namespace Marble
{

class AbstractProjection;

class Q_DECL_HIDDEN AbstractProjectionPrivate
{
public:

    explicit AbstractProjectionPrivate(AbstractProjection* parent);

    virtual ~AbstractProjectionPrivate() = default;

    int levelForResolution(qreal resolution) const;

    Q_DECLARE_PUBLIC(AbstractProjection)

public:

    qreal                       m_maxLat             = 0;
    qreal                       m_minLat             = 0;
    mutable qreal               m_previousResolution = -1;
    mutable qreal               m_level              = -1;

private:

    AbstractProjection* const   q_ptr                = nullptr;
};

} // namespace Marble

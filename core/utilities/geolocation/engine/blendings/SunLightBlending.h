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

// Qt includes

#include <QtGlobal>

// Local includes

#include "Blending.h"

namespace Marble
{

class SunLocator;

class SunLightBlending: public Blending
{
public:

    explicit SunLightBlending(const SunLocator* sunLocator);
    ~SunLightBlending()                                                   override;

    void blend(QImage* const bottom, TextureTile const* const top)  const override;

    void setLevelZeroLayout(int levelZeroColumns, int levelZeroRows);

private:

    static int maxDivisor(int maximum, int fullLength);

private:

    const SunLocator* const m_sunLocator       = nullptr;
    int                     m_levelZeroColumns = 0;
    int                     m_levelZeroRows    = 0;
};

} // Namespace Marble

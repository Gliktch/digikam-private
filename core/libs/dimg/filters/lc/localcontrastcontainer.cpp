/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : LDR ToneMapper <zynaddsubfx.sourceforge.net/other/tonemapping>.
 *
 * SPDX-FileCopyrightText: 2009      by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localcontrastcontainer.h"

// C++ includes

#include <cstdio>
#include <cstring>
#include <cstdlib>

// Qt includes

#include <QtMath>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

LocalContrastContainer::LocalContrastContainer()
{
    stage[0].enabled = true;
}

double LocalContrastContainer::getPower(int nstage) const
{
    float power = stage[nstage].power;
    power       = (float)(qPow(power / 100.0, 1.5) * 100.0);

    return power;
}

double LocalContrastContainer::getBlur(int nstage) const
{
    return stage[nstage].blur;
}

} // namespace Digikam

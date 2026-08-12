/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Local Contrast settings container
 *               LDR ToneMapper zynaddsubfx.sourceforge.net/other/tonemapping
 *
 * SPDX-FileCopyrightText: 2009      by Nasca Octavian Paul <zynaddsubfx at yahoo dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#define TONEMAPPING_MAX_STAGES 4

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LocalContrastContainer
{

public:

    LocalContrastContainer();
    ~LocalContrastContainer() = default;

    double getPower(int nstage)  const;
    double getBlur(int nstage)   const;

public:

    bool stretchContrast    = true;

    int  lowSaturation      = 100;
    int  highSaturation     = 100;
    int  functionId         = 0;

    struct
    {
        bool   enabled  = false;
        double power    = 30.0;
        double blur     = 80.0;
    }
    stage[TONEMAPPING_MAX_STAGES];
};

} // namespace Digikam

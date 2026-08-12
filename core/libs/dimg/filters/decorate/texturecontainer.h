/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : texture settings container.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TextureContainer
{

public:

    enum TextureTypes
    {
        PaperTexture = 0,
        Paper2Texture,
        FabricTexture,
        BurlapTexture,
        BricksTexture,
        Bricks2Texture,
        CanvasTexture,
        MarbleTexture,
        Marble2Texture,
        BlueJeanTexture,
        CellWoodTexture,
        MetalWireTexture,
        ModernTexture,
        WallTexture,
        MossTexture,
        StoneTexture
    };

public:

    TextureContainer()  = default;
    ~TextureContainer() = default;

    static QString getTexturePath(int texture);

public:

    int blendGain       = 200;
    int textureType     = MarbleTexture;
};

} // namespace Digikam

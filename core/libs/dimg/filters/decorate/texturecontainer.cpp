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

#include "texturecontainer.h"

// Qt includes

#include <QStandardPaths>

namespace Digikam
{

QString TextureContainer::getTexturePath(int texture)
{
    QString pattern;

    switch (texture)
    {
        case PaperTexture:
        {
            pattern = QLatin1String("paper-texture");
            break;
        }

        case Paper2Texture:
        {
            pattern = QLatin1String("paper2-texture");
            break;
        }

        case FabricTexture:
        {
            pattern = QLatin1String("fabric-texture");
            break;
        }

        case BurlapTexture:
        {
            pattern = QLatin1String("burlap-texture");
            break;
        }

        case BricksTexture:
        {
            pattern = QLatin1String("bricks-texture");
            break;
        }

        case Bricks2Texture:
        {
            pattern = QLatin1String("bricks2-texture");
            break;
        }

        case CanvasTexture:
        {
            pattern = QLatin1String("canvas-texture");
            break;
        }

        case MarbleTexture:
        {
            pattern = QLatin1String("marble-texture");
            break;
        }

        case Marble2Texture:
        {
            pattern = QLatin1String("marble2-texture");
            break;
        }

        case BlueJeanTexture:
        {
            pattern = QLatin1String("bluejean-texture");
            break;
        }

        case CellWoodTexture:
        {
            pattern = QLatin1String("cellwood-texture");
            break;
        }

        case MetalWireTexture:
        {
            pattern = QLatin1String("metalwire-texture");
            break;
        }

        case ModernTexture:
        {
            pattern = QLatin1String("modern-texture");
            break;
        }

        case WallTexture:
        {
            pattern = QLatin1String("wall-texture");
            break;
        }

        case MossTexture:
        {
            pattern = QLatin1String("moss-texture");
            break;
        }

        case StoneTexture:
        {
            pattern = QLatin1String("stone-texture");
            break;
        }
    }

    return (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/") + pattern + QLatin1String(".webp")));
}

} // namespace Digikam

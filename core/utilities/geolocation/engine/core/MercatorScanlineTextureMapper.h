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

#include <QThreadPool>
#include <QImage>

// Local includes

#include "TextureMapperInterface.h"
#include "MarbleGlobal.h"

namespace Marble
{

class StackedTileLoader;

class MercatorScanlineTextureMapper : public TextureMapperInterface
{
public:

    explicit MercatorScanlineTextureMapper(StackedTileLoader* tileLoader);

    void mapTexture(GeoPainter* painter,
                    const ViewportParams* viewport,
                    int tileZoomLevel,
                    const QRect& dirtyRect,
                    TextureColorizer* texColorizer) override;

private:

    void mapTexture(const ViewportParams* viewport, int tileZoomLevel, MapQuality mapQuality);

private:

    class RenderJob;

private:

    StackedTileLoader* const    m_tileLoader        = nullptr;
    int                         m_radius            = 0;
    QImage                      m_canvasImage;
    int                         m_oldYPaintedTop    = 0;
    QThreadPool                 m_threadPool;
};

} // Namespace Marble

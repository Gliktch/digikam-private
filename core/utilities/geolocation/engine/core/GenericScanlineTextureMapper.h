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

class GenericScanlineTextureMapper : public TextureMapperInterface
{
public:

    explicit GenericScanlineTextureMapper(StackedTileLoader* tileLoader);

    void mapTexture(GeoPainter* painter,
                    const ViewportParams* viewport,
                    int tileZoomLevel,
                    const QRect& dirtyRect,
                    TextureColorizer* texColorizer) override;

private:

    class RenderJob;

    void mapTexture(const ViewportParams* viewport, int tileZoomLevel, MapQuality mapQuality);

private:

    StackedTileLoader* const m_tileLoader = nullptr;
    int                      m_radius;
    QImage                   m_canvasImage;
    QThreadPool              m_threadPool;
};

} // Namespace Marble

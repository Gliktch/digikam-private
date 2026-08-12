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

#include <QPixmap>

// Local includes

#include "GeoDataVec2.h"
#include "ScreenGraphicsItem.h"
#include "digikam_export.h"

namespace Marble
{

class GeoDataScreenOverlay;

class DIGIKAM_EXPORT ScreenOverlayGraphicsItem : public ScreenGraphicsItem
{
public:

    explicit ScreenOverlayGraphicsItem(const GeoDataScreenOverlay* screenOverlay);

    const GeoDataScreenOverlay* screenOverlay() const;

    void setProjection(const ViewportParams* viewport) override;

protected:

    void paint(QPainter* painter) override;

private:

    static qreal pixelValue(GeoDataVec2::Unit unit, qreal screenSize, qreal imageSize, qreal value);

private:

    const GeoDataScreenOverlay* m_screenOverlay = nullptr;

    QPixmap                     m_pixmap;
};

} // namespace Marble

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Image zoom settings
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSizeF>
#include <QRectF>
#include <QPointF>
#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageZoomSettings
{

public:

    enum FitToSizeMode
    {
        AlwaysFit,
        OnlyScaleDown
    };

public:

    ImageZoomSettings() = default;
    explicit ImageZoomSettings(const QSize& imageSize, const QSize& originalSize = QSize());

    /**
     * Sets the size of the (available) image data.
     * Optionally, you can specify an original size, if the available image
     * data is a scaled-down version. In this case, zoom factors will refer
     * to the original size.
     * The zoom factor is unchanged, you need to call fitToSize again.
     */
    void setImageSize(const QSize& size, const QSize& originalSize = QSize());

    /**
     * Set the graphics view widget to track the device pixel ratio.
     */
    void setDisplayWidget(QWidget* const widget);

    /**
     * Return the currently set zoom factor
     */
    double zoomFactor()                                                             const;

    /**
     * Return the real zoom factor dependent on device pixel ratio
     */
    double realZoomFactor()                                                         const;

    /**
     * Returns the (available) image size
     */
    QSizeF imageSize()                                                              const;

    /**
     * Return the original image size. Can be identical to size().
     */
    QSizeF originalImageSize()                                                      const;

    /**
     * Return the size of the image when the current zoom factor is applied.
     * This is the size the image should be displayed at.
     */
    QSizeF zoomedSize()                                                             const;

    bool isFitToSize(const QSizeF& frameSize)                                       const;

    /**
     * Sets the current zoom factor, relative to (original) size.
     */
    void setZoomFactor(double zoom);

    /**
     * Sets the current zoom factor to the factor needed to fit
     * the current (original) image size into the given view size.
     * Aspect ratio will be respected, that means the frameSize may not be
     * completely filled in one dimension, and zoomedSize()
     * can differ from frameSize in one dimension.
     */
    void fitToSize(const QSizeF& frameSize, FitToSizeMode = AlwaysFit);

    /**
     * For a given rectangle contained in ((0,0), zoomedSize())
     * returns the corresponding rectangle in (0,0),imageSize().
     */
    QRectF sourceRect(const QRectF& imageRect)                                      const;

    QRectF mapZoomToImage(const QRectF& imageRect)                                  const;

    /**
     * For a given rect contained in ((0,0), imageSize())
     * returns the corresponding rectangle in (0,0),zoomedSize().
     */
    QRectF mapImageToZoom(const QRectF& imagePoint)                                 const;

    /**
     * For a given point (in (0,0), zoomedSize())
     * returns the corresponding point in (0,0),imageSize().
     */
    QPointF mapZoomToImage(const QPointF& zoomedPoint)                              const;

    /**
     * For a given point (in (0,0), imageSize())
     * returns the corresponding point in (0,0),zoomedSize().
     */
    QPointF mapImageToZoom(const QPointF& imagePoint)                               const;

    /**
     * Returns the zoom factor that would be used by fitToSize()
     * called with the given frameSize.
     */
    double fitToSizeZoomFactor(const QSizeF& frameSize, FitToSizeMode = AlwaysFit)  const;

    /**
     * When changing the zoom from current zoom to given nextZoom,
     * sometimes a special value may be crossed, and this could then be used
     * instead of nextZoom. Returns this special zoom, or nextZoom if not applicable.
     */
    double snappedZoomStep(double nextZoom, const QSizeF& frameSize)                const;

    /**
     * When setting a new zoom factor (absolute value), the new value may be
     * very close to a special value. Returns this special value if this is the case,
     * returns newZoom if not applicable.
     */
    double snappedZoomFactor(double newZoom, const QSizeF& frameSize)               const;

    /**
     * Static functions to define the smooth scaling of the image.
     */
    static void setImageSmoothScale(bool enable);
    static bool getImageSmoothScale();

private:

    double displayRatio()                                                           const;

protected:

    QSizeF   m_size;
    double   m_zoom             = 1.0;
    double   m_zoomConst        = 1.0;

    QWidget* m_displayWidget    = nullptr;
};

} // namespace Digikam

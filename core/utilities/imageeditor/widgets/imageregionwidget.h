/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-15
 * Description : a widget to draw an image clip region.
 *
 * SPDX-FileCopyrightText: 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPoint>
#include <QRect>
#include <QEvent>
#include <QPolygon>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>

// Local includes

#include "dimg.h"
#include "graphicsdimgview.h"
#include "imagezoomsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageRegionWidget : public GraphicsDImgView
{
    Q_OBJECT

public:

    explicit ImageRegionWidget(QWidget* const parent = nullptr, bool paintExtras = true);
    ~ImageRegionWidget()                                                  override;

    /**
     * To get target image region area to render.
     */
    QRect  getOriginalImageRegionToRender()                         const;

    /**
     * To get target image region image to use for render operations
     * If the bool parameter is true a downscaled version of the image
     * region at screen resolution will be sent.
     * Should be use to increase preview speed for the effects whose
     * behaviour is a function of each pixel.
     */
    DImg   getOriginalRegionImage(bool useDownscaledImage = false)  const;

    DImg   getOriginalImage()                                       const;

    void   setPreviewImage(const DImg& img);
    void   updateImage(const DImg& img);

    void   setCapturePointMode(bool b);
    bool   capturePointMode()                                       const;

    void   setHighLightPoints(const QPolygon& pointsList);

    void   ICCSettingsChanged();
    void   exposureSettingsChanged();

    /**
     * @brief: set an internal flag to prevent glitch in the canvas when preview filter render a region.
     *         The canvas update can be processed only when the filter complete the job.
     *         Typical case of dysfunction: zoom out the canvas with a preview redending an half of the image.
     *         Due to a race condition, an older area is flush on the part of the image to render and still present while
     *         the filter is done.
     */
    void   setBusy(bool b);
    bool   isBusy()                                                 const;

Q_SIGNALS:

    void signalOriginalClipFocusChanged();
    void signalSpotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&);
    void signalCapturedPointFromOriginal(const Digikam::DColor&, const QPoint&);

public Q_SLOTS:

    void slotPreviewModeChanged(int mode);
    void slotOriginalImageRegionChanged(bool targetDone = true);

protected:

    void mousePressEvent(QMouseEvent*)                                    override;
    void mouseMoveEvent(QMouseEvent*)                                     override;
    void mouseReleaseEvent(QMouseEvent*)                                  override;

private:

    bool capturedPointFromOriginal(QPointF&, DColor&)               const;

private Q_SLOTS:

    void slotSendSpotPositionFromOrginalDelayed();
    void slotOriginalImageRegionChangedDelayed();
    void slotPanIconSelectionMoved(const QRect&, bool)                    override;

private:

    class Private;
    Private* const d_ptr = nullptr;
};

} // namespace Digikam

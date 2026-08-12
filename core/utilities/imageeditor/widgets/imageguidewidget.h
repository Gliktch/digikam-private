/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-20
 * Description : a widget to display an image with guides
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QColor>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QEvent>
#include <QTimerEvent>

// Local includes

#include "imageiface.h"
#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class DColor;

class DIGIKAM_EXPORT ImageGuideWidget : public QWidget
{
    Q_OBJECT

public:

    enum GuideToolMode
    {
        HVGuideMode = 0,
        PickColorMode
    };

    enum ColorPointSrc
    {
        OriginalImage = 0,
        PreviewImage,
        TargetPreviewImage
    };

public:

    explicit ImageGuideWidget(QWidget* const parent = nullptr,
                              bool spotVisible = true,
                              int guideMode = PickColorMode,
                              const QColor& guideColor = Qt::red,
                              int guideSize = 1,
                              bool blink = false,
                              ImageIface::PreviewType type= ImageIface::FullImage);
    ~ImageGuideWidget()                             override;

    ImageIface* imageIface()                   const;
    QImage      getMask()                      const;
    QPoint      getSpotPosition()              const;
    DColor      getSpotColor(int getColorFrom) const;
    int         previewMode()                  const;

    void   setSpotVisible(bool spotVisible, bool blink = false);
    void   setSpotVisibleNoUpdate(bool spotVisible);
    void   resetSpotPosition();
    void   updatePreview();
    void   setPoints(const QPolygon& p, bool drawLine = false);
    void   resetPoints();

    void   setPaintColor(const QColor& color);
    void   setEraseMode(bool erase);
    void   setMaskEnabled(bool enabled);
    void   setMaskPenSize(int size);
    void   setMaskCursor();

    void   setBackgroundColor(const QColor&);
    void   ICCSettingsChanged();
    void   exposureSettingsChanged();

    void   setBusy(bool b);
    bool   isBusy()                            const;

public Q_SLOTS:

    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);
    void slotPreviewModeChanged(int mode);

Q_SIGNALS:

    void spotPositionChangedFromOriginal(const Digikam::DColor& color, const QPoint& position);
    void spotPositionChangedFromTarget(const Digikam::DColor& color, const QPoint& position);
    void signalResized();

protected:

    void   paintEvent(QPaintEvent*)                 override;
    void   resizeEvent(QResizeEvent*)               override;
    void   timerEvent(QTimerEvent*)                 override;
    void   mousePressEvent(QMouseEvent*)            override;
    void   mouseReleaseEvent(QMouseEvent*)          override;
    void   mouseMoveEvent(QMouseEvent*)             override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent*)                   override;

#else

    void enterEvent(QEvent*)                        override;

#endif

    void   leaveEvent(QEvent*)                      override;
    void   updatePixmap();
    void   drawLineTo(const QPoint& endPoint);
    void   drawLineTo(int width, bool erase,
                      const QColor& color,
                      const QPoint& start,
                      const QPoint& end);
    QPoint translatePointPosition(const QPoint& point) const;
    void   drawText(QPainter* const p, const QPoint& corner, const QString& text);
    void   updateMaskCursor();
    void   setSpotPosition(const QPoint& point);
    void   updateSpotPosition(int x, int y);
    QPoint translateItemPosition(const QPoint& point, bool src) const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View for DImg preview
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QGraphicsView>
#include <QGestureEvent>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class GraphicsDImgItem;
class DImgPreviewItem;
class SinglePhotoPreviewLayout;
class PanIconWidget;

class DIGIKAM_EXPORT GraphicsDImgView : public QGraphicsView
{
    Q_OBJECT

public:

    explicit GraphicsDImgView(QWidget* const parent = nullptr);
    ~GraphicsDImgView()                           override;

    /**
     * @brief Store internal instance of item as GraphicsDImgItem. You can store DImgPreviewItem object also by this method.
     * Use item() or previewItem() to get right version.
     * @note if you store a GraphicsDImgItem object, previewItem() will return 0.
     */
    void setItem(GraphicsDImgItem* const item);

    /**
     * @brief Return the instance of item set by setItem().
     */
    GraphicsDImgItem*         item()        const;

    /**
     * @return a cast of item instance of item set by setItem() as DImgPreviewItem
     * @note if you store a GraphicsDImgItem object using setItem(), this method will return nullptr.
     */
    DImgPreviewItem*          previewItem() const;

    SinglePhotoPreviewLayout* layout()      const;

    /**
     * @brief Scrolls the view such that scenePos (in scene coordinates
     * is displayed on the viewport at viewportPos (in viewport coordinates).
     * E.g., calling scrollPointOnPoint(scenePos, viewport()->rect().center()) is
     * equivalent to calling centerOn(scenePos).
     */
    void scrollPointOnPoint(const QPointF& scenePos, const QPoint& viewportPos);

    /**
     * @note Change from protected to public to be used by ImageRegionWidget and ImageRegionItem
     */
    void drawText(QPainter* p, const QRectF& rect, const QString& text);

    int   contentsX()                       const;
    int   contentsY()                       const;
    QRect visibleArea()                     const;
    void  setContentsPos(int x, int y);
    void  fitToWindow();
    void  toggleFullScreen(bool set);

    void  setMagnifierZoomFactor(qreal factor);
    qreal magnifierZoomFactor()             const;

    void  setMagnifierSize(int size);
    int   magnifierSize()                   const;

    void  setMagnifierVisible(bool b);
    bool  isMagnifierVisible()              const;

    void  updatePanIconWidget();

Q_SIGNALS:

    void contentsMoving(int, int);
    void rightButtonClicked();
    void leftButtonClicked();
    void leftButtonDoubleClicked();
    void activated();
    void toNextImage();
    void toPreviousImage();
    void contentsMoved(bool panningFinished);
    void resized();

    void signalZoomFactorChanged();

    void viewportRectChanged(const QRectF& viewportRect);

protected:

    void drawForeground(QPainter* painter, const QRectF& rect)          override;

    virtual bool viewportEvent(QEvent*)                                 override;
    virtual void mouseDoubleClickEvent(QMouseEvent*)                    override;
    virtual void mousePressEvent(QMouseEvent*)                          override;
    virtual void mouseMoveEvent(QMouseEvent*)                           override;
    virtual void mouseReleaseEvent(QMouseEvent*)                        override;
    virtual void wheelEvent(QWheelEvent*)                               override;
    virtual void resizeEvent(QResizeEvent*)                             override;
    virtual bool event(QEvent*)                                         override;

    PanIconWidget* installPanIcon();
    void startPanning(const QPoint& pos);
    void continuePanning(const QPoint& pos);
    void finishPanning();

    void setShowText(bool value);
    void setScaleFitToWindow(bool value);

    virtual bool acceptsMouseClick(QMouseEvent* e);
    virtual void scrollContentsBy(int dx, int dy)                       override;

protected Q_SLOTS:

    void         slotZoomFactorChanged();
    void         slotContentsMoved();
    void         slotRefreshPanIconSelection();

    virtual void slotPanIconSelectionMoved(const QRect&, bool);

private:

    void gestureEvent(QGestureEvent*);
    void updateMagnifier();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

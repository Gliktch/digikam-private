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

#include "graphicsdimgview_p.h"

namespace Digikam
{

GraphicsDImgView::GraphicsDImgView(QWidget* const parent)
    : QGraphicsView(parent),
      d            (new Private)
{
    setMouseTracking(true);

    d->scene  = new QGraphicsScene(this);
    d->scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(d->scene);
    d->layout = new SinglePhotoPreviewLayout(this);
    d->layout->setGraphicsView(this);

    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    horizontalScrollBar()->setSingleStep(1);
    horizontalScrollBar()->setPageStep(1);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(1);

    // Enable the gesture support

    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::SwipeGesture);

    // ---

    d->magnifier = new MagnifierItem();
    d->magnifier->setVisible(false);
    d->magnifier->setMagnifierSize(150);
    d->scene->addItem(d->magnifier);

    connect(d->layout, &SinglePhotoPreviewLayout::zoomFactorChanged,
            this, &GraphicsDImgView::slotZoomFactorChanged);

    // ---

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotContentsMoved()));

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotContentsMoved()));
}

GraphicsDImgView::~GraphicsDImgView()
{
    if (d->magnifier)
    {
        d->scene->removeItem(d->magnifier);
        delete d->magnifier;
    }

    delete d;
}

int GraphicsDImgView::contentsX() const
{
    return horizontalScrollBar()->value();
}

int GraphicsDImgView::contentsY() const
{
    return verticalScrollBar()->value();
}

void GraphicsDImgView::setContentsPos(int x, int y)
{
    horizontalScrollBar()->setValue(x);
    verticalScrollBar()->setValue(y);
}

void GraphicsDImgView::setShowText(bool val)
{
    d->showText = val;
}

QRect GraphicsDImgView::visibleArea() const
{
    return (mapToScene(viewport()->geometry()).boundingRect().toRect());
}

void GraphicsDImgView::setItem(GraphicsDImgItem* const item)
{
    d->item = item;
    d->item->setDisplayWidget(this);

    d->scene->addItem(d->item);
    d->layout->addItem(d->item);
}

GraphicsDImgItem* GraphicsDImgView::item() const
{
    return d->item;
}

DImgPreviewItem* GraphicsDImgView::previewItem() const
{
    return dynamic_cast<DImgPreviewItem*>(item());
}

SinglePhotoPreviewLayout* GraphicsDImgView::layout() const
{
    return d->layout;
}

void GraphicsDImgView::setScaleFitToWindow(bool value)
{
    d->layout->setScaleFitToWindow(value);
}

void GraphicsDImgView::fitToWindow()
{
    d->layout->fitToWindow();
    update();
}

void GraphicsDImgView::toggleFullScreen(bool set)
{
    if (set)
    {
        d->scene->setBackgroundBrush(Qt::black);
        setFrameShape(QFrame::NoFrame);
    }
    else
    {
        d->scene->setBackgroundBrush(Qt::NoBrush);
        setFrameShape(QFrame::StyledPanel);
    }
}

void GraphicsDImgView::slotZoomFactorChanged()
{
    if (d->magnifierEnabled && d->item)
    {
        updateMagnifier();
    }

    Q_EMIT signalZoomFactorChanged();
}

} // namespace Digikam

#include "moc_graphicsdimgview.cpp"

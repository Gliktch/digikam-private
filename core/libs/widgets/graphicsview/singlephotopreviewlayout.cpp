/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Layout for an item on image preview
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "singlephotopreviewlayout.h"

// C++ includes

#include <math.h>

// Qt includes

#include <QGraphicsView>
#include <QGraphicsScene>

// Local includes

#include "graphicsdimgitem.h"
#include "graphicsdimgview.h"
#include "imagezoomsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN SinglePhotoPreviewLayout::Private
{
public:

    Private() = default;

    ImageZoomSettings* zoomSettings() const
    {
        return item->zoomSettings();
    }

    QSizeF frameSize() const
    {
        return view->maximumViewportSize();
    }

public:

    GraphicsDImgView*                view           = nullptr;
    GraphicsDImgItem*                item           = nullptr;

    ImageZoomSettings::FitToSizeMode fitToSizeMode  = ImageZoomSettings::OnlyScaleDown;

    bool                             isFitToWindow  = true;
    double                           previousZoom   = 1.0;

    double                           zoomMultiplier = 1.2;
    double                           maxZoom        = 16.0;
    double                           minZoom        = 0.1;
};

SinglePhotoPreviewLayout::SinglePhotoPreviewLayout(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

SinglePhotoPreviewLayout::~SinglePhotoPreviewLayout()
{
    delete d;
}

void SinglePhotoPreviewLayout::setGraphicsView(GraphicsDImgView* const view)
{
    d->view = view;
}

void SinglePhotoPreviewLayout::setScaleFitToWindow(bool value)
{
    if (value)
    {
        d->fitToSizeMode = ImageZoomSettings::AlwaysFit;
    }
    else
    {
        d->fitToSizeMode = ImageZoomSettings::OnlyScaleDown;
    }
}

void SinglePhotoPreviewLayout::addItem(GraphicsDImgItem* const item)
{
    if (d->item)
    {
        disconnect(d->item, SIGNAL(imageChanged()),
                   this, SLOT(updateZoomAndSize()));
    }

    d->item = item;

    if (d->item)
    {
        connect(d->item, SIGNAL(imageChanged()),
                this, SLOT(updateZoomAndSize()));
    }
}

bool SinglePhotoPreviewLayout::isFitToWindow() const
{
    return d->isFitToWindow;
}

double SinglePhotoPreviewLayout::zoomFactor() const
{
    if (!d->item || !d->view)
    {
        return 1.0;
    }

    return d->zoomSettings()->zoomFactor();
}

double SinglePhotoPreviewLayout::realZoomFactor() const
{
    if (!d->item || !d->view)
    {
        return 1.0;
    }

    return d->zoomSettings()->realZoomFactor();
}

double SinglePhotoPreviewLayout::maxZoomFactor() const
{
    return d->maxZoom;
}

double SinglePhotoPreviewLayout::minZoomFactor() const
{
    return d->minZoom;
}

bool SinglePhotoPreviewLayout::atMaxZoom() const
{
    return ((round(zoomFactor() * 1000.0) / 1000.0) >= d->maxZoom);
}

bool SinglePhotoPreviewLayout::atMinZoom() const
{
    return ((round(zoomFactor() * 1000.0) / 1000.0) <= d->minZoom);
}

void SinglePhotoPreviewLayout::setMaxZoomFactor(double z)
{
    d->maxZoom = z;
}

void SinglePhotoPreviewLayout::setMinZoomFactor(double z)
{
    d->minZoom = z;
}

void SinglePhotoPreviewLayout::increaseZoom(const QPoint& viewportAnchor)
{
    if (!d->item || !d->view)
    {
        return;
    }

    double zoom = d->zoomSettings()->zoomFactor() * d->zoomMultiplier;
    zoom        = qMin(zoom, d->maxZoom);
    zoom        = d->zoomSettings()->snappedZoomStep(zoom, d->frameSize());
    setZoomFactor(zoom, viewportAnchor);
}

void SinglePhotoPreviewLayout::decreaseZoom(const QPoint& viewportAnchor)
{
    if (!d->item || !d->view)
    {
        return;
    }

    double zoom = d->zoomSettings()->zoomFactor() / d->zoomMultiplier;
    zoom        = qMax(zoom, d->minZoom);
    zoom        = d->zoomSettings()->snappedZoomStep(zoom, d->frameSize());
    setZoomFactor(zoom, viewportAnchor);
}

void SinglePhotoPreviewLayout::setZoomFactorSnapped(double z)
{
    setZoomFactor(z, SnapZoomFactor);
}

void SinglePhotoPreviewLayout::setZoomFactor(double z, const QPoint& givenAnchor, SetZoomFlags flags)
{
    if (!d->item || !d->view)
    {
        return;
    }

    QPoint  viewportAnchor = givenAnchor.isNull() ? d->view->viewport()->rect().center() : givenAnchor;
    QPointF sceneAnchor    = d->view->mapToScene(viewportAnchor);
    QPointF imageAnchor    = d->zoomSettings()->mapZoomToImage(sceneAnchor);

    if (flags & SnapZoomFactor)
    {
        z = d->zoomSettings()->snappedZoomFactor(z, d->frameSize());
    }

    d->isFitToWindow = false;
    double minZoom   = qMin(z, 0.1);
    double maxZoom   = qMax(z, maxZoomFactor());
    minZoom          = qMin(minZoom, minZoomFactor());

    setMinZoomFactor(minZoom);
    setMaxZoomFactor(maxZoom);

    d->zoomSettings()->setZoomFactor(z);
    d->item->sizeHasChanged();

    updateLayout();
    d->item->update();

    Q_EMIT fitToWindowToggled(d->isFitToWindow);
    Q_EMIT zoomFactorChanged(d->zoomSettings()->zoomFactor());

    if (flags & CenterView)
    {
        d->view->centerOn(d->view->scene()->sceneRect().width()  / 2.0,
                          d->view->scene()->sceneRect().height() / 2.0);
    }

    d->view->scrollPointOnPoint(d->zoomSettings()->mapImageToZoom(imageAnchor), viewportAnchor);
}

void SinglePhotoPreviewLayout::setZoomFactor(double z, SetZoomFlags flags)
{
    if (!d->item || !d->view)
    {
        return;
    }

    setZoomFactor(z, QPoint(), flags);
}

void SinglePhotoPreviewLayout::fitToWindow()
{
    if (!d->item || !d->view)
    {
        return;
    }

    if (!d->isFitToWindow)
    {
        d->previousZoom = d->zoomSettings()->zoomFactor();
    }

    d->isFitToWindow = true;

    d->zoomSettings()->fitToSize(d->frameSize(), d->fitToSizeMode);
    d->item->sizeHasChanged();

    updateLayout();
    d->item->update();

    Q_EMIT fitToWindowToggled(d->isFitToWindow);
    Q_EMIT zoomFactorChanged(d->zoomSettings()->zoomFactor());
}

void SinglePhotoPreviewLayout::toggleFitToWindow()
{
    if (!d->item || !d->view)
    {
        return;
    }

    if (d->isFitToWindow)
    {
        setZoomFactor(d->previousZoom);
    }
    else
    {
        fitToWindow();
    }
}

void SinglePhotoPreviewLayout::toggleFitToWindowOr100()
{
    if (!d->item || !d->view)
    {
        return;
    }

    if (d->isFitToWindow || ((qRound(zoomFactor() * 1000) / 1000.0) != 1.0))
    {
        setZoomFactor(1.0);
    }
    else
    {
        fitToWindow();
    }
}

void SinglePhotoPreviewLayout::updateLayout()
{
    if (!d->item || !d->view)
    {
        return;
    }

    d->view->scene()->setSceneRect(d->item->boundingRect());
    d->item->setPos(0, 0);
}

void SinglePhotoPreviewLayout::updateZoomAndSize()
{
    if (!d->item || !d->view)
    {
        return;
    }

    double fitZoom = d->zoomSettings()->fitToSizeZoomFactor(d->frameSize(), d->fitToSizeMode);
    double minZoom = qBound(0.01, fitZoom - 0.01, 0.1);

    setMinZoomFactor(minZoom);
    setMaxZoomFactor(16.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.

    if (!d->zoomSettings()->imageSize().isNull() && d->isFitToWindow)
    {
        fitToWindow();
    }

    updateLayout();
}

void SinglePhotoPreviewLayout::zoomByFactor(qreal factor, const QPoint& viewportAnchor)
{
    if (!d->item || !d->view || (factor <= 0.0))
    {
        return;
    }

    // Compute the new zoom factor

    double currentZoom = d->zoomSettings()->zoomFactor();
    double newZoom = currentZoom * factor;
    newZoom = qBound(d->minZoom, newZoom, d->maxZoom);

    // Apply the centered zoom on the specific point

    setZoomFactor(newZoom, viewportAnchor);
}

} // namespace Digikam

#include "moc_singlephotopreviewlayout.cpp"

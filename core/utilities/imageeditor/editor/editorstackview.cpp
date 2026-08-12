/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : A widget stack to embed editor view.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "editorstackview.h"

// Local includes

#include "dzoombar.h"
#include "canvas.h"
#include "thumbnailsize.h"
#include "graphicsdimgview.h"
#include "singlephotopreviewlayout.h"

namespace Digikam
{

class Q_DECL_HIDDEN EditorStackView::Private
{
public:

    Private() = default;

public:

    QWidget* toolView   = nullptr;
    Canvas*  canvas     = nullptr;
};

EditorStackView::EditorStackView(QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
}

EditorStackView::~EditorStackView()
{
    delete d;
}

void EditorStackView::setCanvas(Canvas* const _canvas)
{
    if (d->canvas)
    {
        return;
    }

    d->canvas = _canvas;
    insertWidget(CanvasMode, d->canvas);

    connect(d->canvas, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->canvas, SIGNAL(signalToggleOffFitToWindow()),
            this, SIGNAL(signalToggleOffFitToWindow()));
}

Canvas* EditorStackView::canvas() const
{
    return d->canvas;
}

void EditorStackView::setToolView(QWidget* const view)
{
    if (d->toolView)
    {
        removeWidget(d->toolView);
    }

    d->toolView = view;

    if (d->toolView)
    {
        insertWidget(ToolViewMode, d->toolView);
    }

    const GraphicsDImgView* const preview = previewWidget();

    if (preview)
    {
        connect(preview->layout(), SIGNAL(zoomFactorChanged(double)),
                this, SLOT(slotZoomChanged(double)));

        connect(preview->layout(), SIGNAL(fitToWindowToggled(bool)),
                this, SLOT(slotToggleOffFitToWindow(bool)));
    }
}

QWidget* EditorStackView::toolView() const
{
    return d->toolView;
}

int EditorStackView::viewMode() const
{
    return indexOf(currentWidget());
}

void EditorStackView::setViewMode(int mode)
{
    if ((mode != CanvasMode) && (mode != ToolViewMode))
    {
        return;
    }

    setCurrentIndex(mode);
}

void EditorStackView::increaseZoom()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->increaseZoom();
    }
    else
    {
        const GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->increaseZoom();
        }
    }
}

void EditorStackView::decreaseZoom()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->decreaseZoom();
    }
    else
    {
        const GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->decreaseZoom();
        }
    }
}

void EditorStackView::toggleFitToWindow()
{
    // Fit to window action is common place to switch view in this mode.
    // User want to see the same behaviors between canvas and tool preview.
    // Both are toggle at the same time.

    d->canvas->layout()->toggleFitToWindow();

    const GraphicsDImgView* const preview = previewWidget();

    if (preview)
    {
        preview->layout()->toggleFitToWindow();
    }
}

void EditorStackView::fitToSelect()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->fitToSelect();
    }
}

void EditorStackView::zoomTo100Percent()
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->toggleFitToWindowOr100();
    }
    else
    {
        const GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->toggleFitToWindowOr100();
        }
    }
}

void EditorStackView::setZoomFactor(double zoom)
{
    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->setZoomFactor(zoom);
    }
    else
    {
        const GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->setZoomFactor(zoom);
        }
    }
}

double EditorStackView::zoomMax() const
{
    if (viewMode() == CanvasMode)
    {
        return d->canvas->layout()->maxZoomFactor();
    }
    else
    {
        const GraphicsDImgView* const preview  = previewWidget();

        if (preview)
        {
            return preview->layout()->maxZoomFactor();
        }
        else
        {
            return (-1.0);
        }
    }
}

double EditorStackView::zoomMin() const
{
    if (viewMode() == CanvasMode)
    {
        return d->canvas->layout()->minZoomFactor();
    }
    else
    {
        const GraphicsDImgView* const preview  = previewWidget();

        if (preview)
        {
            return preview->layout()->minZoomFactor();
        }
        else
        {
            return (-1.0);
        }
    }
}

void EditorStackView::slotZoomSliderChanged(int size)
{
    if ((viewMode() == ToolViewMode) && !isZoomablePreview())
    {
        return;
    }

    double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());

    if (viewMode() == CanvasMode)
    {
        d->canvas->layout()->setZoomFactorSnapped(z);
    }
    else
    {
        const GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            preview->layout()->setZoomFactorSnapped(z);
        }
    }
}

void EditorStackView::slotZoomChanged(double zoom)
{
    bool max, min;

    if (viewMode() == CanvasMode)
    {
        max = d->canvas->layout()->atMaxZoom();
        min = d->canvas->layout()->atMinZoom();

        Q_EMIT signalZoomChanged(max, min, zoom);
    }
    else
    {
        const GraphicsDImgView* const preview = previewWidget();

        if (preview)
        {
            max = preview->layout()->atMaxZoom();
            min = preview->layout()->atMinZoom();

            Q_EMIT signalZoomChanged(max, min, zoom);
        }
    }
}

void EditorStackView::slotToggleOffFitToWindow(bool b)
{
    if (b)
    {
        Q_EMIT signalToggleOffFitToWindow();
    }
}

GraphicsDImgView* EditorStackView::previewWidget() const
{
    GraphicsDImgView* const preview = dynamic_cast<GraphicsDImgView*>(d->toolView);

    if (preview)
    {
        return preview;
    }

    return nullptr;
}

bool EditorStackView::isZoomablePreview() const
{
    return previewWidget();
}

} // namespace Digikam

#include "moc_editorstackview.cpp"

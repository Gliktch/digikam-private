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

#include "imageregionwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QTimer>
#include <QIcon>
#include <QStyle>
#include <QApplication>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "imageregionitem.h"
#include "previewtoolbar.h"
#include "singlephotopreviewlayout.h"
#include "dimgitems_p.h"
#include "paniconwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageRegionWidget::Private
{

public:

    Private() = default;

public:

    bool             busy                       = false;      //< Flag about busy state while filter is rendering in a separated thread.

    DImg             targetImage;

    bool             capturePtMode              = false;

    int              renderingPreviewMode       = PreviewToolBar::PreviewBothImagesVertCont;
    int              oldRenderingPreviewMode    = PreviewToolBar::PreviewBothImagesVertCont;

    QPolygon         hightlightPoints;
    PanIconWidget*   pan                        = nullptr;
    QTimer*          delay                      = nullptr;

    QTimer*          spotTimer                  = nullptr;
    DColor           spotColor;
    QPointF          spotPos;

    ImageRegionItem* item                       = nullptr;
};

ImageRegionWidget::ImageRegionWidget(QWidget* const parent, bool paintExtras)
    : GraphicsDImgView(parent),
      d_ptr           (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);
    setMinimumSize(480, 320);
    setWhatsThis(i18n("<p>Here you can see the original clip image "
                      "which will be used for the preview computation.</p>"
                      "<p>Click and drag the mouse cursor in the "
                      "image to change the clip focus.</p>"));

    d_ptr->item = new ImageRegionItem(this, paintExtras);
    setItem(d_ptr->item);

    d_ptr->delay = new QTimer(this);
    d_ptr->delay->setInterval(500);
    d_ptr->delay->setSingleShot(true);

    connect(d_ptr->delay, SIGNAL(timeout()),
            this, SLOT(slotOriginalImageRegionChanged()));

    // ---

    d_ptr->spotTimer = new QTimer(this);
    d_ptr->spotTimer->setInterval(200);
    d_ptr->spotTimer->setSingleShot(true);

    connect(d_ptr->spotTimer, SIGNAL(timeout()),
            this, SLOT(slotSendSpotPositionFromOrginalDelayed()));

    // ---

    layout()->fitToWindow();

    // ---

    d_ptr->pan = installPanIcon();

    connect(d_ptr->item, &ImageRegionItem::imageChanged,
            this, [this]()
        {
            updatePanIconWidget();
        }
    );

    connect(this, &GraphicsDImgView::viewportRectChanged,
            this, &ImageRegionWidget::slotOriginalImageRegionChangedDelayed);

    connect(this, &GraphicsDImgView::signalZoomFactorChanged,
            this, [this]()
        {
            setBusy(true);
        }
    );

    // ---

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d_ptr->pan, 2, 2, 1, 1);
    grid->setContentsMargins(QMargins(0, 0,
                                      QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent),
                                      QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent)));
    grid->setSpacing(layoutSpacing());
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);

    d_ptr->item->initOriginalImage();
    QTimer::singleShot(0, this, SLOT(slotRefreshPanIconSelection()));
}

ImageRegionWidget::~ImageRegionWidget()
{
    delete d_ptr->item;
    delete d_ptr;
}

void ImageRegionWidget::setBusy(bool b)
{
    d_ptr->busy = b;
}

bool ImageRegionWidget::isBusy() const
{
    return d_ptr->busy;
}

void ImageRegionWidget::setHighLightPoints(const QPolygon& pointsList)
{
    d_ptr->item->setHighLightPoints(pointsList);
    viewport()->update();
}

void ImageRegionWidget::setCapturePointMode(bool b)
{
    if (d_ptr->capturePtMode && b)
    {
        return;
    }

    d_ptr->capturePtMode = b;

    if (b)
    {
        d_ptr->oldRenderingPreviewMode = d_ptr->renderingPreviewMode;
        slotPreviewModeChanged(PreviewToolBar::PreviewOriginalImage);
        viewport()->setCursor(QCursor(QIcon::fromTheme(QLatin1String("color-picker"))
                                      .pixmap(32), 4, 27));
    }
    else
    {
        slotPreviewModeChanged(d_ptr->oldRenderingPreviewMode);
        viewport()->unsetCursor();
    }
}

void ImageRegionWidget::slotPreviewModeChanged(int mode)
{
    d_ptr->renderingPreviewMode  = mode;
    d_ptr->item->setRenderingPreviewMode(mode);
    d_ptr->pan->setPreviewMode(mode);
    slotOriginalImageRegionChanged();
    viewport()->update();
}

void ImageRegionWidget::slotPanIconSelectionMoved(const QRect& imageRect, bool b)
{
    if (b)
    {
        GraphicsDImgView::slotPanIconSelectionMoved(imageRect, b);
    }
}

QRect ImageRegionWidget::getOriginalImageRegionToRender() const
{
    QRect  r = d_ptr->item->getImageRegion();
    double z = layout()->realZoomFactor();

    int x    = qRound((double)r.x()      / z);
    int y    = qRound((double)r.y()      / z);
    int w    = qRound((double)r.width()  / z);
    int h    = qRound((double)r.height() / z);

    QRect rect(x, y, w, h);

    return (rect);
}

void ImageRegionWidget::setPreviewImage(const DImg& img)
{
    d_ptr->targetImage = img;
    QRect r            = d_ptr->item->getImageRegion();
    d_ptr->targetImage.resize(r.width(), r.height());

    // Because tool which only work on image data, the DImg container
    // do not contain metadata from original image. About Color Managed View, we need to
    // restore the embedded ICC color profile.
    // However, some tools may set a profile on the preview image, which we accept of course.

    if (d_ptr->targetImage.getIccProfile().isNull())
    {
        d_ptr->targetImage.setIccProfile(d_ptr->item->image().getIccProfile());
    }

    d_ptr->item->setTargetImage(d_ptr->targetImage);
}

DImg ImageRegionWidget::getOriginalImage() const
{
    return (d_ptr->item->image().copy());
}

DImg ImageRegionWidget::getOriginalRegionImage(bool useDownscaledImage) const
{
    DImg image = d_ptr->item->image().copy(getOriginalImageRegionToRender());

    if (useDownscaledImage)
    {
        QRect r = d_ptr->item->getImageRegion();
        image.resize(r.width(), r.height());
    }

    return (image);
}

void ImageRegionWidget::slotOriginalImageRegionChangedDelayed()
{
    viewport()->update();
    d_ptr->delay->start();
}

void ImageRegionWidget::slotOriginalImageRegionChanged(bool targetDone)
{
    if (targetDone)
    {
        Q_EMIT signalOriginalClipFocusChanged(); // For Image Edit Tools
    }
}

void ImageRegionWidget::exposureSettingsChanged()
{
    d_ptr->item->clearCache();
    d_ptr->item->setTargetImage(d_ptr->targetImage);
}

void ImageRegionWidget::ICCSettingsChanged()
{
    d_ptr->item->clearCache();
    d_ptr->item->setTargetImage(d_ptr->targetImage);
}

void ImageRegionWidget::mousePressEvent(QMouseEvent* e)
{
    if (d_ptr->capturePtMode)
    {
        QPointF imgPt = mapToScene(e->pos());
        DColor  color;
        bool b        = capturedPointFromOriginal(imgPt, color);

        if (b)
        {
            Q_EMIT signalCapturedPointFromOriginal(color, imgPt.toPoint());

            QGraphicsView::mousePressEvent(e);
            return;
        }
    }

    GraphicsDImgView::mousePressEvent(e);
}

void ImageRegionWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (d_ptr->capturePtMode)
    {
        QPointF imgPt = mapToScene(e->pos());
        DColor  color;
        bool b        = capturedPointFromOriginal(imgPt, color);

        if (b)
        {
            d_ptr->spotPos   = imgPt;
            d_ptr->spotColor = color;
            d_ptr->spotTimer->start();

            QGraphicsView::mouseMoveEvent(e);
            return;
        }
    }

    GraphicsDImgView::mouseMoveEvent(e);
}

void ImageRegionWidget::slotSendSpotPositionFromOrginalDelayed()
{
    Q_EMIT signalSpotPositionChangedFromOriginal(d_ptr->spotColor, d_ptr->spotPos.toPoint());
}

void ImageRegionWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (d_ptr->capturePtMode)
    {
        setCapturePointMode(false);
        QGraphicsView::mouseReleaseEvent(e);

        return;
    }

    GraphicsDImgView::mouseReleaseEvent(e);
}

bool ImageRegionWidget::capturedPointFromOriginal(QPointF& pt, DColor& color) const
{
    int x          = (int)(pt.x() / layout()->realZoomFactor());
    int y          = (int)(pt.y() / layout()->realZoomFactor());
    QPoint imgPt(x, y);
    QSize  imgSize = d_ptr->item->imageSize();
    QRect  imgRect(0, 0, imgSize.width(), imgSize.height());

    if (imgRect.contains(imgPt))
    {
        color = d_ptr->item->getPixelColor(imgPt);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        pt    = imgPt.toPointF();

#else

        pt    = QPointF(imgPt);

#endif

        return true;
    }

    return false;
}

void ImageRegionWidget::updateImage(const DImg& img)
{
    d_ptr->item->setImage(img);
}

} // namespace Digikam

#include "moc_imageregionwidget.cpp"

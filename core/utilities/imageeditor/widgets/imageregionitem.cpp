/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-25
 * Description : image region widget item for image editor.
 *
 * SPDX-FileCopyrightText: 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageregionitem.h"

// Qt includes

#include <QPixmap>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimgitems_p.h"
#include "editorcore.h"
#include "exposurecontainer.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "iccsettingscontainer.h"
#include "imageiface.h"
#include "previewtoolbar.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageRegionItem::Private
{
public:

    Private() = default;

public:

    bool               paintExtras                  = true;
    bool               onMouseMovePreviewToggled    = true;
    int                renderingPreviewMode         = PreviewToolBar::PreviewBothImagesVertCont;

    QPixmap            targetPix;    ///< Pixmap of target region to render for paint method.
    QRect              drawRect;
    QPolygon           hightlightPoints;

    ImageRegionWidget* view                         = nullptr;
    ImageIface*        iface                        = nullptr;
};

ImageRegionItem::ImageRegionItem(ImageRegionWidget* const widget, bool paintExtras)
    : GraphicsDImgItem(),
      dd              (new Private)
{
    dd->view        = widget;
    dd->iface       = new ImageIface;
    dd->paintExtras = paintExtras;

    setDisplayWidget(widget);
    setAcceptHoverEvents(true);
}

ImageRegionItem::~ImageRegionItem()
{
    delete dd->iface;
    delete dd;
}

QSize ImageRegionItem::imageSize()
{
    Q_D(GraphicsDImgItem);

    return d->image.size();
}

DColor ImageRegionItem::getPixelColor(const QPoint& pt)
{
    Q_D(GraphicsDImgItem);

    return d->image.getPixelColor(pt.x(), pt.y());
}

QRect ImageRegionItem::getImageRegion() const
{
    return dd->drawRect;
}

void ImageRegionItem::initOriginalImage()
{
    setImage(dd->iface->original() ? dd->iface->original()->copy() : DImg());
}

void ImageRegionItem::setTargetImage(const DImg& img)
{
    dd->targetPix = dd->iface->convertToPixmap(img);
    update();
}

void ImageRegionItem::setRenderingPreviewMode(int mode)
{
    dd->renderingPreviewMode = mode;
    update();
}

void ImageRegionItem::setHighLightPoints(const QPolygon& pointsList)
{
    dd->hightlightPoints = pointsList;
}

void ImageRegionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);

    dd->drawRect         = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;
    QSize   completeSize = boundingRect().size().toSize();

    // scale "as if" scaling to whole image, but clip output to our exposed region

    DImg scaledImage     = d->image.smoothScaleClipped(
                                                       completeSize.width(),
                                                       completeSize.height(),
                                                       dd->drawRect.x(),
                                                       dd->drawRect.y(),
                                                       dd->drawRect.width(),
                                                       dd->drawRect.height(),
                                                       d->zoomSettings.getImageSmoothScale()
                                                      );

    if (d->cachedPixmaps.find(dd->drawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(dd->drawRect.topLeft(), pix);
        }
        else
        {
            painter->drawPixmap(dd->drawRect.topLeft(), pix, pixSourceRect);
        }
    }
    else
    {
        // TODO: factoring ICC settings code using ImageIface/EditorCore methods.

        // Apply CM settings.

        bool doSoftProofing              = EditorCore::defaultInstance()->softProofingEnabled();
        ICCSettingsContainer iccSettings = EditorCore::defaultInstance()->getICCSettings();

        if (iccSettings.enableCM && (iccSettings.useManagedView || doSoftProofing))
        {
            IccManager   manager(scaledImage);
            IccTransform monitorICCtrans;

            if (doSoftProofing)
            {
                monitorICCtrans = manager.displaySoftProofingTransform(IccProfile(iccSettings.defaultProofProfile), widget);
            }
            else
            {
                monitorICCtrans = manager.displayTransform(widget);
            }

            pix = scaledImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pix = scaledImage.convertToPixmap();
        }

        d->cachedPixmaps.insert(dd->drawRect, pix);

        painter->drawPixmap(dd->drawRect.topLeft(), pix);
    }

    if (dd->paintExtras && !dd->view->isBusy())
    {
        paintExtraData(painter);
    }

    // Show the Over/Under exposure pixels indicators

    ExposureSettingsContainer* const expoSettings = EditorCore::defaultInstance()->getExposureSettings();

    if (expoSettings)
    {
        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            painter->drawPixmap(dd->drawRect.topLeft(), pixMask);
        }
    }
}

void ImageRegionItem::paintExtraData(QPainter* const p)
{
    QRect viewportRect   = boundingRect().toAlignedRect();
    QRect fontRectBefore = p->fontMetrics().boundingRect(viewportRect, 0, i18n("Before"));
    QRect fontRectAfter  = p->fontMetrics().boundingRect(viewportRect, 0, i18n("After"));

    p->setRenderHint(QPainter::Antialiasing, true);
    p->setBackgroundMode(Qt::TransparentMode);

    if      (
             (dd->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage) ||
             (
              (dd->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver) &&
              !dd->onMouseMovePreviewToggled
             )
            )
    {
        dd->view->drawText(p, QRectF(QPointF(dd->drawRect.topLeft().x() + 20,
                                             dd->drawRect.topLeft().y() + 20),
                                             fontRectBefore.size()), i18n("Before"));
    }

    else if (
             (dd->renderingPreviewMode == PreviewToolBar::PreviewTargetImage) ||
             (dd->renderingPreviewMode == PreviewToolBar::NoPreviewMode)      ||
             (
              (dd->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver) &&
              dd->onMouseMovePreviewToggled
             )
            )
    {
        p->drawPixmap(dd->drawRect.x(), dd->drawRect.y(), dd->targetPix,
                      0, 0, dd->drawRect.width(), dd->drawRect.height());

        if (
            (dd->renderingPreviewMode == PreviewToolBar::PreviewTargetImage) ||
            (dd->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
           )
        {
            dd->view->drawText(p, QRectF(QPointF(dd->drawRect.topLeft().x() + 20,
                                                 dd->drawRect.topLeft().y() + 20),
                                                 fontRectAfter.size()), i18n("After"));
        }
    }

    else if (
             (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert) ||
             (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
            )
    {
        if (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
        {
            p->drawPixmap(dd->drawRect.x() + dd->drawRect.width() / 2, dd->drawRect.y(),
                          dd->targetPix, 0, 0, dd->drawRect.width() / 2, dd->drawRect.height());
        }

        if (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
        {
            p->drawPixmap(dd->drawRect.x() + dd->drawRect.width() / 2, dd->drawRect.y(),
                          dd->targetPix, dd->drawRect.width() / 2, 0, dd->drawRect.width(),
                          dd->drawRect.height());
        }

        p->setPen(QPen(Qt::white, 2, Qt::SolidLine));

        p->drawLine(dd->drawRect.topLeft().x() + dd->drawRect.width() / 2, dd->drawRect.topLeft().y(),
                    dd->drawRect.topLeft().x() + dd->drawRect.width() / 2, dd->drawRect.bottomLeft().y());

        p->setPen(QPen(Qt::red, 2, Qt::DotLine));

        p->drawLine(dd->drawRect.topLeft().x() + dd->drawRect.width() / 2, dd->drawRect.topLeft().y(),
                    dd->drawRect.topLeft().x() + dd->drawRect.width() / 2, dd->drawRect.bottomLeft().y());

        dd->view->drawText(p, QRectF(QPointF(dd->drawRect.topLeft().x() + 20,
                                             dd->drawRect.topLeft().y() + 20),
                                             fontRectBefore.size()), i18n("Before"));

        dd->view->drawText(p, QRectF(QPointF(dd->drawRect.topLeft().x() + dd->drawRect.width() / 2 + 20,
                                             dd->drawRect.topLeft().y() + 20),
                                             fontRectAfter.size()),  i18n("After"));
    }

    else if (
             (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz) ||
             (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
            )
    {
        if (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
        {
            p->drawPixmap(dd->drawRect.x(), dd->drawRect.y() + dd->drawRect.height() / 2,
                          dd->targetPix, 0, 0, dd->drawRect.width(), dd->drawRect.height() / 2);
        }

        if (dd->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
        {
            p->drawPixmap(dd->drawRect.x(), dd->drawRect.y() + dd->drawRect.height() / 2,
                          dd->targetPix, 0, dd->drawRect.height() / 2,
                          dd->drawRect.width(), dd->drawRect.height());
        }

        p->setPen(QPen(Qt::white, 2, Qt::SolidLine));

        p->drawLine(dd->drawRect.topLeft().x()  + 1, dd->drawRect.topLeft().y() + dd->drawRect.height() / 2,
                    dd->drawRect.topRight().x() - 1, dd->drawRect.topLeft().y() + dd->drawRect.height() / 2);

        p->setPen(QPen(Qt::red, 2, Qt::DotLine));

        p->drawLine(dd->drawRect.topLeft().x()  + 1, dd->drawRect.topLeft().y() + dd->drawRect.height() / 2,
                    dd->drawRect.topRight().x() - 1, dd->drawRect.topLeft().y() + dd->drawRect.height() / 2);

        dd->view->drawText(p, QRectF(QPointF(dd->drawRect.topLeft().x() + 20,
                                             dd->drawRect.topLeft().y() + 20),
                                             fontRectBefore.size()), i18n("Before"));

        dd->view->drawText(p, QRectF(QPointF(dd->drawRect.topLeft().x() + 20,
                                             dd->drawRect.topLeft().y() + dd->drawRect.height() / 2 + 20),
                                             fontRectAfter.size()),  i18n("After"));
    }

    // Drawing highlighted points.

    if (!dd->hightlightPoints.isEmpty())
    {
        QPoint pt;
        QRectF hpArea;

        for (int i = 0 ; i < dd->hightlightPoints.count() ; ++i)
        {
            pt                = dd->hightlightPoints.point(i);
            double zoomFactor = zoomSettings()->realZoomFactor();
            int x             = (int)((double)pt.x() * zoomFactor);
            int y             = (int)((double)pt.y() * zoomFactor);

            // Check if zoomed point is inside, not actual point

            if (dd->drawRect.contains(QPoint(x,y)))
            {
/*
                QPoint hp(contentsToViewport(QPointF(x, y)));
*/
                QPointF hp(mapToScene(QPointF(x, y)));

                hpArea.setSize(QSize((int)(16 * zoomFactor), (int)(16 * zoomFactor)));
                hpArea.moveCenter(hp);

                p->setPen(QPen(Qt::white, 2, Qt::SolidLine));
                p->drawLine(hp.x(),                         hpArea.y(),                     hp.x(),                         hp.y() - (int)(3 * zoomFactor));
                p->drawLine(hp.x(),                         hp.y() + (int)(3 * zoomFactor), hp.x(),                         hpArea.bottom());
                p->drawLine(hpArea.x(),                     hp.y(),                         hp.x() - (int)(3 * zoomFactor), hp.y());
                p->drawLine(hp.x() + (int)(3 * zoomFactor), hp.y(),                         hpArea.right(),                 hp.y());

                p->setPen(QPen(Qt::red, 2, Qt::DotLine));
                p->drawLine(hp.x(),                         hpArea.y(),                     hp.x(),                         hp.y() - (int)(3 * zoomFactor));
                p->drawLine(hp.x(),                         hp.y() + (int)(3 * zoomFactor), hp.x(),                         hpArea.bottom());
                p->drawLine(hpArea.x(),                     hp.y(),                         hp.x() - (int)(3 * zoomFactor), hp.y());
                p->drawLine(hp.x() + (int)(3 * zoomFactor), hp.y(),                         hpArea.right(),                 hp.y());
            }
        }
    }
}

void ImageRegionItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
    dd->onMouseMovePreviewToggled = false;
    update();
}

void ImageRegionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
    dd->onMouseMovePreviewToggled = true;
    update();
}

} // namespace Digikam

#include "moc_imageregionitem.cpp"

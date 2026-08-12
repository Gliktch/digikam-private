/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-27
 * Description : image preview item for image editor.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagepreviewitem.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <QPainter>
#include <QPixmap>

// Local includes

#include "dimg.h"
#include "exposurecontainer.h"
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "editorcore.h"
#include "dimgitems_p.h"

namespace Digikam
{

ImagePreviewItem::ImagePreviewItem()
{
    Q_D(GraphicsDImgItem);

    d->init(this);
}

void ImagePreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);

    QRect   drawRect     = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;
    QSize   completeSize = boundingRect().size().toSize();
    qreal   dpr          = widget->devicePixelRatio();

    /*
     * For high resolution ("retina") displays, Mac OS X / Qt
     * report only half of the physical resolution in terms of
     * pixels, i.e. every logical pixels corresponds to 2x2
     * physical pixels. However, UI elements and fonts are
     * nevertheless rendered at full resolution, and pixmaps
     * as well, provided their resolution is high enough (that
     * is, higher than the reported, logical resolution).
     *
     * To work around this, we render the photos not a logical
     * resolution, but with the photo's full resolution, but
     * at the screen's aspect ratio. When we later draw this
     * high resolution bitmap, it is up to Qt to scale the
     * photo to the true physical resolution.  The ratio
     * computed below is the ratio between the photo and
     * screen resolutions, or equivalently the factor by which
     * we need to increase the pixel size of the rendered
     * pixmap.
     */

    QRect scaledDrawRect(
                         floor(dpr * drawRect.x()),
                         floor(dpr * drawRect.y()),
                         floor(dpr * drawRect.width()),
                         floor(dpr * drawRect.height())
                        );

    // scale "as if" scaling to whole image, but clip output to our exposed region

    QSize scaledCompleteSize(
                             floor(dpr * completeSize.width()),
                             floor(dpr * completeSize.height())
                            );
    DImg scaledImage = d->image.smoothScaleClipped(
                                                   scaledCompleteSize.width(),
                                                   scaledCompleteSize.height(),
                                                   scaledDrawRect.x(),
                                                   scaledDrawRect.y(),
                                                   scaledDrawRect.width(),
                                                   scaledDrawRect.height(),
                                                   d->zoomSettings.getImageSmoothScale()
                                                  );

    if (d->cachedPixmaps.find(scaledDrawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(drawRect, pix);
        }
        else
        {
            painter->drawPixmap(drawRect, pix, pixSourceRect);
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

        d->cachedPixmaps.insert(scaledDrawRect, pix);
        painter->drawPixmap(drawRect, pix);
    }

    // Show the Over/Under exposure pixels indicators

    ExposureSettingsContainer* const expoSettings = EditorCore::defaultInstance()->getExposureSettings();

    if (expoSettings)
    {
        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            painter->drawPixmap(drawRect, pixMask);
        }
    }
}

} // namespace Digikam

#include "moc_imagepreviewitem.cpp"

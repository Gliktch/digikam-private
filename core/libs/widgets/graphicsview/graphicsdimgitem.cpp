/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "graphicsdimgitem.h"
#include "dimgitems_p.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QPoint>
#include <QRect>

// Local includes

#include "dimg.h"
#include "imagezoomsettings.h"
#include "exposurecontainer.h"
#include "editorcore.h"

namespace Digikam
{

CachedPixmaps::CachedPixmaps(int maxCount)
    : maxCount(maxCount)
{
}

CachedPixmaps::~CachedPixmaps()
{
    clear();
}

void CachedPixmaps::setMaxCount(int count)
{
    maxCount = count;
}

void CachedPixmaps::clear()
{
    for (const CachedPixmapKey& key : std::as_const(keys))
    {
        QPixmapCache::remove(key.key);
    }

    keys.clear();
}

bool CachedPixmaps::find(const QRect& region, QPixmap* const pix, QRect* const source)
{
    QQueue<CachedPixmapKey>::iterator key;

    for (key = keys.begin() ; key != keys.end() ; )
    {
        if (!key->region.contains(region))
        {
            ++key;
            continue;
        }

        if (!QPixmapCache::find(key->key, pix))
        {
            key = keys.erase(key);
            continue;
        }

        if (key->region == region)
        {
            *source = QRect();
        }
        else
        {
            QPoint startPoint = region.topLeft() - key->region.topLeft();
            *source           = QRect(startPoint, region.size());
        }

        return true;
    }

    return false;
}

void CachedPixmaps::insert(const QRect& region, const QPixmap& pixmap)
{
    if (keys.size() >= maxCount)
    {
        CachedPixmapKey key = keys.dequeue();
        QPixmapCache::remove(key.key);
    }

    CachedPixmapKey key;
    key.region = region;
    key.key    = QPixmapCache::insert(pixmap);
    keys.enqueue(key);
}

// ---------------------------------------------------------------------------------------

GraphicsDImgItem::GraphicsDImgItem(QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d_ptr          (new GraphicsDImgItemPrivate)
{
    d_ptr->init(this);
}

GraphicsDImgItem::GraphicsDImgItem(GraphicsDImgItemPrivate& dd, QGraphicsItem* const parent)
    : QGraphicsObject(parent),
      d_ptr          (&dd)
{
    d_ptr->init(this);
}

void GraphicsDImgItem::GraphicsDImgItemPrivate::init(GraphicsDImgItem* const q)
{
    // ItemCoordinateCache is very slow, DeviceCoordinateCache makes severe render artifacts

    q->setCacheMode(QGraphicsItem::NoCache);

    // This flag is crucial for our performance! Limits redrawing area.

    q->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
    q->setAcceptedMouseButtons(Qt::NoButton);
}

GraphicsDImgItem::~GraphicsDImgItem()
{
    Q_D(GraphicsDImgItem);

    delete d;
}

void GraphicsDImgItem::setImage(const DImg& img)
{
    Q_D(GraphicsDImgItem);

    d->image = img;
    d->zoomSettings.setImageSize(img.size(), img.originalSize());
    d->cachedPixmaps.clear();
    sizeHasChanged();

    Q_EMIT imageChanged();
}

DImg GraphicsDImgItem::image() const
{
    Q_D(const GraphicsDImgItem);

    return d->image;
}

void GraphicsDImgItem::setEnableUnderExposure(bool b)
{
    Q_D(GraphicsDImgItem);

    d->underExposure = b;
    update();
}

void GraphicsDImgItem::setEnableOverExposure(bool b)
{
    Q_D(GraphicsDImgItem);

    d->overExposure = b;
    update();
}

void GraphicsDImgItem::sizeHasChanged()
{
    Q_D(GraphicsDImgItem);

    QGraphicsItem::prepareGeometryChange();
    d->cachedPixmaps.clear();

    Q_EMIT imageSizeChanged(d->zoomSettings.zoomedSize());
}

void GraphicsDImgItem::clearCache()
{
    Q_D(GraphicsDImgItem);

    d->cachedPixmaps.clear();
}

const ImageZoomSettings* GraphicsDImgItem::zoomSettings() const
{
    Q_D(const GraphicsDImgItem);

    return &d->zoomSettings;
}

ImageZoomSettings* GraphicsDImgItem::zoomSettings()
{
    Q_D(GraphicsDImgItem);

    return &d->zoomSettings;
}

void GraphicsDImgItem::setDisplayWidget(QWidget* const widget)
{
    Q_D(GraphicsDImgItem);

    d->zoomSettings.setDisplayWidget(widget);
}

QRectF GraphicsDImgItem::boundingRect() const
{
    Q_D(const GraphicsDImgItem);

    // always return full integer sizes, we can only scale to integer

    return QRectF(QPointF(0, 0), d->zoomSettings.zoomedSize()).toAlignedRect();
}

void GraphicsDImgItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_D(GraphicsDImgItem);

    QRect   drawRect     = option->exposedRect.intersected(boundingRect()).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;
    QSize   completeSize = boundingRect().size().toSize();
    qreal   dpr          = widget->devicePixelRatio();

    /* For high resolution ("retina") displays, Mac OS X / Qt
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

        pix              = scaledImage.convertToPixmap();

        d->cachedPixmaps.insert(scaledDrawRect, pix);
        painter->drawPixmap(drawRect, pix);
    }

    // Show the Over/Under exposure pixels indicators

    if (EditorCore::defaultInstance())
    {
        ExposureSettingsContainer* const expoSettings = EditorCore::defaultInstance()->getExposureSettings();

        if (expoSettings)
        {
            expoSettings->underExposureIndicator = d->underExposure;
            expoSettings->overExposureIndicator  = d->overExposure;

            if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
            {
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

                QImage pureColorMask = scaledImage.pureColorMask(expoSettings);
                QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
                painter->drawPixmap(drawRect, pixMask);
            }
        }
    }
}

void GraphicsDImgItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e)
{
    Q_EMIT showContextMenu(e);
}

} // namespace Digikam

#include "moc_graphicsdimgitem.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a generic widget to display a panel to choose
 *               a rectangular image area.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "paniconwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QApplication>

// Local includes

#include "previewtoolbar.h"

namespace Digikam
{

class Q_DECL_HIDDEN PanIconWidget::Private
{

public:

    Private() = default;

public:

    bool      moveSelection       = false;

    int       width               = 0;
    int       height              = 0;
    int       zoomedOrgWidth      = 0;
    int       zoomedOrgHeight     = 0;
    int       orgWidth            = 0;
    int       orgHeight           = 0;
    int       xpos                = 0;
    int       ypos                = 0;

    int       previewMode         = PreviewToolBar::PreviewOriginalImage;

    double    zoomFactor          = 1.0;

    QRect     regionSelection;                ///< Original size image selection.

    QRect     rect;
    QRect     localRegionSelection;           ///< Thumbnail size selection.

    QPixmap   pixmap;
};

PanIconWidget::PanIconWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);
}

PanIconWidget::~PanIconWidget()
{
    delete d;
}

void PanIconWidget::setPreviewMode(int previewMode)
{
    d->previewMode = previewMode;
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const QImage& image)
{
    QSize sz(image.width(), image.height());
    sz.scale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    QImage scaledImg = image.scaled(sz.width(), sz.height(),
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setImage(scaledImg, image.size());
}

void PanIconWidget::setImage(const QImage& scaledPreviewImage, const QSize& fullImageSize)
{
    d->width           = scaledPreviewImage.width();
    d->height          = scaledPreviewImage.height();
    d->orgWidth        = fullImageSize.width();
    d->orgHeight       = fullImageSize.height();
    d->zoomedOrgWidth  = fullImageSize.width();
    d->zoomedOrgHeight = fullImageSize.height();
    d->pixmap          = QPixmap(d->width, d->height);
    d->pixmap.fill(palette().color(QPalette::Window));
    QPainter p(&d->pixmap);
    p.drawImage(0, 0, scaledPreviewImage);

    setFixedSize(d->width, d->height);

    d->rect = QRect(width()  / 2 - d->width  / 2,
                    height() / 2 - d->height / 2,
                    d->width,
                    d->height);
    update();
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const DImg& image)
{
    DImg img = DImg(image).smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    setImage(img.copyQImage(), image.size());
}

void PanIconWidget::slotZoomFactorChanged(double factor)
{
    if (d->zoomFactor == factor)
    {
        return;
    }

    d->zoomFactor      = factor;
    d->zoomedOrgWidth  = (int)(d->orgWidth  * factor);
    d->zoomedOrgHeight = (int)(d->orgHeight * factor);
    update();
}

void PanIconWidget::setRegionSelection(const QRect& regionSelection)
{
    if ((d->zoomedOrgHeight == 0) || d->moveSelection)
    {
        return;
    }

    d->regionSelection = regionSelection;
    d->localRegionSelection.setX(d->rect.x() + (int)((float)d->regionSelection.x() *
                                 ((float)d->width / (float)d->zoomedOrgWidth)));

    d->localRegionSelection.setY(d->rect.y() + (int)((float)d->regionSelection.y() *
                                 ((float)d->height / (float)d->zoomedOrgHeight)));

    d->localRegionSelection.setWidth((int)((float)d->regionSelection.width() *
                                           ((float)d->width / (float)d->zoomedOrgWidth)));

    d->localRegionSelection.setHeight((int)((float)d->regionSelection.height() *
                                      ((float)d->height / (float)d->zoomedOrgHeight)));

    update();
}

QRect PanIconWidget::getRegionSelection() const
{
    return (d->regionSelection);
}

void PanIconWidget::regionSelectionMoved(bool targetDone)
{
    if (targetDone)
    {
        update();
    }

    int x = (int)lround(((float)d->localRegionSelection.x() - (float)d->rect.x() ) *
                        ((float)d->zoomedOrgWidth / (float)d->width));

    int y = (int)lround(((float)d->localRegionSelection.y() - (float)d->rect.y() ) *
                        ((float)d->zoomedOrgHeight / (float)d->height));

    int w = (int)lround((float)d->localRegionSelection.width() *
                        ((float)d->zoomedOrgWidth / (float)d->width));

    int h = (int)lround((float)d->localRegionSelection.height() *
                        ((float)d->zoomedOrgHeight / (float)d->height));

    d->regionSelection.setX(x);
    d->regionSelection.setY(y);
    d->regionSelection.setWidth(w);
    d->regionSelection.setHeight(h);

    Q_EMIT signalSelectionMoved(d->regionSelection, targetDone);
}

void PanIconWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.drawPixmap(d->rect.x(), d->rect.y(), d->pixmap);

    // Drawing selection border

    QRect r(d->localRegionSelection);

    // Clamp to widget size. Selection area must always be visible

    if (r.left() < 0)
    {
        r.setLeft(0);
    }

    if (r.top() < 0)
    {
        r.setTop(0);
    }

    if (r.right() > (width() - 2))
    {
        r.setRight(width() - 2);
    }

    if (r.bottom() > (height() - 2))
    {
        r.setBottom(height() - 2);
    }

    QPainterPath maskPath;
    maskPath.addRect(d->rect);  // Full image area
    maskPath.addRect(r);        // Exclude the selection rectangle

    // Increase the contrast to decrease the luminosity outside the selection

    p.save();
    p.setClipPath(maskPath);
    p.fillRect(d->rect, QColor(0, 0, 0, 128)); // Uses middle transparency
    p.restore();

    // Fill the non suitable area and draw the separator
    // from the selection if necessary depending of the preview mode.

    switch (d->previewMode)
    {
        case PreviewToolBar::PreviewBothImagesHorz:
        {
            p.fillRect(r.left(),  r.top() + r.height() / 2,
                       r.width(), r.height() / 2, QColor(0, 0, 0, 128));
            p.drawLine(r.left(),  r.top() + r.height() / 2,
                       r.right(), r.top() + r.height() / 2);
            break;
        }

        case PreviewToolBar::PreviewBothImagesHorzCont:
        {
            p.drawLine(r.left(),  r.top() + r.height() / 2,
                       r.right(), r.top() + r.height() / 2);
            break;
        }

        case PreviewToolBar::PreviewBothImagesVert:
        {
            p.fillRect(r.left() + r.width() / 2, r.top(),
                       r.width() / 2, r.height(), QColor(0, 0, 0, 128));
            p.drawLine(r.left() + r.width() / 2, r.top(),
                       r.left() + r.width() / 2, r.bottom());
            break;
        }

        case PreviewToolBar::PreviewBothImagesVertCont:
        {
            p.drawLine(r.left() + r.width() / 2, r.top(),
                       r.left() + r.width() / 2, r.bottom());
            break;
        }

        default:    // None
        {
            break;
        }
    }

    // Draw the selection rectangle with a white border

    p.setPen(QPen(QColor(255, 255, 255, 255), 2, Qt::SolidLine));
    p.drawRect(r);

    // Draw a rectangle around the whole image.

    p.setPen(QPen(QColor(255, 255, 255, 64), 1, Qt::SolidLine));
    p.drawRect(d->rect);
}

void PanIconWidget::mousePressEvent(QMouseEvent* e)
{
    if (
        ((e->button() == Qt::LeftButton) || (e->button() == Qt::MiddleButton)) &&

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

         d->localRegionSelection.contains(e->position().toPoint().x(), e->position().toPoint().y())

#else

         d->localRegionSelection.contains(e->x(), e->y())

#endif

       )
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        d->xpos          = e->position().toPoint().x();
        d->ypos          = e->position().toPoint().y();

#else

        d->xpos          = e->x();
        d->ypos          = e->y();

#endif

        d->moveSelection = true;
        setCursor(Qt::SizeAllCursor);
    }
}

void PanIconWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (
        d->moveSelection &&
        ((e->buttons() == Qt::LeftButton) || (e->buttons() == Qt::MiddleButton))
       )
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        int newxpos = e->position().toPoint().x();
        int newypos = e->position().toPoint().y();

#else

        int newxpos = e->x();
        int newypos = e->y();

#endif

        d->localRegionSelection.translate(newxpos - d->xpos, newypos - d->ypos);

        d->xpos = newxpos;
        d->ypos = newypos;

        // Perform normalization of selection area.

        if (d->localRegionSelection.left() < d->rect.left())
        {
            d->localRegionSelection.moveLeft(d->rect.left());
        }

        if (d->localRegionSelection.top() < d->rect.top())
        {
            d->localRegionSelection.moveTop(d->rect.top());
        }

        if (d->localRegionSelection.right() > d->rect.right())
        {
            d->localRegionSelection.moveRight(d->rect.right());
        }

        if (d->localRegionSelection.bottom() > d->rect.bottom())
        {
            d->localRegionSelection.moveBottom(d->rect.bottom());
        }

        update();
        regionSelectionMoved(false);

        return;
    }
    else
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        if (d->localRegionSelection.contains(e->position().toPoint().x(), e->position().toPoint().y()))

#else

        if (d->localRegionSelection.contains(e->x(), e->y()))

#endif

        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void PanIconWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (d->moveSelection)
    {
        d->moveSelection = false;
        setCursor(Qt::ArrowCursor);
        regionSelectionMoved(true);
    }
}

} // namespace Digikam

#include "moc_paniconwidget.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * SPDX-FileCopyrightText:      1997 by Martin Jones <mjones at kde dot org>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dhuesaturationselect.h"

// Qt includes

#include <QStyle>
#include <QPainter>
#include <QStyleOptionFrame>
#include <QMouseEvent>

// Local includes

#include "digikam_debug.h"
#include "dcolorchoosermode_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DPointSelect::Private
{
public:

    explicit Private(DPointSelect* const qq)
        : q(qq)
    {
    }

    void setValues(int _xPos, int _yPos);

public:

    DPointSelect* q             = nullptr;
    int           px            = 0;
    int           py            = 0;
    int           xPos          = 0;
    int           yPos          = 0;
    int           minX          = 0;
    int           maxX          = 100;
    int           minY          = 0;
    int           maxY          = 100;
    QColor        m_markerColor = Qt::white;
};

void DPointSelect::Private::setValues(int _xPos, int _yPos)
{
    int w = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    xPos  = _xPos;
    yPos  = _yPos;

    if      (xPos > maxX)
    {
        xPos = maxX;
    }
    else if (xPos < minX)
    {
        xPos = minX;
    }

    if      (yPos > maxY)
    {
        yPos = maxY;
    }
    else if (yPos < minY)
    {
        yPos = minY;
    }

    Q_ASSERT(maxX != minX);

    int xp = w + (q->width() - 2 * w) * xPos / (maxX - minX);

    Q_ASSERT(maxY != minY);

    int yp = q->height() - w - (q->height() - 2 * w) * yPos / (maxY - minY);

    q->setPosition(xp, yp);
}

DPointSelect::DPointSelect(QWidget* const parent)
    : QWidget(parent),
      d      (new Private(this))
{
}

DPointSelect::~DPointSelect()
{
    delete d;
}

int DPointSelect::xValue() const
{
    return d->xPos;
}

int DPointSelect::yValue() const
{
    return d->yPos;
}

void DPointSelect::setRange(int _minX, int _minY, int _maxX, int _maxY)
{
    if (_maxX == _minX)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DPointSelect::setRange invalid range: "
                                       << _maxX << " == " << _minX << " (for X) ";
        return;
    }

    if (_maxY == _minY)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "DPointSelect::setRange invalid range: "
                                       << _maxY << " == " << _minY << " (for Y) ";
        return;
    }

    int w   = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    d->px   = w;
    d->py   = w;
    d->minX = _minX;
    d->minY = _minY;
    d->maxX = _maxX;
    d->maxY = _maxY;
}

void DPointSelect::setXValue(int _xPos)
{
    setValues(_xPos, d->yPos);
}

void DPointSelect::setYValue(int _yPos)
{
    setValues(d->xPos, _yPos);
}

void DPointSelect::setValues(int _xPos, int _yPos)
{
    d->setValues(_xPos, _yPos);
}

void DPointSelect::setMarkerColor(const QColor &col)
{
    d->m_markerColor =  col;
}

QRect DPointSelect::contentsRect() const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    return rect().adjusted(w, w, -w, -w);
}

QSize DPointSelect::minimumSizeHint() const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    return QSize(2 * w, 2 * w);
}

void DPointSelect::paintEvent(QPaintEvent * /* ev */)
{
    QStyleOptionFrame opt;
    opt.initFrom(this);

    QPainter painter;
    painter.begin(this);

    drawContents(&painter);
    drawMarker(&painter, d->px, d->py);

    style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, this);

    painter.end();
}

void DPointSelect::mousePressEvent(QMouseEvent* e)
{
    mouseMoveEvent(e);
}

void DPointSelect::mouseMoveEvent(QMouseEvent* e)
{
    int xVal, yVal;
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    valuesFromPosition(e->pos().x() - w, e->pos().y() - w, xVal, yVal);
    setValues(xVal, yVal);

    Q_EMIT valueChanged(d->xPos, d->yPos);
}

void DPointSelect::wheelEvent(QWheelEvent* e)
{
    QPoint delta = e->angleDelta();

    if      (delta.x() != 0)
    {
        setValues(xValue() + delta.x() / 120, yValue());
    }
    else if (delta.y() != 0)
    {
        setValues(xValue(), yValue() + delta.y() / 120);
    }

    Q_EMIT valueChanged(d->xPos, d->yPos);
}

void DPointSelect::valuesFromPosition(int x, int y, int& xVal, int& yVal) const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    xVal  = ((d->maxX - d->minX) * (x - w)) / (width() - 2 * w);
    yVal  = d->maxY - (((d->maxY - d->minY) * (y - w)) / (height() - 2 * w));

    if      (xVal > d->maxX)
    {
        xVal = d->maxX;
    }
    else if (xVal < d->minX)
    {
        xVal = d->minX;
    }

    if      (yVal > d->maxY)
    {
        yVal = d->maxY;
    }
    else if (yVal < d->minY)
    {
        yVal = d->minY;
    }
}

void DPointSelect::setPosition(int xp, int yp)
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    if      (xp < w)
    {
        xp = w;
    }
    else if (xp > width() - w)
    {
        xp = width() - w;
    }

    if      (yp < w)
    {
        yp = w;
    }
    else if (yp > height() - w)
    {
        yp = height() - w;
    }

    d->px = xp;
    d->py = yp;

    update();
}

void DPointSelect::drawMarker(QPainter* p, int xp, int yp)
{
    QPen pen(d->m_markerColor);
    p->setPen(pen);
    p->drawEllipse(xp - 4, yp - 4, 8, 8);
}

// --------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DHueSaturationSelector::Private
{
public:

    explicit Private(DHueSaturationSelector* const qq)
        : q(qq)
    {
    }

public:

    DHueSaturationSelector* q           = nullptr;
    QPixmap                 pixmap;

    /**
     * Stores the chooser mode
     */
    DColorChooserMode       mode        = ChooserClassic;

    /**
     * Stores the values for hue, saturation and luminosity
     */
    int                     hue         = 0;
    int                     saturation  = 0;
    int                     color       = 0;
};

DHueSaturationSelector::DHueSaturationSelector(QWidget* const parent)
    : DPointSelect(parent),
      d           (new Private(this))
{
    setChooserMode(ChooserClassic);
}

DHueSaturationSelector::~DHueSaturationSelector()
{
    delete d;
}

DColorChooserMode DHueSaturationSelector::chooserMode() const
{
    return d->mode;
}

void DHueSaturationSelector::setChooserMode(DColorChooserMode chooserMode)
{
    int x = 0;
    int y = 255;

    switch (chooserMode)
    {
        case ChooserSaturation:
        case ChooserValue:
        {
            x = 359;
            break;
        }

        default:
        {
            x = 255;
            break;
        }
    }

    setRange(0, 0, x, y);
    d->mode = chooserMode;
}

int DHueSaturationSelector::hue() const
{
    return d->hue;
}

void DHueSaturationSelector::setHue(int hue)
{
    d->hue = hue;
}

int DHueSaturationSelector::saturation() const
{
    return d->saturation;
}

void DHueSaturationSelector::setSaturation(int saturation)
{
    d->saturation = saturation;
}

int DHueSaturationSelector::colorValue() const
{
    return d->color;
}

void DHueSaturationSelector::setColorValue(int color)
{
    d->color = color;
}

void DHueSaturationSelector::updateContents()
{
    drawPalette(&d->pixmap);
}

void DHueSaturationSelector::resizeEvent(QResizeEvent*)
{
    updateContents();
}

void DHueSaturationSelector::drawContents(QPainter* painter)
{
    painter->drawPixmap(contentsRect().x(), contentsRect().y(), d->pixmap);
}

void DHueSaturationSelector::drawPalette(QPixmap* pixmap)
{
    int xSteps = componentXSteps(chooserMode());
    int ySteps = componentYSteps(chooserMode());

    QColor color;
    color.setHsv(hue(), saturation(), (chooserMode() == ChooserClassic) ? 192 : colorValue());

    QImage image(QSize(xSteps + 1, ySteps + 1), QImage::Format_RGB32);

    for (int y = 0 ; y <= ySteps ; ++y)
    {
        setComponentY(color, chooserMode(), y * (1.0 / ySteps));

        for (int x = 0 ; x <= xSteps ; ++x)
        {
            setComponentX(color, chooserMode(), x * (1.0 / xSteps));
            image.setPixel(x, ySteps - y, color.rgb());
        }
    }

    QPixmap pix(contentsRect().size());
    QPainter painter(&pix);

    // Bilinear filtering

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QRectF srcRect(0.5, 0.5, xSteps, ySteps);
    QRectF destRect(QPointF(0, 0), contentsRect().size());
    painter.drawImage(destRect, image, srcRect);
    painter.end();

    *pixmap = pix;
}

} // namespace Digikam

#include "moc_dhuesaturationselect.cpp"

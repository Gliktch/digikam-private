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

#include "dcolorvalueselector.h"

// Qt includes

#include <QPainter>
#include <QImage>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>

// Local includes

#include "dcolorchoosermode_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DSelector::Private
{
public:

    Private() = default;

public:

    const int                arrowsize  = 5;
    bool                     m_indent   = true;
    QStyle::PrimitiveElement arrowPE    = QStyle::PE_IndicatorArrowLeft;
};

DSelector::DSelector(QWidget* const parent)
    : QAbstractSlider(parent),
      d              (new Private)
{
    setOrientation(Qt::Horizontal);
}

DSelector::DSelector(Qt::Orientation o, QWidget* const parent)
    : QAbstractSlider(parent),
      d              (new Private)
{
    setOrientation(o);

    if (o == Qt::Horizontal)
    {
        setArrowDirection(Qt::UpArrow);
    }
}

DSelector::~DSelector()
{
    delete d;
}

void DSelector::setIndent(bool i)
{
    d->m_indent = i;
}

bool DSelector::indent() const
{
    return d->m_indent;
}

QRect DSelector::contentsRect() const
{
    int w  = indent() ? style()->pixelMetric(QStyle::PM_DefaultFrameWidth) : 0;
    int iw = (w < d->arrowsize) ? d->arrowsize : w;

    if (orientation() == Qt::Vertical)
    {
        if (arrowDirection() == Qt::RightArrow)
        {
            return QRect(w + d->arrowsize, iw,
                         width() - w*2 - d->arrowsize,
                         height() - iw*2);
        }
        else
        {
            return QRect(w, iw,
                         width() - w*2 - d->arrowsize,
                         height() - iw*2);
        }
    }
    else
    {
        // Qt::Horizontal

        if (arrowDirection() == Qt::UpArrow)
        {
            return QRect(iw, w,
                         width() - 2*iw,
                         height() - w*2 - d->arrowsize);
        }
        else
        {
            return QRect(iw, w + d->arrowsize,
                         width() - 2*iw,
                         height() - w*2 - d->arrowsize);
        }
    }
}

void DSelector::paintEvent(QPaintEvent*)
{
    QPainter painter;
    int w  = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int iw = (w < d->arrowsize) ? d->arrowsize : w;

    painter.begin(this);

    drawContents(&painter);

    QPoint pos = calcArrowPos(value());
    drawArrow(&painter, pos);

    if (indent())
    {
        QStyleOptionFrame opt;
        opt.initFrom(this);
        opt.state = QStyle::State_Sunken;

        if (orientation() == Qt::Vertical)
        {
            opt.rect.adjust(0, iw - w, -5, w - iw);
        }
        else
        {
            opt.rect.adjust(iw - w, 0, w - iw, -5);
        }

        QBrush oldBrush = painter.brush();
        painter.setBrush(Qt::NoBrush);
        style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, this);
        painter.setBrush(oldBrush);
    }

    painter.end();
}

void DSelector::mousePressEvent(QMouseEvent* e)
{
    setSliderDown(true);
    moveArrow(e->pos());
}

void DSelector::mouseMoveEvent(QMouseEvent* e)
{
    moveArrow(e->pos());
}

void DSelector::mouseReleaseEvent(QMouseEvent* e)
{
    moveArrow(e->pos());
    setSliderDown(false);
}

void DSelector::wheelEvent(QWheelEvent* e)
{
    int val = value() + e->angleDelta().y() / 120;
    setSliderDown(true);
    setValue(val);
    setSliderDown(false);
}

void DSelector::moveArrow(const QPoint& pos)
{
    int val;
    int w  = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int iw = (w < d->arrowsize) ? d->arrowsize : w;

    if (orientation() == Qt::Vertical)
    {
        val = (maximum() - minimum()) * (height() - pos.y() - iw) /
              (height() - iw * 2) + minimum();
    }
    else
    {
        val = (maximum() - minimum()) * ( pos.x() - iw) /
              (width() - iw * 2) + minimum();
    }

    setValue(val);
    update();
}

QPoint DSelector::calcArrowPos(int val)
{
    QPoint p;
    int w  = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int iw = (w < d->arrowsize) ? d->arrowsize : w;

    if (orientation() == Qt::Vertical)
    {
        p.setY(height() - iw - 1 - (height() - 2 * iw - 1) * val  / (maximum() - minimum()));

        if (d->arrowPE == QStyle::PE_IndicatorArrowRight)
        {
            p.setX(0);
        }
        else
        {
            p.setX(width() - 5);
        }
    }
    else
    {
        p.setX(iw + (width() - 2 * iw - 1) * val  / (maximum() - minimum()));

        if (d->arrowPE == QStyle::PE_IndicatorArrowDown)
        {
            p.setY(0);
        }
        else
        {
            p.setY(height() - 5);
        }
    }

    return p;
}

void DSelector::setArrowDirection(Qt::ArrowType direction)
{
    switch (direction)
    {
        case Qt::UpArrow:
        {
            if (orientation() == Qt::Horizontal)
            {
                d->arrowPE = QStyle::PE_IndicatorArrowUp;
            }
            else
            {
                d->arrowPE = QStyle::PE_IndicatorArrowLeft;
            }

            break;
        }

        case Qt::DownArrow:
        {
            if (orientation() == Qt::Horizontal)
            {
                d->arrowPE = QStyle::PE_IndicatorArrowDown;
            }
            else
            {
                d->arrowPE = QStyle::PE_IndicatorArrowRight;
            }

            break;
        }

        case Qt::LeftArrow:
        {
            if (orientation() == Qt::Vertical)
            {
                d->arrowPE = QStyle::PE_IndicatorArrowLeft;
            }
            else
            {
                d->arrowPE = QStyle::PE_IndicatorArrowDown;
            }

            break;
        }

        case Qt::RightArrow:
        {
            if (orientation() == Qt::Vertical)
            {
                d->arrowPE = QStyle::PE_IndicatorArrowRight;
            }
            else
            {
                d->arrowPE = QStyle::PE_IndicatorArrowUp;
            }

            break;
        }

        case Qt::NoArrow:
        {
            break;
        }
    }
}

Qt::ArrowType DSelector::arrowDirection() const
{
    switch (d->arrowPE)
    {
        case QStyle::PE_IndicatorArrowUp:
        {
            return Qt::UpArrow;
        }

        case QStyle::PE_IndicatorArrowDown:
        {
            return Qt::DownArrow;
        }

        case QStyle::PE_IndicatorArrowRight:
        {
            return Qt::RightArrow;
        }

        case QStyle::PE_IndicatorArrowLeft:
        default:
        {
            return Qt::LeftArrow;
        }
    }
}

void DSelector::drawArrow(QPainter* painter, const QPoint& pos)
{
    painter->setPen(QPen());
    painter->setBrush(QBrush(palette().color(QPalette::ButtonText)));

    QStyleOption o;

    if (orientation() == Qt::Vertical)
    {
        o.rect = QRect(pos.x(), pos.y() - d->arrowsize / 2,
                       d->arrowsize, d->arrowsize);
    }
    else
    {
        o.rect = QRect(pos.x() - d->arrowsize / 2, pos.y(),
                       d->arrowsize, d->arrowsize);

    }

    style()->drawPrimitive(d->arrowPE, &o, painter, this);
}

// -------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DColorValueSelector::Private
{
public:

    explicit Private(DColorValueSelector* const qq)
        : q(qq)
    {
    }

    DColorValueSelector* q          = nullptr;
    int                  hue        = 0;
    int                  saturation = 0;
    int                  color      = 0;
    DColorChooserMode    mode       = ChooserClassic;
    QPixmap              pixmap;
};

DColorValueSelector::DColorValueSelector(QWidget* const parent)
    : DSelector(Qt::Vertical, parent),
      d        (new Private(this))
{
    setRange(0, 255);
}

DColorValueSelector::DColorValueSelector(Qt::Orientation o, QWidget* const parent)
    : DSelector(o, parent),
      d        (new Private(this))
{
    setRange(0, 255);
}

DColorValueSelector::~DColorValueSelector()
{
    delete d;
}

int DColorValueSelector::hue() const
{
    return d->hue;
}

void DColorValueSelector::setHue(int hue)
{
    d->hue = hue;
}

int DColorValueSelector::saturation() const
{
    return d->saturation;
}

void DColorValueSelector::setSaturation(int saturation)
{
    d->saturation = saturation;
}

int DColorValueSelector::colorValue() const
{
    return d->color;
}

void DColorValueSelector::setColorValue(int colorValue)
{
    d->color = colorValue;
}

void DColorValueSelector::updateContents()
{
    drawPalette(&d->pixmap);
}

void DColorValueSelector::resizeEvent(QResizeEvent*)
{
    updateContents();
}

void DColorValueSelector::drawContents(QPainter* painter)
{
    painter->drawPixmap(contentsRect().x(), contentsRect().y(), d->pixmap);
}

void DColorValueSelector::setChooserMode(DColorChooserMode c)
{
    if (c == ChooserHue)
    {
        setRange(0, 360);
    }
    else
    {
        setRange(0, 255);
    }

    d->mode = c;
}

DColorChooserMode DColorValueSelector::chooserMode() const
{
    return d->mode;
}

void DColorValueSelector::drawPalette(QPixmap* pixmap)
{
    QColor color;

    if (chooserMode() == ChooserHue)
    {
        color.setHsv(hue(), 255, 255);
    }
    else
    {
        color.setHsv(hue(), saturation(), colorValue());
    }

    QLinearGradient gradient;

    if (orientation() == Qt::Vertical)
    {
        gradient.setStart(0, contentsRect().height());
        gradient.setFinalStop(0, 0);
    }
    else
    {
        gradient.setStart(0, 0);
        gradient.setFinalStop(contentsRect().width(), 0);
    }

    const int steps = componentValueSteps(chooserMode());

    for (int v = 0 ; v <= steps ; ++v)
    {
        setComponentValue(color, chooserMode(), v * (1.0 / steps));
        gradient.setColorAt(v * (1.0 / steps), color);
    }

    *pixmap = QPixmap(contentsRect().size());
    QPainter painter(pixmap);
    painter.fillRect(pixmap->rect(), gradient);
}

} // namespace Digikam

#include "moc_dcolorvalueselector.cpp"

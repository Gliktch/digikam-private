/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : a color gradient slider
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2008 by Cyrille Berger <cberger at cberger dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dgradientslider.h"

// Qt includes

#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QMouseEvent>

namespace Digikam
{

class Q_DECL_HIDDEN DGradientSlider::Private
{

public:

    enum Cursor
    {
        NoCursor = 0,
        LeftCursor,
        RightCursor,
        MiddleCursor
    };

public:

    Private()
    {
        middleColor.setRgb((leftColor.red()   + rightColor.red())   / 2,
                           (leftColor.green() + rightColor.green()) / 2,
                           (leftColor.blue()  + rightColor.blue())  / 2);
    }

    int gradientHeight() const
    {
        return (parent->height() / 3);
    }

    int cursorWidth() const
    {
        return gradientHeight();
    }

    int gradientWidth() const
    {
        return (parent->width() - cursorWidth());
    }

    int gradientOffset() const
    {
        return (cursorWidth() / 2);
    }

public:

    bool             showMiddleCursor   = false;

    double           leftCursor         = 0.0;
    double           middleCursor       = 0.5;
    double           rightCursor        = 1.0;

    QColor           leftColor          = Qt::black;
    QColor           rightColor         = Qt::white;
    QColor           middleColor;

    DGradientSlider* parent             = nullptr;

    Cursor           activeCursor       = NoCursor;
};

DGradientSlider::DGradientSlider(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->parent = this;

    setMinimumWidth(256);
    setFixedHeight(20);
}

DGradientSlider::~DGradientSlider()
{
    delete d;
}

int DGradientSlider::gradientOffset() const
{
    return d->gradientOffset();
}

void DGradientSlider::drawCursorAt(QPainter& painter,
                                   double pos,
                                   const QColor& brushColor,
                                   int width,
                                   int height,
                                   int gradientWidth)
{
    painter.setBrush(brushColor);
    int pos2  = (int)(gradientWidth * pos);
    QPoint points[3];
    points[0] = QPoint(pos2, 3 * height - 1);
    points[1] = QPoint(pos2 + width / 2, 2 * height);
    points[2] = QPoint(pos2 + width, 3 * height - 1);
    painter.drawPolygon(points, 3);
}

void DGradientSlider::paintEvent(QPaintEvent*)
{
    int grdHeight = d->gradientHeight();
    int curWidth  = d->cursorWidth();
    int grdWidth  = d->gradientWidth();
    int grdOffset = d->gradientOffset();

    QPainter painter(this);

    // Draw first gradient

    QLinearGradient lrGradient(QPointF(0, 0), QPointF(grdWidth, 0));
    lrGradient.setColorAt(0.0, d->leftColor);
    lrGradient.setColorAt(1.0, d->rightColor);
    painter.setPen(Qt::NoPen);
    painter.setBrush(lrGradient);
    painter.drawRect(grdOffset, 0, grdWidth, grdHeight);

    // Draw second gradient

    QLinearGradient lrcGradient(QPointF(0, 0), QPointF(grdWidth, 0));
    lrcGradient.setColorAt(d->leftCursor, d->leftColor);

    if (d->showMiddleCursor)
    {
        lrcGradient.setColorAt(d->middleCursor, d->middleColor);
    }

    lrcGradient.setColorAt(d->rightCursor, d->rightColor);
    painter.setBrush(lrcGradient);
    painter.drawRect(grdOffset, grdHeight, grdWidth, grdHeight);

    // Draw cursors

    painter.setPen(palette().color(QPalette::Text));
    drawCursorAt(painter, d->leftCursor, d->leftColor, curWidth, grdHeight, grdWidth);

    if (d->showMiddleCursor)
    {
        drawCursorAt(painter, d->middleCursor, d->middleColor, curWidth, grdHeight, grdWidth);
    }

    drawCursorAt(painter, d->rightCursor, d->rightColor, curWidth, grdHeight, grdWidth);
}

inline bool isCursorClicked(const QPoint& pos,
                            double cursorPos,
                            int width,
                            int height,
                            int gradientWidth)
{
    int pos2 = (int)(gradientWidth * cursorPos);

    return ((pos.y() >= 2 * height)    &&
            (pos.y()  < 3 * height)     &&
            (pos.x() >= pos2)          &&
            (pos.x() <= (pos2 + width)));
}

void DGradientSlider::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        int grdHeight = d->gradientHeight();
        int curWidth  = d->cursorWidth();
        int grdWidth  = d->gradientWidth();

        // Select cursor

        if      (isCursorClicked(e->pos(), d->leftCursor , curWidth, grdHeight, grdWidth))
        {
            d->activeCursor = Private::LeftCursor;
        }
        else if (d->showMiddleCursor && isCursorClicked(e->pos(), d->middleCursor , curWidth, grdHeight, grdWidth))
        {
            d->activeCursor = Private::MiddleCursor;
        }
        else if (isCursorClicked(e->pos(), d->rightCursor, curWidth, grdHeight, grdWidth))
        {
            d->activeCursor = Private::RightCursor;
        }
    }
}

void DGradientSlider::mouseReleaseEvent(QMouseEvent*)
{
    d->activeCursor = Private::NoCursor;
}

void DGradientSlider::mouseMoveEvent(QMouseEvent* e)
{
    double v = (e->pos().x() - d->gradientOffset()) / (double) d->gradientWidth();

    switch (d->activeCursor)
    {
        case Private::LeftCursor:
        {
            setLeftValue(v);
            break;
        }

        case Private::MiddleCursor:
        {
            setMiddleValue(v);
            break;
        }

        case Private::RightCursor:
        {
            setRightValue(v);
            break;
        }

        default:
        {
            break;
        }
    }
}

void DGradientSlider::leaveEvent(QEvent*)
{
    d->activeCursor = Private::NoCursor;
}

void DGradientSlider::showMiddleCursor(bool b)
{
    d->showMiddleCursor = b;
}

double DGradientSlider::leftValue() const
{
    return d->leftCursor;
}

double DGradientSlider::rightValue() const
{
    return d->rightCursor;
}

double DGradientSlider::middleValue() const
{
    return d->middleCursor;
}

void DGradientSlider::adjustMiddleValue(double newLeftValue,
                                        double newRightValue)
{
    double newDist  = newRightValue   - newLeftValue;
    double oldDist  = d->rightCursor  - d->leftCursor;
    double oldPos   = d->middleCursor - d->leftCursor;
    d->middleCursor = oldPos * newDist / oldDist + newLeftValue;
}

void DGradientSlider::setRightValue(double v)
{
    if ((v <= 1.0)          &&
        (v > d->leftCursor) &&
        (v != d->rightCursor))
    {
        adjustMiddleValue(d->leftCursor, v);
        d->rightCursor = v;
        update();

        Q_EMIT rightValueChanged(v);
        Q_EMIT middleValueChanged(d->middleCursor);
    }
}

void DGradientSlider::setLeftValue(double v)
{
    if ((v >= 0.0)           &&
        (v != d->leftCursor) &&
        (v < d->rightCursor))
    {
        adjustMiddleValue(v, d->rightCursor);
        d->leftCursor = v;
        update();

        Q_EMIT leftValueChanged(v);
        Q_EMIT middleValueChanged(d->middleCursor);
    }
}

void DGradientSlider::setMiddleValue(double v)
{
    if ((v > d->leftCursor) && (v != d->middleCursor) && (v < d->rightCursor))
    {
        d->middleCursor = v;
        update();

        Q_EMIT middleValueChanged(v);
    }
}

void DGradientSlider::setColors(const QColor& lcolor, const QColor& rcolor)
{
    d->leftColor  = lcolor;
    d->rightColor = rcolor;
    update();
}

} // namespace Digikam

#include "moc_dgradientslider.cpp"

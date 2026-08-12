/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * SPDX-FileCopyrightText: 2005      by Owen Hirst <n8rider@sbcglobal.net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ratingwidget_p.h"

namespace Digikam
{

RatingWidget::RatingWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    slotThemeChanged();

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

RatingWidget::~RatingWidget()
{
    delete d;
}

void RatingWidget::setupTimeLine()
{
    delete d->fadingTimeLine;
    d->fadingTimeLine = new QTimeLine(d->duration, this);
    d->fadingTimeLine->setFrameRange(0, 255);

    connect(d->fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));

    d->fadingTimeLine->start();
}

int RatingWidget::regPixmapWidth() const
{
    return d->width;
}

void RatingWidget::setRating(int val)
{
    if (((val < RatingMin) || (val > RatingMax)) && (val != NoRating))
    {
        return;
    }

    d->rating = val;

    update();
}

int RatingWidget::rating() const
{
    return d->rating;
}

void RatingWidget::setTracking(bool tracking)
{
    d->tracking = tracking;
}

bool RatingWidget::hasTracking() const
{
    return d->tracking;
}

void RatingWidget::setFading(bool fading)
{
    d->fading = fading;
}

bool RatingWidget::hasFading() const
{
    return d->fading;
}

void RatingWidget::setFadingValue(int value)
{
    d->fadingValue = value;

    if ((d->fadingValue >= 255) && d->fadingTimeLine)
    {
        d->fadingTimeLine->stop();
    }

    update();
}

void RatingWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);

    if (visible)
    {
        startFading();
    }
    else
    {
        stopFading();
    }
}

int RatingWidget::maximumVisibleWidth() const
{
    return RatingMax * (d->width + 1);
}

void RatingWidget::startFading()
{
    if (!hasFading())
    {
        return;
    }

    if (!d->isHovered)
    {
        d->isHovered   = true;
        d->fadingValue = 0;
        setupTimeLine();
    }
}

void RatingWidget::stopFading()
{
    if (!hasFading())
    {
        return;
    }

    if (d->fadingTimeLine)
    {
        d->fadingTimeLine->stop();
    }

    d->isHovered   = false;
    d->fadingValue = 0;
    update();
}

void RatingWidget::setVisibleImmediately()
{
    setFadingValue(255);
}

QPixmap RatingWidget::starPixmapDisabled() const
{
    return d->disPixmap;
}

QPixmap RatingWidget::starPixmapFilled() const
{
    return d->selPixmap;
}

QPixmap RatingWidget::starPixmap() const
{
    return d->regPixmap;
}

void RatingWidget::regeneratePixmaps()
{
    slotThemeChanged();
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        return;
    }

    if (hasFading() && (d->fadingValue < 255))
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    int pos = (e->position().toPoint().x() - d->offset) / d->width + 1;

#else

    int pos = (e->x() - d->offset) / d->width + 1;

#endif

    if (d->rating == pos)
    {
        d->rating--;
    }
    else
    {
        d->rating = pos;
    }

    if (d->rating > RatingMax)
    {
        d->rating = RatingMax;
    }

    if (d->rating < RatingMin)
    {
        d->rating = RatingMin;
    }

    if (d->tracking)
    {
        Q_EMIT signalRatingChanged(d->rating);
    }

    Q_EMIT signalRatingModified(d->rating);

    update();
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton))
    {
        return;
    }

    if (hasFading() && (d->fadingValue < 255))
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    int pos = (e->position().toPoint().x() - d->offset) / d->width + 1;

#else

    int pos = (e->x() - d->offset) / d->width + 1;

#endif

    if (d->rating != pos)
    {
        if (pos > RatingMax)       // NOTE: bug. #151357
        {
            pos = RatingMax;
        }

        if (pos < RatingMin)
        {
            pos = RatingMin;
        }

        d->rating = pos;

        if (d->tracking)
        {
            Q_EMIT signalRatingChanged(d->rating);
        }

        Q_EMIT signalRatingModified(d->rating);

        update();
    }
}

void RatingWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        return;
    }

    if (hasFading() && (d->fadingValue < 255))
    {
        return;
    }

    Q_EMIT signalRatingChanged(d->rating);
}

void RatingWidget::slotThemeChanged()
{
    qreal dpr    = devicePixelRatio();

    d->regPixmap = QPixmap(d->width * dpr, d->width * dpr);
    d->regPixmap.fill(Qt::transparent);
    d->selPixmap = QPixmap(d->width * dpr, d->width * dpr);
    d->selPixmap.fill(Qt::transparent);
    d->disPixmap = QPixmap(d->width * dpr, d->width * dpr);
    d->disPixmap.fill(Qt::transparent);

    d->regPixmap.setDevicePixelRatio(dpr);
    d->selPixmap.setDevicePixelRatio(dpr);
    d->disPixmap.setDevicePixelRatio(dpr);

    QPainter p1(&d->regPixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(palette().color(QPalette::Active, backgroundRole()));
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawPolygon(starPolygon(), Qt::WindingFill);
    p1.end();

    QPainter p2(&d->selPixmap);
    p2.setRenderHint(QPainter::Antialiasing, true);
    p2.setBrush(qApp->palette().color(QPalette::Link));
    p2.setPen(palette().color(QPalette::Active, foregroundRole()));
    p2.drawPolygon(starPolygon(), Qt::WindingFill);
    p2.end();

    QPainter p3(&d->disPixmap);
    p3.setRenderHint(QPainter::Antialiasing, true);
    p3.setBrush(palette().color(QPalette::Disabled, backgroundRole()));
    p3.setPen(palette().color(QPalette::Disabled, foregroundRole()));
    p3.drawPolygon(starPolygon(), Qt::WindingFill);
    p3.end();

    setMinimumSize(QSize((d->width + 1) * RatingMax, d->width));
    update();
}

QPolygon RatingWidget::starPolygon()
{
    QPolygon star;
    star << QPoint(0,  6);
    star << QPoint(5,  5);
    star << QPoint(7,  0);
    star << QPoint(9,  5);
    star << QPoint(14, 6);
    star << QPoint(10, 9);
    star << QPoint(11, 14);
    star << QPoint(7,  11);
    star << QPoint(3,  14);
    star << QPoint(4,  9);

    return star;
}

QIcon RatingWidget::buildIcon(int rate, int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    QTransform transform;
    transform.scale(size / 15.0, size / 15.0);
    p.setTransform(transform);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(qApp->palette().color(QPalette::Active, QPalette::ButtonText));

    if (rate > 0)
    {
        p.setBrush(qApp->palette().color(QPalette::Link));
    }

    p.drawPolygon(starPolygon(), Qt::WindingFill);
    p.end();

    return QIcon(pix);
}

void RatingWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    d->offset = (width() - RatingMax * (d->width + 1)) / 2;

    // Widget is disable : drawing grayed frame.

    if (!isEnabled())
    {
        int x = d->offset;

        for (int i = 0 ; i < RatingMax ; ++i)
        {
            p.drawPixmap(x, 0, d->disPixmap);
            x += d->width + 1;
        }
    }
    else
    {
        int x       = d->offset;
        int rate    = d->rating != NoRating ? d->rating : 0;
        QPixmap sel = d->selPixmap;
        applyFading(sel);

        for (int i = 0 ; i < rate ; ++i)
        {
            p.drawPixmap(x, 0, sel);
            x += d->width + 1;
        }

        QPixmap reg = d->regPixmap;
        applyFading(reg);

        for (int i = rate ; i < RatingMax ; ++i)
        {
            p.drawPixmap(x, 0, reg);
            x += d->width + 1;
        }
    }

    p.end();
}

void RatingWidget::applyFading(QPixmap& pix)
{
    if (hasFading() && d->fadingValue < 255)
    {
        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(pix.rect(), QColor(0, 0, 0, d->fadingValue));
        p.end();
    }
}

} // namespace Digikam

#include "moc_ratingwidget.cpp"

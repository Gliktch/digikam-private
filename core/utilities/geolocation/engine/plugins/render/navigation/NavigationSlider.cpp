/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "NavigationSlider.h"

// Qt includes

#include <QPainter>
#include <QMouseEvent>
#include <QPixmapCache>

namespace Marble
{

namespace
{
const int handleImageHeight = 32;
}

NavigationSlider::NavigationSlider(QWidget* parent) :
    QAbstractSlider(parent),
    m_handleImagePath(QLatin1String("marble/navigation/navigational_slider_handle"))
{
    setMouseTracking(true);
}

NavigationSlider::~NavigationSlider()
{
    QPixmapCache::remove(QLatin1String("marble/navigation/navigational_slider_groove"));
    QPixmapCache::remove(QLatin1String("marble/navigation/navigational_slider_handle"));
    QPixmapCache::remove(QLatin1String("marble/navigation/navigational_slider_handle_hover"));
    QPixmapCache::remove(QLatin1String("marble/navigation/navigational_slider_handle_press"));
}

QPixmap NavigationSlider::pixmap(const QString& id)
{
    QPixmap result;

    if (!QPixmapCache::find(id, &result))
    {
        result = QPixmap(QLatin1String(":/") + id + QLatin1String(".png"));
        QPixmapCache::insert(id, result);
    }

    return result;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void NavigationSlider::enterEvent(QEnterEvent*)

#else

void NavigationSlider::enterEvent(QEvent*)

#endif

{
    setSliderDown(false);

    if (m_handleImagePath != QLatin1String("marble/navigation/navigational_slider_handle_hover"))
    {
        m_handleImagePath = QLatin1String("marble/navigation/navigational_slider_handle_hover");
        repaint();
    }
}

void NavigationSlider::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if (!isSliderDown() && mouseEvent->buttons() & Qt::LeftButton)
    {
        setSliderDown(true);
    }

    if (isSliderDown())
    {
        qreal const fraction = (mouseEvent->pos().y() - handleImageHeight / 2) / qreal(height() - handleImageHeight);
        int v = (int) minimum() + ((maximum() - minimum())) * (1 - fraction);
        setValue(v);
        repaint();
    }
}

void NavigationSlider::mousePressEvent(QMouseEvent*)
{
    setSliderDown(true);

    if (m_handleImagePath != QLatin1String("marble/navigation/navigational_slider_handle_press"))
    {
        m_handleImagePath = QLatin1String("marble/navigation/navigational_slider_handle_press");
        repaint();
    }
}

void NavigationSlider::mouseReleaseEvent(QMouseEvent*)
{
    setSliderDown(false);

    if (m_handleImagePath != QLatin1String("marble/navigation/navigational_slider_handle_hover"))
    {
        m_handleImagePath = QLatin1String("marble/navigation/navigational_slider_handle_hover");
        repaint();
    }
}

void NavigationSlider::leaveEvent(QEvent*)
{
    setSliderDown(false);

    if (m_handleImagePath != QLatin1String("marble/navigation/navigational_slider_handle"))
    {
        m_handleImagePath = QLatin1String("marble/navigation/navigational_slider_handle");
        repaint();
    }
}

void NavigationSlider::repaint()
{
    Q_EMIT repaintNeeded();
}

void NavigationSlider::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    for (int y = 0; y <= 160; y += 10)
    {
        painter.drawPixmap(0, y, pixmap(QLatin1String("marble/navigation/navigational_slider_groove")));

    }

    qreal const fraction = (value() - minimum()) / qreal(maximum() - minimum());
    int const y = (height() - handleImageHeight) * (1 - fraction);
    painter.drawPixmap(0, y, pixmap(m_handleImagePath));
    painter.end();
}

} // namespace Marble

#include "moc_NavigationSlider.cpp"

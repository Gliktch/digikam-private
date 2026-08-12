/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-15-08
 * Description : A floatable/dockable widget for thumbnail bars,
 *               providing i drag handle similar to the
 *               one on toolbars and a standard QAction to show/hide the
 *               thumbnail bar. It inherits QDockWidget and can be used in
 *               the dock area's of a QMainWindow.
 *
 * SPDX-FileCopyrightText: 2009 by Pieter Edelman <p dot edelman at gmx dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbbardock.h"

// Qt includes

#include <QKeySequence>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN DragHandle::Private
{

public:

    Private() = default;

    QDockWidget*       parent       = nullptr;
    Qt::DockWidgetArea currentArea  = Qt::LeftDockWidgetArea;
};

DragHandle::DragHandle(QDockWidget* const parent)
    : QWidget(),
      d      (new Private)
{
    d->parent = parent;

    setCursor(Qt::PointingHandCursor);
    setToolTip(i18n("Drag to reposition"));

    // When the dock location changes, check if the orientation has changed.

    connect(d->parent, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
}

DragHandle::~DragHandle()
{
    delete d;
}

void DragHandle::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QStyle* const style = d->parent->style();

    // The QStyleOptionToolBar contains every parameter needed to draw the
    // handle.

    QStyleOptionToolBar opt;
    opt.initFrom(d->parent);
    opt.features = QStyleOptionToolBar::Movable;

    // If the thumbnail bar is laid out horizontally, the state should be set
    // to horizontal to draw the handle in the proper orientation.

    if (
        (d->currentArea == Qt::LeftDockWidgetArea) ||
        (d->currentArea == Qt::RightDockWidgetArea)
       )
    {
        opt.rect = QRect(opt.rect.x(), opt.rect.y(),
                         d->parent->width(),
                         style->pixelMetric(QStyle::PM_ToolBarHandleExtent));
    }
    else
    {
        opt.state |= QStyle::State_Horizontal;
        opt.rect   = QRect(opt.rect.x(), opt.rect.y(),
                           style->pixelMetric(QStyle::PM_ToolBarHandleExtent),
                           d->parent->height());
    }

    // Draw the toolbar handle.

    style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

void DragHandle::dockLocationChanged(Qt::DockWidgetArea area)
{
    d->currentArea = area;

    // When the dock widget that contains this handle changes to a different
    // orientation, the DockWidgetVerticalTitleBar feature needs to be adjusted:
    // present when the thumbbar orientation is horizontal, absent when it is
    // vertical(!)

    if (
        (d->currentArea == Qt::LeftDockWidgetArea) ||
        (d->currentArea == Qt::RightDockWidgetArea)
       )
    {
        d->parent->setFeatures(d->parent->features() & ~QDockWidget::DockWidgetVerticalTitleBar);
    }
    else
    {
        d->parent->setFeatures(d->parent->features() | QDockWidget::DockWidgetVerticalTitleBar);
    }
}

QSize DragHandle::sizeHint() const
{
    // Size is the sum of the margin, frame width and the handle itself.

    QStyle* const style = d->parent->style();
    int handleWidth     = style->pixelMetric(QStyle::PM_ToolBarHandleExtent);
    int margin          = style->pixelMetric(QStyle::PM_ToolBarItemMargin) +
                          style->pixelMetric(QStyle::PM_ToolBarFrameWidth);

    if (
        (d->currentArea == Qt::LeftDockWidgetArea) ||
        (d->currentArea == Qt::RightDockWidgetArea)
       )
    {
        return QSize(d->parent->width(), handleWidth + 2*margin);
    }
    else
    {
        return QSize(handleWidth + 2*margin, d->parent->height());
    }
}

QSize DragHandle::minimumSizeHint() const
{
    return QSize(0, 0);
}

// ----------------------------------------------------------------------------

ThumbBarDock::ThumbBarDock(QWidget* const parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    // Use a DragHandle as title bar widget.

    setTitleBarWidget(new DragHandle(this));
    setContextMenuPolicy(Qt::PreventContextMenu);
}

void ThumbBarDock::reInitialize()
{
    // Measure orientation of the widget and adjust the child thumbbar to this
    // orientation and size.

    QMainWindow* const parent = qobject_cast<QMainWindow*>(parentWidget());

    Q_EMIT dockLocationChanged(parent->dockWidgetArea(this));

    widget()->resize(size());
    update();
}

QAction* ThumbBarDock::getToggleAction(QObject* const parent, const QString& caption) const
{
    QAction* const action = new QAction(QIcon::fromTheme(QLatin1String("view-choose")),
                                        caption.isNull() ? i18n("Show Thumbbar") : caption,
                                        parent);

    action->setCheckable(true);

    // Connect the triggered signal, which is only emitted after a user action
    // and not programmatically, to the show/hide method.

    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(showThumbBar(bool)));

    // Connect the show/hide signal to the state of the toggle action.

    connect(this, SIGNAL(visibilityChanged(bool)),
            action, SLOT(setChecked(bool)));

    return action;
}

void ThumbBarDock::restoreVisibility()
{
    // As long as the dock has not been attached to a QMainWindow it is a
    // top-level widget on its own and showing it would map a stray window.
    // The pending state is left untouched, the owning window calls this
    // method again from its show event.

    if (!parentWidget())
    {
        return;
    }

    // Set the visibility to what it should be or to what it was. Reset
    // SHOULD_BE_ values to their WAS_ values, to implement correct behavior
    // on subsequent calls.

    if      (m_visible == SHOULD_BE_SHOWN)
    {
        m_visible = WAS_SHOWN;
    }
    else if (m_visible == SHOULD_BE_HIDDEN)
    {
        m_visible = WAS_HIDDEN;
    }

    setVisible(m_visible == WAS_SHOWN);
}

bool ThumbBarDock::shouldBeVisible() const
{
    if ((m_visible == WAS_SHOWN) || (m_visible == SHOULD_BE_SHOWN))
    {
        return true;
    }

    return false;
}

void ThumbBarDock::setShouldBeVisible(bool status)
{
    if (status)
    {
        m_visible = SHOULD_BE_SHOWN;
    }
    else
    {
        m_visible = SHOULD_BE_HIDDEN;
    }
}

void ThumbBarDock::showThumbBar(bool status)
{
    if (status)
    {
        m_visible = WAS_SHOWN;
    }
    else
    {
        m_visible = WAS_HIDDEN;
    }

    setVisible(status);
}

QPixmap ThumbBarDock::generateFuzzyRect(const QSize& size, const QColor& color, int radius, const QColor& fillColor)
{
    QPixmap pix(size);
    pix.fill(fillColor);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw corners ----------------------------------

    QRadialGradient gradient;
    gradient.setColorAt(1, Qt::transparent);
    gradient.setColorAt(0, color);
    gradient.setRadius(radius);
    QPoint center;

    // Top Left

    center = QPoint(radius, radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(0, 0, radius, radius, gradient);

    // Top right

    center = QPoint(size.width() - radius, radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(center.x(), 0, radius, radius, gradient);

    // Bottom left

    center = QPoint(radius, size.height() - radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(0, center.y(), radius, radius, gradient);

    // Bottom right

    center = QPoint(size.width() - radius, size.height() - radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(center.x(), center.y(), radius, radius, gradient);

    // Draw borders ----------------------------------

    QLinearGradient linearGradient;
    linearGradient.setColorAt(1, Qt::transparent);
    linearGradient.setColorAt(0, color);

    // Top

    linearGradient.setStart(0, radius);
    linearGradient.setFinalStop(0, 0);
    painter.fillRect(radius, 0, size.width() - 2*radius, radius, linearGradient);

    // Bottom

    linearGradient.setStart(0, size.height() - radius);
    linearGradient.setFinalStop(0, size.height());
    painter.fillRect(radius, int(linearGradient.start().y()), size.width() - 2*radius, radius, linearGradient);

    // Left

    linearGradient.setStart(radius, 0);
    linearGradient.setFinalStop(0, 0);
    painter.fillRect(0, radius, radius, size.height() - 2*radius, linearGradient);

    // Right

    linearGradient.setStart(size.width() - radius, 0);
    linearGradient.setFinalStop(size.width(), 0);
    painter.fillRect(int(linearGradient.start().x()), radius, radius, size.height() - 2*radius, linearGradient);

    return pix;
}

QPixmap ThumbBarDock::generateFuzzyRectForGroup(const QSize& size, const QColor& color, int radius)
{
    // Create two normal borders

    QSize groupSize = size.scaled(size.width()-10, size.height()-10, Qt::KeepAspectRatio);
    QPixmap border1 = generateFuzzyRect(groupSize, color, radius, Qt::white);
    QPixmap border2 = border1.copy();

    QTransform rm;

    // Rotate first border right by 4 degrees

    rm.rotate(4);
    border1         = border1.transformed(rm, Qt::SmoothTransformation);

    // Rotate second border left by 4 degrees

    rm.rotate(-8);
    border2         = border2.transformed(rm, Qt::SmoothTransformation);

    // Combine both borders

    int width       = qMax(border1.size().width(), border2.size().width());
    int height      = qMax(border1.size().height(), border2.size().height());

    QPixmap result(QSize(width, height));
    result.fill(Qt::transparent); // force alpha channel

    {
        QPainter painter(&result);
        painter.setRenderHints(QPainter::Antialiasing, true);
        painter.drawPixmap(0, 0, border1);
        painter.drawPixmap(0, 0, border2);
    }

    return result;
}

} // namespace Digikam

#include "moc_thumbbardock.cpp"

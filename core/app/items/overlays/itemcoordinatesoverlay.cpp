/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-28
 * Description : overlay for GPS location indicator
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemcoordinatesoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itemdelegate.h"
#include "itemmodel.h"
#include "itemcategorizedview.h"

namespace Digikam
{

CoordinatesOverlayWidget::CoordinatesOverlayWidget(QWidget* const parent)
    : QAbstractButton(parent)
{
}

void CoordinatesOverlayWidget::paintEvent(QPaintEvent*)
{
}

// -----------------------------------------------------------------------------------------

ItemCoordinatesOverlay::ItemCoordinatesOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

CoordinatesOverlayWidget* ItemCoordinatesOverlay::buttonWidget() const
{
    return static_cast<CoordinatesOverlayWidget*>(m_widget);
}

QWidget* ItemCoordinatesOverlay::createWidget()
{
    QAbstractButton* const button = new CoordinatesOverlayWidget(parentWidget());
    button->setCursor(Qt::PointingHandCursor);

    return button;
}

void ItemCoordinatesOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(buttonWidget(), SIGNAL(clicked()),
                this, SLOT(slotButtonClicked()));
    }
}

void ItemCoordinatesOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ItemCoordinatesOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ItemDelegate*>(delegate())->coordinatesIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool ItemCoordinatesOverlay::checkIndex(const QModelIndex& index) const
{
    ItemInfo info = ItemModel::retrieveItemInfo(index);
    QRect rect    = static_cast<ItemDelegate*>(delegate())->coordinatesIndicatorRect();

    if (!rect.isNull() && info.hasCoordinates())
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item has geolocation information."));
        return true;
    }

    // If info.hasCoordinates() = false, no need to show a tooltip, because there is no icon over thumbnail.

    return false;
}

void ItemCoordinatesOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

void ItemCoordinatesOverlay::slotButtonClicked()
{
    Q_EMIT signalOpenGeolocationMap(m_index);
}

} // namespace Digikam

#include "moc_itemcoordinatesoverlay.cpp"

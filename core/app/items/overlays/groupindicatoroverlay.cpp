/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-28
 * Description : overlay for extra functionality of the group indicator
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2008      by Peter Penz <peter dot penz at gmx dot at>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "groupindicatoroverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemdelegate.h"
#include "itemmodel.h"
#include "itemcategorizedview.h"
#include "itemfiltermodel.h"

namespace Digikam
{

GroupIndicatorOverlayWidget::GroupIndicatorOverlayWidget(QWidget* const parent)
    : QAbstractButton(parent)
{
}

void GroupIndicatorOverlayWidget::contextMenuEvent(QContextMenuEvent* event)
{
    Q_EMIT contextMenu(event);
}

void GroupIndicatorOverlayWidget::paintEvent(QPaintEvent*)
{
}

// --------------------------------------------------------------------------------

GroupIndicatorOverlay::GroupIndicatorOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

GroupIndicatorOverlayWidget* GroupIndicatorOverlay::buttonWidget() const
{
    return static_cast<GroupIndicatorOverlayWidget*>(m_widget);
}

QWidget* GroupIndicatorOverlay::createWidget()
{
    QAbstractButton* const button = new GroupIndicatorOverlayWidget(parentWidget());
    button->setCursor(Qt::PointingHandCursor);

    return button;
}

void GroupIndicatorOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(buttonWidget(), SIGNAL(clicked()),
                this, SLOT(slotButtonClicked()));

        connect(buttonWidget(), SIGNAL(contextMenu(QContextMenuEvent*)),
                this, SLOT(slotButtonContextMenu(QContextMenuEvent*)));
    }
    else
    {
        // widget is deleted
    }
}

void GroupIndicatorOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void GroupIndicatorOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ItemDelegate*>(delegate())->groupIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool GroupIndicatorOverlay::checkIndex(const QModelIndex& index) const
{
    ItemInfo info = ItemModel::retrieveItemInfo(index);
    QRect rect    = static_cast<ItemDelegate*>(delegate())->groupIndicatorRect();

    if (!rect.isNull() && info.hasGroupedImages())
    {
        QString tip = i18ncp("@info:tooltip",
                             "1 grouped item.\n",
                             "%1 grouped items.\n",
                             info.numberOfGroupedImages() + 1);

        if (index.data(ItemFilterModel::GroupIsOpenRole).toBool())
        {
            tip += i18nc("@info:tooltip", "Group is open.");
        }
        else
        {
            tip += i18nc("@info:tooltip", "Group is closed.");
        }

        m_widget->setToolTip(tip);

        return true;
    }

    return false;
}

void GroupIndicatorOverlay::slotButtonClicked()
{
    Q_EMIT toggleGroupOpen(m_index);
}

void GroupIndicatorOverlay::slotButtonContextMenu(QContextMenuEvent* event)
{
    Q_EMIT showButtonContextMenu(m_index, event);
}

void GroupIndicatorOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

} // namespace Digikam

#include "moc_groupindicatoroverlay.cpp"

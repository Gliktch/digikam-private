/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-30
 * Description : fullscreen overlay
 *
 * SPDX-FileCopyrightText:      2015 by Luca Carlon <carlon dot luca at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemfullscreenoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itemcategorizedview.h"
#include "iteminfo.h"
#include "itemmodel.h"

namespace Digikam
{

ItemFullScreenOverlayButton::ItemFullScreenOverlayButton(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
    setup();
}

QSize ItemFullScreenOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ItemFullScreenOverlayButton::icon()
{
    return QIcon::fromTheme(QLatin1String("media-playback-start"));
}

void ItemFullScreenOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "Show Fullscreen"));
}

// --------------------------------------------------------------------

ItemFullScreenOverlay::ItemFullScreenOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

ItemFullScreenOverlay* ItemFullScreenOverlay::instance(QObject* const parent)
{
    return new ItemFullScreenOverlay(parent);
}

void ItemFullScreenOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ItemFullScreenOverlay::createButton()
{
    return new ItemFullScreenOverlayButton(view());
}

void ItemFullScreenOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.right() - 3*gap - size*4 + 2;
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));
}

void ItemFullScreenOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        Q_EMIT signalFullscreen(affectedIndexes(index));
    }
}

bool ItemFullScreenOverlay::checkIndex(const QModelIndex& index) const
{
    ItemInfo info = ItemModel::retrieveItemInfo(index);

    return (
            (info.category() == DatabaseItem::Image) ||
            (info.category() == DatabaseItem::Video)
           );
}

void ItemFullScreenOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ItemFullScreenOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam

#include "moc_itemfullscreenoverlay.cpp"

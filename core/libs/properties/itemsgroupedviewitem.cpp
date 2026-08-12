/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-04-11
 * Description : A list view to show grouped items
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemsgroupedviewitem.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QDrag>
#include <QMenu>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "itemsgroupedview.h"
#include "applicationsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemsGroupedViewItem::Private
{

public:

    Private() = default;

public:

    QPixmap           preview;

    ItemsGroupedView* view     = nullptr;

    ItemInfo          info;
};

ItemsGroupedViewItem::ItemsGroupedViewItem(ItemsGroupedView* const view, const ItemInfo& info)
    : QTreeWidgetItem(view),
      d              (new Private)
{
    d->view = view;
    setInfo(info);
}

ItemsGroupedViewItem::ItemsGroupedViewItem(ItemsGroupedViewItem* const item, const ItemInfo& info)
    : QTreeWidgetItem(item),
      d              (new Private)
{
    d->view = item->view();
    setInfo(info);
}

ItemsGroupedViewItem::~ItemsGroupedViewItem()
{
    delete d;
}

void ItemsGroupedViewItem::setInfo(const ItemInfo& info)
{
    d->info = info;
    updateTitle();
}

void ItemsGroupedViewItem::updateTitle()
{
    if ((d->view->indexOfTopLevelItem(this) != -1) && ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        setText(1, QString::fromUtf8("%1 (%2)").arg(d->info.name())
                                               .arg(d->info.numberOfGroupedImages() + 1));
    }
    else
    {
        setText(1, d->info.name());
    }
}

ItemInfo ItemsGroupedViewItem::info() const
{
    return d->info;
}

ItemsGroupedView* ItemsGroupedViewItem::view() const
{
    return d->view;
}

void ItemsGroupedViewItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);

    //  We make sure the preview icon stays the same regardless of the role

    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);
}

void ItemsGroupedViewItem::setThumb(const QPixmap& pix)
{
    QSize iSize = treeWidget()->iconSize();
    QPixmap pixmap(iSize.width() + 2, iSize.height() + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width() / 2) - (pix.width() / 2), (pixmap.height() / 2) - (pix.height() / 2), pix);
    d->preview  = pixmap;
    setPixmap(d->preview);
}

} // namespace Digikam

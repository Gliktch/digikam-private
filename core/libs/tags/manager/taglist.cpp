/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-07-31
 * Description : Tag List implementation as Quick Access for various
 *               subtrees in Tag Manager
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "taglist.h"

// Qt includes

#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "albumtreeview.h"
#include "tagmngrtreeview.h"
#include "tagmngrlistmodel.h"
#include "tagmngrlistview.h"
#include "tagmngrlistitem.h"

namespace Digikam
{

class Q_DECL_HIDDEN TagList::Private
{
public:

    Private() = default;

public:

    QPushButton*                 addButton      = nullptr;
    TagMngrListView*             tagList        = nullptr;
    TagMngrListModel*            tagListModel   = nullptr;
    TagMngrTreeView*             treeView       = nullptr;
    QMap<int, QList<ListItem*> > tagMap;
};

TagList::TagList(TagMngrTreeView* const treeView, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->treeView               = treeView;
    QVBoxLayout* const layout = new QVBoxLayout();
    d->addButton              = new QPushButton(i18n("Add to List"));
    d->addButton->setToolTip(i18n("Add selected tags to Quick Access List"));
    d->tagList                = new TagMngrListView(this);
    d->tagListModel           = new TagMngrListModel(this);

    d->tagList->setModel(d->tagListModel);
    d->tagList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->tagList->setDragEnabled(true);
    d->tagList->setAcceptDrops(true);
    d->tagList->setDropIndicatorShown(true);

    layout->addWidget(d->addButton);
    layout->addWidget(d->tagList);

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddPressed()));

    connect(d->tagList->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this,
            SLOT(slotSelectionChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotTagDeleted(Album*)));

    restoreSettings();
    this->setLayout(layout);
}

TagList::~TagList()
{
    delete d;
}

void TagList::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Tags Manager List"));
    QStringList itemList;
    const auto items = d->tagListModel->allItems();

    for (const ListItem* const listItem : items)
    {
        QList<int> ids = listItem->getTagIds();

        if (!ids.isEmpty())
        {
            itemList << QString::number(ids.first());
        }
    }

    group.writeEntry(QLatin1String("Items"), itemList);
}

void TagList::restoreSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Tags Manager List"));
    QStringList itemList    = group.readEntry(QLatin1String("Items"), QStringList());

    /**
     * If config is empty add generic All Tags
     */
    d->tagListModel->addItem(QList<QVariant>() << QBrush(Qt::cyan, Qt::Dense2Pattern));

    if (itemList.isEmpty())
    {
        return;
    }

    for (const QString& item : std::as_const(itemList))
    {
        QList<QVariant> itemData;
        itemData << QBrush(Qt::cyan, Qt::Dense2Pattern);

        const TAlbum* const talbum = AlbumManager::instance()->findTAlbum(item.toInt());

        if (talbum)
        {
            itemData << talbum->id();
        }

        ListItem* const listItem = d->tagListModel->addItem(itemData);

        /**
         * Use this map to find all List Items that contain specific tag
         * usually to remove deleted tag
         */
        const auto tags = listItem->getTagIds();

        for (int tagId : tags)
        {
            d->tagMap[tagId].append(listItem);
        }
    }

    /**
     * "All Tags" item should be selected
     */
    QModelIndex rootIndex = d->tagList->model()->index(0, 0);
    d->tagList->setCurrentIndex(rootIndex);
}

void TagList::slotAddPressed()
{
    QModelIndexList selected = d->treeView->selectionModel()->selectedIndexes();

    if (selected.isEmpty())
    {
        return;
    }

    QList<QVariant> itemData;
    itemData << QBrush(Qt::cyan, Qt::Dense2Pattern);

    for (const QModelIndex& index : std::as_const(selected))
    {
        const TAlbum* const album = static_cast<TAlbum*>(d->treeView->albumForIndex(index));
        itemData << album->id();
    }

    ListItem* const listItem = d->tagListModel->addItem(itemData);

    /**
     * Use this map to find all List Items that contain specific tag
     * usually to remove deleted tag
     */
    const auto tags = listItem->getTagIds();

    for (int tagId : tags)
    {
        d->tagMap[tagId].append(listItem);
    }
}

void TagList::slotSelectionChanged()
{
    QModelIndexList indexList = d->tagList->mySelectedIndexes();
    QSet<int> mySet;

    for (const QModelIndex& index : std::as_const(indexList))
    {
        const ListItem* const item = static_cast<ListItem*>(index.internalPointer());

        if (item->getTagIds().isEmpty())
        {
            mySet.clear();
            break;
        }

        const auto tags = item->getTagIds();

        for (int tagId : tags)
        {
            mySet.insert(tagId);
        }
    }

    TagsManagerFilterModel* const filterModel = d->treeView->getFilterModel();
    QList<int> lstFromSet(mySet.begin(), mySet.end());
    filterModel->setQuickListTags(lstFromSet);
}

void TagList::slotTagDeleted(Album* album)
{
    const TAlbum* const talbum = dynamic_cast<TAlbum*>(album);

    if (!talbum)
    {
        return;
    }

    int delId = talbum->id();

    QList<ListItem*> items = d->tagMap[delId];

    for (ListItem* const item : std::as_const(items))
    {
        item->removeTagId(delId);

        if (item->getTagIds().isEmpty())
        {
            d->tagListModel->deleteItem(item);
            d->tagMap[delId].removeOne(item);
            d->treeView->getFilterModel()->setQuickListTags(QList<int>());
        }
    }
}

void TagList::slotDeleteSelected()
{
    QModelIndexList sel = d->tagList->selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    for (const QModelIndex& index : std::as_const(sel))
    {
        ListItem* const item = static_cast<ListItem*>(index.internalPointer());
        d->tagListModel->deleteItem(item);
    }

    d->tagList->selectionModel()->select(d->tagList->model()->index(0, 0),
                                         QItemSelectionModel::SelectCurrent);
}

void TagList::enableAddButton(bool value)
{
    d->addButton->setEnabled(value);
}

} // namespace Digikam

#include "moc_taglist.cpp"

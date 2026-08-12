/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates tree-view search album.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "findduplicatesalbum.h"

// Qt includes

#include <QApplication>
#include <QHeaderView>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "findduplicatesalbumitem.h"
#include "deletedialog.h"
#include "dio.h"

namespace Digikam
{

class Q_DECL_HIDDEN FindDuplicatesAlbum::Private
{
public:

    Private() = default;

public:

    const int            iconSize        = 64;

    ThumbnailLoadThread* thumbLoadThread = nullptr;
};

FindDuplicatesAlbum::FindDuplicatesAlbum(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    setColumnCount(FindDuplicatesAlbumItem::NUMBER_COLUMNS);
    setSortingEnabled(true);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setIconSize(QSize(d->iconSize, d->iconSize));
    setHeaderLabels(QStringList() << i18n("Ref. images")
                                  << i18n("Ref. dates")
                                  << i18n("Ref. albums")
                                  << i18n("Items")
                                  << i18n("Similarity"));
    header()->setSectionResizeMode(FindDuplicatesAlbumItem::REFERENCE_IMAGE, QHeaderView::Interactive);
    header()->setSectionResizeMode(FindDuplicatesAlbumItem::REFERENCE_DATE,  QHeaderView::Interactive);
    header()->setSectionResizeMode(FindDuplicatesAlbumItem::REFERENCE_ALBUM, QHeaderView::Interactive);
    header()->setSectionResizeMode(FindDuplicatesAlbumItem::RESULT_COUNT,    QHeaderView::Interactive);
    header()->setSectionResizeMode(FindDuplicatesAlbumItem::AVG_SIMILARITY,  QHeaderView::Interactive);
    setWhatsThis(i18n("This shows all found duplicate items."));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

FindDuplicatesAlbum::~FindDuplicatesAlbum()
{
    delete d;
}

QTreeWidgetItem* FindDuplicatesAlbum::firstItem()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

        if (item)
        {
            return item;
        }

        ++it;
    }

    return nullptr;
}

void FindDuplicatesAlbum::selectFirstItem()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

        if (item)
        {
            setCurrentItem(item);
            return;
        }

        ++it;
    }
}

void FindDuplicatesAlbum::slotThumbnailLoaded(const LoadingDescription& desc,
                                              const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

        if (item && (item->refUrl().toLocalFile() == desc.filePath))
        {
            if (!pix.isNull())
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }
        }

        ++it;
    }
}

void FindDuplicatesAlbum::updateDuplicatesAlbumItems(const QList<SAlbum*>& sAlbumsToRebuild,
                                                     const QList<qlonglong>& deletedImages)
{
    FindDuplicatesAlbumItem* currentItem = nullptr;
    const auto sels                      = selectedItems();

    for (QTreeWidgetItem* const selectedItem : sels)
    {
        FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(selectedItem);

        if (item && sAlbumsToRebuild.contains(item->album()))
        {
            item->calculateInfos(deletedImages);
            item->waitForCalculate();

            if (item->itemCount() == 1)
            {
                FindDuplicatesAlbumItem* beforeItem = nullptr;
                QTreeWidgetItemIterator it(this);

                while (*it)
                {
                    FindDuplicatesAlbumItem* const item2 = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

                    if      (item2 && (item2->refUrl() == item->refUrl()))
                    {
                        ++it;

                        while (*it)
                        {
                            FindDuplicatesAlbumItem* const item3 = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

                            if (item3 && !item3->isHidden())
                            {
                                currentItem = item3;

                                break;
                            }

                            ++it;
                        }

                        if (!currentItem)
                        {
                            currentItem = beforeItem;
                        }

                        break;
                    }
                    else if (item2 && !item2->isHidden())
                    {
                        beforeItem = item2;
                    }

                    ++it;
                }
            }
        }
    }

    if (currentItem)
    {
        setCurrentItem(currentItem);
    }
}

void FindDuplicatesAlbum::drawRow(QPainter* p,
                                  const QStyleOptionViewItem& opt,
                                  const QModelIndex& index) const
{
    const FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        d->thumbLoadThread->find(ThumbnailIdentifier(item->refUrl().toLocalFile()));
    }

    QTreeWidget::drawRow(p, opt, index);
}

void FindDuplicatesAlbum::removeDuplicates()
{
    QTreeWidgetItemIterator it(this);

    QList<ItemInfo> duplicatedItems;

    while (*it)
    {
        FindDuplicatesAlbumItem* const item = dynamic_cast<FindDuplicatesAlbumItem*>(*it);

        if (item)
        {
            duplicatedItems += item->duplicatedItems();
        }

        ++it;
    }

    QList<QUrl> urlList;

    DeleteDialog dialog(qApp->activeWindow());

    // Buffer the urls for deletion and imageids
    // for notification of the AlbumManager

    for (const ItemInfo& info : std::as_const(duplicatedItems))
    {
        urlList  << info.fileUrl();
    }

    if (!dialog.confirmDeleteList(urlList,
                                  DeleteDialogMode::Files,
                                  DeleteDialogMode::NoChoiceTrash))
    {
        return;
    }

    DIO::del(duplicatedItems, true);
}

} // namespace Digikam

#include "moc_findduplicatesalbum.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Wrapper model for table view
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>
#include <QUrl>

// Local includes

#include "coredbchangesets.h"
#include "tableview_shared.h"

class QMimeData;

namespace Digikam
{

class ImageChangeset;
class ItemFilterModel;
class ItemFilterSettings;
class ItemInfo;
class ItemInfoList;
class TableViewColumn;
class TableViewColumnConfiguration;
class TableViewColumnDescription;
class TableViewColumnFactory;
class TableViewColumnProfile;

class TableViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum GroupingMode
    {
        GroupingHideGrouped    = 0,
        GroupingIgnoreGrouping = 1,
        GroupingShowSubItems   = 2
    };

    typedef DatabaseFields::Hash<QVariant> DatabaseFieldsHashRaw;

public:

    class Item
    {
    public:

        Item() = default;
        ~Item();

        void  addChild(Item* const newChild);
        void  insertChild(const int pos, Item* const newChild);
        void  takeChild(Item* const oldChild);
        Item* findChildWithImageId(const qlonglong searchImageId);

    public:

        qlonglong    imageId    = 0;
        Item*        parent     = nullptr;
        QList<Item*> children;

    private:

        /// @note disabled
        Item(const Item&)            = delete;
        Item& operator=(const Item&) = delete;
    };

public:

    explicit TableViewModel(TableViewShared* const sharedObject, QObject* const parent = nullptr);
    ~TableViewModel()                                                                                                 override;

    void                    addColumnAt(const TableViewColumnDescription& description,
                                        const int targetColumn = -1);
    void                    addColumnAt(const TableViewColumnConfiguration& cpp,
                                        const int targetColumn = -1);
    void                    removeColumnAt(const int columnIndex);
    TableViewColumn*        getColumnObject(const int columnIndex);
    QList<TableViewColumn*> getColumnObjects();
    QModelIndex             fromItemFilterModelIndex(const QModelIndex& imageFilterModelIndex);
    QModelIndex             fromItemModelIndex(const QModelIndex& imageModelIndex);
    QModelIndex             toItemFilterModelIndex(const QModelIndex& i)                                        const;
    QModelIndex             toItemModelIndex(const QModelIndex& i)                                              const;
    void                    loadColumnProfile(const TableViewColumnProfile& columnProfile);
    TableViewColumnProfile  getColumnProfile()                                                                  const;

    QModelIndex deepRowIndex(const int rowNumber)                                                               const;
    int indexToDeepRowNumber(const QModelIndex& index)                                                          const;
    int deepRowCount()                                                                                          const;
    int firstDeepRowNotInList(const QList<QModelIndex>& needleList);
    QModelIndex toCol0(const QModelIndex& anIndex)                                                              const;

    QModelIndex   itemIndex(Item* const item)                                                                   const;
    QModelIndex   indexFromImageId(const qlonglong imageId, const int columnIndex)                              const;
    Item*         itemFromImageId(const qlonglong imageId)                                                      const;
    Item*         itemFromIndex(const QModelIndex& i)                                                           const;
    ItemInfo      infoFromItem(Item* const item)                                                                const;
    ItemInfoList  infosFromItems(const QList<Item*>& items)                                                     const;

    QVariant              itemDatabaseFieldRaw(Item* const item, const DatabaseFields::Set& requestedField);
    DatabaseFieldsHashRaw itemDatabaseFieldsRaw(Item* const item, const DatabaseFields::Set& requestedSet);

    qlonglong        imageId(const QModelIndex& anIndex)                                                        const;
    QList<qlonglong> imageIds(const QModelIndexList& indexList)                                                 const;
    QList<ItemInfo>  imageInfos(const QModelIndexList& indexList)                                               const;
    ItemInfo         imageInfo(const QModelIndex& index)                                                        const;
    QList<ItemInfo>  allItemInfo()                                                                              const;

    QList<Item*> sortItems(const QList<Item*>& itemList);
    class LessThan;
    bool lessThan(Item* const itemA, Item* const itemB);
    int findChildSortedPosition(Item* const parentItem, Item* const childItem);

    void scheduleResort();
    GroupingMode groupingMode()                                                                                 const;
    void setGroupingMode(const GroupingMode newGroupingMode);

public:

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())                   const override;
    QModelIndex parent(const QModelIndex& childIndex)                                                   const override;
    int rowCount(const QModelIndex& parent)                                                             const override;
    int columnCount(const QModelIndex& i)                                                               const override;
    QVariant data(const QModelIndex& i, int role)                                                       const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role)                             const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                                       const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex())                                         const override;

    /// drag-and-drop related functions
    Qt::DropActions supportedDropActions()                                                              const override;
    QStringList mimeTypes()                                                                             const override;
    bool dropMimeData(const QMimeData* data,
                              Qt::DropAction action,
                              int row,
                              int column,
                              const QModelIndex& parent) override;
    QMimeData* mimeData(const QModelIndexList& indexes)                                                 const override;

protected:

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder)                                           override;

private Q_SLOTS:

    void slotPopulateModelWithNotifications();
    void slotPopulateModel(const bool sendNotifications);

    void slotColumnDataChanged(const qlonglong imageId);
    void slotColumnAllDataChanged();

    void slotSourceModelAboutToBeReset();
    void slotSourceModelReset();
    void slotSourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void slotSourceRowsInserted(const QModelIndex& parent, int start, int end);
    void slotSourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotSourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotSourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                      const QModelIndex& destinationParent, int destinationRow);
    void slotSourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                             const QModelIndex& destinationParent, int destinationRow);
    void slotSourceLayoutAboutToBeChanged();
    void slotSourceLayoutChanged();

    void slotDatabaseImageChanged(const ImageChangeset& imageChangeset);

    void slotFilterSettingsChanged(const ItemFilterSettings& settings);
    void slotResortModel();
    void slotClearModel(const bool sendNotifications);

public Q_SLOTS:

    void slotSetActive(const bool isActive);

Q_SIGNALS:

    void signalGroupingModeChanged();

private:

    Item* createItemFromSourceIndex(const QModelIndex& imageFilterModelIndex);
    void addSourceModelIndex(const QModelIndex& imageModelIndex, const bool sendNotifications);

private:

    TableViewShared* const s = nullptr;

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TableViewModel::GroupingMode)

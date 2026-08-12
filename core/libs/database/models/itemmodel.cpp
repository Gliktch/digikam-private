/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemmodel.h"

// Qt includes

#include <QHash>
#include <QItemSelection>

// Local includes

#include "digikam_debug.h"
#include "coredbchangesets.h"
#include "coredbfields.h"
#include "coredbwatch.h"
#include "iteminfolist.h"
#include "abstractitemdragdrophandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemModel::Private
{
public:

    Private() = default;

public:

    ItemInfoList                       infos;
    ItemInfo                           itemInfo;

    QList<QVariant>                    extraValues;
    QMultiHash<qlonglong, int>         idHash;

    bool                               keepFilePathCache            = false;
    QHash<QString, qlonglong>          filePathHash;

    bool                               sendRemovalSignals           = false;

    QObject*                           preprocessor                 = nullptr;
    bool                               refreshing                   = false;
    bool                               reAdding                     = false;
    bool                               incrementalRefreshRequested  = false;

    DatabaseFields::Set                watchFlags;

    class ItemModelIncrementalUpdater* incrementalUpdater           = nullptr;

    ItemInfoList                       pendingInfos;
    QList<QVariant>                    pendingExtraValues;

public:

    inline bool isValid(const QModelIndex& index)
    {
        if (!index.isValid())
        {
            return false;
        }

        if ((index.row() < 0) || (index.row() >= infos.size()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid index" << index;

            return false;
        }

        return true;
    }

    inline bool extraValueValid(const QModelIndex& index)
    {
        // we assume isValid() being called before, no duplicate checks

        if      (extraValues.size() == 0)
        {
            return false;
        }
        else if (index.row() >= extraValues.size())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid index for extraData" << index;

            return false;
        }

        return true;
    }
};

// -------------------------------------------------------------------------------------

typedef QPair<int, int> IntPair;        // to make foreach macro happy
typedef QList<IntPair>  IntPairList;

class Q_DECL_HIDDEN ItemModelIncrementalUpdater
{
public:

    explicit ItemModelIncrementalUpdater(const ItemModel::Private* const d);

    void                  appendInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void                  aboutToBeRemovedInModel(const IntPairList& aboutToBeRemoved);
    QList<IntPair>        oldIndexes();

    static QList<IntPair> toContiguousPairs(const QList<int>& ids);

public:

    QMultiHash<qlonglong, int> oldIds;
    QList<QVariant>            oldExtraValues;
    QList<ItemInfo>            newInfos;
    QList<QVariant>            newExtraValues;
    QList<IntPairList>         modelRemovals;
};

// -------------------------------------------------------------------------------------

ItemModel::ItemModel(QObject* const parent)
    : QAbstractListModel(parent),
      d                 (new Private)
{
    // --- NOTE: use dynamic binding as slotAlbumChange() is a virtual slot which can be re-implemented in derived classes.

    connect(CoreDbAccess::databaseWatch(), static_cast<void (CoreDbWatch::*)(const AlbumChangeset&)>(&CoreDbWatch::albumChange),
            this, &ItemModel::slotAlbumChange);

    // --- NOTE: use dynamic binding as slotImageChange() is a virtual slot which can be re-implemented in derived classes.

    connect(CoreDbAccess::databaseWatch(), static_cast<void (CoreDbWatch::*)(const ImageChangeset&)>(&CoreDbWatch::imageChange),
            this, &ItemModel::slotImageChange);

    // --- NOTE: use dynamic binding as slotImageTagChange() is a virtual slot which can be re-implemented in derived classes.

    connect(CoreDbAccess::databaseWatch(), static_cast<void (CoreDbWatch::*)(const ImageTagChangeset&)>(&CoreDbWatch::imageTagChange),
            this, &ItemModel::slotImageTagChange);
}

ItemModel::~ItemModel()
{
    delete d->incrementalUpdater;
    delete d;
}

// ------------ Access methods -------------

void ItemModel::setKeepsFilePathCache(bool keepCache)
{
    d->keepFilePathCache = keepCache;
}

bool ItemModel::keepsFilePathCache() const
{
    return d->keepFilePathCache;
}

bool ItemModel::isEmpty() const
{
    return d->infos.isEmpty();
}

int ItemModel::itemCount() const
{
    return d->infos.count();
}

void ItemModel::setWatchFlags(const DatabaseFields::Set& set)
{
    d->watchFlags = set;
}

ItemInfo ItemModel::imageInfo(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return ItemInfo();
    }

    return d->infos.at(index.row());
}

ItemInfo& ItemModel::imageInfoRef(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return d->itemInfo;
    }

    return d->infos[index.row()];
}

qlonglong ItemModel::imageId(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return 0;
    }

    return d->infos.at(index.row()).id();
}

QList<ItemInfo> ItemModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    QList<ItemInfo> infos;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        infos << imageInfo(ind);
    }

    return infos;
}

QList<qlonglong> ItemModel::imageIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        ids << imageId(ind);
    }

    return ids;
}

ItemInfo ItemModel::imageInfo(int row) const
{
    if ((row < 0) || (row >= d->infos.size()))
    {
        return ItemInfo();
    }

    return d->infos.at(row);
}

ItemInfo& ItemModel::imageInfoRef(int row) const
{
    if ((row < 0) || (row >= d->infos.size()))
    {
        return d->itemInfo;
    }

    return d->infos[row];
}

qlonglong ItemModel::imageId(int row) const
{
    if ((row < 0) || (row >= d->infos.size()))
    {
        return -1;
    }

    return d->infos.at(row).id();
}

QModelIndex ItemModel::indexForItemInfo(const ItemInfo& info) const
{
    return indexForImageId(info.id());
}

QModelIndex ItemModel::indexForItemInfo(const ItemInfo& info, const QVariant& extraValue) const
{
    return indexForImageId(info.id(), extraValue);
}

QList<QModelIndex> ItemModel::indexesForItemInfo(const ItemInfo& info) const
{
    return indexesForImageId(info.id());
}

QModelIndex ItemModel::indexForImageId(qlonglong id) const
{
    int ind = d->idHash.value(id, -1);

    if (ind != -1)
    {
        return createIndex(ind, 0);
    }

    return QModelIndex();
}

QModelIndex ItemModel::indexForImageId(qlonglong id, const QVariant& extraValue) const
{
    if (d->extraValues.isEmpty())
    {
        return indexForImageId(id);
    }

    QMultiHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        if (d->extraValues.at(it.value()) == extraValue)
        {
            return createIndex(it.value(), 0);
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ItemModel::indexesForImageId(qlonglong id) const
{
    QList<QModelIndex> indexes;
    QMultiHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        indexes << createIndex(it.value(), 0);
    }

    return indexes;
}

int ItemModel::numberOfIndexesForItemInfo(const ItemInfo& info) const
{
    return numberOfIndexesForImageId(info.id());
}

int ItemModel::numberOfIndexesForImageId(qlonglong id) const
{
    if (d->extraValues.isEmpty())
    {
        return 0;
    }

    int count = 0;
    QMultiHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; (it != d->idHash.constEnd()) && (it.key() == id) ; ++it)
    {
        ++count;
    }

    return count;
}

// static method

ItemInfo ItemModel::retrieveItemInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return ItemInfo();
    }

    const ItemModel* const model = index.data(ItemModelPointerRole).value<ItemModel*>();
    int row                      = index.data(ItemModelInternalId).toInt();

    if (!model)
    {
        return ItemInfo();
    }

    return model->imageInfo(row);
}

// static method

qlonglong ItemModel::retrieveImageId(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return 0;
    }

    const ItemModel* const model = index.data(ItemModelPointerRole).value<ItemModel*>();
    int row                      = index.data(ItemModelInternalId).toInt();

    if (!model)
    {
        return 0;
    }

    return model->imageId(row);
}

QModelIndex ItemModel::indexForPath(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        return indexForImageId(d->filePathHash.value(filePath));
    }
    else
    {
        const int size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).filePath() == filePath)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ItemModel::indexesForPath(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        return indexesForImageId(d->filePathHash.value(filePath));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).filePath() == filePath)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
    }
}

ItemInfo ItemModel::imageInfo(const QString& filePath) const
{
    if (d->keepFilePathCache)
    {
        qlonglong id = d->filePathHash.value(filePath, -1);

        if (id != -1)
        {
            int ind = d->idHash.value(id, -1);

            if (ind != -1)
            {
                return d->infos.at(ind);
            }
        }
    }
    else
    {
        for (const ItemInfo& info : std::as_const(d->infos))
        {
            if (info.filePath() == filePath)
            {    // cppcheck-suppress useStlAlgorithm
                return info;
            }
        }
    }

    return ItemInfo();
}

QList<ItemInfo> ItemModel::imageInfos(const QString& filePath) const
{
    QList<ItemInfo> infos;

    if (d->keepFilePathCache)
    {
        qlonglong id = d->filePathHash.value(filePath, -1);

        if (id != -1)
        {
            const auto ids = d->idHash.values(id);

            for (int ind : ids)
            {
                infos << d->infos.at(ind);
            }
        }
    }
    else
    {
        for (const ItemInfo& info : std::as_const(d->infos))
        {
            if (info.filePath() == filePath)
            {
                infos << info;
            }
        }
    }

    return infos;
}

void ItemModel::addItemInfo(const ItemInfo& info)
{
    addItemInfos(QList<ItemInfo>() << info, QList<QVariant>());
}

void ItemModel::addItemInfos(const QList<ItemInfo>& infos)
{
    addItemInfos(infos, QList<QVariant>());
}

void ItemModel::addItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->appendInfos(infos, extraValues);
    }
    else
    {
        appendInfos(infos, extraValues);
    }
}

void ItemModel::addItemInfoSynchronously(const ItemInfo& info)
{
    addItemInfosSynchronously(QList<ItemInfo>() << info, QList<QVariant>());
}

void ItemModel::addItemInfosSynchronously(const QList<ItemInfo>& infos)
{
    addItemInfos(infos, QList<QVariant>());
}

void ItemModel::addItemInfosSynchronously(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos, extraValues);

    Q_EMIT processAdded(infos, extraValues);
}

void ItemModel::ensureHasItemInfo(const ItemInfo& info)
{
    ensureHasItemInfos(QList<ItemInfo>() << info, QList<QVariant>());
}

void ItemModel::ensureHasItemInfos(const QList<ItemInfo>& infos)
{
    ensureHasItemInfos(infos, QList<QVariant>());
}

void ItemModel::ensureHasItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        if (!d->pendingExtraValues.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ExtraValue / No Extra Value mismatch. Ignoring added infos.";
            return;
        }
    }
    else
    {
        if (d->pendingInfos.size() != d->pendingExtraValues.size())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ExtraValue / No Extra Value mismatch. Ignoring added infos.";
            return;
        }
    }

    d->pendingInfos << infos;
    d->pendingExtraValues << extraValues;
    cleanSituationChecks();
}

void ItemModel::clearItemInfos()
{
    beginResetModel();

    d->infos.clear();
    d->extraValues.clear();
    d->idHash.clear();
    d->filePathHash.clear();

    delete d->incrementalUpdater;

    d->incrementalUpdater          = nullptr;
    d->pendingInfos.clear();
    d->pendingExtraValues.clear();
    d->refreshing                  = false;
    d->reAdding                    = false;
    d->incrementalRefreshRequested = false;

    imageInfosCleared();
    endResetModel();
}

void ItemModel::setItemInfos(const QList<ItemInfo>& infos)
{
    clearItemInfos();
    addItemInfos(infos);
}

QList<ItemInfo> ItemModel::imageInfos() const
{
    return d->infos;
}

QList<qlonglong> ItemModel::imageIds() const
{
    return d->idHash.keys();
}

bool ItemModel::hasImage(qlonglong id) const
{
    return d->idHash.contains(id);
}

bool ItemModel::hasImage(const ItemInfo& info) const
{
    return d->idHash.contains(info.id());
}

bool ItemModel::hasImage(const ItemInfo& info, const QVariant& extraValue) const
{
    return hasImage(info.id(), extraValue);
}

bool ItemModel::hasImage(qlonglong id, const QVariant& extraValue) const
{
    if (d->extraValues.isEmpty())
    {
        return hasImage(id);
    }

    QMultiHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; (it != d->idHash.constEnd()) && (it.key() == id) ; ++it)
    {
        if (d->extraValues.at(it.value()) == extraValue)
        {
            return true;
        }
    }

    return false;;
}

QList<ItemInfo> ItemModel::uniqueItemInfos() const
{
    if (d->extraValues.isEmpty())
    {
        return d->infos;
    }

    QList<ItemInfo> uniqueInfos;
    const int size = d->infos.size();

    for (int i = 0 ; i < size ; ++i)
    {
        const ItemInfo& info = d->infos.at(i);

        if (d->idHash.value(info.id()) == i)
        {
            uniqueInfos << info;
        }
    }

    return uniqueInfos;
}

void ItemModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last  = createIndex(d->infos.size() - 1, 0);

    Q_EMIT dataChanged(first, last);
}

void ItemModel::emitDataChangedForSelection(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        for (const QItemSelectionRange& range : std::as_const(selection))
        {
            Q_EMIT dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

void ItemModel::ensureHasGroupedImages(const ItemInfo& groupLeader)
{
    ensureHasItemInfos(groupLeader.groupedImages());
}

// ------------ Preprocessing -------------

void ItemModel::setPreprocessor(QObject* const preprocessor)
{
    unsetPreprocessor(d->preprocessor);
    d->preprocessor = preprocessor;
}

void ItemModel::unsetPreprocessor(QObject* const preprocessor)
{
    if (preprocessor && (d->preprocessor == preprocessor))
    {
        disconnect(this, SIGNAL(preprocess(QList<ItemInfo>,QList<QVariant>)),
                   nullptr, nullptr);

        disconnect(d->preprocessor, nullptr,
                   this, SLOT(reAddItemInfos(QList<ItemInfo>,QList<QVariant>)));

        disconnect(d->preprocessor, nullptr,
                   this, SLOT(reAddingFinished()));
    }
}

void ItemModel::appendInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (d->preprocessor)
    {
        d->reAdding = true;

        Q_EMIT preprocess(infos, extraValues);
    }
    else
    {
        publiciseInfos(infos, extraValues);
    }
}

void ItemModel::appendInfosChecked(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    // This method does deduplication. It is private because in context of
    // re-adding or refreshing it is of no use.

    if (extraValues.isEmpty())
    {
        QList<ItemInfo> checkedInfos;

        for (const ItemInfo& info : std::as_const(infos))
        {
            if (!hasImage(info))
            {
                checkedInfos << info;
            }
        }

        appendInfos(checkedInfos, QList<QVariant>());
    }
    else
    {
        QList<ItemInfo> checkedInfos;
        QList<QVariant> checkedExtraValues;
        const int size = infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (!hasImage(infos[i], extraValues[i]))
            {
                checkedInfos       << infos[i];
                checkedExtraValues << extraValues[i];
            }
        }

        appendInfos(checkedInfos, checkedExtraValues);
    }
}

void ItemModel::reAddItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    // addItemInfos -> appendInfos -> preprocessor -> reAddItemInfos

    publiciseInfos(infos, extraValues);
}

void ItemModel::reAddingFinished()
{
    d->reAdding = false;
    cleanSituationChecks();
}

void ItemModel::startRefresh()
{
    d->refreshing = true;
}

void ItemModel::finishRefresh()
{
    d->refreshing = false;
    cleanSituationChecks();
}

bool ItemModel::isRefreshing() const
{
    return d->refreshing;
}

void ItemModel::cleanSituationChecks()
{
    // For starting an incremental refresh we want a clear situation:
    // Any remaining batches from non-incremental refreshing subclasses have been received in appendInfos(),
    // any batches sent to preprocessor for re-adding have been re-added.

    if (d->refreshing || d->reAdding)
    {
        return;
    }

    if (!d->pendingInfos.isEmpty())
    {
        appendInfosChecked(d->pendingInfos, d->pendingExtraValues);
        d->pendingInfos.clear();
        d->pendingExtraValues.clear();
        cleanSituationChecks();

        return;
    }

    if (d->incrementalRefreshRequested)
    {
        d->incrementalRefreshRequested = false;

        Q_EMIT readyForIncrementalRefresh();
    }
    else
    {
        Q_EMIT allRefreshingFinished();
    }
}

void ItemModel::publiciseInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (infos.isEmpty())
    {
        return;
    }

    Q_ASSERT(
             (infos.size() == extraValues.size()) ||
             (extraValues.isEmpty() && d->extraValues.isEmpty())
            );

    Q_EMIT imageInfosAboutToBeAdded(infos);

    const int firstNewIndex = d->infos.size();
    const int lastNewIndex  = d->infos.size() + infos.size() - 1;
    beginInsertRows(QModelIndex(), firstNewIndex, lastNewIndex);
    d->infos       << infos;
    d->extraValues << extraValues;

    for (int i = firstNewIndex ; i <= lastNewIndex ; ++i)
    {
        const ItemInfo& info = d->infos.at(i);
        qlonglong id         = info.id();
        d->idHash.insert(id, i);

        if (d->keepFilePathCache)
        {
            d->filePathHash[info.filePath()] = id;
        }
    }

    endInsertRows();

    Q_EMIT imageInfosAdded(infos);
}

void ItemModel::requestIncrementalRefresh()
{
    if (d->reAdding)
    {
        d->incrementalRefreshRequested = true;
    }
    else
    {
        Q_EMIT readyForIncrementalRefresh();
    }
}

bool ItemModel::hasIncrementalRefreshPending() const
{
    return d->incrementalRefreshRequested;
}

void ItemModel::startIncrementalRefresh()
{
    delete d->incrementalUpdater;

    d->incrementalUpdater = new ItemModelIncrementalUpdater(d);
}

void ItemModel::finishIncrementalRefresh()
{
    if (!d->incrementalUpdater)
    {
        return;
    }

    // remove old entries

    QList<QPair<int, int> > pairs = d->incrementalUpdater->oldIndexes();
    removeRowPairs(pairs);

    // add new indexes

    appendInfos(d->incrementalUpdater->newInfos, d->incrementalUpdater->newExtraValues);

    delete d->incrementalUpdater;
    d->incrementalUpdater = nullptr;
}

void ItemModel::removeIndex(const QModelIndex& index)
{
    removeIndexes(QList<QModelIndex>() << index);
}

void ItemModel::removeIndexes(const QList<QModelIndex>& indexes)
{
    QList<int> listIndexes;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        if (d->isValid(ind))
        {
            listIndexes << ind.row();
        }
    }

    if (listIndexes.isEmpty())
    {
        return;
    }

    removeRowPairsWithCheck(ItemModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ItemModel::removeItemInfo(const ItemInfo& info)
{
    removeItemInfos(QList<ItemInfo>() << info);
}

void ItemModel::removeItemInfos(const QList<ItemInfo>& infos)
{
    QList<int> listIndexes;

    for (const ItemInfo& info : std::as_const(infos))
    {
        QModelIndex ind = indexForImageId(info.id());

        if (ind.isValid())
        {
            listIndexes << ind.row();
        }
    }

    removeRowPairsWithCheck(ItemModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ItemModel::removeItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        removeItemInfos(infos);
        return;
    }

    QList<int> listIndexes;

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        QModelIndex ind = indexForImageId(infos.at(i).id(), extraValues.at(i));

        if (ind.isValid())
        {
            listIndexes << ind.row();
        }
    }

    removeRowPairsWithCheck(ItemModelIncrementalUpdater::toContiguousPairs(listIndexes));
}

void ItemModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}
/*
template <class List, typename T>
static bool pairsContain(const List& list, T value)
{
    typename List::const_iterator middle;
    typename List::const_iterator begin = list.begin();
    typename List::const_iterator end   = list.end();
    int n                               = int(end - begin);
    int half;

    while (n > 0)
    {
        half   = n >> 1;
        middle = begin + half;

        if      ((middle->first <= value) && (middle->second >= value))
        {
            return true;
        }
        else if (middle->second < value)
        {
            begin = middle + 1;
            n    -= half   + 1;
        }
        else
        {
            n = half;
        }
    }

    return false;
}
*/
void ItemModel::removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove)
{
    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->aboutToBeRemovedInModel(toRemove);
    }

    removeRowPairs(toRemove);
}

void ItemModel::removeRowPairs(const QList<QPair<int, int> >& toRemove)
{
    if (toRemove.isEmpty())
    {
        return;
    }

    // Remove old indexes
    // Keep in mind that when calling beginRemoveRows all structures announced to be removed
    // must still be valid, and this includes our hashes as well, which limits what we can optimize

    int                     removedRows = 0;
    int                     offset      = 0;
    QList<qlonglong>        removeFilePaths;
    typedef QPair<int, int> IntPair;            // to make foreach macro happy

    for (const IntPair& pair : std::as_const(toRemove))
    {
        const int begin = pair.first  - offset;
        const int end   = pair.second - offset; // inclusive
        removedRows     = end - begin + 1;

        // when removing from the list, all subsequent indexes are affected

        offset         += removedRows;

        QList<ItemInfo> removedInfos;

        if (d->sendRemovalSignals)
        {
            // cppcheck-suppress knownEmptyContainer
            std::copy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());

            Q_EMIT imageInfosAboutToBeRemoved(removedInfos);
        }

        prepareImageInfosAboutToBeRemoved(begin, end);
        beginRemoveRows(QModelIndex(), begin, end);

        // update idHash - which points to indexes of d->infos, and these change now!

        QMultiHash<qlonglong, int>::iterator it;

        for (it = d->idHash.begin() ; it != d->idHash.end() ; )
        {
            if (it.value() >= begin)
            {
                if (it.value() > end)
                {
                    // after the removed interval: adjust index

                    it.value() -= removedRows;
                }
                else
                {
                    // in the removed interval

                    removeFilePaths << it.key();
                    it = d->idHash.erase(it);

                    continue;
                }
            }

            ++it;
        }

        // remove from list

        d->infos.erase(d->infos.begin() + begin, d->infos.begin() + (end + 1));

        if (!d->extraValues.isEmpty())
        {
            d->extraValues.erase(d->extraValues.begin() + begin, d->extraValues.begin() + (end + 1));
        }

        endRemoveRows();

        if (d->sendRemovalSignals)
        {
            Q_EMIT imageInfosRemoved(removedInfos);
        }
    }

    // tidy up: remove old indexes from file path hash now

    if (d->keepFilePathCache)
    {
        QHash<QString, qlonglong>::iterator it;

        for (it = d->filePathHash.begin() ; it != d->filePathHash.end() ; )
        {
            if (removeFilePaths.contains(it.value()))
            {
                it = d->filePathHash.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

// ---------------------------------------------------------------------------------

ItemModelIncrementalUpdater::ItemModelIncrementalUpdater(const ItemModel::Private* const d)
    : oldIds        (d->idHash),
      oldExtraValues(d->extraValues)
{
}

void ItemModelIncrementalUpdater::appendInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues)
{
    if (extraValues.isEmpty())
    {
        for (const ItemInfo& info : std::as_const(infos))
        {
            QMultiHash<qlonglong, int>::iterator it = oldIds.find(info.id());

            if (it != oldIds.end())
            {
                oldIds.erase(it);
            }
            else
            {
                newInfos << info;
            }
        }
    }
    else
    {
        for (int i = 0 ; i < infos.size() ; ++i)
        {
            const ItemInfo& info = infos.at(i);
            bool found           = false;
            QMultiHash<qlonglong, int>::iterator it;

            for (it = oldIds.find(info.id()) ; (it != oldIds.end()) && (it.key() == info.id()) ; ++it)
            {
                // first check is for bug #262596. Not sure if needed.

                if (
                    (it.value() < oldExtraValues.size()) &&
                    (extraValues.at(i) == oldExtraValues.at(it.value()))
                   )
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                oldIds.erase(it);

                // do not erase from oldExtraValues - oldIds is a hash id -> index.
            }
            else
            {
                newInfos << info;
                newExtraValues << extraValues.at(i);
            }
        }
    }
}

void ItemModelIncrementalUpdater::aboutToBeRemovedInModel(const IntPairList& toRemove)
{
    modelRemovals << toRemove;
}

QList<QPair<int, int> > ItemModelIncrementalUpdater::oldIndexes()
{
    // first, apply all changes to indexes by direct removal in model
    // while the updater was active

    for (const IntPairList& list : std::as_const(modelRemovals))
    {
        int removedRows = 0;
        int offset      = 0;

        for (const IntPair& pair : std::as_const(list))
        {
            const int begin = pair.first  - offset;
            const int end   = pair.second - offset; // inclusive
            removedRows     = end - begin + 1;

            // when removing from the list, all subsequent indexes are affected

            offset         += removedRows;

            // update idHash - which points to indexes of d->infos, and these change now!

            QMultiHash<qlonglong, int>::iterator it;

            for (it = oldIds.begin() ; it != oldIds.end() ; )
            {
                if (it.value() >= begin)
                {
                    if (it.value() > end)
                    {
                        // after the removed interval: adjust index

                        it.value() -= removedRows;
                    }
                    else
                    {
                        // in the removed interval

                        it = oldIds.erase(it);
                        continue;
                    }
                }

                ++it;
            }
        }
    }

    modelRemovals.clear();

    return toContiguousPairs(oldIds.values());
}

QList<QPair<int, int> > ItemModelIncrementalUpdater::toContiguousPairs(const QList<int>& unsorted)
{
    // Take the given indices and return them as contiguous pairs [begin, end]

    QList<QPair<int, int> > pairs;

    if (unsorted.isEmpty())
    {
        return pairs;
    }

    QList<int> indices(unsorted);
    std::sort(indices.begin(), indices.end());

    QPair<int, int> pair(indices.first(), indices.first());

    for (int i = 1 ; i < indices.size() ; ++i)
    {
        const int& index = indices.at(i);

        if (index == (pair.second + 1))
        {
            pair.second = index;
            continue;
        }

        pairs << pair;          // insert last pair
        pair.first  = index;
        pair.second = index;
    }

    pairs << pair;

    return pairs;
}

// ------------ QAbstractItemModel implementation -------------

QVariant ItemModel::data(const QModelIndex& index, int role) const
{
    if (!d->isValid(index))
    {
        return QVariant();
    }

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
            return d->infos.at(index.row()).name();
        }

        case ItemModelPointerRole:
        {
            return QVariant::fromValue(const_cast<ItemModel*>(this));
        }

        case ItemModelInternalId:
        {
            return index.row();
        }

        case CreationDateRole:
        {
            return d->infos.at(index.row()).dateTime();
        }

        case ExtraDataRole:
        {
            if (d->extraValueValid(index))
            {
                return d->extraValues.at(index.row());
            }
            else
            {
                return QVariant();
            }
        }

        case ExtraDataDuplicateCount:
        {
            qlonglong id = d->infos.at(index.row()).id();

            return numberOfIndexesForImageId(id);
        }
    }

    return QVariant();
}

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

int ItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}

Qt::ItemFlags ItemModel::flags(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    f              |= dragDropFlags(index);

    return f;
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column != 0) || (row < 0) || parent.isValid() || (row >= d->infos.size()))
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

// ------------ Database watch -------------

void ItemModel::slotAlbumChange(const AlbumChangeset& changeset)
{
    if (d->infos.isEmpty() || !d->keepFilePathCache)
    {
        return;
    }

    if (changeset.operation() & AlbumChangeset::Renamed)
    {
        for (const ItemInfo& info : std::as_const(d->infos))
        {
            if (changeset.albumId() == info.albumId())
            {
                d->filePathHash.remove(d->filePathHash.key(info.id()));
                d->filePathHash[info.filePath()] = info.id();
            }
        }
    }
}

void ItemModel::slotImageChange(const ImageChangeset& changeset)
{
    if (d->infos.isEmpty())
    {
        return;
    }

    if (d->watchFlags & changeset.changes())
    {
        if (changeset.changes() & DatabaseFields::Name)
        {
            if (d->keepFilePathCache)
            {
                const auto ids = changeset.ids();

                for (const qlonglong& id : ids)
                {
                    d->filePathHash.remove(d->filePathHash.key(id));
                    int ind = d->idHash.value(id, -1);

                    if (ind != -1)
                    {
                        const ItemInfo& info             = d->infos.at(ind);
                        d->filePathHash[info.filePath()] = id;
                    }
                }
            }

            return;
        }

        QItemSelection items;
        const auto ids = changeset.ids();

        for (const qlonglong& id : ids)
        {
            QModelIndex ind = indexForImageId(id);

            if (ind.isValid())
            {
                items.select(ind, ind);
            }
        }

        if (!items.isEmpty())
        {
            emitDataChangedForSelection(items);

            Q_EMIT imageChange(changeset, items);
        }
    }
}

void ItemModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QItemSelection items;
    const auto ids = changeset.ids();

    for (const qlonglong& id : ids)
    {
        QModelIndex ind = indexForImageId(id);

        if (ind.isValid())
        {
            items.select(ind, ind);
        }
    }

    if (!items.isEmpty())
    {
        emitDataChangedForSelection(items);

        Q_EMIT imageTagChange(changeset, items);
    }
}

} // namespace Digikam

#include "moc_itemmodel.cpp"

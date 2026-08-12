/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-22
 * Description : Qt item model for camera entries
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importimagemodel.h"

// Qt includes

#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "coredbdownloadhistory.h"
#include "cameracontroller.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportItemModel::Private
{
public:

    Private() = default;

    inline bool isValid(const QModelIndex& index)
    {
        return (
                index.isValid()              &&
                (index.row() >= 0)           &&
                (index.row() < infos.size())
               );
    }

public:

    CameraController*                        controller                     = nullptr;
    CamItemInfoList                          infos;
    CamItemInfo                              camItemInfo;

    QMultiHash<qlonglong, int>               idHash;
    QHash<QString, qlonglong>                fileUrlHash;

    bool                                     keepFileUrlCache               = false;

    bool                                     refreshing                     = false;
    bool                                     reAdding                       = false;
    bool                                     incrementalRefreshRequested    = false;

    bool                                     sendRemovalSignals             = false;

    class ImportItemModelIncrementalUpdater* incrementalUpdater             = nullptr;
};

// ----------------------------------------------------------------------------------------------------

typedef QPair<int, int> IntPair;
typedef QList<IntPair>  IntPairList;

class Q_DECL_HIDDEN ImportItemModelIncrementalUpdater
{
public:

    explicit ImportItemModelIncrementalUpdater(const ImportItemModel::Private* const d);

    void            appendInfos(const QList<CamItemInfo>& infos);
    void            aboutToBeRemovedInModel(const IntPairList& aboutToBeRemoved);
    QList<IntPair>  oldIndexes();

    static QList<IntPair> toContiguousPairs(const QList<int>& ids);

public:

    QMultiHash<qlonglong, int> oldIds;
    QList<CamItemInfo>         newInfos;
    QList<IntPairList>         modelRemovals;
};

// ----------------------------------------------------------------------------------------------------

ImportItemModel::ImportItemModel(QObject* const parent)
    : QAbstractListModel(parent),
      d                 (new Private)
{
}

ImportItemModel::~ImportItemModel()
{
    delete d;
}

void ImportItemModel::setCameraThumbsController(CameraThumbsCtrl* const thumbsCtrl)
{
    d->controller = thumbsCtrl->cameraController();

    connect(d->controller, SIGNAL(signalFileList(CamItemInfoList)),
            SLOT(addCamItemInfos(CamItemInfoList)));

    connect(d->controller, SIGNAL(signalDeleted(QString,QString,bool)),
            SLOT(slotFileDeleted(QString,QString,bool)));

    connect(d->controller, SIGNAL(signalUploaded(CamItemInfo)),
            SLOT(slotFileUploaded(CamItemInfo)));
}

void ImportItemModel::setKeepsFileUrlCache(bool keepCache)
{
    d->keepFileUrlCache = keepCache;
}

bool ImportItemModel::keepsFileUrlCache() const
{
    return d->keepFileUrlCache;
}

bool ImportItemModel::isEmpty() const
{
    return d->infos.isEmpty();
}

CamItemInfo ImportItemModel::camItemInfo(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return CamItemInfo();
    }

    return d->infos.at(index.row());
}

CamItemInfo& ImportItemModel::camItemInfoRef(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return d->camItemInfo;
    }

    return d->infos[index.row()];
}

qlonglong ImportItemModel::camItemId(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return -1;
    }

    return d->infos.at(index.row()).id;
}

QList<CamItemInfo> ImportItemModel::camItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<CamItemInfo> infos;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        infos << camItemInfo(ind);
    }

    return infos;
}

QList<qlonglong> ImportItemModel::camItemIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        ids << camItemId(ind);
    }

    return ids;
}

CamItemInfo ImportItemModel::camItemInfo(int row) const
{
    if ((row < 0) || (row >= d->infos.size()))
    {
        return CamItemInfo();
    }

    return d->infos.at(row);
}

CamItemInfo& ImportItemModel::camItemInfoRef(int row) const
{
    if ((row < 0) || (row >= d->infos.size()))
    {
        return d->camItemInfo;
    }

    return d->infos[row];
}

qlonglong ImportItemModel::camItemId(int row) const
{
    if ((row < 0) || (row >= d->infos.size()))
    {
        return -1;
    }

    return d->infos.at(row).id;
}

QModelIndex ImportItemModel::indexForCamItemInfo(const CamItemInfo& info) const
{
    return indexForCamItemId(info.id);
}

QList<QModelIndex> ImportItemModel::indexesForCamItemInfo(const CamItemInfo& info) const
{
    return indexesForCamItemId(info.id);
}

QModelIndex ImportItemModel::indexForCamItemId(qlonglong id) const
{
    int ind = d->idHash.value(id, -1);

    if (ind == -1)
    {
        return QModelIndex();
    }

    return createIndex(ind, 0);
}

QList<QModelIndex> ImportItemModel::indexesForCamItemId(qlonglong id) const
{
    QList<QModelIndex> indexes;

    QMultiHash<qlonglong, int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
       indexes << createIndex(it.value(), 0);
    }

    return indexes;
}

int ImportItemModel::numberOfIndexesForCamItemInfo(const CamItemInfo& info) const
{
    return numberOfIndexesForCamItemId(info.id);
}

int ImportItemModel::numberOfIndexesForCamItemId(qlonglong id) const
{
    int count = 0;
    QMultiHash<qlonglong,int>::const_iterator it;

    for (it = d->idHash.constFind(id) ; it != d->idHash.constEnd() && it.key() == id ; ++it)
    {
        ++count;
    }

    return count;
}

// static method

CamItemInfo ImportItemModel::retrieveCamItemInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return CamItemInfo();
    }

    const ImportItemModel* const model = index.data(ImportItemModelPointerRole).value<ImportItemModel*>();
    int                          row   = index.data(ImportItemModelInternalId).toInt();

    if (!model)
    {
        return CamItemInfo();
    }

    return model->camItemInfo(row);
}

// static method

qlonglong ImportItemModel::retrieveCamItemId(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return -1;
    }

    const ImportItemModel* const model = index.data(ImportItemModelPointerRole).value<ImportItemModel*>();
    int                          row   = index.data(ImportItemModelInternalId).toInt();

    if (!model)
    {
        return -1;
    }

    return model->camItemId(row);
}

QModelIndex ImportItemModel::indexForUrl(const QUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        return indexForCamItemId(d->fileUrlHash.value(fileUrl.toLocalFile()));
    }
    else
    {
        const int size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).url() == fileUrl)
            {
                return createIndex(i, 0);
            }
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ImportItemModel::indexesForUrl(const QUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        return indexesForCamItemId(d->fileUrlHash.value(fileUrl.toLocalFile()));
    }
    else
    {
        QList<QModelIndex> indexes;
        const int          size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).url() == fileUrl)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
    }
}

CamItemInfo ImportItemModel::camItemInfo(const QUrl& fileUrl) const
{
    if (d->keepFileUrlCache)
    {
        qlonglong id = d->fileUrlHash.value(fileUrl.toLocalFile(), -1);

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
        for (const CamItemInfo& info : std::as_const(d->infos))
        {
            if (info.url() == fileUrl)
            {   // cppcheck-suppress useStlAlgorithm
                return info;
            }
        }
    }

    return CamItemInfo();
}

QList<CamItemInfo> ImportItemModel::camItemInfos(const QUrl& fileUrl) const
{
    QList<CamItemInfo> infos;

    if (d->keepFileUrlCache)
    {
        qlonglong id = d->fileUrlHash.value(fileUrl.toLocalFile(), -1);

        if (id != -1)
        {
            const auto idx = d->idHash.values(id);

            for (int ind : idx)
            {
                infos << d->infos.at(ind);
            }
        }
    }
    else
    {
        for (const CamItemInfo& info : std::as_const(d->infos))
        {
            if (info.url() == fileUrl)
            {
                infos << info;
            }
        }
    }

    return infos;
}

void ImportItemModel::addCamItemInfo(const CamItemInfo& info)
{
    addCamItemInfos(QList<CamItemInfo>() << info);
}

void ImportItemModel::addCamItemInfos(const CamItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->appendInfos(infos);
    }
    else
    {
        appendInfos(infos);
    }
}

void ImportItemModel::addCamItemInfoSynchronously(const CamItemInfo& info)
{
    addCamItemInfosSynchronously(QList<CamItemInfo>() << info);
}

void ImportItemModel::addCamItemInfosSynchronously(const CamItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);

    Q_EMIT processAdded(infos);
}

void ImportItemModel::clearCamItemInfos()
{
    beginResetModel();

    d->infos.clear();
    d->idHash.clear();
    d->fileUrlHash.clear();

    delete d->incrementalUpdater;

    d->incrementalUpdater          = nullptr;
    d->reAdding                    = false;
    d->refreshing                  = false;
    d->incrementalRefreshRequested = false;

    camItemInfosCleared();
    endResetModel();
}

// TODO unused
void ImportItemModel::setCamItemInfos(const CamItemInfoList& infos)
{
    clearCamItemInfos();
    addCamItemInfos(infos);
}

QList<CamItemInfo> ImportItemModel::camItemInfos() const
{
    return d->infos;
}

QList<qlonglong> ImportItemModel::camItemIds() const
{
    return d->idHash.keys();
}

QList<CamItemInfo> ImportItemModel::uniqueCamItemInfos() const
{
    QList<CamItemInfo> uniqueInfos;
    const int          size = d->infos.size();

    for (int i = 0 ; i < size ; ++i)
    {
        const CamItemInfo& info = d->infos.at(i);

        if (d->idHash.value(info.id) == i)
        {
            uniqueInfos << info;
        }
    }

    return uniqueInfos;
}

bool ImportItemModel::hasImage(qlonglong id) const
{
    return d->idHash.contains(id);
}

bool ImportItemModel::hasImage(const CamItemInfo& info) const
{
    return d->fileUrlHash.contains(info.url().toLocalFile());
}

void ImportItemModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last  = createIndex(d->infos.size() - 1, 0);

    Q_EMIT dataChanged(first, last);
}

void ImportItemModel::emitDataChangedForSelections(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        for (const QItemSelectionRange& range : std::as_const(selection))
        {
            Q_EMIT dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

void ImportItemModel::appendInfos(const CamItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);
}

void ImportItemModel::reAddCamItemInfos(const CamItemInfoList& infos)
{
    publiciseInfos(infos);
}

void ImportItemModel::reAddingFinished()
{
    d->reAdding = false;
    cleanSituationChecks();
}

void ImportItemModel::slotFileDeleted(const QString& folder, const QString& file, bool status)
{
    Q_UNUSED(status)

    QUrl url = QUrl::fromLocalFile(folder);
    url      = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + file);
    CamItemInfo info = camItemInfo(url);
    removeCamItemInfo(info);
}

void ImportItemModel::slotFileUploaded(const CamItemInfo& info)
{
    addCamItemInfo(info);
}

void ImportItemModel::startRefresh()
{
    d->refreshing = true;
}

void ImportItemModel::finishRefresh()
{
    d->refreshing = false;
    cleanSituationChecks();
}

bool ImportItemModel::isRefreshing() const
{
    return d->refreshing;
}

void ImportItemModel::cleanSituationChecks()
{
    // For starting an incremental refresh we want a clear situation:
    // Any remaining batches from non-incremental refreshing subclasses have been received in appendInfos(),
    // any batches sent to preprocessor for re-adding have been re-added.

    if (d->refreshing || d->reAdding)
    {
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

void ImportItemModel::publiciseInfos(const CamItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    Q_EMIT itemInfosAboutToBeAdded(infos);

    const int firstNewIndex = d->infos.size();
    const int lastNewIndex  = d->infos.size() + infos.size() -1;
    beginInsertRows(QModelIndex(), firstNewIndex, lastNewIndex);
    d->infos << infos;

    for (int i = firstNewIndex ; i <= lastNewIndex ; ++i)
    {
        CamItemInfo& info = d->infos[i];

        // TODO move this to a separate thread, see CameraHistoryUpdater
        // TODO can we/do we want to differentiate at all between whether the status is unknown and not downloaded?

        info.downloaded   = CoreDbDownloadHistory::status(QString::fromUtf8(d->controller->cameraMD5ID()),
                                                          info.name, info.size, info.ctime);

        // TODO is this safe? if so, is there a need to store this inside idHash separately?

        info.id           = i;
        qlonglong id      = info.id;
        d->idHash.insert(id, i);

        if (d->keepFileUrlCache)
        {
            d->fileUrlHash[info.url().toLocalFile()] = id;
        }
    }

    endInsertRows();

    Q_EMIT processAdded(infos);
    Q_EMIT itemInfosAdded(infos);
}

void ImportItemModel::requestIncrementalRefresh()
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

bool ImportItemModel::hasIncrementalRefreshPending() const
{
    return d->incrementalRefreshRequested;
}

void ImportItemModel::startIncrementalRefresh()
{
    delete d->incrementalUpdater;

    d->incrementalUpdater = new ImportItemModelIncrementalUpdater(d);
}

void ImportItemModel::finishIncrementalRefresh()
{
    if (!d->incrementalUpdater)
    {
        return;
    }

    // remove old entries

    QList<QPair<int, int> > pairs = d->incrementalUpdater->oldIndexes();
    removeRowPairs(pairs);

    // add new indexes

    appendInfos(d->incrementalUpdater->newInfos);

    delete d->incrementalUpdater;
    d->incrementalUpdater = nullptr;
}
/*
template <class List, typename T>
static bool pairsContain(const List& list, T value)
{
    typename List::const_iterator middle;
    typename List::const_iterator begin = list.begin();
    typename List::const_iterator end   = list.end();
    int n                               = int(end - begin);

    while (n > 0)
    {
        int half   = (n >> 1);
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
void ImportItemModel::removeIndex(const QModelIndex& index)
{
    removeIndexs(QList<QModelIndex>() << index);
}

void ImportItemModel::removeIndexs(const QList<QModelIndex>& indexes)
{
    QList<int> indexesList;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        if (d->isValid(ind))
        {
            indexesList << ind.row();
        }
    }

    if (indexesList.isEmpty())
    {
        return;
    }

    removeRowPairsWithCheck(ImportItemModelIncrementalUpdater::toContiguousPairs(indexesList));
}

void ImportItemModel::removeCamItemInfo(const CamItemInfo& info)
{
    removeCamItemInfos(QList<CamItemInfo>() << info);
}

void ImportItemModel::removeCamItemInfos(const QList<CamItemInfo>& infos)
{
    QList<int> indexesList;

    for (const CamItemInfo& info : std::as_const(infos))
    {
        QModelIndex ind = indexForCamItemId(info.id);

        if (ind.isValid())
        {
            indexesList << ind.row();
        }
    }

    removeRowPairsWithCheck(ImportItemModelIncrementalUpdater::toContiguousPairs(indexesList));
}

void ImportItemModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}

void ImportItemModel::removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove)
{
    if (d->incrementalUpdater)
    {
        d->incrementalUpdater->aboutToBeRemovedInModel(toRemove);
    }

    removeRowPairs(toRemove);
}

void ImportItemModel::removeRowPairs(const QList<QPair<int, int> >& toRemove)
{
    if (toRemove.isEmpty())
    {
        return;
    }

    // Remove old indexes
    // Keep in mind that when calling beginRemoveRows all structures announced to be removed
    // must still be valid, and this includes our hashes as well, which limits what we can optimize

    int              removedRows = 0;
    int              offset      = 0;
    QList<qlonglong> removeFileUrls;

    for (const IntPair& pair : std::as_const(toRemove))
    {
        const int begin = pair.first  - offset;
        const int end   = pair.second - offset;
        removedRows     = end - begin + 1;

        // when removing from the list, all subsequent indexes are affected

        offset += removedRows;

        QList<CamItemInfo> removedInfos;

        if (d->sendRemovalSignals)
        {
            // cppcheck-suppress knownEmptyContainer
            std::copy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());
            Q_EMIT itemInfosAboutToBeRemoved(removedInfos);
        }

        prepareItemInfosAboutToBeRemoved(begin, end);
        beginRemoveRows(QModelIndex(), begin, end);

        // update idHash - which points to indexes of d->infos

        QMultiHash<qlonglong, int>::iterator it;

        for (it = d->idHash.begin() ; it != d->idHash.end() ; )
        {
            if (it.value() >= begin)
            {
                if (it.value() > end)
                {
                    // after the removed interval, adjust index

                    it.value() -= removedRows;
                }
                else
                {
                    // in the removed interval

                    removeFileUrls << it.key();
                    it = d->idHash.erase(it);
                    continue;
                }
            }

            ++it;
        }

        // remove from list

        d->infos.erase(d->infos.begin() + begin, d->infos.begin() + (end + 1));

        endRemoveRows();

        if (d->sendRemovalSignals)
        {
            Q_EMIT itemInfosRemoved(removedInfos);
        }
    }

    // tidy up: remove old indexes from file path hash now

    if (d->keepFileUrlCache)
    {
        QHash<QString, qlonglong>::iterator it;

        for (it = d->fileUrlHash.begin() ; it != d->fileUrlHash.end() ; )
        {
            if (removeFileUrls.contains(it.value()))
            {
                it = d->fileUrlHash.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

// ------------ ImportItemModelIncrementalUpdater ------------

ImportItemModelIncrementalUpdater::ImportItemModelIncrementalUpdater(const ImportItemModel::Private* const d)
    : oldIds(d->idHash)
{
}

void ImportItemModelIncrementalUpdater::aboutToBeRemovedInModel(const IntPairList& toRemove)
{
    modelRemovals << toRemove;
}

void ImportItemModelIncrementalUpdater::appendInfos(const QList<CamItemInfo>& infos)
{
    for (int i = 0 ; i < infos.size() ; ++i)
    {
        const CamItemInfo& info = infos.at(i);
        bool found              = false;
        QMultiHash<qlonglong, int>::iterator it;

        for (it = oldIds.find(info.id) ; it != oldIds.end() ; ++it)
        {
            if (it.key() == info.id)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            oldIds.erase(it);
        }
        else
        {
            newInfos << info;
        }
    }
}

QList<QPair<int, int> > ImportItemModelIncrementalUpdater::toContiguousPairs(const QList<int>& unsorted)
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

        pairs << pair; // insert last pair
        pair.first  = index;
        pair.second = index;
    }

    pairs << pair;

    return pairs;
}

QList<QPair<int, int> > ImportItemModelIncrementalUpdater::oldIndexes()
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

            offset += removedRows;

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

// ------------ QAbstractItemModel implementation -------------

QVariant ImportItemModel::data(const QModelIndex& index, int role) const
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
            return d->infos.at(index.row()).name;
        }

        case ImportItemModelPointerRole:
        {
            return QVariant::fromValue(const_cast<ImportItemModel*>(this));
        }

        case ImportItemModelInternalId:
        {
            return index.row();
        }
    }

    return QVariant();
}

QVariant ImportItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

int ImportItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}

Qt::ItemFlags ImportItemModel::flags(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    f |= dragDropFlags(index);

    return f;
}

QModelIndex ImportItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column != 0) || (row < 0) || parent.isValid() || (row >= d->infos.size()))
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

} // namespace Digikam

#include "moc_importimagemodel.cpp"

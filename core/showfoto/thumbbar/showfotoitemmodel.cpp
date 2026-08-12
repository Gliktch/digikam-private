/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-05
 * Description : Qt model for Showfoto entries
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotoitemmodel.h"

// Qt includes

#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "showfoto.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoItemModel::Private
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

    ShowfotoItemInfoList       infos;
    QMultiHash<qlonglong, int> idHash;
    QHash<QString, qlonglong>  fileUrlHash;

    bool                       keepFileUrlCache             = false;

    bool                       refreshing                   = false;
    bool                       reAdding                     = false;
    bool                       incrementalRefreshRequested  = false;

    bool                       sendRemovalSignals           = false;
};

// ----------------------------------------------------------------------------------------------------

ShowfotoItemModel::ShowfotoItemModel(QObject* const parent)
    : QAbstractListModel(parent),
      d                 (new Private)
{
}

ShowfotoItemModel::~ShowfotoItemModel()
{
    delete d;
}

bool ShowfotoItemModel::isEmpty() const
{
    return d->infos.isEmpty();
}

ShowfotoItemInfo ShowfotoItemModel::showfotoItemInfo(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return ShowfotoItemInfo();
    }

    return d->infos.at(index.row());
}

ShowfotoItemInfo& ShowfotoItemModel::showfotoItemInfoRef(const QModelIndex& index) const
{
    return d->infos[index.row()];
}

QList<ShowfotoItemInfo> ShowfotoItemModel::showfotoItemInfos(const QList<QModelIndex>& indexes) const
{
    QList<ShowfotoItemInfo> infos;

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        infos << showfotoItemInfo(ind);
    }

    return infos;
}

ShowfotoItemInfo ShowfotoItemModel::showfotoItemInfo(int row) const
{
    if (row >= d->infos.size())
    {
        return ShowfotoItemInfo();
    }

    return d->infos.at(row);
}

ShowfotoItemInfo& ShowfotoItemModel::showfotoItemInfoRef(int row) const
{
    return d->infos[row];
}

QModelIndex ShowfotoItemModel::indexForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return indexForUrl(info.url);
}

QList<QModelIndex> ShowfotoItemModel::indexesForShowfotoItemInfo(const ShowfotoItemInfo& info) const
{
    return indexesForUrl(info.url);
}

QModelIndex ShowfotoItemModel::indexForShowfotoItemId(qlonglong id) const
{
    int ind = d->idHash.value(id, 0);

    if (ind != -1)
    {
        return createIndex(ind, 0);
    }

    return QModelIndex();
}

// static method
ShowfotoItemInfo ShowfotoItemModel::retrieveShowfotoItemInfo(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return ShowfotoItemInfo();
    }

    const ShowfotoItemModel* const model = index.data(ShowfotoItemModelPointerRole).value<ShowfotoItemModel*>();
    int row                              = index.data(ShowfotoItemModelInternalId).toInt();

    if (!model)
    {
        return ShowfotoItemInfo();
    }

    return model->showfotoItemInfo(row);
}

QModelIndex ShowfotoItemModel::indexForUrl(const QUrl& fileUrl) const
{
    const int size = d->infos.size();

    for (int i = 0 ; i < size ; ++i)
    {
        if (d->infos.at(i).url == fileUrl)
        {
            return createIndex(i, 0);
        }
    }

    return QModelIndex();
}

QList<QModelIndex> ShowfotoItemModel::indexesForUrl(const QUrl& fileUrl) const
{
        QList<QModelIndex> indexes;
        const int          size = d->infos.size();

        for (int i = 0 ; i < size ; ++i)
        {
            if (d->infos.at(i).url == fileUrl)
            {
                indexes << createIndex(i, 0);
            }
        }

        return indexes;
}

ShowfotoItemInfo ShowfotoItemModel::showfotoItemInfo(const QUrl& fileUrl) const
{
    for (const ShowfotoItemInfo& info : std::as_const(d->infos))
    {
        if (info.url == fileUrl)
        {   // cppcheck-suppress useStlAlgorithm
            return info;
        }
    }

    return ShowfotoItemInfo();
}

QList<ShowfotoItemInfo> ShowfotoItemModel::showfotoItemInfos(const QUrl& fileUrl) const
{
    QList<ShowfotoItemInfo> infos;

    for (const ShowfotoItemInfo& info : std::as_const(d->infos))
    {
        if (info.url == fileUrl)
        {
            infos << info;
        }
    }

    return infos;
}

void ShowfotoItemModel::addShowfotoItemInfo(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfos(QList<ShowfotoItemInfo>() << info);
}

void ShowfotoItemModel::addShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    appendInfos(infos);
}

void ShowfotoItemModel::addShowfotoItemInfoSynchronously(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfosSynchronously(QList<ShowfotoItemInfo>() << info);
}

void ShowfotoItemModel::addShowfotoItemInfosSynchronously(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);

    Q_EMIT processAdded(infos);
}

void ShowfotoItemModel::clearShowfotoItemInfos()
{
    QList<ShowfotoItemInfo> list = d->infos;

    if (d->sendRemovalSignals)
    {
        Q_EMIT itemInfosAboutToBeRemoved(list);
    }

    beginResetModel();

    d->infos.clear();
    d->fileUrlHash.clear();
    d->reAdding                    = false;
    d->refreshing                  = false;
    d->incrementalRefreshRequested = false;

    showfotoItemInfosCleared();
    endResetModel();

    if (d->sendRemovalSignals)
    {
        Q_EMIT itemInfosRemoved(list);
    }
}

void ShowfotoItemModel::setShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    clearShowfotoItemInfos();
    addShowfotoItemInfos(infos);
}

QList<ShowfotoItemInfo> ShowfotoItemModel::showfotoItemInfos() const
{
    return d->infos;
}

bool ShowfotoItemModel::hasImage(const ShowfotoItemInfo& info) const
{
    return d->fileUrlHash.contains(info.url.toDisplayString());
}

void ShowfotoItemModel::emitDataChangedForAll()
{
    if (d->infos.isEmpty())
    {
        return;
    }

    QModelIndex first = createIndex(0, 0);
    QModelIndex last  = createIndex(d->infos.size() - 1, 0);

    Q_EMIT dataChanged(first, last);
}

void ShowfotoItemModel::emitDataChangedForSelections(const QItemSelection& selection)
{
    if (!selection.isEmpty())
    {
        for (const QItemSelectionRange& range : std::as_const(selection))
        {
            Q_EMIT dataChanged(range.topLeft(), range.bottomRight());
        }
    }
}

void ShowfotoItemModel::appendInfos(const QList<ShowfotoItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    publiciseInfos(infos);
}

void ShowfotoItemModel::reAddShowfotoItemInfos(const ShowfotoItemInfoList& infos)
{
    publiciseInfos(infos);
}

void ShowfotoItemModel::reAddingFinished()
{
    d->reAdding = false;
/*
    cleanSituationChecks();
*/
}

void ShowfotoItemModel::slotFileDeleted(const QString& folder, const QString& file, bool status)
{
    Q_UNUSED(status)

    ShowfotoItemInfo info = showfotoItemInfo(QUrl::fromLocalFile(folder + file));
/*
    removeShowfotoItemInfo(info);
*/
}

void ShowfotoItemModel::slotFileUploaded(const ShowfotoItemInfo& info)
{
    addShowfotoItemInfo(info);
}

void ShowfotoItemModel::publiciseInfos(const QList<ShowfotoItemInfo>& infos)
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
        const ShowfotoItemInfo& info = d->infos.at(i);
        qlonglong id                 = info.id;
        d->idHash.insert(id, i);

        if (d->keepFileUrlCache)
        {
            d->fileUrlHash[info.url.toDisplayString()] = id;
        }
    }

    endInsertRows();

    Q_EMIT itemInfosAdded(infos);
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
            n    -= half + 1;
        }
        else
        {
            n = half;
        }
    }

    return false;
}
*/

void ShowfotoItemModel::removeShowfotoItemInfo(const ShowfotoItemInfo& info)
{
    removeIndex(indexForShowfotoItemInfo(info));
}

void ShowfotoItemModel::removeShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos)
{
    for (const ShowfotoItemInfo& inf : std::as_const(infos))
    {
        removeIndex(indexForShowfotoItemInfo(inf));
    }
}

void ShowfotoItemModel::removeIndex(const QModelIndex& index)
{
    removeIndexs(QList<QModelIndex>() << index);
}

void ShowfotoItemModel::removeIndexs(const QList<QModelIndex>& indexes)
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

    removeRowPairs(toContiguousPairs(indexesList));
}

void ShowfotoItemModel::setSendRemovalSignals(bool send)
{
    d->sendRemovalSignals = send;
}

void ShowfotoItemModel::removeRowPairs(const QList<QPair<int, int> >& toRemove)
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
    typedef QPair<int, int> IntPair;

    for (const IntPair& pair : std::as_const(toRemove))
    {
        const int begin = pair.first  - offset;
        const int end   = pair.second - offset;
        removedRows     = end - begin + 1;

        // when removing from the list, all subsequent indexes are affected

        offset += removedRows;

        QList<ShowfotoItemInfo> removedInfos;

        if (d->sendRemovalSignals)
        {
            // cppcheck-suppress knownEmptyContainer
            std::copy(d->infos.begin() + begin, d->infos.begin() + end, removedInfos.begin());

            Q_EMIT itemInfosAboutToBeRemoved(removedInfos);
        }

        showfotoItemInfosAboutToBeRemoved(begin, end);
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

                    removeFilePaths << it.key();
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
            if (removeFilePaths.contains(it.value()))
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

QList<QPair<int, int> > ShowfotoItemModel::toContiguousPairs(const QList<int>& unsorted)
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
        const int& ind = indices.at(i);

        if (ind == pair.second + 1)
        {
            pair.second = ind;

            continue;
        }

        pairs << pair; // insert last pair
        pair.first  = ind;
        pair.second = ind;
    }

    pairs << pair;

    return pairs;
}

// ------------ QAbstractItemModel implementation -------------

int ShowfotoItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->infos.size();
}

Qt::ItemFlags ShowfotoItemModel::flags(const QModelIndex& index) const
{
    if (!d->isValid(index))
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    f              |= dragDropFlags(index);

    return f;
}

QModelIndex ShowfotoItemModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column != 0) || (row < 0) || parent.isValid() || (row >= d->infos.size()))
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

QVariant ShowfotoItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

QVariant ShowfotoItemModel::data(const QModelIndex& index, int role) const
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

        case ShowfotoItemModelPointerRole:
        {
            return QVariant::fromValue(const_cast<ShowfotoItemModel*>(this));
        }

        case ShowfotoItemModelInternalId:
        {
            return index.row();
        }
    }

    return QVariant();
}

} // namespace Showfoto

#include "moc_showfotoitemmodel.cpp"

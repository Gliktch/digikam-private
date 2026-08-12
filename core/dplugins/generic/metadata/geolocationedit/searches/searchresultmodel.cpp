/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchresultmodel.h"

// Qt includes

#include <QContextMenuEvent>
#include <QPainter>
#include <QAction>
#include <QStandardPaths>

// Local includes

#include "gpscommon.h"
#include "gpsundocommand.h"
#include "gpsitemmodel.h"

namespace DigikamGenericGeolocationEditPlugin
{

static bool RowRangeLessThan(const QPair<int, int>& a, const QPair<int, int>& b)
{
    return (a.first < b.first);
}

class Q_DECL_HIDDEN SearchResultModel::Private
{
public:

    Private()
    {
        markerNormalUrl   = QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QLatin1String("digikam/geolocationedit/searchmarker-normal.png")));
        markerNormal      = QPixmap(markerNormalUrl.toLocalFile());
        markerSelectedUrl = QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QLatin1String("digikam/geolocationedit/searchmarker-selected.png")));
        markerSelected    = QPixmap(markerSelectedUrl.toLocalFile());
        selectionModel    = nullptr;
    }

public:

    QList<SearchResultModel::SearchResultItem> searchResults;
    QUrl                                       markerNormalUrl;
    QUrl                                       markerSelectedUrl;
    QPixmap                                    markerNormal;
    QPixmap                                    markerSelected;
    QItemSelectionModel*                       selectionModel;
};

SearchResultModel::SearchResultModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
}

SearchResultModel::~SearchResultModel()
{
    delete d;
}

int SearchResultModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 1;
}

bool SearchResultModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)

    return false;
}

QVariant SearchResultModel::data(const QModelIndex& index, int role) const
{
    const int rowNumber = index.row();

    if ((rowNumber < 0) || (rowNumber >= d->searchResults.count()))
    {
        return QVariant();
    }

    const int columnNumber = index.column();

    if (columnNumber == 0)
    {
        switch (role)
        {
            case Qt::DisplayRole:
            {
                return d->searchResults.at(rowNumber).result.name;
            }

            case Qt::DecorationRole:
            {
                QPixmap markerIcon;
                getMarkerIcon(index, nullptr, nullptr, &markerIcon, nullptr);
                return markerIcon;
            }

            default:
            {
                return QVariant();
            }
        }
    }

    return QVariant();
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        // there are no child items, only top level items

        return QModelIndex();
    }

    if ((column < 0) || (column >= 1) || (row < 0) || (row >= d->searchResults.count()))
    {
        return QModelIndex();
    }

    return createIndex(row, column, (void*)nullptr);
}

QModelIndex SearchResultModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)

    // we have only top level items

    return QModelIndex();
}

int SearchResultModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->searchResults.count();
}

bool SearchResultModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(value)
    Q_UNUSED(role)

    return false;
}

QVariant SearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(role)

    if ((section >= 1) || (orientation != Qt::Horizontal))
    {
        return false;
    }

    return QVariant(QLatin1String("Name"));
}

Qt::ItemFlags SearchResultModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index);
}

void SearchResultModel::addResults(const SearchResultBackend::SearchResult::List& results)
{
    // first check which items are not duplicates

    QList<int> nonDuplicates;

    for (int i = 0 ; i < results.count() ; ++i)
    {
        const SearchResultBackend::SearchResult& currentResult = results.at(i);
        bool isDuplicate                                       = false;

        for (int j = 0 ; j < d->searchResults.count() ; ++j)
        {
            if (currentResult.internalId == d->searchResults.at(j).result.internalId)
            {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate)
        {
            nonDuplicates << i;
        }
    }

    if (nonDuplicates.isEmpty())
    {
        return;
    }

    beginInsertRows(QModelIndex(), d->searchResults.count(), d->searchResults.count()+nonDuplicates.count()-1);

    for (int i = 0 ; i < nonDuplicates.count() ; ++i)
    {
        SearchResultItem item;
        item.result = results.at(nonDuplicates.at(i));
        d->searchResults << item;
    }

    endInsertRows();
}

SearchResultModel::SearchResultItem SearchResultModel::resultItem(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return SearchResultItem();
    }

    return d->searchResults.at(index.row());
}

bool SearchResultModel::getMarkerIcon(const QModelIndex& index, QPoint* const offset, QSize* const size, QPixmap* const pixmap, QUrl* const url) const
{
    // determine the id of the marker

    const int markerNumber    = index.row();
    const bool itemIsSelected = d->selectionModel ? d->selectionModel->isSelected(index) : false;
    QPixmap markerPixmap      = itemIsSelected    ? d->markerSelected                    : d->markerNormal;

    // if the caller requests a URL and the marker will not get
    // a special label, return a URL. Otherwise, return a pixmap.

    const bool returnViaUrl   = url && (markerNumber > 26);

    if (returnViaUrl)
    {
        *url = itemIsSelected ? d->markerSelectedUrl : d->markerNormalUrl;

        if (size)
        {
            *size = markerPixmap.size();
        }
    }
    else
    {
        if (markerNumber <= 26)
        {
            const QString markerId = QChar('A' + markerNumber);
            QPainter painter(&markerPixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::black);
            QRect textRect(0, 2, markerPixmap.width(), markerPixmap.height());
            painter.drawText(textRect, Qt::AlignHCenter, markerId);
        }

        *pixmap = markerPixmap;
    }

    if (offset)
    {
        *offset = QPoint(markerPixmap.width() / 2, markerPixmap.height() - 1);
    }

    return true;
}

void SearchResultModel::setSelectionModel(QItemSelectionModel* const selectionModel)
{
    d->selectionModel = selectionModel;
}

void SearchResultModel::clearResults()
{
    beginResetModel();
    d->searchResults.clear();
    endResetModel();
}

void SearchResultModel::removeRowsByIndexes(const QModelIndexList& rowsList)
{
    // extract the row numbers first:

    QList<int> rowNumbers;

    for (const QModelIndex& idx : std::as_const(rowsList))
    {
        if (idx.isValid())
        {
            rowNumbers << idx.row();
        }
    }

    if (rowNumbers.isEmpty())
    {
        return;
    }

    std::sort(rowNumbers.begin(), rowNumbers.end());

    // now delete the rows, starting with the last row:

    for (int i = (rowNumbers.count() - 1) ; i >= 0 ; --i)
    {
        const int rowNumber = rowNumbers.at(i);

        /// @todo This is very slow for several indexes, because the views update after every removal

        beginRemoveRows(QModelIndex(), rowNumber, rowNumber);
        d->searchResults.removeAt(rowNumber);
        endRemoveRows();
    }
}

void SearchResultModel::removeRowsBySelection(const QItemSelection& selectionList)
{
    // extract the row numbers first:

    QList<QPair<int, int> > rowRanges;

    for (const QItemSelectionRange& range : std::as_const(selectionList))
    {
        rowRanges << QPair<int, int>(range.top(), range.bottom());
    }

    // we expect the ranges to be sorted here

    std::sort(rowRanges.begin(), rowRanges.end(), RowRangeLessThan);

    // now delete the rows, starting with the last row:

    for (int i = (rowRanges.count() - 1) ; i >= 0 ; --i)
    {
        const QPair<int, int> currentRange = rowRanges.at(i);

        /// @todo This is very slow for several indexes, because the views update after every removal

        beginRemoveRows(QModelIndex(), currentRange.first, currentRange.second);

        for (int j = currentRange.second ; j >= currentRange.first ; --j)
        {
            d->searchResults.removeAt(j);
        }

        endRemoveRows();
    }
}

} // namespace DigikamGenericGeolocationEditPlugin

#include "moc_searchresultmodel.cpp"

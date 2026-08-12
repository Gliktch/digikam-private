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

#pragma once

// Qt includes

#include <QAbstractItemModel>
#include <QItemSelectionModel>

// Local includes

#include "searchresultbackend.h"

class QItemSelection;

namespace DigikamGenericGeolocationEditPlugin
{

class SearchResultItem;

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    class SearchResultItem
    {
    public:

        SearchResultBackend::SearchResult result;
    };

public:

    explicit SearchResultModel(QObject* const parent = nullptr);
    ~SearchResultModel()                                                  override;

    void addResults(const SearchResultBackend::SearchResult::List& results);
    SearchResultItem resultItem(const QModelIndex& index)           const;
    bool getMarkerIcon(const QModelIndex& index,
                       QPoint* const offset,
                       QSize* const size,
                       QPixmap* const pixmap,
                       QUrl* const url)                             const;
    void setSelectionModel(QItemSelectionModel* const selectionModel);
    void clearResults();
    void removeRowsByIndexes(const QModelIndexList& rowsList);
    void removeRowsBySelection(const QItemSelection& selection);

    // QAbstractItemModel:
    int columnCount(const QModelIndex& parent = QModelIndex())      const override;
    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role)                                                override;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole)                       const override;
    QModelIndex index(int row,
                      int column,
                      const QModelIndex& parent = QModelIndex())    const override;
    QModelIndex parent(const QModelIndex& index)                    const override;
    int rowCount(const QModelIndex& parent = QModelIndex())         const override;
    bool setHeaderData(int section,
                       Qt::Orientation orientation,
                       const QVariant& value, int role)                   override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role)                                   const override;
    Qt::ItemFlags flags(const QModelIndex& index)                   const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGeolocationEditPlugin

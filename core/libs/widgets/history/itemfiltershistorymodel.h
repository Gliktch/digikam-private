/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : model for view with used filters on currently loaded image
 *
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractListModel>
#include <QList>
#include <QUrl>

// Local includes

#include "dimagehistory.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemFiltersHistoryTreeItem;

class ItemFiltersHistoryModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit ItemFiltersHistoryModel(QObject* const parent = nullptr, const QUrl& url = QUrl());
    ~ItemFiltersHistoryModel()                                                                      override;

    void setUrl(const QUrl& url);
    bool removeRows(int row, int count, const QModelIndex& parent)                                  override;
    void setupModelData(const QList<DImageHistory::Entry>& entries,
                        ItemFiltersHistoryTreeItem* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index)                                             const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const override;
    int columnCount(const QModelIndex& parent = QModelIndex())                                const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                   const override;
    QModelIndex parent(const QModelIndex& index)                                              const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())         const override;

public Q_SLOTS:

    void removeEntry(const QModelIndex& index);
    void setEnabledEntries(int count);
    void disableEntries(int count);
    void enableEntries(int count);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

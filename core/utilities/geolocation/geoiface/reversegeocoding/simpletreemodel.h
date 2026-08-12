/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : A simple model to hold a tree structure.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SimpleTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    class Item
    {
    public:

        Item() = default;

        ~Item()
        {
            qDeleteAll(children);
        }

    public:

        QString                     data;

    private:

        QList<QMap<int, QVariant> > dataColumns;
        Item*                       parent  = nullptr;
        QList<Item*>                children;

    private:

        /// @note disabled
        Item(const Item&)            = delete;
        Item& operator=(const Item&) = delete;

        friend class SimpleTreeModel;
    };

    explicit SimpleTreeModel(const int columnCount, QObject* const parent = nullptr);
    ~SimpleTreeModel()                                                                                    override;

    /// QAbstractItemModel:
    int columnCount(const QModelIndex& parent = QModelIndex())                                      const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role)                               override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                             const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())               const override;
    QModelIndex parent(const QModelIndex& index)                                                    const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                         const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)         override;
    QVariant headerData(int section, Qt::Orientation orientation, int role)                         const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                                   const override;

    Item* addItem(Item* const parentItem = nullptr, const int rowNumber = -1);
    Item* indexToItem(const QModelIndex& itemIndex)                                                 const;
    Item* rootItem()                                                                                const;
    QModelIndex itemToIndex(const Item* const item)                                                 const;


private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

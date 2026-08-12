/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : List View Model with support for mime data and drag-n-drop
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace Digikam
{

class ListItem;

class TagMngrListModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit TagMngrListModel(QObject* const parent = nullptr);
    ~TagMngrListModel() override;

    /**
     * @brief add new item to list.
     * @param values A list of data for item: Name as QString, QBrush as
     *               background and qlonglong as id.
     * @return pointer to newly created listitem.
     */
    ListItem* addItem(QList<QVariant> values);

    /**
     * @brief return all items from List, usually to be saved in KConfig.
     */
    QList<ListItem*> allItems()                                                                 const;

    void deleteItem(ListItem* const item);

    /**
     * @brief Standard methods to be implemented when subcassing QAbstractItemModel.
     */
    QVariant data(const QModelIndex& index, int role)                                           const override;

    Qt::ItemFlags flags(const QModelIndex& index)                                               const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)   const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())           const override;

    QModelIndex parent(const QModelIndex& index)                                                const override;

    int rowCount(const QModelIndex& parent = QModelIndex())                                     const override;

    int columnCount(const QModelIndex& parent = QModelIndex())                                  const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    /**
     * @brief Reimplemented methods for handling drag-n-drop, encoding and decoding mime types.
     */
    Qt::DropActions supportedDropActions()                                                      const override;
    QStringList mimeTypes()                                                                     const override;
    QMimeData* mimeData(const QModelIndexList& indexes)                                         const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                      int column, const QModelIndex& parent) override;

    QList<int> getDragNewSelection()                                                            const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

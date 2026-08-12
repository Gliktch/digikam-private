/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-27
 * Description : Model to an ItemHistoryGraph
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "dragdropimplementations.h"
#include "itemhistorygraph.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemHistoryGraph;
class ItemInfo;
class ItemListModel;
class FilterAction;

class DIGIKAM_DATABASE_EXPORT ItemHistoryGraphModel : public QAbstractItemModel,
                                                      public DragDropModelImplementation
{
    Q_OBJECT

public:

    enum Mode
    {
        ImagesListMode,
        ImagesTreeMode,
        CombinedTreeMode
    };

    enum ExtraRoles
    {
        IsImageItemRole        = Qt::UserRole + 1000,
        IsFilterActionItemRole = Qt::UserRole + 1001,
        IsHeaderItemRole       = Qt::UserRole + 1002,
        IsCategoryItemRole     = Qt::UserRole + 1003,
        IsSeparatorItemRole    = Qt::UserRole + 1004,

        IsSubjectImageRole     = Qt::UserRole + 1010,

        FilterActionRole       = Qt::UserRole + 1020
    };

public:

    explicit ItemHistoryGraphModel(QWidget* const parent);
    ~ItemHistoryGraphModel()                                                                        override;

    void setMode(Mode mode);
    Mode mode()                                                                               const;

    /**
     * Set the history subject and the history graph.
     * Per default, the subject's history graph is read.
     */
    void setHistory(const ItemInfo& subject,
                    const ItemHistoryGraph& graph = ItemHistoryGraph());

    ItemInfo subject()                                                                        const;

    bool isImage(const QModelIndex& index)                                                    const;
    bool hasImage(const ItemInfo& info);
    ItemInfo imageInfo(const QModelIndex& index)                                              const;

    /**
     * Note: There may be multiple indexes for an info. The index found first is returned.
     */
    QModelIndex indexForInfo(const ItemInfo& info)                                            const;

    bool isFilterAction(const QModelIndex& index)                                             const;
    FilterAction filterAction(const QModelIndex& index)                                       const;

    ///@{
    /// QAbstractItemModel implementation.

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                   const override;
    int columnCount(const QModelIndex& parent = QModelIndex())                                const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                             const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex())                               const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())         const override;
    QModelIndex parent(const QModelIndex& index)                                              const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role)                         override;
    ///@}

    DECLARE_MODEL_DRAG_DROP_METHODS

    /**
     * Returns an internal image model used for entries representing images.
     * Note: Set a thumbnail thread on this model if you need thumbnails.
     */
    ItemListModel* imageModel()                                                               const;

    /**
     * If the given index is represented by the internal image model,
     * return the image model's index.
     * Otherwise an invalid index is returned.
     */
    QModelIndex imageModelIndex(const QModelIndex& index)                                     const;

private:

    /// @note disabled
    ItemHistoryGraphModel(const ItemHistoryGraphModel&)            = delete;
    ItemHistoryGraphModel& operator=(const ItemHistoryGraphModel&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

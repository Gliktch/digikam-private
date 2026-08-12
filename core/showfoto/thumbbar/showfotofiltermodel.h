/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 30-07-2013
 * Description : Qt filter model for showfoto items
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "showfotoitemmodel.h"
#include "showfotothumbnailmodel.h"
#include "showfotoitemsortsettings.h"

namespace ShowFoto
{

class ShowfotoFilterModel;

class ShowfotoSortFilterModel : public DCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit ShowfotoSortFilterModel(QObject* const parent = nullptr);
    ~ShowfotoSortFilterModel() override;

    void setSourceShowfotoModel(ShowfotoItemModel* const sourceModel);
    ShowfotoItemModel* sourceShowfotoModel()                                                  const;

    void setSourceFilterModel(ShowfotoSortFilterModel* const sourceModel);
    ShowfotoSortFilterModel* sourceFilterModel()                                              const;

    /**
     *Convenience methods mapped to ShowfotoItemModel.
     * Mentioned indexes returned come from the source Showfoto image model.
     */
    QModelIndex mapToSourceShowfotoModel(const QModelIndex& proxyIndex)                       const;
    QModelIndex mapFromSourceShowfotoModel(const QModelIndex& showfotoModelIndex)             const;
    QModelIndex mapFromDirectSourceToSourceShowfotoModel(const QModelIndex& sourceModelIndex) const;

    QList<QModelIndex> mapListToSource(const QList<QModelIndex>& indexes)                     const;
    QList<QModelIndex> mapListFromSource(const QList<QModelIndex>& sourceIndexes)             const;

    ShowfotoItemInfo        showfotoItemInfo(const QModelIndex& index)                        const;
    qlonglong          showfotoItemId(const QModelIndex& index)                               const;
    QList<ShowfotoItemInfo> showfotoItemInfos(const QList<QModelIndex>& indexes)              const;
    QList<qlonglong>   showfotoItemIds(const QList<QModelIndex>& indexes)                     const;

    QModelIndex indexForUrl(const QUrl& fileUrl)                                              const;
    QModelIndex indexForShowfotoItemInfo(const ShowfotoItemInfo& info)                        const;
    QModelIndex indexForShowfotoItemId(qlonglong id)                                          const;

    /**
     * Returns a list of all showfoto infos, sorted according to this model.
     * If you do not need a sorted list, use ShowfotoItemModel's showfotoItemInfo() method.
     */
    QList<ShowfotoItemInfo> showfotoItemInfosSorted()                                         const;

    /// Returns this, any chained ShowfotoFilterModel, or 0.
    virtual ShowfotoFilterModel* showfotoFilterModel()                                        const;

protected:

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    /// Reimplement if needed. Called only when model shall be set as (direct) sourceModel.
    virtual void setDirectSourceShowfotoModel(ShowfotoItemModel* const sourceModel);

protected:

    ShowfotoSortFilterModel* m_chainedModel = nullptr;
};

// ------------------------------------------------------------------------------------------

class ShowfotoFilterModel : public ShowfotoSortFilterModel
{
    Q_OBJECT

public:

    enum ShowfotoFilterModelRoles
    {
        /// Returns the current categorization mode.
        CategorizationModeRole         = ShowfotoItemModel::FilterModelRoles + 1,

        /// Returns the current sort order.
        SortOrderRole                  = ShowfotoItemModel::FilterModelRoles + 2,

        /// Returns the format of the index which is used for category.
        CategoryFormatRole             = ShowfotoItemModel::FilterModelRoles + 3,

        /**
         * Returns true if the given showfoto item is a group leader, and the group is opened.
         * TODO: GroupIsOpenRole        = ShowfotoItemModel::FilterModelRoles + 4
         */
        ShowfotoFilterModelPointerRole = ShowfotoItemModel::FilterModelRoles + 50
    };

public:

    explicit ShowfotoFilterModel(QObject* const parent = nullptr);
    ~ShowfotoFilterModel() override;

    ShowfotoItemSortSettings showfotoItemSortSettings()                                 const;

    void setShowfotoItemSortSettings(const ShowfotoItemSortSettings& sorter);

    /// Enables sending ShowfotoItemInfosAdded and ShowfotoItemInfosAboutToBeRemoved.
    void setSendShowfotoItemInfoSignals(bool sendSignals);
/*
    TODO: Implement grouping in Showfoto tool.
    bool isGroupOpen(qlonglong group) const;
    bool isAllGroupsOpen() const;
*/
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                 const override;
    ShowfotoFilterModel* showfotoFilterModel()                                          const override;

public Q_SLOTS:

    void setCategorizationMode(ShowfotoItemSortSettings::CategorizationMode mode);
    void setSortRole(ShowfotoItemSortSettings::SortRole role);
    void setSortOrder(ShowfotoItemSortSettings::SortOrder order);
/*
    TODO: Implement grouping in Showfoto tool.
    void setGroupOpen(qlonglong group, bool open);
    void toggleGroupOpen(qlonglong group);
    void setAllGroupsOpen(bool open);

    /// Changes the current image filter settings and refilters.
    TODO: Implement filtering in Showfoto tool.
    virtual void setItemFilterSettings(const ItemFilterSettings& settings);

    /// Changes the current image sort settings and resorts.
    TODO: virtual void setItemSortSettings(const ItemSortSettings& settings);
*/

Q_SIGNALS:

    /**
     * These signals need to be explicitly enabled with setSendItemInfoSignals().
     */
    void showfotoItemInfosAdded(const QList<ShowfotoItemInfo>& infos);
    void showfotoItemInfosAboutToBeRemoved(const QList<ShowfotoItemInfo>& infos);

protected Q_SLOTS:

    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

public:

    /// Declared as public because of use in sub-classes.
    class ShowfotoFilterModelPrivate;

protected:

    ShowfotoFilterModelPrivate* const d_ptr = nullptr;

protected:

    void setDirectSourceShowfotoModel(ShowfotoItemModel* const sourceModel)                   override;
/*
    TODO
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
*/
    int compareCategories(const QModelIndex& left, const QModelIndex& right)            const override;
    bool subSortLessThan(const QModelIndex& left, const QModelIndex& right)             const override;

    /**
     * Reimplement to customize category sorting,
     * Return negative if category of left < category right,
     * Return 0 if left and right are in the same category, else return positive.
     */
    virtual int compareInfosCategories(const ShowfotoItemInfo& left,
                                       const ShowfotoItemInfo& right)                   const;

    /**
     * Reimplement to customize sorting. Do not take categories into account here.
     */
    virtual bool infosLessThan(const ShowfotoItemInfo& left,
                               const ShowfotoItemInfo& right)                           const;

    /**
     * Returns a unique identifier for the category if info. The string need not be for user display.
     */
    virtual QString categoryIdentifier(const ShowfotoItemInfo& info)                    const;

private:

    Q_DECLARE_PRIVATE(ShowfotoFilterModel)
};

// -----------------------------------------------------------------------------------------------------

class NoDuplicatesShowfotoFilterModel : public ShowfotoSortFilterModel
{
    Q_OBJECT

public:

    explicit NoDuplicatesShowfotoFilterModel(QObject* const parent = nullptr);

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent)             const override;
};

} // namespace ShowFoto

Q_DECLARE_METATYPE(ShowFoto::ShowfotoFilterModel*)

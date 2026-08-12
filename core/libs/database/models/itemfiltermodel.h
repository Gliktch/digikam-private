/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText:      2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "textfilter.h"
#include "itemfiltersettings.h"
#include "itemmodel.h"
#include "itemsortsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageChangeset;
class ItemFilterModel;
class ImageTagChangeset;
class FaceTagsIface;

class DIGIKAM_DATABASE_EXPORT ItemFilterModelPrepareHook
{
public:

    virtual ~ItemFilterModelPrepareHook() = default;
    virtual void prepare(const QVector<ItemInfo>& infos) = 0;

private:

    Q_DISABLE_COPY(ItemFilterModelPrepareHook)
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageSortFilterModel : public DCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit ImageSortFilterModel(QObject* const parent = nullptr);

    void       setSourceItemModel(ItemModel* const model);
    ItemModel* sourceItemModel()                                                            const;

    void                  setSourceFilterModel(ImageSortFilterModel* const model);
    ImageSortFilterModel* sourceFilterModel()                                               const;

    QModelIndex mapToSourceItemModel(const QModelIndex& index)                              const;
    QModelIndex mapFromSourceItemModel(const QModelIndex& imagemodel_index)                 const;
    QModelIndex mapFromDirectSourceToSourceItemModel(const QModelIndex& sourceModel_index)  const;

    /**
     * @brief Convenience methods mapped to ItemModel.
     * Mentioned indexes returned come from the source image model.
     */
    QList<QModelIndex> mapListToSource(const QList<QModelIndex>& indexes)                   const;
    QList<QModelIndex> mapListFromSource(const QList<QModelIndex>& sourceIndexes)           const;

    ItemInfo         imageInfo(const QModelIndex& index)                                    const;
    qlonglong        imageId(const QModelIndex& index)                                      const;
    QList<ItemInfo>  imageInfos(const QList<QModelIndex>& indexes)                          const;
    QList<qlonglong> imageIds(const QList<QModelIndex>& indexes)                            const;

    QModelIndex indexForPath(const QString& filePath)                                       const;
    QModelIndex indexForItemInfo(const ItemInfo& info)                                      const;
    QModelIndex indexForImageId(qlonglong id)                                               const;

    /**
     * @return a list of all image infos, sorted according to this model.
     * If you do not need a sorted list, use ItemModel's imageInfos() method.
     */
    QList<ItemInfo> imageInfosSorted()                                                      const;

    /**
     * @return This instance if any chained ItemFilterModel, or nullptr.
     */
    virtual ItemFilterModel* imageFilterModel()                                             const;

protected:

    /**
     * @brief Reimplement if needed. Called only when model shall be set as (direct) sourceModel.
     */
    virtual void setDirectSourceItemModel(ItemModel* const model);

    /// @note made protected
    void setSourceModel(QAbstractItemModel* const model)                                   override;

protected:

    ImageSortFilterModel* m_chainedModel = nullptr;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterModel : public ImageSortFilterModel
{
    Q_OBJECT

public:

    enum ItemFilterModelRoles
    {
        /// @return The current categorization mode
        CategorizationModeRole      = ItemModel::FilterModelRoles + 1,

        /// @return The current sort order
        SortOrderRole               = ItemModel::FilterModelRoles + 2,

        /// @return The number of items in the index category
        //CategoryCountRole         = ItemModel::FilterModelRoles + 3,

        /// @return The id of the PAlbum of the index which is used for category
        CategoryAlbumIdRole         = ItemModel::FilterModelRoles + 3,

        /// @return The format of the index which is used for category
        CategoryFormatRole          = ItemModel::FilterModelRoles + 4,

        /// @return The date of the index which is used for category
        CategoryDateRole            = ItemModel::FilterModelRoles + 5,

        /// @return The suggested name for the face in this index
        CategoryFaceRole            = ItemModel::FilterModelRoles + 6,

        /// @return True if the given image is a group leader, and the group is opened
        GroupIsOpenRole             = ItemModel::FilterModelRoles + 7,
        ItemFilterModelPointerRole  = ItemModel::FilterModelRoles + 50
    };

public:

    explicit ItemFilterModel(QObject* const parent = nullptr);
    ~ItemFilterModel() override;

    /**
     * @brief Add a hook to get added images for preparation tasks before they are added in the model
     */
    void addPrepareHook(ItemFilterModelPrepareHook* const hook);
    void removePrepareHook(ItemFilterModelPrepareHook* const hook);

    /**
     * @return a set of DatabaseFields suggested to set as watch flags on the source ItemModel.
     * The contained flags will be those that this model can sort or filter by.
     */
    DatabaseFields::Set suggestedWatchFlags()                                               const;

    ItemFilterSettings        imageFilterSettings()                                         const;
    VersionItemFilterSettings versionItemFilterSettings()                                   const;
    GroupItemFilterSettings   groupItemFilterSettings()                                     const;
    ItemSortSettings          imageSortSettings()                                           const;

    /**
     * @brief Group is identified by the id of its group leader
     */
    bool isGroupOpen(qlonglong group)                                                       const;
    bool isAllGroupsOpen()                                                                  const;

    /**
     * @brief Enables sending imageInfosAdded and imageInfosAboutToBeRemoved
     */
    void setSendItemInfoSignals(bool sendSignals);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                     const override;
    ItemFilterModel* imageFilterModel()                                                     const override;

public Q_SLOTS:

    /**
     * @brief Changes the current version image filter settings and refilters.
     */
    void setVersionItemFilterSettings(const VersionItemFilterSettings& settings);

    /**
     * @brief Changes the current version image filter settings and refilters.
     */
    void setGroupItemFilterSettings(const GroupItemFilterSettings& settings);

    /**
     * @brief Adjust the current ItemFilterSettings.
     * Equivalent to retrieving the current filter settings, adjusting the parameter
     * and calling setItemFilterSettings.
     * Provided for convenience.
     * It is encouraged to use setItemFilterSettings if you change more than one
     * parameter at a time.
     */
    void setDayFilter(const QList<QDateTime>& days);
    void setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags,
                      ItemFilterSettings::MatchingCondition matchingCond, bool showUnTagged,
                      const QList<int>& clTagIds, const QList<int>& plTagIds);
    void setRatingFilter(int rating, ItemFilterSettings::RatingCondition ratingCond, bool isUnratedExcluded);
    void setMimeTypeFilter(int mimeTypeFilter);
    void setGeolocationFilter(const ItemFilterSettings::GeolocationCondition& condition);
    void setTextFilter(const SearchTextFilterSettings& settings);

    void setCategorizationMode(ItemSortSettings::CategorizationMode mode);
    void setCategorizationSortOrder(ItemSortSettings::SortOrder order);
    void setSortRole(ItemSortSettings::SortRole role);
    void setSortOrder(ItemSortSettings::SortOrder order);
    void setStringTypeNatural(bool natural);
    void setUrlWhitelist(const QList<QUrl>& urlList, const QString& id);
    void setIdWhitelist(const QList<qlonglong>& idList, const QString& id);

    void setVersionManagerSettings(const VersionManagerSettings& settings);
    void setExceptionList(const QList<qlonglong>& idlist, const QString& id);

    void setGroupOpen(qlonglong group, bool open);
    void toggleGroupOpen(qlonglong group);
    void setAllGroupsOpen(bool open);

    /**
     * @brief Changes the current image filter settings and refilters.
     */
    virtual void setItemFilterSettings(const ItemFilterSettings& settings);

    /**
     * @brief Changes the current image sort settings and resorts.
     */
    virtual void setItemSortSettings(const ItemSortSettings& settings);

Q_SIGNALS:

    /**
     * @brief Signals that the set filter matches at least one index
     */
    void filterMatches(bool matches);

    /**
     * @brief Signals that the set text filter matches at least one entry.
     * If no text filter is set, this signal is emitted
     * with 'false' when filterMatches() is emitted.
     */
    void filterMatchesForText(bool matchesByText);

    /**
     * @brief Emitted when the filter settings have been changed
     * (the model may not yet have been updated)
     */
    void filterSettingsChanged(const ItemFilterSettings& settings);

    /**
     * @brief These signals need to be explicitly enabled with setSendItemInfoSignals()
     */
    void imageInfosAdded(const QList<ItemInfo>& infos);
    void imageInfosAboutToBeRemoved(const QList<ItemInfo>& infos);

    /**
     * @brief This signal is emitted when a group is open or closed.
     */
    void signalGroupIsOpen(qlonglong group, bool open);

public:

    /// @note Declared as public because of use in sub-classes.
    class ItemFilterModelPrivate;

protected:

    ItemFilterModelPrivate* const d_ptr = nullptr;

protected:

    ItemFilterModel(ItemFilterModelPrivate& dd, QObject* const parent);

    void setDirectSourceItemModel(ItemModel* const model)                                 override;

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent)         const override;

    int  compareCategories(const QModelIndex& left, const QModelIndex& right)       const override;
    bool subSortLessThan(const QModelIndex& left, const QModelIndex& right)         const override;
/*
    virtual int  categoryCount(const ItemInfo& info)                                        const;
*/
    /**
     * @brief Reimplement to customize category sorting,
     * @return negative if category of left < category right,
     * or 0 if left and right are in the same category, else return positive.
     */
    virtual int compareInfosCategories(const ItemInfo& left, const ItemInfo& right)          const;

    /**
     * @brief In order to be able to Categorize by Faces, it's necessary to pass in the
     * face as well. One image may have multiple Faces in it, hence just the ItemInfo
     * isn't sufficient.
     */
    virtual int compareInfosCategories(const ItemInfo& left, const ItemInfo& right,
                                       const FaceTagsIface& leftFace,
                                       const FaceTagsIface& rightFace)                       const;

    /**
     * @brief Reimplement to customize sorting. Do not take categories into account here.
     */
    virtual bool infosLessThan(const ItemInfo& left, const ItemInfo& right)                  const;

    /**
     * @return A unique identifier for the category if info. The string need not be for user display.
     */
    virtual QString categoryIdentifier(const ItemInfo& info, const FaceTagsIface& face)      const;

protected Q_SLOTS:

    void slotModelReset();
    void slotUpdateFilter();

    void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotImageChange(const ImageChangeset& changeset);

    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

private:

    Q_DECLARE_PRIVATE(ItemFilterModel)
};

// -----------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT NoDuplicatesItemFilterModel : public ImageSortFilterModel
{
    Q_OBJECT

public:

    explicit NoDuplicatesItemFilterModel(QObject* const parent = nullptr);

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent)         const override;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemFilterModel*)

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : Sort settings for use with ItemFilterModel
 *
 * SPDX-FileCopyrightText: 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "itemsortcollator.h"

namespace Digikam
{

class ItemInfo;
class FaceTagsIface;

namespace DatabaseFields
{
    class Set;
}

class DIGIKAM_DATABASE_EXPORT ItemSortSettings
{
public:

    enum SortOrder
    {
        AscendingOrder  = Qt::AscendingOrder,
        DescendingOrder = Qt::DescendingOrder,
        DefaultOrder                ///< sort order depends on the chosen sort role
    };

    enum CategorizationMode
    {
        NoCategories,               ///< categorization switched off
        OneCategory,                ///< all items in one global category
        CategoryByAlbum,
        CategoryByFormat,
        CategoryByFaces,
        CategoryByMonth,
        CategoryByDay
    };

    enum SortRole
    {
        // NOTE: For legacy reasons, the order of the first five entries must remain unchanged

        /**
         * Compare by file name in alphabetical order.
         */
        SortByFileName,

        /**
         * Compare by file path in alphabetical order.
         */
        SortByFilePath,

        /**
         * Compare by camera creation date.
         */
        SortByCreationDate,

        /**
         * Compare by date modified by the computer.
         */
        SortByModificationDate,

        /**
         * Compare by file size in bytes.
         */
        SortByFileSize,

        /**
         * Compare by type-mime in alphabetical order.
         */
        SortByFormat,

        /**
         * Compare by Rating order.
         */
        SortByRating,

        /**
         * Compare by Pick Label order.
         */
        SortByPickLabel,

        /**
         * Compare by Color Label order.
         */
        SortByColorLabel,

        /**
         * Compare by pixels number.
         */
        SortByImageSize,

        /**
         * Compare by Exif Rotation flag.
         */
        SortByOrientation,

        /**
         * Compare by aspect ratio computed as ((width / height) * 100000).
         */
        SortByAspectRatio,

        /**
         * Compare by GPS location properties.
         * If an image lacks GPS coordinates it is placed after images with GPS data.
         * If both images lack GPS data, they are treated as equal.
         * The primary sort is by latitude, followed by longitude if latitudes are equal.
         */
        SortByGpsPosition,

        /**
         * Compare by counting the number of unconfirmed faces.
         */
        SortByFaces,

        /**
         * Compare by similarity results (only available in the Similarity Search View).
         */
        SortBySimilarity,

        /**
         * Compare by the manual sorting order and the file name.
         */
        SortByManualOrderAndName,

        /**
         * Compare by the manual sorting order and the creation date.
         */
        SortByManualOrderAndDate
    };

public:

    ItemSortSettings() = default;

    bool operator==(const ItemSortSettings& other)                              const;

    /**
     * @brief Compares the categories of left and right.
     * Return -1 if left is less than right, 0 if both fall in the same category,
     * and 1 if left is greater than right.
     * Adheres to set categorization mode and current category sort order.
     * Face passed in to allow Categorization by Faces. Pass in an empty
     * Face if not needed.
     */
    int compareCategories(const ItemInfo& left,
                          const ItemInfo& right,
                          const FaceTagsIface& leftFace,
                          const FaceTagsIface& rightFace)                       const;

    /**
     * @return True if left is less than right.
     * Adheres to current sort role and sort order.
     */
    bool lessThan(const ItemInfo& left, const ItemInfo& right)                  const;

    /**
     * @brief Compares the ItemInfos left and right.
     * @return -1 if left is less than right, 1 if left is greater than right,
     * and 0 if left equals right comparing the current sort role's value.
     * Adheres to set sort role and sort order.
     */
    int compare(const ItemInfo& left, const ItemInfo& right)                    const;

    /**
     * @return True if left QVariant is less than right.
     * Adheres to current sort role and sort order.
     * Use for extraValue, if necessary.
     */
    bool lessThan(const QVariant& left, const QVariant& right)                  const;

    void setSortRole(SortRole role);
    void setSortOrder(SortOrder order);
    void setStringTypeNatural(bool natural);

public:

    /// --- Categories ---

    void setCategorizationMode(CategorizationMode mode);
    void setCategorizationSortOrder(SortOrder order);


    bool isCategorized()                                                        const
    {
        return (categorizationMode >= CategoryByAlbum);
    }

public:

    /// --- Image Sorting ---

    int compare(const ItemInfo& left, const ItemInfo& right, SortRole sortRole) const;

    static Qt::SortOrder defaultSortOrderForCategorizationMode(CategorizationMode mode);
    static Qt::SortOrder defaultSortOrderForSortRole(SortRole role);

public:

    /// --- Change notification ---

    /**
     * @return Database fields a change in which would affect the current sorting.
     */
    DatabaseFields::Set watchFlags()                                            const;

public:

    /// --- Utilities ---

    /**
     * @return a < b if sortOrder is Ascending, or b < a if order is descending.
     */
    template <typename T>
    static inline bool lessThanByOrder(const T& a, const T& b, Qt::SortOrder sortOrder)
    {
        if (sortOrder == Qt::AscendingOrder)
        {
            return (a < b);
        }

        return (b < a);
    }

    /**
     * @return The usual compare result of -1, 0, or 1 for lessThan, equals and greaterThan.
     */
    template <typename T>
    static inline int compareValue(const T& a, const T& b)
    {
        if (a == b)
        {
            return 0;
        }

        if (a > b)
        {
            return 1;
        }

        return (-1);
    }

    /**
     * @brief Takes a typical result from a compare method (0 is equal, -1 is less than, 1 is greater than)
     * and applies the given sort order to it.
     */
    static inline int compareByOrder(int compareResult, Qt::SortOrder sortOrder)
    {
        if (sortOrder == Qt::AscendingOrder)
        {
            return compareResult;
        }

        return (- compareResult);
    }

    template <typename T>
    static inline int compareByOrder(const T& a, const T& b, Qt::SortOrder sortOrder)
    {
        return compareByOrder(compareValue(a, b), sortOrder);
    }

    /**
     * @brief Compares the two string by natural comparison and adheres to given sort order
     */
    static inline int naturalCompare(const QString& a,
                                     const QString& b,
                                     Qt::SortOrder sortOrder,
                                     Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive,
                                     bool natural = true)
    {
        const ItemSortCollator* const sorter = ItemSortCollator::instance();

        return compareByOrder(sorter->itemCompare(a, b, caseSensitive, natural), sortOrder);
    }

public:

    CategorizationMode   categorizationMode             = NoCategories;
    SortOrder            categorizationSortOrder        = DefaultOrder;

    /// Only Ascending or Descending, never DefaultOrder
    Qt::SortOrder        currentCategorizationSortOrder = Qt::AscendingOrder;
    Qt::CaseSensitivity  categorizationCaseSensitivity  = Qt::CaseSensitive;

    SortRole             sortRole                       = SortByFileName;
    SortOrder            sortOrder                      = DefaultOrder;
    bool                 strTypeNatural                 = true;

    Qt::SortOrder        currentSortOrder               = Qt::AscendingOrder;
    Qt::CaseSensitivity  sortCaseSensitivity            = Qt::CaseSensitive;
};

} // namespace Digikam

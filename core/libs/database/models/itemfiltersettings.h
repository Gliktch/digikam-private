/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ItemFilterModel
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

// Qt includes

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QSet>
#include <QUrl>
#include <QDateTime>

// Local includes

#include "searchtextbar.h"
#include "mimefilter.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemInfo;
class VersionManagerSettings;

namespace DatabaseFields
{
    class Set;
}

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchTextFilterSettings : public SearchTextSettings
{

public:

    enum TextFilterFields
    {
        None             = 0x00,
        ImageName        = 0x01,
        ImagePath        = 0x02,
        ImageTitle       = 0x04,
        ImageComment     = 0x08,
        TagName          = 0x10,
        AlbumName        = 0x20,
        ImageAspectRatio = 0x40,
        ImagePixelSize   = 0x80,
        All              = ImageName | ImagePath | ImageTitle | ImageComment | TagName | AlbumName | ImageAspectRatio | ImagePixelSize
    };

public:

    SearchTextFilterSettings() = default;

    explicit SearchTextFilterSettings(const SearchTextSettings& settings)
    {
        caseSensitive = settings.caseSensitive;
        text          = settings.text;
        invert        = false;
        textFields    = None;
    }

    TextFilterFields textFields = None;
    bool             invert     = false;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterSettings
{
public:

    /// Possible logical matching condition used to sort tags id.
    enum MatchingCondition
    {
        OrCondition,
        AndCondition
    };

    /// @brief Possible conditions used to filter rating: >=, =, <=
    enum RatingCondition
    {
        GreaterEqualCondition,
        EqualCondition,
        LessEqualCondition
    };

    /// @brief Possible logical matching condition used to sort geolocation.
    enum GeolocationCondition
    {
        GeolocationNoFilter       = 0,
        GeolocationNoCoordinates  = 1 << 1,
        GeolocationHasCoordinates = 1 << 2
    };

public:

    ItemFilterSettings() = default;

    /**
     *  @return True if the given ItemInfo matches the filter criteria.
     *  Optionally, foundText is set to true if it matched by text search.
     */
    bool matches(const ItemInfo& info, bool* const foundText = nullptr) const;

public:

    // --- Tags filter ---

    void setTagFilter(const QList<int>& includedTags,
                      const QList<int>& excludedTags,
                      MatchingCondition matchingCond,
                      bool              showUnTagged,
                      const QList<int>& clTagIds,
                      const QList<int>& plTagIds);

public:

    // --- Rating filter ---

    void setRatingFilter(int rating, RatingCondition ratingCond, bool isUnratedExcluded);

public:

    // --- Date filter ---

    void setDayFilter(const QList<QDateTime>& days);

public:

    // --- Text filter ---

    void setTextFilter(const SearchTextFilterSettings& settings);
    void setTagNames(const QHash<int, QString>& tagNameHash);
    void setAlbumNames(const QHash<int, QString>& albumNameHash);

public:

    // --- Mime filter ---

    void setMimeTypeFilter(int mimeTypeFilter);

public:

    // --- Geolocation filter

    void setGeolocationFilter(const GeolocationCondition& condition);

public:

    /// @return True if the day is a filter criteria
    bool isFilteringByDay()                                 const;

    /// @return True if the type mime is a filter criteria
    bool isFilteringByTypeMime()                            const;

    /// @return Whether geolocation is a filter criteria
    bool isFilteringByGeolocation()                         const;

    /// @return Ttue if the rating is a filter criteria
    bool isFilteringByRating()                              const;

    /// @return True if the pick labels is a filter criteria
    bool isFilteringByPickLabels()                          const;

    /// @return True if the color labels is a filter criteria
    bool isFilteringByColorLabels()                         const;

    /// @return True if the tag is a filter criteria
    bool isFilteringByTags()                                const;

    /// @return True if the text (including comment) is a filter criteria
    bool isFilteringByText()                                const;

    /// @return True if images will be filtered by these criteria at all
    bool isFiltering()                                      const;

public:

    // --- URL whitelist filter

    void setUrlWhitelist(const QList<QUrl>& urlList, const QString& id);

public:

    // --- ID whitelist filter

    void setIdWhitelist(const QList<qlonglong>& idList, const QString& id);

public:

    // --- Change notification ---

    /**
     * @return Database fields a change in which would affect the current filtering.
     * To find out if an image tag change affects filtering, test isFilteringByTags().
     * The text filter will also be affected by changes in tags and album names.
     */
    DatabaseFields::Set watchFlags()                        const;

private:

    /**
     * @return Whether some internal filtering (whitelist by id or URL) or normal filtering is going on
     */
    bool isFilteringInternally()                            const;

private:

    // --- Tags filter ---

    bool                              m_untaggedFilter       = false;
    QList<int>                        m_includeTagFilter;
    QList<int>                        m_excludeTagFilter;
    MatchingCondition                 m_matchingCond         = OrCondition;
    QList<int>                        m_colorLabelTagFilter;
    QList<int>                        m_pickLabelTagFilter;

    // --- Rating filter ---

    int                               m_ratingFilter         = 0;
    RatingCondition                   m_ratingCond           = GreaterEqualCondition;
    bool                              m_isUnratedExcluded    = false;

    // --- Date filter ---

    QHash<QDateTime, bool>            m_dayFilter;

    // --- Text filter ---

    SearchTextFilterSettings          m_textFilterSettings;

    /// @brief Helpers for text search: Set these if you want to search album or tag names with text search

    QHash<int, QString>               m_tagNameHash;
    QHash<int, QString>               m_albumNameHash;

    // --- Mime filter ---

    MimeFilter::TypeMimeFilter        m_mimeTypeFilter       = MimeFilter::AllFiles;

    // --- Geolocation filter

    GeolocationCondition              m_geolocationCondition = GeolocationNoFilter;

    // --- URL whitelist filter

    QHash<QString, QList<QUrl> >      m_urlWhitelists;

    // --- ID whitelist filter

    QHash<QString, QList<qlonglong> > m_idWhitelists;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT VersionItemFilterSettings
{
public:

    VersionItemFilterSettings() = default;
    explicit VersionItemFilterSettings(const VersionManagerSettings& settings);

    bool operator==(const VersionItemFilterSettings& other) const;

    /**
     * @return True if the given ItemInfo matches the filter criteria.
     */
    bool matches(const ItemInfo& info)                      const;

    bool isHiddenBySettings(const ItemInfo& info)           const;
    bool isExemptedBySettings(const ItemInfo& info)         const;

    // --- Tags filter ---

    void setVersionManagerSettings(const VersionManagerSettings& settings);

    /**
     * @brief Add list with exceptions: These images will be exempted from filtering by this filter
     */
    void setExceptionList(const QList<qlonglong>& idlist, const QString& id);

    /// @return True if images will be filtered by these criteria at all
    bool isFiltering()                                      const;

    /// @return True if the tag is a filter criteria
    bool isFilteringByTags()                                const;

    /// @note DatabaseFields::Set watchFlags() const Would return 0

protected:

    QList<int>                        m_excludeTagFilter;
    int                               m_includeTagFilter    = 0;
    int                               m_exceptionTagFilter  = 0;
    QHash<QString, QList<qlonglong> > m_exceptionLists;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT GroupItemFilterSettings
{
public:

    GroupItemFilterSettings() = default;

    bool operator==(const GroupItemFilterSettings& other)   const;

    /**
     * @return True if the given ItemInfo matches the filter criteria.
     */
    bool matches(const ItemInfo& info)                      const;

    /**
     * @brief Open or close a group.
     */
    void setOpen(qlonglong group, bool open);
    bool isOpen(qlonglong group)                            const;

    /**
     * @brief Open all groups
     */
    void setAllOpen(bool open);
    bool isAllOpen()                                        const;

    /// @return True if images will be filtered by these criteria at all
    bool isFiltering()                                      const;

    DatabaseFields::Set watchFlags()                        const;

protected:

    bool            m_allOpen       = false;
    QSet<qlonglong> m_openGroups;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemFilterSettings::GeolocationCondition)

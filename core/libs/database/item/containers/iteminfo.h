/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QList>
#include <QSize>
#include <QString>
#include <QDateTime>
#include <QExplicitlySharedDataPointer>

// Local includes

#include "digikam_export.h"
#include "coredbalbuminfo.h"
#include "coredburl.h"
#include "coredbfields.h"
#include "iteminfolist.h"

namespace Digikam
{

class DImageHistory;
class HistoryImageId;
class ItemComments;
class ImageCommonContainer;
class ItemCopyright;
class ItemExtendedProperties;
class ItemInfoData;
class ItemListerRecord;
class ImageMetadataContainer;
class VideoMetadataContainer;
class ItemPosition;
class ItemTagPair;
class PhotoInfoContainer;
class VideoInfoContainer;
class Template;
class ThumbnailIdentifier;
class ThumbnailInfo;

/**
 * @brief The ItemInfo class contains provides access to the database for a single item.
 *        The properties can be read and written. Information will be cached.
 *
 * @note access rules for all methods in this class:
 *       ItemInfoData members shall be accessed only under CoreDbAccess lock.
 *       The id and albumId are the exception to this rule, as they are
 *       primitive and will never change during the lifetime of an object.
 */
class DIGIKAM_DATABASE_EXPORT ItemInfo
{
public:

    typedef DatabaseFields::Hash<QVariant> DatabaseFieldsHashRaw;

public:

    /**
     * @brief Constructor
     * Creates a null item info
     */
    ItemInfo();

    /**
     * @brief Constructor. Creates an ItemInfo object without any cached data initially.
     * @param ID the unique ID for this item
     */
    explicit ItemInfo(qlonglong ID);

    /**
     * @brief Constructor. Creates an ItemInfo object where the provided information
     * will initially be available cached, without database access.
     */
    explicit ItemInfo(const ItemListerRecord& record);

    /**
     * @brief Copy constructor.
     */
    ItemInfo(const ItemInfo& info);

    /**
     * @brief Destructor
     */
    ~ItemInfo();

    /**
     * @brief Creates an ItemInfo object from a file url.
     */
    static ItemInfo fromLocalFile(const QString& path);
    static ItemInfo fromUrl(const QUrl& url);

    /**
     * @brief Create an ItemInfo object from the given combination, which
     * must be cleaned and corresponding to the values in the database
     */
    static ItemInfo fromLocationAlbumAndName(int locationId, const QString& album, const QString& _name);

    ItemInfo& operator=(const ItemInfo& info);

    bool operator==(const ItemInfo& info)                                               const;
    bool operator!=(const ItemInfo& info)                                               const;
    bool operator<(const ItemInfo& info)                                                const;

    /**
     * @brief  Copy database information of this item to a newly created item
     * @param  dstAlbumID  destination album id
     * @param  dstFileName new filename
     * @return an ItemInfo object of the new item
     */
    ItemInfo copyItem(int dstAlbumID, const QString& dstFileName);

    /**
     * @return true if this is a valid ItemInfo,
     * and the location of the item is currently available
     * (information freshly obtained from CollectionManager)
     */
    bool isLocationAvailable()                                                          const;

public:

    // -----------------------------------------------------------------------------

    /** @name Operations with Properties
     */

    //@{

    /**
     * @return true if this objects contains valid data
     */
    bool isNull()                                                                       const;

    /**
     * @return the name of the item
     */
    QString name()                                                                      const;

    /**
     * @return the datetime of the item
     */
    QDateTime dateTime()                                                                const;

    /**
     * @return the modification datetime of the item
     */
    QDateTime modDateTime()                                                             const;

    /**
     * @return the filesize of the item
     */
    qlonglong fileSize()                                                                const;

    /**
     * @return the dimensions of the item (valid only if dimensions
     * have been requested)
     */
    QSize dimensions()                                                                  const;

    /**
     * @return the file:// url.
     * This is equivalent to QUrl::fromLocalFile(filePath())
     */
    QUrl fileUrl()                                                                      const;

    /**
     * @return the file path to the item
     */
    QString filePath()                                                                  const;

    /**
     * @return the relative path part to the item
     */
    QString relativePath()                                                              const;

    /**
     * @return the unique id for this item
     */
    qlonglong id()                                                                      const;

    /**
     * @return the id of the PAlbum to which this item belongs
     */
    int albumId()                                                                       const;

    /**
     * @return The album root id
     */
    int albumRootId()                                                                   const;

    /**
     * @return the id of the Aspect Ratio for this item
     */
    double aspectRatio()                                                                const;

    /**
     * @return the manual sort order
     */
    qlonglong manualOrder()                                                             const;

    /**
     * @return the category of the item: Image, Audio, Video
     */
    DatabaseItem::Category category()                                                   const;

    /**
     * @return the item format / mimetype as a standardized
     * string (see project/documents/DBSCHEMA.ODS).
     */
    QString format()                                                                    const;

    /**
     * @return true if the item is marked as visible in the database.
     */
    bool isVisible()                                                                    const;

    /**
     * @return true if the corresponding file was not deleted.
     */
    bool isRemoved()                                                                    const;

    /**
     * @return the orientation of the item,
     * (MetaEngine::ImageOrientation, EXIF standard)
     */
    int orientation()                                                                   const;

    /**
     * @return the default title for this item
     */
    QString title()                                                                     const;

    /**
     * @return the default comment for this item
     */
    QString comment()                                                                   const;

    /**
     * @return the number of Faces in this item.
     */
    int faceCount()                                                                     const;

    /**
     * @return the number of Unconfirmed Faces in this item.
     */
    int unconfirmedFaceCount()                                                          const;

    /**
     * @return the map of Tag Region (in XML form) to Suggested Names for all
     * Faces in the Image.
     * Used to categorize items based on Face Suggestions.
     */
    QMap<QString, QString> getSuggestedNames()                                          const;

    /**
     * @brief Set the name (write it to database)
     * @param newName the new name.
     */
    void setName(const QString& newName);

    /**
     * @brief Set the date and time (write it to database)
     * @param dateTime the new date and time.
     */
    void setDateTime(const QDateTime& _dateTime);

    /**
     * @brief Set the modification date and time (write it to database)
     * @param dateTime the new modification date and time.
     */
    void setModDateTime(const QDateTime& dateTime);

    /**
     * @brief Set the manual sorting order for the item
     */
    void setManualOrder(qlonglong value);

    /**
     * @brief Set the orientation for the item
     */
    void setOrientation(int value);

    /**
     * @brief Set the visibility flag - triggers between Visible and Hidden
     */
    void setVisible(bool _isVisible);

    /**
     * @todo Supports only VideoMetadataField and ImageMetadataField values for now.
     */
    DatabaseFieldsHashRaw getDatabaseFieldsRaw(const DatabaseFields::Set& requestedSet) const;
    QVariant getDatabaseFieldRaw(const DatabaseFields::Set& requestedField)             const;

    //@}

public:

    // -----------------------------------------------------------------------------

    /** @name Operations with Geolocation
     */

    //@{

    /**
     * @brief Retrieve the ItemPosition object for this item.
     */
    ItemPosition imagePosition()                                                        const;

    /**
     * @brief Retrieves the coordinates and the altitude.
     * @return 0 if hasCoordinates(), or hasAltitude resp, is false.
     */
    double longitudeNumber()                                                            const;
    double latitudeNumber()                                                             const;
    double altitudeNumber()                                                             const;
    bool   hasCoordinates()                                                             const;
    bool   hasAltitude()                                                                const;

    //@}

public:

    // -----------------------------------------------------------------------------

    /** @name Operations with History
     */

    //@{

    /**
     * @brief Retrieves and sets the item history from the database.
     * @note: The item history retrieved here does typically include all
     * steps from the original to this item, but does not reference this item
     * itself.
     */
    DImageHistory imageHistory()                                                        const;
    void setItemHistory(const DImageHistory& history);
    bool hasImageHistory()                                                              const;

    /**
     * @brief Retrieves and sets this' item UUID
     */
    QString uuid()                                                                      const;
    void setUuid(const QString& _uuid);

    /**
     * @brief Constructs a HistoryImageId with all available information for this item.
     */
    HistoryImageId historyImageId()                                                     const;

    /**
     * @brief Retrieve information about items from which this item
     * is derived (ancestorImages) and items that have been derived
     * from this items (derivedImages).
     */
    bool hasDerivedImages()                                                             const;
    bool hasAncestorImages()                                                            const;

    QList<ItemInfo> derivedImages()                                                     const;
    QList<ItemInfo> ancestorImages()                                                    const;

    /**
     * @return the cloud of all directly or indirectly related items,
     * derived items or ancestors, in from of "a derived from b" pairs.
     */
    QList<QPair<qlonglong, qlonglong> > relationCloud()                                 const;

    /**
     * @brief Add a relation to the database:
     * This item is derived from the ancestorImage.
     */
    void markDerivedFrom(const ItemInfo& ancestorImage);

    //@}

public:

    // -----------------------------------------------------------------------------

    /** @name Operations with Groups
     */

    //@{

    /**
     * @brief The item is grouped in the group of another (leading) item.
     */
    bool isGrouped()                                                                    const;

    /**
     * @brief The item is the leading item of a group,
     * there are other items grouped behind this one.
     */
    bool hasGroupedImages()                                                             const;
    int  numberOfGroupedImages()                                                        const;

    /**
     * @return the leading item of the group.
     * @return a null item if this item is not grouped (isGrouped())
     */
    ItemInfo groupImage()                                                               const;
    qlonglong groupImageId()                                                            const;

    /**
     * @return the list of items grouped behind this item (not including this
     * item itself) and an empty list if there is none.
     */
    QList<ItemInfo> groupedImages()                                                     const;

    /**
     * @brief Group this item behind the given item
     */
    void addToGroup(const ItemInfo& info);

    /**
     * @brief This item is grouped behind another item:
     * Remove this item from its group
     */
    void removeFromGroup();

    /**
     * @brief This item hasGroupedImages(): Split up the group,
     * remove all groupedImages() from this item's group.
     */
    void clearGroup();

    //@}

public:

    // -----------------------------------------------------------------------------

    /** @name Operations with Containers
     */

    //@{

    /**
     * @brief Retrieve information about the item,
     * in form of numbers and user presentable strings,
     * for certain defined fields of information (see databaseinfocontainers.h)
     */
    ImageCommonContainer   imageCommonContainer()                                       const;
    ImageMetadataContainer imageMetadataContainer()                                     const;
    VideoMetadataContainer videoMetadataContainer()                                     const;
    PhotoInfoContainer     photoInfoContainer()                                         const;
    VideoInfoContainer     videoInfoContainer()                                         const;

    /**
     * @brief Retrieve metadata template information about the item.
     */
    Template metadataTemplate()                                                         const;

    /**
     * @brief Set metadata template information (write it to database)
     * @param t the new template data.
     */
    void setMetadataTemplate(const Template& t);

    /**
     * @brief Remove all template info about the item from database.
     */
    void removeMetadataTemplate();

    /**
     * @brief Retrieve the ItemComments object for this item.
     * This object allows full read and write access to all comments
     * and their properties.
     * You need to hold CoreDbAccess to ensure the validity.
     * For simple, cached read access see comment().
     */
    ItemComments imageComments(const CoreDbAccess& access)                                    const;

    /**
     * @brief Retrieve the ItemCopyright object for this item.
     * This object allows full read and write access to all copyright
     * values.
     */
    ItemCopyright imageCopyright()                                                      const;

    /**
     * @brief Retrieve the ItemExtendedProperties object for this item.
     * This object allows full read and write access to all extended properties
     * values.
     */
    ItemExtendedProperties imageExtendedProperties()                                    const;

    //@}

public:

    // -----------------------------------------------------------------------------

    /** @name Operations with Tags
     */

    //@{

    /**
     * @brief Adds a tag to the item (writes it to database)
     * @param tagID the ID of the tag to add
     */
    void setTag(int tagID);

    /**
     * @brief Adds tags in the list to the item.
     * Tags are created if they do not yet exist
     */
    void addTagPaths(const QStringList& tagPaths);

    /**
     * @brief Remove a tag from the item (removes it from database)
     * @param tagID the ID of the tag to remove
     */
    void removeTag(int tagID);

    /**
     * @brief Remove all tags from the item (removes it from database)
     */
    void removeAllTags();

    /**
     * @brief Retrieve an ItemTagPair object for a single tag, or for all
     * item/tag pairs for which properties are available
     * (not necessarily the assigned tags)
     */
    ItemTagPair imageTagPair(int tagId)                                                 const;
    QList<ItemTagPair> availableItemTagPairs()                                          const;

    /**
     * @return a list of IDs of tags assigned to this item
     * @see tagNames
     * @see tagPaths
     * @see Album::id()
     */
    QList<int> tagIds()                                                                 const;

private:

    void loadTagIds()                                                                   const;

    //@}

public:

    // -----------------------------------------------------------------------------

    /**
     * @name Operations with Labels
     */

    //@{

    /**
     * @return the Pick Label Id (see PickLabel values in globals.h)
     */
    int pickLabel()                                                                     const;

    /**
     * @return the Color Label Id (see ColorLabel values in globals.h)
     */
    int colorLabel()                                                                    const;

    /**
     * @return the rating
     */
    int rating()                                                                        const;

    /**
     * @brief Set the pick Label Id for the item (see PickLabel values from globals.h)
     */
    void setPickLabel(int value);

    /**
     * @brief Set the color Label Id for the item (see ColorLabel values from globals.h)
     */
    void setColorLabel(int value);

    /**
     * @brief Set the rating for the item
     */
    void setRating(int value);

    //@}

public:

    // -----------------------------------------------------------------------------

    /**
     * @name Operations with Thumbnails
     */

    //@{

    /**
     * @brief Fills a ThumbnailIdentifier / ThumbnailInfo from this ItemInfo
     */
    ThumbnailIdentifier thumbnailIdentifier()                                           const;
    ThumbnailInfo thumbnailInfo()                                                       const;
    static ThumbnailIdentifier thumbnailIdentifier(qlonglong id);

    //@}

public:

    // -----------------------------------------------------------------------------

    /**
     * @name Operations with Similarity
     */

    //@{

    double similarityTo(const qlonglong imageId)                                        const;
    double currentSimilarity()                                                          const;

    /**
     * @return the id of the current fuzzy search reference item.
     */
    qlonglong currentReferenceImage()                                                   const;

    /**
     * @return a signature for the item.
     */

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    size_t hash()                                                                       const;

#else

    uint hash()                                                                         const;

#endif

    /**
     * @brief Scans the database for items with the given signature.
     */
    QList<ItemInfo> fromUniqueHash(const QString& uniqueHash, qlonglong fileSize);

    /**
     * @return the unique hash signature as string of the item.
     */
    QString uniqueHash()                                                                const;

    //@}

private:

    friend class ItemInfoCache;
    friend class ItemInfoList;

    QExplicitlySharedDataPointer<ItemInfoData> m_data;
};

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

inline size_t qHash(const ItemInfo& info)

#else

inline uint qHash(const ItemInfo& info)

#endif

{
    return info.hash();
}

//! @brief qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
DIGIKAM_DATABASE_EXPORT QDebug operator<<(QDebug stream, const ItemInfo& info);

} // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::ItemInfo, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(Digikam::ItemInfo)

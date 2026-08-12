/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-01-05
 * Description : Metadata handling
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014-2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatahub.h"

// Qt includes

#include <QFileInfo>
#include <QMutex>
#include <QScopedPointer>
#include <QMutexLocker>

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbwatch.h"
#include "iteminfo.h"
#include "itemcomments.h"
#include "itemposition.h"
#include "template.h"
#include "templatemanager.h"
#include "applicationsettings.h"
#include "itemattributeswatch.h"
#include "tagscache.h"
#include "facetagseditor.h"
#include "metadatahubmngr.h"

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN MetadataHub::Private
{
public:

    Private() = default;

public:

    int                            pickLabel        = -1;
    int                            colorLabel       = -1;
    int                            rating           = -1;
    int                            count            = 0;

    QDateTime                      dateTime;
    QPair<QSize, int>              imageProp;

    CaptionsMap                    titles;
    CaptionsMap                    comments;

    Template                       metadataTemplate;

    QMap<int, MetadataHub::Status> tags;

    QStringList                    tagList;

    QList<FaceTagsIface>           facesList;
    ItemPosition                   itemPosition;

    MetadataHub::Status            dateTimeStatus   = MetadataHub::MetadataInvalid;
    MetadataHub::Status            titlesStatus     = MetadataHub::MetadataInvalid;
    MetadataHub::Status            commentsStatus   = MetadataHub::MetadataInvalid;
    MetadataHub::Status            pickLabelStatus  = MetadataHub::MetadataInvalid;
    MetadataHub::Status            colorLabelStatus = MetadataHub::MetadataInvalid;
    MetadataHub::Status            ratingStatus     = MetadataHub::MetadataInvalid;
    MetadataHub::Status            templateStatus   = MetadataHub::MetadataInvalid;

public:

    template <class T> void loadSingleValue(const T& data, T& storage, MetadataHub::Status& status);
};

// ------------------------------------------------------------------------------------------

MetadataHub::MetadataHub()
    : d(new Private)
{
}

MetadataHub::~MetadataHub()
{
    delete d;
}

void MetadataHub::reset()
{
    (*d) = Private();
}

// --------------------------------------------------

void MetadataHub::load(const ItemInfo& info)
{
    d->count++;

    //qCDebug(DIGIKAM_GENERAL_LOG) << "---------------------------------Load from ItemInfo ----------------";

    CaptionsMap commentMap;
    CaptionsMap titleMap;

    {
        CoreDbAccess access;
        ItemComments comments = info.imageComments(access);
        commentMap            = comments.toCaptionsMap();
        titleMap              = comments.toCaptionsMap(DatabaseComment::Title);
    }

    load(info.dateTime(),
         titleMap,
         commentMap,
         info.colorLabel(),
         info.pickLabel(),
         info.rating(),
         info.metadataTemplate());

    QList<int> tagIds = info.tagIds();
    loadTags(tagIds);
    loadFaceTags(info);

    d->itemPosition   = info.imagePosition();
}

/**
 * private common code to merge tags
 */
void MetadataHub::loadTags(const QList<int>& loadedTags)
{
    d->tags.clear();

    for (int tagId : std::as_const(loadedTags))
    {
        if (TagsCache::instance()->isInternalTag(tagId))
        {
            continue;
        }

        d->tags[tagId] = MetadataAvailable;
    }
}

/**
 * private common code to load dateTime, comment, color label, pick label, rating
 */
void MetadataHub::load(const QDateTime& dateTime,
                       const CaptionsMap& titles,
                       const CaptionsMap& comments,
                       int colorLabel, int pickLabel,
                       int rating, const Template& t)
{
    if (dateTime.isValid())
    {
        d->loadSingleValue<QDateTime>(dateTime, d->dateTime, d->dateTimeStatus);
    }

    d->loadSingleValue<int>(pickLabel, d->pickLabel, d->pickLabelStatus);

    d->loadSingleValue<int>(colorLabel, d->colorLabel, d->colorLabelStatus);

    d->loadSingleValue<int>(rating, d->rating, d->ratingStatus);

    d->loadSingleValue<CaptionsMap>(titles, d->titles, d->titlesStatus);

    d->loadSingleValue<CaptionsMap>(comments, d->comments, d->commentsStatus);

    d->loadSingleValue<Template>(t, d->metadataTemplate, d->templateStatus);
}

/**
 * template method used by comment and template
 */
template <class T> void MetadataHub::Private::loadSingleValue(const T& data, T& storage,
                                                              MetadataHub::Status& status)
{
    switch (status)
    {
        case MetadataHub::MetadataInvalid:
        {
            storage = data;
            status  = MetadataHub::MetadataAvailable;
            break;
        }

        case MetadataHub::MetadataAvailable:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "You should not load more than one image info in metadatahub";
            break;
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

/**
 * safe method
 */
bool MetadataHub::writeToMetadata(const ItemInfo& info,
                                  WriteComponent writeMode,
                                  bool ignoreLazySync,
                                  const MetaEngineSettingsContainer &settings)
{
    applyChangeNotifications();

    // if no metadata container is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!

    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    if (!ignoreLazySync && settings.useLazySync)
    {
        MetadataHubMngr::instance()->addPending(info);

        return true;
    }

    writeToBaloo(info.filePath());

    QScopedPointer<DMetadata> metadata(new DMetadata(info.filePath()));

    if (write(*metadata, writeMode, settings))
    {
        bool success = metadata->applyChanges();
        ItemAttributesWatch::instance()->fileMetadataChanged(QUrl::fromLocalFile(info.filePath()));
        return success;
    }

    return false;
}

bool MetadataHub::write(DMetadata& metadata,
                        WriteComponent writeMode,
                        const MetaEngineSettingsContainer& settings)
{
    applyChangeNotifications();

    bool dirty = false;

    metadata.setSettings(settings);

    // find out in advance if we have something to write - needed for FullWriteIfChanged mode

    bool saveTitle      = (settings.saveComments   && (d->titlesStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_TITLE));
    bool saveComment    = (settings.saveComments   && (d->commentsStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_COMMENTS));
    bool saveDateTime   = (settings.saveDateTime   && (d->dateTimeStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_DATETIME));
    bool savePickLabel  = (settings.savePickLabel  && (d->pickLabelStatus  == MetadataAvailable) && writeMode.testFlag(WRITE_PICKLABEL));
    bool saveColorLabel = (settings.saveColorLabel && (d->colorLabelStatus == MetadataAvailable) && writeMode.testFlag(WRITE_COLORLABEL));
    bool saveRating     = (settings.saveRating     && (d->ratingStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_RATING));
    bool saveTemplate   = (settings.saveTemplate   && (d->templateStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_TEMPLATE));
    bool saveTags       = (settings.saveTags       && writeMode.testFlag(WRITE_TAGS));
    bool saveFaces      = (settings.saveFaceTags   && writeMode.testFlag(WRITE_TAGS));
    bool savePosition   = (settings.savePosition   && writeMode.testFlag(WRITE_POSITION));

    if (saveTitle)
    {
        // Store titles in image as Iptc Object name and Xmp.

        dirty |= metadata.setItemTitles(d->titles);
    }

    if (saveComment)
    {
        // Store comments in image as JFIF comments, Exif comments, Iptc Caption, and Xmp.

        dirty |= metadata.setItemComments(d->comments);
    }

    if (saveDateTime)
    {
        // Store Image Date & Time as Exif and Iptc tags.

        dirty |= metadata.setImageDateTime(d->dateTime, false);
    }

    if (savePickLabel)
    {
        // Store Image Pick Label as XMP tag.

        dirty |= metadata.setItemPickLabel(d->pickLabel);
    }

    if (saveColorLabel)
    {
        // Store Image Color Label as XMP tag.

        dirty |= metadata.setItemColorLabel(d->colorLabel);
    }

    if (saveRating)
    {
        // Store Image rating as Iptc tag.

        dirty |= metadata.setItemRating(d->rating);
    }

    if (saveTemplate)
    {
        if (d->metadataTemplate.isEmpty())
        {
            dirty |= metadata.removeMetadataTemplate();
        }
        else
        {
            // Store metadata template as XMP tag.

            dirty |= metadata.setMetadataTemplate(d->metadataTemplate);
        }
    }

    if (savePosition)
    {
        if (d->itemPosition.hasCoordinates())
        {
            double altitude = d->itemPosition.altitude();

            dirty |= metadata.setGPSInfo(d->itemPosition.hasAltitude() ? &altitude : nullptr,
                                         d->itemPosition.latitudeNumber(),
                                         d->itemPosition.longitudeNumber());
        }
        else
        {
            dirty |= metadata.removeGPSInfo();
        }
    }

    dirty |= writeFaceTagsMap(metadata, saveFaces);

    dirty |= writeTags(metadata, saveTags);

    return dirty;
}

bool MetadataHub::write(const QString& filePath,
                        WriteComponent writeMode,
                        bool ignoreLazySync,
                        const MetaEngineSettingsContainer& settings)
{
    applyChangeNotifications();

    // if no metadata container is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!

    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    if (!ignoreLazySync && settings.useLazySync)
    {
        ItemInfo info = ItemInfo::fromLocalFile(filePath);
        MetadataHubMngr::instance()->addPending(info);

        return true;
    }

    writeToBaloo(filePath);

    QScopedPointer<DMetadata> metadata(new DMetadata(filePath));

    if (write(*metadata, writeMode, settings))
    {
        bool success = metadata->applyChanges();
        ItemAttributesWatch::instance()->fileMetadataChanged(QUrl::fromLocalFile(filePath));

        return success;
    }

    return false;
}

bool MetadataHub::write(const DImg& image,
                        WriteComponent writeMode,
                        bool ignoreLazySync,
                        const MetaEngineSettingsContainer& settings)
{
    applyChangeNotifications();

    // if no metadata container is needed at all, don't construct one

    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    // See DImgLoader::readMetadata() and saveMetadata()

    QScopedPointer<DMetadata> metadata(new DMetadata);
    metadata->setData(image.getMetadata());

    QString filePath = image.originalFilePath();

    if (filePath.isEmpty())
    {
        filePath = image.lastSavedFilePath();
    }

    if (!ignoreLazySync && settings.useLazySync && !filePath.isEmpty())
    {
        ItemInfo info = ItemInfo::fromLocalFile(filePath);
        MetadataHubMngr::instance()->addPending(info);

        return true;
    }

    if (!filePath.isEmpty())
    {
        writeToBaloo(filePath);
    }

    return write(*metadata, writeMode, settings);
}

bool MetadataHub::writeTags(const QString& filePath,
                            WriteComponent writeMode,
                            const MetaEngineSettingsContainer& settings)
{
    applyChangeNotifications();

    // if no metadata container is needed at all, don't construct one -
    // important optimization if writing to file is turned off in setup!

    if (!willWriteMetadata(writeMode, settings))
    {
        return false;
    }

    QScopedPointer<DMetadata> metadata(new DMetadata(filePath));
    metadata->setSettings(settings);
    bool saveFaces = settings.saveFaceTags;
    bool saveTags  = settings.saveTags;

    writeFaceTagsMap(*metadata, saveFaces);

    writeToBaloo(filePath);

    if (writeTags(*metadata, saveTags))
    {
        bool success = metadata->applyChanges();
        ItemAttributesWatch::instance()->fileMetadataChanged(QUrl::fromLocalFile(filePath));

        return success;
    }
    else
    {
        return false;
    }
}

bool MetadataHub::writeTags(const DMetadata& metadata, bool saveTags)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Writing tags";

    bool dirty = false;

    if (saveTags)
    {
        // Store tag paths as Iptc keywords tags.
        // DatabaseMode == ManagedTags is assumed.
        // To fix this constraint (not needed currently), an oldKeywords parameter is needed
        // create list of keywords to be added and to be removed

        QStringList tagsPathList, newKeywords;

        QList<int> keys = d->tags.keys();

        for (int tagId : std::as_const(keys))
        {
            if (!TagsCache::instance()->canBeWrittenToMetadata(tagId))
            {
                continue;
            }

            // WARNING: Do not use write(QFilePath ...) when multiple image info are loaded
            // otherwise disjoint tags will not be used, use writeToMetadata(ItemInfo...)

            if (d->tags.value(tagId) == MetadataAvailable)
            {
                // This works for single and multiple selection.
                // In both situations, tags which had originally been loaded
                // have explicitly been removed with setTag.

                QString tagName = TagsCache::instance()->tagName(tagId);
                QString tagPath = TagsCache::instance()->tagPath(tagId, TagsCache::NoLeadingSlash);

                if (!tagsPathList.contains(tagPath))
                {
                    tagsPathList << tagPath;
                }

                if (!tagName.isEmpty())
                {
                    newKeywords << tagName;
                }
            }
        }

        tagsPathList = cleanupTags(tagsPathList);
        newKeywords  = cleanupTags(newKeywords);

        if (!newKeywords.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "-------------------------- New Keywords" << newKeywords;

            // NOTE: See bug #175321 : we remove all old keyword from IPTC and XMP before to
            // synchronize metadata, else contents is not coherent.

            dirty |= metadata.setItemTagsPath(tagsPathList);
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Delete all keywords";

            // Delete all IPTC and XMP keywords

            dirty |= metadata.setItemTagsPath(QStringList());
        }
    }

    return dirty;
}

bool MetadataHub::writeFaceTagsMap(const DMetadata& metadata, bool saveFaces)
{
    QSize size      = d->imageProp.first;
    int orientation = d->imageProp.second;

    QMultiMap<QString, QVariant> faceTagsMap;

    // Add confirmed face regions from the database.

    for (const FaceTagsIface& face : std::as_const(d->facesList))
    {
        if (face.isConfirmedName())
        {
            QString faceName = FaceTags::faceNameForTag(face.tagId());

            // Rotate face region back to the unaligned image.

            QRect  tempRect  = face.region().toRect();
            TagRegion::reverseToOrientation(tempRect, orientation, size);
            QRectF faceRect  = TagRegion::absoluteToRelative(tempRect, size);
            faceTagsMap.insert(faceName, faceRect);
        }
    }

    // Add person tags to which no region is
    // assigned to Microsoft Photo Region schema.

    const auto keys = d->tags.keys();

    for (int tagId : keys)
    {
        if ((d->tags.value(tagId) == MetadataAvailable) && FaceTags::isPerson(tagId))
        {
            QString faceName = FaceTags::faceNameForTag(tagId);

            if (!faceName.isEmpty() && !faceTagsMap.contains(faceName))
            {
                faceTagsMap.insert(faceName, QRectF());
            }
        }
    }

    return metadata.setItemFacesMap(faceTagsMap, saveFaces, size);
}

QStringList MetadataHub::cleanupTags(const QStringList& toClean)
{
    QSet<QString> deduplicator;

    for (int index = 0 ; index < toClean.size() ; ++index)
    {
        QString keyword = toClean.at(index);

        if (!keyword.isEmpty())
        {
            // digiKam_root_tag is present in some photos tagged with older
            // version of digiKam, must be removed

            if (keyword.contains(QRegularExpression(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)"))))
            {
                keyword = keyword.replace(QRegularExpression(QLatin1String("(_Digikam_root_tag_/|/_Digikam_root_tag_|_Digikam_root_tag_)")),
                                          QLatin1String(""));
            }

            deduplicator.insert(keyword);
        }
    }

    return deduplicator.values();
}

bool MetadataHub::willWriteMetadata(WriteComponent writeMode, const MetaEngineSettingsContainer& settings) const
{
    // This is the same logic as in write(DMetadata) but without actually writing.
    // Adapt if the method above changes

    bool saveTitle      = (settings.saveComments   && (d->titlesStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_TITLE));
    bool saveComment    = (settings.saveComments   && (d->commentsStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_COMMENTS));
    bool saveDateTime   = (settings.saveDateTime   && (d->dateTimeStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_DATETIME));
    bool savePickLabel  = (settings.savePickLabel  && (d->pickLabelStatus  == MetadataAvailable) && writeMode.testFlag(WRITE_PICKLABEL));
    bool saveColorLabel = (settings.saveColorLabel && (d->colorLabelStatus == MetadataAvailable) && writeMode.testFlag(WRITE_COLORLABEL));
    bool saveRating     = (settings.saveRating     && (d->ratingStatus     == MetadataAvailable) && writeMode.testFlag(WRITE_RATING));
    bool saveTemplate   = (settings.saveTemplate   && (d->templateStatus   == MetadataAvailable) && writeMode.testFlag(WRITE_TEMPLATE));
    bool saveTags       = (settings.saveTags       && writeMode.testFlag(WRITE_TAGS));
    bool saveFaces      = (settings.saveFaceTags   && writeMode.testFlag(WRITE_TAGS));
    bool savePosition   = (settings.savePosition   && writeMode.testFlag(WRITE_POSITION));

    return (
               saveTitle       ||
               saveComment     ||
               saveDateTime    ||
               savePickLabel   ||
               saveColorLabel  ||
               saveRating      ||
               saveTemplate    ||
               saveTags        ||
               saveFaces       ||
               savePosition
           );
}

void MetadataHub::writeToBaloo(const QString& filePath, const MetaEngineSettingsContainer& settings)
{

#ifdef HAVE_KFILEMETADATA

    BalooWrap* const baloo = BalooWrap::instance();

    if (!baloo->getSyncToBaloo())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No write to baloo +++++++++++++++++++++++++++++++++++++";
        return;
    }

    BalooInfo bInfo;

    bool saveComment = (settings.saveComments && (d->commentsStatus == MetadataAvailable));
    bool saveRating  = (settings.saveRating   && (d->ratingStatus   == MetadataAvailable));

    QStringList tagPathList;

    for (QMap<int, MetadataHub::Status>::iterator it = d->tags.begin() ; it != d->tags.end() ; ++it)
    {
        if (!TagsCache::instance()->canBeWrittenToMetadata(it.key()))
        {
            continue;
        }

        // it is important that MetadataDisjoint keywords are not touched

        if (it.value() == MetadataAvailable)
        {
            QString tagPath = TagsCache::instance()->tagPath(it.key(), TagsCache::NoLeadingSlash);

            if (!tagPath.isEmpty())
            {
                tagPathList << tagPath;
            }
        }
    }

    if (saveComment)
    {
        bInfo.comment = d->comments.value(QLatin1String("x-default")).caption;
    }

    if (saveRating)
    {
        bInfo.rating = d->rating;
    }

    bInfo.tags = cleanupTags(tagPathList);
    baloo->setSemanticInfo(QUrl::fromLocalFile(filePath), bInfo);

#else

    Q_UNUSED(filePath);
    Q_UNUSED(settings);

#endif

}

void MetadataHub::applyChangeNotifications()
{
}

void MetadataHub::loadFaceTags(const ItemInfo& info)
{
    FaceTagsEditor editor;
    d->facesList = editor.databaseFaces(info.id());
    d->imageProp = qMakePair(info.dimensions(),
                             info.orientation());
}

// NOTE: Unused code
//void MetadataHub::load(const DMetadata& metadata)
//{
//    d->count++;

//    CaptionsMap comments;
//    CaptionsMap titles;
//    QStringList keywords;
//    QDateTime   datetime;
//    int         pickLabel;
//    int         colorLabel;
//    int         rating;

//    titles = metadata.getItemTitles();

//    // Try to get comments from image :
//    // In first, from Xmp comments tag,
//    // In second, from standard JPEG JFIF comments section,
//    // In third, from Exif comments tag,
//    // In four, from Iptc comments tag.
//    comments = metadata.getItemComments();

//    // Try to get date and time from image :
//    // In first, from Exif date & time tags,
//    // In second, from Xmp date & time tags, or
//    // In third, from Iptc date & time tags.
//    // else use file system time stamp.
//    datetime = metadata.getItemDateTime();

//    if ( !datetime.isValid() )
//    {
//        QFileInfo info( metadata.getFilePath() );
//        datetime = info.lastModified();
//    }

//    // Try to get image pick label from Xmp tag
//    pickLabel = metadata.getItemPickLabel();

//    // Try to get image color label from Xmp tag
//    colorLabel = metadata.getItemColorLabel();

//    // Try to get image rating from Xmp tag, or Iptc Urgency tag
//    rating = metadata.getItemRating();

//    Template tref = metadata.getMetadataTemplate();
//    Template t    = TemplateManager::defaultManager()->findByContents(tref);

//    qCDebug(DIGIKAM_GENERAL_LOG) << "Found Metadata Template: " << t.templateTitle();

//    load(datetime, titles, comments, colorLabel, pickLabel, rating, t.isNull() ? tref : t);

//    // Try to get image tags from Xmp using digiKam namespace tags.

//    QStringList tagPaths;

//    if (metadata.getItemTagsPath(tagPaths))
//    {
//        QList<int> tagIds = TagsCache::instance()->tagsForPaths(tagPaths);
//        loadTags(tagIds);
//    }
//}

//bool MetadataHub::load(const QString& filePath, const MetaEngineSettingsContainer& settings)
//{

//    QScopedPointer<DMetadata> metadata(new DMetadata);
//    metadata->setSettings(settings);
//    bool success = metadata->load(filePath);
//    load(*metadata); // increments count
//    return success;
//}

/*
// private code to merge tags with d->tagList
void MetadataHub::loadTags(const QStringList& loadedTagPaths)
{
    if (d->count == 1)
    {
        d->tagList = loadedTagPaths;
    }
    else
    {
        // a simple intersection

        QStringList toBeAdded;

        for (QStringList::iterator it = d->tagList.begin(); it != d->tagList.end(); ++it)
        {
            if (loadedTagPaths.indexOf(*it) == -1)
            {
                // it's not in the loadedTagPaths list. Remove it from intersection list.
                it = d->tagList.erase(it);
            }

            // else, it is in both lists, so no need to change d->tagList, it's already added.
        }
    }
}
*/

} // namespace Digikam

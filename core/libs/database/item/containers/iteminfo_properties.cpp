/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling access to one item and associated data - Properties
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iteminfo_p.h"

namespace Digikam
{

bool ItemInfo::isNull() const
{
    return !m_data;
}

qlonglong ItemInfo::id() const
{
    return (m_data ? m_data->id : -1);
}

int ItemInfo::albumId() const
{
    return (m_data ? m_data->albumId : -1);
}

int ItemInfo::albumRootId() const
{
    return (m_data ? m_data->albumRootId : -1);
}

QString ItemInfo::name() const
{
    if (!m_data)
    {
        return QString();
    }

    ItemInfoReadLocker lock;

    return m_data->name;
}

qlonglong ItemInfo::fileSize() const
{
    if (!m_data)
    {
        return 0;
    }

    RETURN_IF_CACHED(fileSize)

    QVariantList values = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::FileSize);

    STORE_IN_CACHE_AND_RETURN(fileSize, values.first().toLongLong())
}

QString ItemInfo::title() const
{
    if (!m_data)
    {
        return QString();
    }

    RETURN_IF_CACHED(defaultTitle)

    QString title;
    {
        CoreDbAccess access;
        ItemComments comments(access, m_data->id);
        title = comments.defaultComment(DatabaseComment::Title);
    }

    ItemInfoWriteLocker lock;
    m_data.data()->defaultTitle       = title;
    m_data.data()->defaultTitleCached = true;

    return m_data->defaultTitle;
}

QString ItemInfo::comment() const
{
    if (!m_data)
    {
        return QString();
    }

    RETURN_IF_CACHED(defaultComment)

    QString comment;
    {
        CoreDbAccess access;
        ItemComments comments(access, m_data->id);
        comment = comments.defaultComment();
    }

    ItemInfoWriteLocker lock;
    m_data.data()->defaultComment       = comment;
    m_data.data()->defaultCommentCached = true;

    return m_data->defaultComment;
}

double ItemInfo::aspectRatio() const
{
    if (!m_data)
    {
        return 0;
    }

    RETURN_ASPECTRATIO_IF_IMAGESIZE_CACHED()

    return (double)m_data->imageSize.width() / m_data->imageSize.height();
}

qlonglong ItemInfo::manualOrder() const
{
    if (!m_data)
    {
        return 0;
    }

    RETURN_IF_CACHED(manualOrder)

    QVariantList values = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::ManualOrder);

    STORE_IN_CACHE_AND_RETURN(manualOrder, values.first().toLongLong())
}

QString ItemInfo::format() const
{
    if (!m_data)
    {
        return QString();
    }

    RETURN_IF_CACHED(format)

    QVariantList values = CoreDbAccess().db()->getItemInformation(m_data->id, DatabaseFields::Format);

    STORE_IN_CACHE_AND_RETURN(format, values.first().toString())
}

DatabaseItem::Category ItemInfo::category() const
{
    if (!m_data)
    {
        return DatabaseItem::UndefinedCategory;
    }

    RETURN_IF_CACHED(category)

    QVariantList values = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::Category);

    STORE_IN_CACHE_AND_RETURN(category, (DatabaseItem::Category)values.first().toInt())
}

QDateTime ItemInfo::dateTime() const
{
    if (!m_data)
    {
        return QDateTime();
    }

    RETURN_IF_CACHED(creationDate)

    QVariantList values = CoreDbAccess().db()->getItemInformation(m_data->id, DatabaseFields::CreationDate);

    STORE_IN_CACHE_AND_RETURN(creationDate, values.first().toDateTime())
}

QDateTime ItemInfo::modDateTime() const
{
    if (!m_data)
    {
        return QDateTime();
    }

    RETURN_IF_CACHED(modificationDate)

    QVariantList values = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::ModificationDate);

    STORE_IN_CACHE_AND_RETURN(modificationDate, values.first().toDateTime())
}

QSize ItemInfo::dimensions() const
{
    if (!m_data)
    {
        return QSize();
    }

    RETURN_IF_CACHED(imageSize)

    QVariantList values = CoreDbAccess().db()->getItemInformation(m_data->id, DatabaseFields::Width | DatabaseFields::Height);

    ItemInfoWriteLocker lock;
    m_data.data()->imageSizeCached = true;

    if (values.size() == 2)
    {
        m_data.data()->imageSize = QSize(values.at(0).toInt(), values.at(1).toInt());
    }

    return m_data->imageSize;
}

int ItemInfo::faceCount() const
{
    if (!m_data)
    {
        return 0;
    }

    RETURN_IF_CACHED(faceCount);

    FaceTagsEditor fte;
    int count = fte.databaseFaces(m_data->id).count();

    ItemInfoWriteLocker lock;
    m_data.data()->faceCountCached = true;
    m_data.data()->faceCount       = count;

    return m_data->faceCount;
}

int ItemInfo::unconfirmedFaceCount() const
{
    if (!m_data)
    {
        return 0;
    }

    RETURN_IF_CACHED(unconfirmedFaceCount);

    FaceTagsEditor fte;
    int count = fte.unconfirmedNameFaceTagsIfaces(m_data->id).count();

    ItemInfoWriteLocker lock;
    m_data.data()->unconfirmedFaceCountCached = true;
    m_data.data()->unconfirmedFaceCount       = count;

    return m_data->unconfirmedFaceCount;
}

QMap<QString, QString> ItemInfo::getSuggestedNames() const
{
    if (!m_data)
    {
        return QMap<QString, QString>();
    }

    RETURN_IF_CACHED(faceSuggestions);

    FaceTagsEditor fte;
    QMap<QString, QString> faceSuggestions = fte.getSuggestedNames(m_data->id);

    ItemInfoWriteLocker lock;
    m_data.data()->faceSuggestionsCached = true;
    m_data.data()->faceSuggestions       = faceSuggestions;

    return m_data->faceSuggestions;
}

int ItemInfo::orientation() const
{
    if (!m_data)
    {
        return 0; // ORIENTATION_UNSPECIFIED
    }

    RETURN_IF_CACHED(orientation)

    QVariantList values = CoreDbAccess().db()->getItemInformation(m_data->id, DatabaseFields::Orientation);

    STORE_IN_CACHE_AND_RETURN(orientation, values.first().toInt());
}

QUrl ItemInfo::fileUrl() const
{
    return QUrl::fromLocalFile(filePath());
}

QString ItemInfo::filePath() const
{
    if (!m_data)
    {
        return QString();
    }

    QString albumRoot = CollectionManager::instance()->albumRootPath(m_data->albumRootId);

    if (albumRoot.isNull())
    {
        return QString();
    }

    QString album     = ItemInfoStatic::cache()->albumRelativePath(m_data->albumId);
    ItemInfoReadLocker lock;

    if (album == QLatin1String("/"))
    {
        return (albumRoot + album + m_data->name);
    }
    else
    {
        return (albumRoot + album + QLatin1Char('/') + m_data->name);
    }
}

QString ItemInfo::relativePath() const
{
    if (!m_data)
    {
        return QString();
    }

    return (ItemInfoStatic::cache()->albumRelativePath(m_data->albumId));
}

bool ItemInfo::isVisible() const
{
    if (!m_data)
    {
        return false;
    }

    QVariantList value = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::Status);

    if (!value.isEmpty())
    {
        return (value.first().toInt() == DatabaseItem::Visible);
    }

    return false;
}

bool ItemInfo::isRemoved() const
{
    if (!m_data)
    {
        return true;
    }

    QVariantList value = CoreDbAccess().db()->getImagesFields(m_data->id, DatabaseFields::Status);

    if (!value.isEmpty())
    {
        return (value.first().toInt() == DatabaseItem::Trashed) || (value.first().toInt() == DatabaseItem::Obsolete);
    }

    return false;
}

void ItemInfo::setVisible(bool _isVisible)
{
    if (!m_data)
    {
        return;
    }

    if (m_data->albumId == 0)
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Attempt to make a Removed item visible with ItemInfo::setVisible";
        return;
    }

    CoreDbAccess().db()->setItemStatus(m_data->id, _isVisible ? DatabaseItem::Visible : DatabaseItem::Hidden);
}

void ItemInfo::setManualOrder(qlonglong value)
{
    if (!m_data)
    {
        return;
    }

    {
        ItemInfoWriteLocker lock;
        m_data->manualOrder       = value;
        m_data->manualOrderCached = true;
    }

    CoreDbAccess().db()->setItemManualOrder(m_data->id, value);
}

void ItemInfo::setOrientation(int value)
{
    if (!m_data)
    {
        return;
    }

    {
        ItemInfoWriteLocker lock;
        m_data->orientation       = value;
        m_data->orientationCached = true;
    }

    CoreDbAccess().db()->changeItemInformation(m_data->id, QVariantList() << value, DatabaseFields::Orientation);
}

void ItemInfo::setName(const QString& newName)
{
    if (!m_data || newName.isEmpty())
    {
        return;
    }

    {
        ItemInfoWriteLocker lock;
        m_data->name = newName;
        ItemInfoStatic::cache()->cacheByName(m_data);
    }

    CoreDbAccess().db()->renameItem(m_data->id, newName);
}

void ItemInfo::setDateTime(const QDateTime& _dateTime)
{
    if (!m_data || !_dateTime.isValid())
    {
        return;
    }

    {
        ItemInfoWriteLocker lock;
        m_data->creationDate       = _dateTime;
        m_data->creationDateCached = true;
    }

    CoreDbAccess().db()->changeItemInformation(m_data->id, QVariantList() << _dateTime, DatabaseFields::CreationDate);
}

void ItemInfo::setModDateTime(const QDateTime& _dateTime)
{
    if (!m_data || !_dateTime.isValid())
    {
        return;
    }

    {
        ItemInfoWriteLocker lock;
        m_data->modificationDate       = _dateTime;
        m_data->modificationDateCached = true;
    }

    CoreDbAccess().db()->setItemModificationDate(m_data->id, _dateTime);
}

ItemInfo::DatabaseFieldsHashRaw ItemInfo::getDatabaseFieldsRaw(const DatabaseFields::Set& requestedSet) const
{
    if (!m_data || (!m_data->hasVideoMetadata && !m_data->hasImageMetadata))
    {
        return DatabaseFieldsHashRaw();
    }

    DatabaseFields::VideoMetadataMinSizeType cachedVideoMetadata;
    DatabaseFields::ImageMetadataMinSizeType cachedImageMetadata;
    ItemInfo::DatabaseFieldsHashRaw cachedHash;

    // consolidate to one ReadLocker. In particular, the shallow copy of the QHash must be done under protection

    {
        ItemInfoReadLocker lock;
        cachedVideoMetadata = m_data->videoMetadataCached;
        cachedImageMetadata = m_data->imageMetadataCached;
        cachedHash = m_data->databaseFieldsHashRaw;
    }

    if (requestedSet.hasFieldsFromVideoMetadata() && m_data->hasVideoMetadata)
    {
        const DatabaseFields::VideoMetadata requestedVideoMetadata = requestedSet.getVideoMetadata();
        const DatabaseFields::VideoMetadata missingVideoMetadata = requestedVideoMetadata & ~cachedVideoMetadata;

        if (missingVideoMetadata)
        {
            const QVariantList fieldValues = CoreDbAccess().db()->getVideoMetadata(m_data->id, missingVideoMetadata);

            ItemInfoWriteLocker lock;

            if (fieldValues.isEmpty())
            {
                m_data.data()->hasVideoMetadata    = false;
                m_data.data()->databaseFieldsHashRaw.removeAllFields(DatabaseFields::VideoMetadataAll);
                m_data.data()->videoMetadataCached = DatabaseFields::VideoMetadataNone;
            }
            else
            {
                int fieldsIndex = 0;

                for (DatabaseFields::VideoMetadataIteratorSetOnly it(missingVideoMetadata) ; !it.atEnd() ; ++it)
                {
                    const QVariant fieldValue = fieldValues.at(fieldsIndex);
                    ++fieldsIndex;

                    m_data.data()->databaseFieldsHashRaw.insertField(*it, fieldValue);
                }

                m_data.data()->videoMetadataCached |= missingVideoMetadata;
            }

            // update for return value

            cachedHash = m_data->databaseFieldsHashRaw;
        }
    }

    if (requestedSet.hasFieldsFromImageMetadata() && m_data->hasImageMetadata)
    {
        const DatabaseFields::ImageMetadata requestedImageMetadata = requestedSet.getImageMetadata();
        const DatabaseFields::ImageMetadata missingImageMetadata   = requestedImageMetadata & ~cachedImageMetadata;

        if (missingImageMetadata)
        {
            const QVariantList fieldValues = CoreDbAccess().db()->getImageMetadata(m_data->id, missingImageMetadata);

            ItemInfoWriteLocker lock;

            if (fieldValues.isEmpty())
            {
                m_data.data()->hasImageMetadata    = false;
                m_data.data()->databaseFieldsHashRaw.removeAllFields(DatabaseFields::ImageMetadataAll);
                m_data.data()->imageMetadataCached = DatabaseFields::ImageMetadataNone;
            }
            else
            {
                int fieldsIndex = 0;

                for (DatabaseFields::ImageMetadataIteratorSetOnly it(missingImageMetadata) ; !it.atEnd() ; ++it)
                {
                    const QVariant fieldValue = fieldValues.at(fieldsIndex);
                    ++fieldsIndex;

                    m_data.data()->databaseFieldsHashRaw.insertField(*it, fieldValue);
                }

                m_data.data()->imageMetadataCached |= missingImageMetadata;
            }

            cachedHash = m_data->databaseFieldsHashRaw;
        }
    }

    // We always return all fields, the caller can just retrieve the ones he needs.

    return cachedHash;
}

QVariant ItemInfo::getDatabaseFieldRaw(const DatabaseFields::Set& requestedField) const
{
    DatabaseFieldsHashRaw rawHash = getDatabaseFieldsRaw(requestedField);

    if (requestedField.hasFieldsFromImageMetadata())
    {
        const DatabaseFields::ImageMetadata requestedFieldFlag = requestedField;
        const QVariant value                                   = rawHash.value(requestedFieldFlag);

        return value;
    }

    if (requestedField.hasFieldsFromVideoMetadata())
    {
        const DatabaseFields::VideoMetadata requestedFieldFlag = requestedField;
        const QVariant value                                   = rawHash.value(requestedFieldFlag);

        return value;
    }

    return QVariant();
}

} // namespace Digikam

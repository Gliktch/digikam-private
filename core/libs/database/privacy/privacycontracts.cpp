/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycontracts.h"

// Qt includes

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QUuid>

namespace Digikam
{

namespace
{

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() && (uuid == parsed.toString(QUuid::WithoutBraces)));
}

bool isValidPath(const QString& path)
{
    return (!path.isEmpty() && QDir::isAbsolutePath(path) && !path.contains(QChar::Null));
}

bool isValidActionKind(PrivacyActionKind kind)
{
    return ((kind >= PrivacyActionKind::Preview) && (kind <= PrivacyActionKind::Analysis));
}

bool isValidRequestedSource(PrivacyRequestedSource source)
{
    return ((source >= PrivacyRequestedSource::NoPixels) &&
            (source <= PrivacyRequestedSource::PublicOriginal));
}

bool isValidMutationPolicy(PrivacyMutationPolicy policy)
{
    return ((policy >= PrivacyMutationPolicy::ReadOnly) &&
            (policy <= PrivacyMutationPolicy::DestructiveMutation));
}

} // namespace

bool PrivacyActionItem::isValid() const
{
    return ((imageId > 0) && isValidPath(publicPath));
}

bool PrivacyActionRequest::isValid() const
{
    if ((contractVersion != 1) || !isValidActionKind(actionKind)               ||
        consumerIdentity.trimmed().isEmpty() || items.isEmpty()                ||
        !isValidRequestedSource(requestedSource) || !isValidMutationPolicy(mutationPolicy))
    {
        return false;
    }

    QSet<qlonglong> seenIds;

    for (const PrivacyActionItem& item : items)
    {
        if (!item.isValid() || seenIds.contains(item.imageId))
        {
            return false;
        }

        seenIds.insert(item.imageId);
    }

    return true;
}

bool PrivacyLeaseToken::isValid() const
{
    return (isCanonicalUuid(uuid) && isCanonicalUuid(itemUuid) &&
            (itemGeneration >= 0) && (categoryEpoch > 0) &&
            (publicRootEpoch > 0));
}

bool PreparedPrivacyItem::isValid() const
{
    if (!logicalItem.isValid())
    {
        return false;
    }

    if (disposition == PrivacyPreparedDisposition::Allowed)
    {
        return (isValidPath(physicalPath) && lease.isValid());
    }

    if (disposition == PrivacyPreparedDisposition::UnprotectedPassThrough)
    {
        return ((physicalPath == logicalItem.publicPath) && !lease.isValid());
    }

    return (((disposition == PrivacyPreparedDisposition::Excluded) ||
             (disposition == PrivacyPreparedDisposition::Canceled)) &&
            physicalPath.isEmpty() && !lease.isValid());
}

bool PreparedPrivacySelection::isValid() const
{
    if ((contractVersion != 1) || items.isEmpty() ||
        ((disposition != PrivacyPreparedDisposition::Allowed)  &&
         (disposition != PrivacyPreparedDisposition::Excluded) &&
         (disposition != PrivacyPreparedDisposition::Canceled)))
    {
        return false;
    }

    bool hasIncludedItem = false;
    QSet<qlonglong> seenIds;

    for (const PreparedPrivacyItem& item : items)
    {
        if (!item.isValid() || seenIds.contains(item.logicalItem.imageId))
        {
            return false;
        }

        seenIds.insert(item.logicalItem.imageId);

        if (((disposition == PrivacyPreparedDisposition::Canceled) &&
             (item.disposition != PrivacyPreparedDisposition::Canceled)) ||
            ((disposition == PrivacyPreparedDisposition::Excluded) &&
             (item.disposition != PrivacyPreparedDisposition::Excluded)))
        {
            return false;
        }

        if (disposition == PrivacyPreparedDisposition::Allowed)
        {
            if (item.disposition == PrivacyPreparedDisposition::Canceled)
            {
                return false;
            }

            hasIncludedItem = (hasIncludedItem ||
                               (item.disposition == PrivacyPreparedDisposition::Allowed) ||
                               (item.disposition == PrivacyPreparedDisposition::UnprotectedPassThrough));
        }
    }

    return ((disposition != PrivacyPreparedDisposition::Allowed) || hasIncludedItem);
}

QByteArray PrivacyRootIdentityCodec::encodeAlbumRootV1(int albumRootId,
                                                       const QString& collectionIdentifier)
{
    if ((albumRootId <= 0) || collectionIdentifier.isEmpty() ||
        collectionIdentifier.contains(QChar::Null))
    {
        return QByteArray();
    }

    QJsonObject object;
    object.insert(QLatin1String("albumRootId"), albumRootId);
    object.insert(QLatin1String("collectionIdentifier"), collectionIdentifier);
    object.insert(QLatin1String("kind"), QLatin1String("album-root-v1"));

    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool PrivacyRootIdentityCodec::matchesAlbumRootV1(const QByteArray& identityData,
                                                  int albumRootId,
                                                  const QString& collectionIdentifier)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(identityData, &error);

    if ((error.error != QJsonParseError::NoError) || !document.isObject())
    {
        return false;
    }

    const QJsonObject object = document.object();

    return ((object.size() == 3) &&
            (object.value(QLatin1String("kind")).toString() == QLatin1String("album-root-v1")) &&
            (object.value(QLatin1String("albumRootId")).toInt(-1) == albumRootId) &&
            (object.value(QLatin1String("collectionIdentifier")).toString() == collectionIdentifier));
}

QString PrivacyRootIdentityCodec::managedRootMarkerRelativePathV1()
{
    return QLatin1String(".digikam-private/root-marker-v1.json");
}

QByteArray PrivacyRootIdentityCodec::encodeManagedRootV1(
    const QString& markerUuid,
    const QString& filesystemIdentity)
{
    if (!isCanonicalUuid(markerUuid) || filesystemIdentity.contains(QChar::Null))
    {
        return QByteArray();
    }

    QJsonObject object;
    object.insert(QLatin1String("kind"), QLatin1String("managed-store-root-v1"));
    object.insert(QLatin1String("markerUuid"), markerUuid);

    if (!filesystemIdentity.isEmpty())
    {
        object.insert(QLatin1String("filesystemIdentity"), filesystemIdentity);
    }

    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool PrivacyRootIdentityCodec::matchesManagedRootV1(
    const QByteArray& identityData,
    const QString& markerUuid,
    const QString& filesystemIdentity)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(identityData, &error);

    if ((error.error != QJsonParseError::NoError) || !document.isObject() ||
        !isCanonicalUuid(markerUuid))
    {
        return false;
    }

    const QJsonObject object = document.object();
    const QJsonValue expectedFilesystem = object.value(QLatin1String("filesystemIdentity"));

    if ((object.size() != (expectedFilesystem.isUndefined() ? 2 : 3)) ||
        (!expectedFilesystem.isUndefined() && !expectedFilesystem.isString()) ||
        (object.value(QLatin1String("kind")).toString() !=
         QLatin1String("managed-store-root-v1")) ||
        (object.value(QLatin1String("markerUuid")).toString() != markerUuid) ||
        (!expectedFilesystem.isUndefined() &&
         (expectedFilesystem.toString() != filesystemIdentity)))
    {
        return false;
    }

    return true;
}

QByteArray PrivacyRootIdentityCodec::encodeManagedRootMarkerV1(
    const QString& rootUuid,
    const QString& markerUuid)
{
    if (!isCanonicalUuid(rootUuid) || !isCanonicalUuid(markerUuid))
    {
        return QByteArray();
    }

    QJsonObject object;
    object.insert(QLatin1String("kind"), QLatin1String("digikam-private-root-marker-v1"));
    object.insert(QLatin1String("markerUuid"), markerUuid);
    object.insert(QLatin1String("rootUuid"), rootUuid);

    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool PrivacyRootIdentityCodec::matchesManagedRootMarkerV1(
    const QByteArray& markerData,
    const QString& rootUuid,
    const QString& markerUuid)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(markerData, &error);

    if ((error.error != QJsonParseError::NoError) || !document.isObject())
    {
        return false;
    }

    const QJsonObject object = document.object();

    return ((object.size() == 3) &&
            (object.value(QLatin1String("kind")).toString() ==
             QLatin1String("digikam-private-root-marker-v1")) &&
            (object.value(QLatin1String("rootUuid")).toString() == rootUuid) &&
            (object.value(QLatin1String("markerUuid")).toString() == markerUuid));
}

} // namespace Digikam

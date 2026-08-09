/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycasualoriginalreader.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QDir>
#include <QFileInfo>

namespace Digikam
{

namespace
{

template <typename Value, typename Predicate>
const Value* uniqueValue(const QList<Value>& values, Predicate predicate)
{
    const Value* result = nullptr;

    for (const Value& value : values)
    {
        if (!predicate(value))
        {
            continue;
        }

        if (result)
        {
            return nullptr;
        }

        result = &value;
    }

    return result;
}

QString absoluteObjectPath(const PrivacyStorageRoot& root,
                           const QString& relativePath)
{
    if (!root.isValid() ||
        (root.kind != PrivacyStorageRootKind::AlbumRoot) ||
        relativePath.isEmpty() || QDir::isAbsolutePath(relativePath) ||
        (QDir::cleanPath(relativePath) != relativePath) ||
        relativePath.startsWith(QLatin1String("../")))
    {
        return {};
    }

    return QDir(root.configuredPath).absoluteFilePath(relativePath);
}

QByteArray sha256Bytes(const QString& algorithm, const QString& encoded)
{
    if ((algorithm != QLatin1String("sha256")) || (encoded.size() != 64))
    {
        return {};
    }

    const QByteArray bytes = QByteArray::fromHex(encoded.toLatin1());
    return (bytes.size() == 32) ? bytes : QByteArray();
}

} // namespace

bool PrivacyCasualOriginalSource::isValid() const
{
    return ((imageId > 0) && (itemGeneration >= 0) &&
            QDir::isAbsolutePath(logicalFilePath) &&
            !categoryUuid.isEmpty() && !itemUuid.isEmpty() &&
            !originalName.isEmpty() && (originalSize >= 0) &&
            !originalHash.isEmpty() &&
            !restore.archivePath.isEmpty() &&
            (restore.itemUuid == itemUuid) &&
            (restore.categoryUuid == categoryUuid) &&
            (restore.originalName == originalName) &&
            (restore.expectedMemberSize == originalSize) &&
            !restore.expectedArchiveSha256.isEmpty() &&
            !restore.expectedMemberSha256.isEmpty());
}

bool PrivacyCasualOriginalReader::prepare(
    const PrivacyRepositorySnapshot& snapshot,
    qlonglong imageId,
    const QString& logicalFilePath,
    PrivacyCasualOriginalSource* const source) const
{
    if (!source || (imageId <= 0) || !QDir::isAbsolutePath(logicalFilePath))
    {
        return false;
    }

    *source = PrivacyCasualOriginalSource();
    const QString cleanLogicalPath = QDir::cleanPath(logicalFilePath);
    const PrivacyItem* const item = uniqueValue(
        snapshot.items,
        [imageId](const PrivacyItem& candidate)
        {
            return (candidate.imageId == imageId);
        });

    if (!item || !item->isValid())
    {
        return false;
    }

    const PrivacyCategory* const category = uniqueValue(
        snapshot.categories,
        [item](const PrivacyCategory& candidate)
        {
            return (candidate.uuid == item->categoryUuid);
        });
    const PrivacyContainer* const container = uniqueValue(
        snapshot.containers,
        [item](const PrivacyContainer& candidate)
        {
            return (candidate.itemUuid == item->uuid);
        });
    const PrivacyAsset* const primary = uniqueValue(
        snapshot.assets,
        [item](const PrivacyAsset& candidate)
        {
            return ((candidate.itemUuid == item->uuid) &&
                    (candidate.role == PrivacyAsset::PrimaryMediaRole) &&
                    (candidate.ordinal == 0));
        });

    if (!category || !category->isValid() ||
        (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        !container || !container->isValid() ||
        (container->kind != PrivacyContainerKind::CasualArchive) ||
        (container->state != PrivacyContainerState::Verified) ||
        !primary || !primary->isValid() ||
        (primary->containerUuid != container->uuid) ||
        (primary->publicRootUuid != container->rootUuid) ||
        (primary->proxyGeneration != item->generation) ||
        (primary->originalHash != item->originalHash) ||
        (primary->originalSize != item->originalSize))
    {
        return false;
    }

    if (std::any_of(snapshot.transactions.cbegin(), snapshot.transactions.cend(),
                    [item](const PrivacyTransaction& transaction)
                    {
                        return (transaction.isActive() &&
                                ((transaction.itemUuid == item->uuid) ||
                                 (transaction.categoryUuid == item->categoryUuid)));
                    }))
    {
        return false;
    }

    const PrivacyStorageRoot* const root = uniqueValue(
        snapshot.storageRoots,
        [container](const PrivacyStorageRoot& candidate)
        {
            return (candidate.uuid == container->rootUuid);
        });

    if (!root || !root->isValid() ||
        (QDir::cleanPath(QDir(root->configuredPath)
                             .absoluteFilePath(primary->publicRelativePath)) !=
         cleanLogicalPath))
    {
        return false;
    }

    const QByteArray archiveHash = sha256Bytes(
        container->protectedHashAlgorithm, container->protectedHash);
    const QByteArray memberHash = sha256Bytes(
        primary->hashAlgorithm, primary->originalHash);
    const QString archivePath = absoluteObjectPath(
        *root, container->objectRelativePath);

    if (archiveHash.isEmpty() || memberHash.isEmpty() || archivePath.isEmpty())
    {
        return false;
    }

    PrivacyCasualOriginalSource prepared;
    prepared.imageId          = imageId;
    prepared.itemGeneration   = item->generation;
    prepared.logicalFilePath  = cleanLogicalPath;
    prepared.categoryUuid     = category->uuid;
    prepared.itemUuid         = item->uuid;
    prepared.originalName     = primary->originalName;
    prepared.originalHash     = primary->originalHash;
    prepared.originalSize     = primary->originalSize;
    prepared.restore.archivePath = archivePath;
    prepared.restore.categoryUuid = category->uuid;
    prepared.restore.containerUuid = container->uuid;
    prepared.restore.itemUuid = item->uuid;
    prepared.restore.protectedRelativePath = primary->protectedRelativePath;
    prepared.restore.originalName = primary->originalName;
    prepared.restore.role = primary->role;
    prepared.restore.ordinal = primary->ordinal;
    prepared.restore.expectedArchiveSize = container->protectedSize;
    prepared.restore.expectedArchiveSha256 = archiveHash;
    prepared.restore.expectedMemberSize = primary->originalSize;
    prepared.restore.expectedMemberSha256 = memberHash;

    if (!prepared.isValid())
    {
        return false;
    }

    *source = prepared;
    return true;
}

bool PrivacyCasualOriginalReader::restore(
    const PrivacyCasualOriginalSource& source,
    const PrivacyPassword& password,
    QIODevice* const destination,
    PrivacyCasualArchiveError* const error) const
{
    if (!source.isValid() || !password.isValid() || !destination)
    {
        if (error)
        {
            *error = PrivacyCasualArchiveError::InvalidRequest;
        }

        return false;
    }

    return PrivacyCasualArchiveEngine().restoreMember(
        source.restore, password, destination, {}, error);
}

} // namespace Digikam

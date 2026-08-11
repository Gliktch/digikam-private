/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacystrongoriginalreader.h"

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace Digikam
{

namespace
{

template <typename T, typename Matches>
const T* uniqueValue(const QList<T>& values,
                     const Matches& matches)
{
    const T* found = nullptr;

    for (const T& value : values)
    {
        if (matches(value))
        {
            if (found)
            {
                return nullptr;
            }

            found = &value;
        }
    }

    return found;
}

bool hasActiveTransaction(const PrivacyRepositorySnapshot& snapshot,
                          const QString& itemUuid,
                          const QString& categoryUuid)
{
    return std::any_of(
        snapshot.transactions.cbegin(), snapshot.transactions.cend(),
        [itemUuid, categoryUuid](const PrivacyTransaction& transaction)
        {
            return (transaction.isActive() &&
                    ((transaction.itemUuid == itemUuid) ||
                     (transaction.categoryUuid == categoryUuid)));
        });
}

} // namespace

bool PrivacyStrongOriginalSource::isValid() const
{
    return ((imageId > 0) && (itemGeneration >= 0) &&
            QDir::isAbsolutePath(logicalFilePath) &&
            !categoryUuid.isEmpty() && !itemUuid.isEmpty() &&
            !originalName.isEmpty() && !originalHash.isEmpty() &&
            (originalSize >= 0) && !storeUuid.isEmpty() &&
            !containerObjectRelativePath.isEmpty() &&
            protectedRelativePath.startsWith(QLatin1String("originals/")) &&
            !vaultPlaintextRoot.isEmpty());
}

bool PrivacyStrongOriginalReader::prepare(
    const PrivacyRepositorySnapshot& snapshot,
    qlonglong imageId,
    const QString& logicalFilePath,
    PrivacyStrongOriginalSource* const source) const
{
    return prepareAsset(snapshot, imageId, logicalFilePath,
                        PrivacyAsset::PrimaryMediaRole, 0, source);
}

bool PrivacyStrongOriginalReader::prepareAsset(
    const PrivacyRepositorySnapshot& snapshot,
    qlonglong imageId,
    const QString& logicalFilePath,
    int role, int ordinal,
    PrivacyStrongOriginalSource* const source) const
{
    if (!source || (imageId <= 0) || !QDir::isAbsolutePath(logicalFilePath) ||
        (role <= 0) || (ordinal < 0))
    {
        return false;
    }

    *source = PrivacyStrongOriginalSource();
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
    const PrivacyAsset* const selected = uniqueValue(
        snapshot.assets,
        [item, role, ordinal](const PrivacyAsset& candidate)
        {
            return ((candidate.itemUuid == item->uuid) &&
                    (candidate.role == role) &&
                    (candidate.ordinal == ordinal));
        });

    if (!category || !category->isValid() ||
        (category->backend != PrivacyBackend::Strong) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        !container || !container->isValid() ||
        (container->kind != PrivacyContainerKind::StrongObject) ||
        (container->state != PrivacyContainerState::Verified) ||
        !container->rootUuid.isEmpty() || container->storeUuid.isEmpty() ||
        !primary || !primary->isValid() || !selected || !selected->isValid() ||
        (primary->containerUuid != container->uuid) ||
        (selected->containerUuid != container->uuid) ||
        (primary->proxyGeneration != item->generation) ||
        (primary->originalHash != item->originalHash) ||
        (primary->originalSize != item->originalSize) ||
        !selected->protectedRelativePath.startsWith(
            QLatin1String("originals/")) ||
        hasActiveTransaction(snapshot, item->uuid, category->uuid))
    {
        return false;
    }

    const PrivacyStorageRoot* const selectedRoot = uniqueValue(
        snapshot.storageRoots,
        [selected](const PrivacyStorageRoot& candidate)
        {
            return (candidate.uuid == selected->publicRootUuid);
        });

    if (!selectedRoot || !selectedRoot->isValid() ||
        (QDir::cleanPath(QDir(selectedRoot->configuredPath)
                             .absoluteFilePath(selected->publicRelativePath)) !=
         cleanLogicalPath))
    {
        return false;
    }

    const PrivacyStore* const store = uniqueValue(
        snapshot.stores,
        [container](const PrivacyStore& candidate)
        {
            return (candidate.uuid == container->storeUuid);
        });

    if (!store || !store->isValid() ||
        (store->categoryUuid != category->uuid) ||
        (store->lifecycleState != PrivacyStoreLifecycleState::Active))
    {
        return false;
    }

    bool originalsBinding = false;

    for (const PrivacyStoreBinding& binding : snapshot.storeBindings)
    {
        if ((binding.categoryUuid == category->uuid) &&
            (binding.role == PrivacyStoreRole::Originals) &&
            (binding.storeUuid == container->storeUuid))
        {
            originalsBinding = true;
            break;
        }
    }

    if (!originalsBinding)
    {
        return false;
    }

    PrivacyStrongOriginalSource prepared;
    prepared.imageId = imageId;
    prepared.itemGeneration = item->generation;
    prepared.logicalFilePath = cleanLogicalPath;
    prepared.categoryUuid = category->uuid;
    prepared.itemUuid = item->uuid;
    prepared.originalName = selected->originalName;
    prepared.originalHash = selected->originalHash;
    prepared.originalSize = selected->originalSize;
    prepared.storeUuid = container->storeUuid;
    prepared.containerObjectRelativePath = container->objectRelativePath;
    prepared.protectedRelativePath = selected->protectedRelativePath;
    prepared.vaultPlaintextRoot.clear();
    *source = prepared;
    return true;
}

bool PrivacyStrongOriginalReader::restore(
    const PrivacyStrongOriginalSource& source,
    QIODevice* const destination,
    QString* const error,
    const std::function<bool()>& isCancelled) const
{
    if (!source.isValid() || !destination)
    {
        if (error)
        {
            *error = QLatin1String("invalid Strong original source or destination");
        }

        return false;
    }

    const QString objectPath = QDir(source.vaultPlaintextRoot).filePath(
        source.protectedRelativePath);
    const QFileInfo objectInfo(objectPath);

    if (!objectInfo.isFile() || objectInfo.isSymLink())
    {
        if (error)
        {
            *error = QString::fromLatin1(
                "Strong original object is missing: %1").arg(objectPath);
        }

        return false;
    }

    QFile object(objectPath);

    if (!object.open(QIODevice::ReadOnly))
    {
        if (error)
        {
            *error = QString::fromLatin1(
                "Strong original object cannot be opened: %1").arg(objectPath);
        }

        return false;
    }

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    QByteArray buffer;
    buffer.resize(1024 * 1024);
    qlonglong total = 0;

    while (!object.atEnd())
    {
        if (isCancelled && isCancelled())
        {
            if (error)
            {
                *error = QLatin1String("Strong original restore was canceled");
            }

            return false;
        }

        const qint64 read = object.read(buffer.data(), buffer.size());

        if (read <= 0)
        {
            if (error)
            {
                *error = QLatin1String("Strong original object read failed");
            }

            return false;
        }

        hasher.addData(buffer.constData(), read);
        total += read;

        if (destination->write(buffer.constData(), read) != read)
        {
            if (error)
            {
                *error = QLatin1String("Strong original destination write failed");
            }

            return false;
        }
    }

    if ((total != source.originalSize) ||
        (hasher.result() != QByteArray::fromHex(source.originalHash.toLatin1())))
    {
        if (error)
        {
            *error = QLatin1String(
                "Strong original object fails exact hash/size verification");
        }

        return false;
    }

    return true;
}

} // namespace Digikam

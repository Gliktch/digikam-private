/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyruntime.h"

// Qt includes

#include <QFileInfo>
#include <QCryptographicHash>
#include <QGlobalStatic>
#include <QReadLocker>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QWriteLocker>

// C++ includes

#include <cerrno>

#ifdef Q_OS_UNIX

// POSIX includes

#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>

#endif

// Local includes

#include "collectionlocation.h"
#include "collectionmanager.h"
#include "privacycategorysessionowner.h"
#include "privacyrepository.h"
#include "privacyrootidentity_p.h"

namespace Digikam
{

namespace
{

constexpr qint64 managedRootMarkerMaximumBytes = 4096;
constexpr qint64 proxyValidationChunkBytes = 1024 * 1024;

#ifdef Q_OS_UNIX

class ScopedFileDescriptor
{
public:

    explicit ScopedFileDescriptor(int descriptor = -1)
        : m_descriptor(descriptor)
    {
    }

    ~ScopedFileDescriptor()
    {
        if (m_descriptor >= 0)
        {
            close(m_descriptor);
        }
    }

    int get() const
    {
        return m_descriptor;
    }

private:

    Q_DISABLE_COPY(ScopedFileDescriptor)

private:

    int m_descriptor = -1;
};

#endif

#ifdef Q_OS_UNIX

bool pathHasSymlinkComponent(const QString& absolutePath);

#endif

bool stablePublicProxySha256(const QString& absolutePath,
                             qlonglong expectedProxySize,
                             qlonglong expectedOriginalSize,
                             QByteArray* const sha256)
{
    if (!sha256 || !QDir::isAbsolutePath(absolutePath) ||
        (QDir::cleanPath(absolutePath) != absolutePath) ||
        absolutePath.contains(QChar::Null) ||
        (expectedProxySize < 0) || (expectedOriginalSize < 0))
    {
        return false;
    }

#ifdef Q_OS_UNIX

    if (pathHasSymlinkComponent(absolutePath))
    {
        return false;
    }

    const QByteArray encodedPath = QFile::encodeName(absolutePath);
    ScopedFileDescriptor descriptor(::open(encodedPath.constData(),
                                            O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    struct stat before = {};

    if ((descriptor.get() < 0) || (::fstat(descriptor.get(), &before) != 0) ||
        !S_ISREG(before.st_mode) || S_ISLNK(before.st_mode) ||
        (before.st_size != expectedProxySize &&
         before.st_size != expectedOriginalSize))
    {
        return false;
    }

    QFile file;

    if (!file.open(descriptor.get(), QIODevice::ReadOnly,
                   QFileDevice::DontCloseHandle))
    {
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!file.atEnd())
    {
        const QByteArray chunk = file.read(proxyValidationChunkBytes);

        if (chunk.isEmpty() && !file.atEnd())
        {
            return false;
        }

        hash.addData(chunk);
    }

    if (file.error() != QFileDevice::NoError)
    {
        return false;
    }

    struct stat after = {};
    struct stat named = {};

    if ((::fstat(descriptor.get(), &after) != 0) ||
        (::lstat(encodedPath.constData(), &named) != 0) ||
        !S_ISREG(named.st_mode) || S_ISLNK(named.st_mode) ||
        (before.st_dev != after.st_dev) ||
        (before.st_ino != after.st_ino) ||
        (before.st_mode != after.st_mode) ||
        (before.st_nlink != after.st_nlink) ||
        (before.st_size != after.st_size) ||
        (before.st_mtim.tv_sec != after.st_mtim.tv_sec) ||
        (before.st_mtim.tv_nsec != after.st_mtim.tv_nsec) ||
        (before.st_ctim.tv_sec != after.st_ctim.tv_sec) ||
        (before.st_ctim.tv_nsec != after.st_ctim.tv_nsec) ||
        (named.st_dev != before.st_dev) || (named.st_ino != before.st_ino))
    {
        return false;
    }

    *sha256 = hash.result();
    return true;

#else

    const QFileInfo before(absolutePath);

    if (!before.isFile() || before.isSymLink() ||
        (before.size() != expectedProxySize &&
         before.size() != expectedOriginalSize))
    {
        return false;
    }

    QFile file(absolutePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!file.atEnd())
    {
        const QByteArray chunk = file.read(proxyValidationChunkBytes);

        if (chunk.isEmpty() && !file.atEnd())
        {
            return false;
        }

        hash.addData(chunk);
    }

    if (file.error() != QFileDevice::NoError)
    {
        return false;
    }

    const QFileInfo after(absolutePath);

    if (!after.isFile() || after.isSymLink() ||
        (before.absoluteFilePath() != after.absoluteFilePath()) ||
        (before.size() != after.size()) ||
        (before.lastModified() != after.lastModified()))
    {
        return false;
    }

    *sha256 = hash.result();
    return true;

#endif
}

#ifdef Q_OS_UNIX

bool pathHasSymlinkComponent(const QString& absolutePath)
{
    const QString cleanPath = QDir::cleanPath(absolutePath);

    if (!QDir::isAbsolutePath(cleanPath))
    {
        return true;
    }

    QString current = QDir::rootPath();

    for (const QString& part : cleanPath.split(QDir::separator(), Qt::SkipEmptyParts))
    {
        current = QDir(current).filePath(part);

        if (QFileInfo(current).isSymLink())
        {
            return true;
        }
    }

    return false;
}

PrivacyRootRuntimeState readManagedRootMarker(const PrivacyStorageRoot& root,
                                              QByteArray* markerData,
                                              QString* filesystemIdentity)
{
    if (!markerData || !filesystemIdentity || root.configuredPath.contains(QChar::Null) ||
        !QDir::isAbsolutePath(root.configuredPath))
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    const QStringList markerParts =
        PrivacyRootIdentityCodec::managedRootMarkerRelativePathV1().split(QLatin1Char('/'));

    if ((markerParts.size() != 2) || markerParts.at(0).isEmpty() ||
        markerParts.at(1).isEmpty())
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    const QByteArray markerDirectoryName = QFile::encodeName(markerParts.at(0));
    const QByteArray markerFileName = QFile::encodeName(markerParts.at(1));

    const QByteArray rootPath = QFile::encodeName(QDir::cleanPath(root.configuredPath));
    struct stat pathStat = {};

    if (::lstat(rootPath.constData(), &pathStat) != 0)
    {
        return (errno == ENOENT)
             ? PrivacyRootRuntimeState::Offline
             : PrivacyRootRuntimeState::IdentityMismatch;
    }

    if (!S_ISDIR(pathStat.st_mode) || S_ISLNK(pathStat.st_mode) ||
        pathHasSymlinkComponent(root.configuredPath))
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    const ScopedFileDescriptor rootDescriptor(
        ::open(rootPath.constData(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));

    if (rootDescriptor.get() < 0)
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    struct stat rootStat = {};

    if ((::fstat(rootDescriptor.get(), &rootStat) != 0) ||
        !S_ISDIR(rootStat.st_mode) || (rootStat.st_uid != geteuid()) ||
        ((rootStat.st_mode & (S_IWGRP | S_IWOTH)) != 0))
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    const ScopedFileDescriptor markerDirectory(
        ::openat(rootDescriptor.get(), markerDirectoryName.constData(),
                 O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));

    if (markerDirectory.get() < 0)
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    struct stat markerDirectoryStat = {};

    if ((::fstat(markerDirectory.get(), &markerDirectoryStat) != 0) ||
        !S_ISDIR(markerDirectoryStat.st_mode) ||
        (markerDirectoryStat.st_uid != geteuid()) ||
        (markerDirectoryStat.st_dev != rootStat.st_dev) ||
        ((markerDirectoryStat.st_mode & (S_IWGRP | S_IWOTH)) != 0))
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    const ScopedFileDescriptor markerDescriptor(
        ::openat(markerDirectory.get(), markerFileName.constData(),
                 O_RDONLY | O_NOFOLLOW | O_CLOEXEC));

    if (markerDescriptor.get() < 0)
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    struct stat markerStat = {};

    if ((::fstat(markerDescriptor.get(), &markerStat) != 0) ||
        !S_ISREG(markerStat.st_mode) || (markerStat.st_uid != geteuid()) ||
        (markerStat.st_dev != rootStat.st_dev) || (markerStat.st_nlink != 1) ||
        ((markerStat.st_mode & (S_IWGRP | S_IWOTH)) != 0) ||
        (markerStat.st_size <= 0) ||
        (markerStat.st_size > managedRootMarkerMaximumBytes))
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    QByteArray data;
    data.resize(static_cast<int>(markerStat.st_size));
    qint64 totalRead = 0;

    while (totalRead < data.size())
    {
        const ssize_t readCount = ::read(markerDescriptor.get(),
                                         data.data() + totalRead,
                                         static_cast<size_t>(data.size() - totalRead));

        if (readCount <= 0)
        {
            return PrivacyRootRuntimeState::IdentityMismatch;
        }

        totalRead += readCount;
    }

    char extraByte = 0;

    if (::read(markerDescriptor.get(), &extraByte, 1) != 0)
    {
        return PrivacyRootRuntimeState::IdentityMismatch;
    }

    *markerData = data;
    *filesystemIdentity =
        PrivacyRootIdentityInternal::filesystemUuidForDevice(rootStat.st_dev);

    return PrivacyRootRuntimeState::VerifiedAvailable;
}

#else

PrivacyRootRuntimeState readManagedRootMarker(const PrivacyStorageRoot& root,
                                              QByteArray*,
                                              QString*)
{
    return QFileInfo::exists(root.configuredPath)
         ? PrivacyRootRuntimeState::IdentityMismatch
         : PrivacyRootRuntimeState::Offline;
}

#endif

class CollectionPrivacyRootVerifier final : public PrivacyRootVerifier
{
public:

    PrivacyRootRuntimeState verify(const PrivacyStorageRoot& root) const override
    {
        if (root.kind == PrivacyStorageRootKind::AlbumRoot)
        {
            const CollectionLocation location =
                CollectionManager::instance()->locationForAlbumRootId(root.albumRootId);

            if (location.isNull() || !location.isAvailable())
            {
                return PrivacyRootRuntimeState::Offline;
            }

            if ((root.identityVersion == 1) &&
                PrivacyRootIdentityCodec::matchesAlbumRootV1(root.identityData,
                                                              root.albumRootId,
                                                              location.identifier))
            {
                return PrivacyRootRuntimeState::VerifiedAvailable;
            }

            return PrivacyRootRuntimeState::IdentityMismatch;
        }

        if ((root.kind != PrivacyStorageRootKind::ManagedStoreRoot) ||
            (root.identityVersion != 1))
        {
            return PrivacyRootRuntimeState::IdentityMismatch;
        }

        QByteArray markerData;
        QString filesystemIdentity;
        const PrivacyRootRuntimeState readState = readManagedRootMarker(
                                                       root, &markerData,
                                                       &filesystemIdentity);

        if (readState != PrivacyRootRuntimeState::VerifiedAvailable)
        {
            return readState;
        }

        return (PrivacyRootIdentityCodec::matchesManagedRootV1(
                    root.identityData, root.markerUuid, filesystemIdentity) &&
                PrivacyRootIdentityCodec::matchesManagedRootMarkerV1(
                    markerData, root.uuid, root.markerUuid))
             ? PrivacyRootRuntimeState::VerifiedAvailable
             : PrivacyRootRuntimeState::IdentityMismatch;
    }
};

class CollectionPrivacyIntegrityInspector final : public PrivacyRootIntegrityInspector
{
public:

    PrivacyRootInspectionResult inspect(
        const PrivacyStorageRoot& root,
        const PrivacyRepositorySnapshot& snapshot) const override
    {
        PrivacyRootInspectionResult result;
        result.summary.rootUuid = root.uuid;
        result.summary.configuredPath = root.configuredPath;
        QSet<QString> protectedItemUuids;
        QString rootPath;

        if (root.kind == PrivacyStorageRootKind::AlbumRoot)
        {
            const CollectionLocation location =
                CollectionManager::instance()->locationForAlbumRootId(root.albumRootId);

            if (location.isNull() || !location.isAvailable())
            {
                return result;
            }

            rootPath = QDir::cleanPath(location.albumRootPath());
        }
        else if (root.kind == PrivacyStorageRootKind::ManagedStoreRoot)
        {
            rootPath = QDir::cleanPath(root.configuredPath);
        }
        else
        {
            return result;
        }

        const auto checkedPath = [&rootPath](const QString& relativePath, QString* path)
        {
            const QString candidate = QDir::cleanPath(QDir(rootPath).filePath(relativePath));

            if ((candidate == rootPath) ||
                !candidate.startsWith(rootPath + QDir::separator()))
            {
                return false;
            }

#ifdef Q_OS_UNIX

            if (pathHasSymlinkComponent(candidate))
            {
                return false;
            }

#endif

            *path = candidate;

            return true;
        };

        for (const PrivacyAsset& asset : snapshot.assets)
        {
            if (asset.publicRootUuid != root.uuid)
            {
                continue;
            }

            protectedItemUuids.insert(asset.itemUuid);

            QString path;

            if (!checkedPath(asset.publicRelativePath, &path))
            {
                result.proxyIssueItemUuids.insert(asset.itemUuid);
                result.disposition = PrivacyIntegrityDisposition::Failed;
                return result;
            }

            const QFileInfo info(path);

            if (info.isSymLink())
            {
                result.proxyIssueItemUuids.insert(asset.itemUuid);
                result.disposition = PrivacyIntegrityDisposition::Failed;
                return result;
            }

            if (asset.proxySize >= 0)
            {
                if (!info.exists())
                {
                    ++result.summary.missingProxyCount;
                    result.proxyIssueItemUuids.insert(asset.itemUuid);
                }
                else if (!info.isFile() || (info.size() != asset.proxySize))
                {
                    ++result.summary.changedProxySizeCount;
                    result.proxyIssueItemUuids.insert(asset.itemUuid);
                }
            }
            else if (info.exists())
            {
                // An associated asset with no public proxy is expected to
                // remain absent while protected.

                ++result.summary.unexpectedPublicAssetCount;
                result.proxyIssueItemUuids.insert(asset.itemUuid);
            }
        }

        for (const PrivacyContainer& container : snapshot.containers)
        {
            if (container.rootUuid != root.uuid)
            {
                continue;
            }

            protectedItemUuids.insert(container.itemUuid);

            QString path;

            if (!checkedPath(container.objectRelativePath, &path))
            {
                result.originalIssueItemUuids.insert(container.itemUuid);
                result.disposition = PrivacyIntegrityDisposition::Failed;
                return result;
            }

            const QFileInfo info(path);

            if (info.isSymLink())
            {
                result.originalIssueItemUuids.insert(container.itemUuid);
                result.disposition = PrivacyIntegrityDisposition::Failed;
                return result;
            }

            if (!info.exists())
            {
                ++result.summary.missingProtectedObjectCount;
                result.originalIssueItemUuids.insert(container.itemUuid);
            }
            else if (!info.isFile() || (info.size() != container.protectedSize))
            {
                ++result.summary.changedProtectedObjectSizeCount;
                result.originalIssueItemUuids.insert(container.itemUuid);
            }
        }

        for (const PrivacyStore& store : snapshot.stores)
        {
            if (store.rootUuid != root.uuid)
            {
                continue;
            }

            for (const PrivacyContainer& container : snapshot.containers)
            {
                if (container.storeUuid == store.uuid)
                {
                    protectedItemUuids.insert(container.itemUuid);
                }
            }
        }

        result.summary.protectedItemCount = protectedItemUuids.size();
        result.disposition = PrivacyIntegrityDisposition::Verified;

        return result;
    }
};

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() && (uuid == parsed.toString(QUuid::WithoutBraces)));
}

bool isValidRootState(PrivacyRootRuntimeState state)
{
    return ((state == PrivacyRootRuntimeState::Unknown)           ||
            (state == PrivacyRootRuntimeState::Recovering)        ||
            (state == PrivacyRootRuntimeState::VerifiedAvailable) ||
            (state == PrivacyRootRuntimeState::Offline)           ||
            (state == PrivacyRootRuntimeState::IdentityMismatch));
}

PrivacyScanDisposition dispositionForRootState(PrivacyRootRuntimeState state)
{
    switch (state)
    {
        case PrivacyRootRuntimeState::VerifiedAvailable:
        {
            return PrivacyScanDisposition::Unprotected;
        }

        case PrivacyRootRuntimeState::Offline:
        {
            return PrivacyScanDisposition::RootOffline;
        }

        case PrivacyRootRuntimeState::IdentityMismatch:
        {
            return PrivacyScanDisposition::RootIdentityMismatch;
        }

        case PrivacyRootRuntimeState::Unknown:
        case PrivacyRootRuntimeState::Recovering:
        {
            return PrivacyScanDisposition::RootRecovering;
        }
    }

    return PrivacyScanDisposition::RootRecovering;
}

QSet<QString> itemUuidsForRoot(const PrivacyRepositorySnapshot& snapshot,
                               const QString& rootUuid)
{
    QSet<QString> itemUuids;

    for (const PrivacyAsset& asset : snapshot.assets)
    {
        if (asset.publicRootUuid == rootUuid)
        {
            itemUuids.insert(asset.itemUuid);
        }
    }

    for (const PrivacyContainer& container : snapshot.containers)
    {
        if (container.rootUuid == rootUuid)
        {
            itemUuids.insert(container.itemUuid);
        }
    }

    for (const PrivacyStore& store : snapshot.stores)
    {
        if (store.rootUuid != rootUuid)
        {
            continue;
        }

        for (const PrivacyContainer& container : snapshot.containers)
        {
            if (container.storeUuid == store.uuid)
            {
                itemUuids.insert(container.itemUuid);
            }
        }

        for (const PrivacyDerivative& derivative : snapshot.derivatives)
        {
            if (derivative.storeUuid == store.uuid)
            {
                itemUuids.insert(derivative.itemUuid);
            }
        }
    }

    return itemUuids;
}

bool transactionAffectsRoot(const PrivacyTransaction& transaction,
                            const QList<PrivacyTransactionJournal>& journals,
                            const PrivacyRepositorySnapshot& snapshot,
                            const QString& rootUuid)
{
    for (const PrivacyTransactionJournal& journal : journals)
    {
        if (journal.rootUuid == rootUuid)
        {
            return true;
        }
    }

    const QSet<QString> rootItems = itemUuidsForRoot(snapshot, rootUuid);

    if (!transaction.itemUuid.isEmpty())
    {
        return rootItems.contains(transaction.itemUuid);
    }

    for (const PrivacyItem& item : snapshot.items)
    {
        if ((item.categoryUuid == transaction.categoryUuid) && rootItems.contains(item.uuid))
        {
            return true;
        }
    }

    return false;
}

void clearArtifactCounts(PrivacyRootIntegritySummary* summary)
{
    if (!summary)
    {
        return;
    }

    summary->missingProxyCount = 0;
    summary->changedProxySizeCount = 0;
    summary->failedProxyValidationCount = 0;
    summary->exposedOriginalAtProxyPathCount = 0;
    summary->unexpectedPublicAssetCount = 0;
    summary->missingProtectedObjectCount = 0;
    summary->changedProtectedObjectSizeCount = 0;
}

bool sameItem(const PrivacyItem& left, const PrivacyItem& right)
{
    return ((left.imageId == right.imageId) &&
            (left.uuid == right.uuid) &&
            (left.categoryUuid == right.categoryUuid) &&
            (left.originalHash == right.originalHash) &&
            (left.originalSize == right.originalSize) &&
            (left.originalWidth == right.originalWidth) &&
            (left.originalHeight == right.originalHeight) &&
            (left.originalCreationDate == right.originalCreationDate) &&
            (left.expectedProxyHash == right.expectedProxyHash) &&
            (left.expectedProxySize == right.expectedProxySize) &&
            (left.presentationVersion == right.presentationVersion) &&
            (left.generation == right.generation) &&
            (left.transactionState == right.transactionState));
}

bool sameContainer(const PrivacyContainer& left, const PrivacyContainer& right)
{
    return ((left.uuid == right.uuid) &&
            (left.itemUuid == right.itemUuid) &&
            (left.kind == right.kind) &&
            (left.rootUuid == right.rootUuid) &&
            (left.storeUuid == right.storeUuid) &&
            (left.objectRelativePath == right.objectRelativePath) &&
            (left.protectedSize == right.protectedSize) &&
            (left.protectedHashAlgorithm == right.protectedHashAlgorithm) &&
            (left.protectedHash == right.protectedHash) &&
            (left.formatVersion == right.formatVersion) &&
            (left.credentialGeneration == right.credentialGeneration) &&
            (left.state == right.state) &&
            (left.createdAt == right.createdAt) &&
            (left.updatedAt == right.updatedAt));
}

bool sameAsset(const PrivacyAsset& left, const PrivacyAsset& right)
{
    return ((left.itemUuid == right.itemUuid) &&
            (left.role == right.role) &&
            (left.ordinal == right.ordinal) &&
            (left.originalName == right.originalName) &&
            (left.publicRootUuid == right.publicRootUuid) &&
            (left.publicRelativePath == right.publicRelativePath) &&
            (left.containerUuid == right.containerUuid) &&
            (left.protectedRelativePath == right.protectedRelativePath) &&
            (left.hashAlgorithm == right.hashAlgorithm) &&
            (left.originalHash == right.originalHash) &&
            (left.originalSize == right.originalSize) &&
            (left.originalCreationDate == right.originalCreationDate) &&
            (left.originalModificationDate == right.originalModificationDate) &&
            (left.portableAttributes == right.portableAttributes) &&
            (left.proxyHashAlgorithm == right.proxyHashAlgorithm) &&
            (left.proxyHash == right.proxyHash) &&
            (left.proxySize == right.proxySize) &&
            (left.proxyPresentationVersion == right.proxyPresentationVersion) &&
            (left.proxyGeneration == right.proxyGeneration));
}

QString assetIdentity(const PrivacyAsset& asset)
{
    return QString::number(asset.role) + QLatin1Char(':') +
           QString::number(asset.ordinal);
}

bool hasProtectedFactsForRoot(const PrivacyRepositorySnapshot& snapshot,
                              const QString& rootUuid)
{
    for (const PrivacyAsset& asset : snapshot.assets)
    {
        if (asset.publicRootUuid == rootUuid)
        {
            return true;
        }
    }

    for (const PrivacyContainer& container : snapshot.containers)
    {
        if (container.rootUuid == rootUuid)
        {
            return true;
        }
    }

    for (const PrivacyStore& store : snapshot.stores)
    {
        if (store.rootUuid == rootUuid)
        {
            return true;
        }
    }

    return false;
}

struct ValidatedProtectedItemFacts
{
    QString publicRootUuid;
    QString publicRelativePath;
    QString originalRootUuid;
    qlonglong expectedProxySize = -1;
    bool originalInspectable = false;
    QSet<QString> rootUuids;
};

bool validateProtectedItemFacts(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets,
    const PrivacyRepositorySnapshot& snapshot,
    const QHash<QString, PrivacyRootRuntimeState>& rootStates,
    const QSet<QString>& conflictingRootUuids,
    ValidatedProtectedItemFacts* facts,
    bool allowRecoveringRoots = false)
{
    if (!facts || !item.isValid() || !container.isValid() || assets.isEmpty() ||
        item.originalHash.isEmpty() || (item.originalSize < 0) ||
        item.expectedProxyHash.isEmpty() || (item.expectedProxySize < 0) ||
        (container.itemUuid != item.uuid) ||
        (container.state != PrivacyContainerState::Verified))
    {
        return false;
    }

    int matchingCategoryCount = 0;
    PrivacyCategory category;

    for (const PrivacyCategory& candidate : snapshot.categories)
    {
        if (candidate.uuid == item.categoryUuid)
        {
            category = candidate;
            ++matchingCategoryCount;
        }
    }

    if ((matchingCategoryCount != 1) || !category.isValid() ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (container.credentialGeneration != category.currentCredentialGeneration))
    {
        return false;
    }

    if ((category.backend != PrivacyBackend::Casual) ||
        (container.kind != PrivacyContainerKind::CasualArchive) ||
        container.rootUuid.isEmpty())
    {
        return false;
    }

    const QString originalRootUuid = container.rootUuid;

    QSet<QString> assetIdentities;
    QSet<QString> publicRootUuids;
    int primaryCount = 0;
    PrivacyAsset primary;

    for (const PrivacyAsset& asset : assets)
    {
        const QString identity = assetIdentity(asset);

        if (!asset.isValid() || (asset.itemUuid != item.uuid) ||
            (asset.containerUuid != container.uuid) ||
            assetIdentities.contains(identity))
        {
            return false;
        }

        assetIdentities.insert(identity);
        publicRootUuids.insert(asset.publicRootUuid);

        if ((asset.role == PrivacyAsset::PrimaryMediaRole) && (asset.ordinal == 0))
        {
            primary = asset;
            ++primaryCount;
        }

        if ((asset.proxySize >= 0) &&
            ((asset.proxyGeneration != item.generation) ||
             (asset.proxyPresentationVersion != item.presentationVersion)))
        {
            return false;
        }
    }

    if ((primaryCount != 1) ||
        (primary.originalHash != item.originalHash) ||
        (primary.originalSize != item.originalSize) ||
        (primary.proxyHash != item.expectedProxyHash) ||
        (primary.proxySize != item.expectedProxySize) ||
        (primary.proxyGeneration != item.generation) ||
        (primary.proxyPresentationVersion != item.presentationVersion))
    {
        return false;
    }

    QSet<QString> requiredRootUuids = publicRootUuids;
    requiredRootUuids.insert(originalRootUuid);

    for (const QString& rootUuid : std::as_const(requiredRootUuids))
    {
        const PrivacyRootRuntimeState rootState = rootStates.value(
            rootUuid, PrivacyRootRuntimeState::Unknown);

        if (rootUuid.isEmpty() || conflictingRootUuids.contains(rootUuid) ||
            ((rootState != PrivacyRootRuntimeState::VerifiedAvailable) &&
             (!allowRecoveringRoots ||
              (rootState != PrivacyRootRuntimeState::Recovering))))
        {
            return false;
        }
    }

    facts->publicRootUuid = primary.publicRootUuid;
    facts->publicRelativePath = primary.publicRelativePath;
    facts->originalRootUuid = originalRootUuid;
    facts->expectedProxySize = primary.proxySize;
    facts->originalInspectable = true;
    facts->rootUuids = requiredRootUuids;

    return true;
}

class PrivacyStartupData
{
public:

    QReadWriteLock                             lock;
    QSharedPointer<PrivacyRuntimeCoordinator>  coordinator;
    QSharedPointer<PrivacyCategorySessionOwner> categorySessions;
    PrivacyStartupRecovery::TransactionRecoveryFactory transactionRecoveryFactory;
    PrivacyStartupReport                      report;
};

Q_GLOBAL_STATIC(PrivacyStartupData, startupData)

class PrivacyManualTagVisibilityGateData
{
public:

    QReadWriteLock lock;
    QSharedPointer<const PrivacyManualTagVisibilityProvider> provider;
    quint64 generation = 0;
};

Q_GLOBAL_STATIC(PrivacyManualTagVisibilityGateData, manualTagGateData)

} // namespace

bool PrivacyRootIntegritySummary::hasReportableIssues(
    bool includeProxySizeChanges) const
{
    return ((state != PrivacyRootRuntimeState::VerifiedAvailable) ||
            (missingProxyCount > 0) ||
            (includeProxySizeChanges && (changedProxySizeCount > 0)) ||
            (failedProxyValidationCount > 0) ||
            (exposedOriginalAtProxyPathCount > 0) ||
            (unexpectedPublicAssetCount > 0) ||
            (missingProtectedObjectCount > 0) ||
            (changedProtectedObjectSizeCount > 0) ||
            (unresolvedTransactionCount > 0) ||
            (compatibilityExposureCount > 0) || identityMismatch);
}

bool PrivacyStartupReport::hasOnlyProxySizeIssues() const
{
    if ((state != PrivacyStartupState::Ready) ||
        (offlineRootCount != 0) || (mismatchedRootCount != 0) ||
        (recoveringRootCount != 0) || (unresolvedTransactionCount != 0) ||
        !diagnostics.isEmpty())
    {
        return false;
    }

    bool foundProxySizeIssue = false;

    for (const PrivacyRootIntegritySummary& root : roots)
    {
        if (root.hasReportableIssues(false))
        {
            return false;
        }

        foundProxySizeIssue = foundProxySizeIssue ||
                              (root.changedProxySizeCount > 0);
    }

    return foundProxySizeIssue;
}

bool PrivacyStartupReport::hasReportableIssues(bool suppressProxySizeOnly) const
{
    bool found = (state == PrivacyStartupState::Degraded) ||
                 (offlineRootCount > 0) || (mismatchedRootCount > 0) ||
                 (recoveringRootCount > 0) ||
                 (unresolvedTransactionCount > 0) || !diagnostics.isEmpty();

    for (const PrivacyRootIntegritySummary& root : roots)
    {
        found = found || root.hasReportableIssues();
    }

    return (found && !(suppressProxySizeOnly && hasOnlyProxySizeIssues()));
}

class Q_DECL_HIDDEN PrivacyRuntimeCoordinator::Private
{
public:

    struct ItemRuntime
    {
        PrivacyItem item;
        QString     publicRootUuid;
        QString     publicRelativePath;
        QString     originalRootUuid;
        qlonglong   expectedProxySize = -1;
        bool        originalInspectable = false;
        bool        mappingConflict = false;
    };

public:

    mutable QReadWriteLock                 lock;
    PrivacyService                         service;
    PrivacyRepositorySnapshot              snapshot;
    PrivacyStartupReport                   report;
    QHash<QString, PrivacyRootRuntimeState> rootStates;
    QHash<QString, quint64>                 rootEpochs;
    QHash<QString, PrivacyRootIntegritySummary> rootSummaries;
    QHash<int, QString>                     albumRootUuids;
    QHash<qlonglong, ItemRuntime>           items;
    QHash<QString, qlonglong>               imageIdsByItemUuid;
    QSet<QString>                           conflictingItemUuids;
    QHash<QString, QSet<QString> >          proxyIssueItemsByRoot;
    QHash<QString, QSet<QString> >          displayProxyIssueItemsByRoot;
    QHash<QString, QSet<QString> >          exposedOriginalItemsByRoot;
    QHash<QString, QSet<QString> >          originalIssueItemsByRoot;
    QSet<QString>                           conflictingRootUuids;
    QSet<qlonglong>                         compatibilityExposedItems;
    QSet<QString>                           protectedRootUuids;
    bool                                    hasUnassignedProtectedItems = false;
    bool                                    initialized = false;
    QSharedPointer<const PrivacyRootVerifier> rootVerifier;
    QSharedPointer<const PrivacyTransactionRecovery> recovery;
    QSharedPointer<const PrivacyRootIntegrityInspector> integrityInspector;

    void setRootState(const QString& rootUuid, PrivacyRootRuntimeState state)
    {
        rootStates.insert(rootUuid, state);
        quint64 epoch = rootEpochs.value(rootUuid, 0) + 1;

        if (epoch == 0)
        {
            ++epoch;
        }

        rootEpochs.insert(rootUuid, epoch);
    }

    void refreshProtectedRootFacts(const QSet<QString>& rootUuids)
    {
        for (const QString& rootUuid : rootUuids)
        {
            if (hasProtectedFactsForRoot(snapshot, rootUuid))
            {
                protectedRootUuids.insert(rootUuid);
            }
            else
            {
                protectedRootUuids.remove(rootUuid);
            }

            PrivacyRootIntegritySummary summary = rootSummaries.value(rootUuid);
            summary.rootUuid = rootUuid;
            summary.protectedItemCount = itemUuidsForRoot(snapshot, rootUuid).size();
            rootSummaries.insert(rootUuid, summary);

            for (PrivacyRootIntegritySummary& reported : report.roots)
            {
                if (reported.rootUuid == rootUuid)
                {
                    reported = summary;
                    break;
                }
            }
        }
    }

    void refreshDisplayProxyIssueCount(const QString& rootUuid)
    {
        PrivacyRootIntegritySummary summary = rootSummaries.value(rootUuid);
        summary.failedProxyValidationCount =
            qMax(0, displayProxyIssueItemsByRoot.value(rootUuid).size() -
                    exposedOriginalItemsByRoot.value(rootUuid).size());
        summary.exposedOriginalAtProxyPathCount =
            exposedOriginalItemsByRoot.value(rootUuid).size();
        rootSummaries.insert(rootUuid, summary);

        for (PrivacyRootIntegritySummary& reported : report.roots)
        {
            if (reported.rootUuid == rootUuid)
            {
                reported = summary;
                break;
            }
        }
    }
};

PrivacyRuntimeCoordinator::PrivacyRuntimeCoordinator()
    : d(new Private)
{
}

PrivacyRuntimeCoordinator::~PrivacyRuntimeCoordinator()
{
    delete d;
}

PrivacyStartupReport PrivacyRuntimeCoordinator::initialize(
    const PrivacyRepositorySnapshot& snapshot,
    const QSharedPointer<const PrivacyRootVerifier>& rootVerifier,
    const QSharedPointer<const PrivacyTransactionRecovery>& recovery,
    const QSharedPointer<const PrivacyRootIntegrityInspector>& integrityInspector)
{
    if (recovery)
    {
        // Publish the durable snapshot fail-closed before recovery callbacks.
        // The first pass deliberately omits integrity inspection: affected
        // roots remain Recovering, and the final pass inspects reconciled facts
        // exactly once.
        initialize(snapshot, rootVerifier, {}, {});

        {
            QWriteLocker locker(&d->lock);
            d->recovery = recovery;
            d->integrityInspector = integrityInspector;
        }

        QHash<QString, int> rootUuidCounts;
        QHash<int, int> albumRootIdCounts;

        for (const PrivacyStorageRoot& root : snapshot.storageRoots)
        {
            ++rootUuidCounts[root.uuid];

            if (root.kind == PrivacyStorageRootKind::AlbumRoot)
            {
                ++albumRootIdCounts[root.albumRootId];
            }
        }

        QSet<QString> recoveredTransactionUuids;

        for (const PrivacyTransaction& transaction : snapshot.transactions)
        {
            if (!transaction.isActive())
            {
                continue;
            }

            QList<PrivacyTransactionJournal> journals;

            for (const PrivacyTransactionJournal& journal :
                 snapshot.transactionJournals)
            {
                if (journal.transactionUuid == transaction.uuid)
                {
                    journals << journal;
                }
            }

            bool affectedAnyRoot = false;
            bool recoveredEveryRoot = true;

            for (const PrivacyStorageRoot& root : snapshot.storageRoots)
            {
                if (!transactionAffectsRoot(transaction, journals, snapshot,
                                            root.uuid))
                {
                    continue;
                }

                affectedAnyRoot = true;
                PrivacyRecoveryDisposition disposition =
                    PrivacyRecoveryDisposition::Deferred;
                const bool uniqueRoot =
                    (rootUuidCounts.value(root.uuid) == 1) &&
                    ((root.kind != PrivacyStorageRootKind::AlbumRoot) ||
                     (albumRootIdCounts.value(root.albumRootId) == 1));

                if (uniqueRoot && rootVerifier &&
                    (rootVerifier->verify(root) ==
                     PrivacyRootRuntimeState::VerifiedAvailable))
                {
                    QList<PrivacyTransactionJournal> rootJournals;

                    for (const PrivacyTransactionJournal& journal : journals)
                    {
                        if (journal.rootUuid == root.uuid)
                        {
                            rootJournals << journal;
                        }
                    }

                    disposition = recovery->recoverRoot(root, transaction,
                                                        rootJournals);
                }

                if (disposition != PrivacyRecoveryDisposition::Recovered)
                {
                    recoveredEveryRoot = false;
                }
            }

            if (affectedAnyRoot && recoveredEveryRoot)
            {
                recoveredTransactionUuids.insert(transaction.uuid);
            }
        }

        PrivacyRepositorySnapshot reconciled;
        QSet<QString> removedActiveTransactionUuids;

        {
            QReadLocker locker(&d->lock);
            reconciled = d->snapshot;
        }

        for (int i = reconciled.transactions.size() - 1 ; i >= 0 ; --i)
        {
            const PrivacyTransaction& transaction =
                reconciled.transactions.at(i);

            if (transaction.isActive() &&
                recoveredTransactionUuids.contains(transaction.uuid))
            {
                removedActiveTransactionUuids.insert(transaction.uuid);
                reconciled.transactions.removeAt(i);
            }
        }

        for (int i = reconciled.transactionJournals.size() - 1 ; i >= 0 ; --i)
        {
            if (removedActiveTransactionUuids.contains(
                    reconciled.transactionJournals.at(i).transactionUuid))
            {
                reconciled.transactionJournals.removeAt(i);
            }
        }

        PrivacyStartupReport result = initialize(reconciled, rootVerifier, {},
                                                  integrityInspector);

        {
            QWriteLocker locker(&d->lock);
            d->recovery = recovery;
        }

        return result;
    }

    QHash<QString, PrivacyRootRuntimeState> verifiedRootStates;
    QHash<QString, int> rootUuidCounts;
    QHash<int, int> albumRootIdCounts;

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        ++rootUuidCounts[root.uuid];

        if (root.kind == PrivacyStorageRootKind::AlbumRoot)
        {
            ++albumRootIdCounts[root.albumRootId];
        }
    }

    QSet<QString> conflictingRootUuids;

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        if ((rootUuidCounts.value(root.uuid) != 1) ||
            ((root.kind == PrivacyStorageRootKind::AlbumRoot) &&
             (albumRootIdCounts.value(root.albumRootId) != 1)))
        {
            conflictingRootUuids.insert(root.uuid);
            verifiedRootStates.insert(root.uuid, PrivacyRootRuntimeState::Recovering);

            continue;
        }

        const PrivacyRootRuntimeState state = rootVerifier
                                            ? rootVerifier->verify(root)
                                            : PrivacyRootRuntimeState::IdentityMismatch;
        verifiedRootStates.insert(root.uuid,
                                  isValidRootState(state)
                                  ? state
                                  : PrivacyRootRuntimeState::IdentityMismatch);
    }

    QSet<QString> protectedRootUuids;
    QSet<QString> assignedPrimaryItemUuids;

    for (const PrivacyAsset& asset : snapshot.assets)
    {
        protectedRootUuids.insert(asset.publicRootUuid);

        if ((asset.role == PrivacyAsset::PrimaryMediaRole) && (asset.ordinal == 0))
        {
            assignedPrimaryItemUuids.insert(asset.itemUuid);
        }
    }

    for (const PrivacyContainer& container : snapshot.containers)
    {
        if (!container.rootUuid.isEmpty())
        {
            protectedRootUuids.insert(container.rootUuid);
        }
    }

    for (const PrivacyStore& store : snapshot.stores)
    {
        protectedRootUuids.insert(store.rootUuid);
    }

    QHash<QString, int> unresolvedTransactionsByRoot;
    QHash<QString, int> compatibilityExposuresByRoot;
    QSet<QString> unresolvedTransactionUuids;

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if (!transaction.isActive())
        {
            continue;
        }

        QList<PrivacyTransactionJournal> journals;

        for (const PrivacyTransactionJournal& journal : snapshot.transactionJournals)
        {
            if (journal.transactionUuid == transaction.uuid)
            {
                journals << journal;
            }
        }

        bool affectedAnyRoot = false;
        bool recoveredEveryRoot = true;

        for (const PrivacyStorageRoot& root : snapshot.storageRoots)
        {
            if (!transactionAffectsRoot(transaction, journals, snapshot, root.uuid))
            {
                continue;
            }

            affectedAnyRoot = true;
            PrivacyRecoveryDisposition disposition = PrivacyRecoveryDisposition::Deferred;

            if ((verifiedRootStates.value(root.uuid) ==
                 PrivacyRootRuntimeState::VerifiedAvailable) && recovery)
            {
                QList<PrivacyTransactionJournal> rootJournals;

                for (const PrivacyTransactionJournal& journal : journals)
                {
                    if (journal.rootUuid == root.uuid)
                    {
                        rootJournals << journal;
                    }
                }

                disposition = recovery->recoverRoot(root, transaction, rootJournals);
            }

            if (disposition != PrivacyRecoveryDisposition::Recovered)
            {
                recoveredEveryRoot = false;
                ++unresolvedTransactionsByRoot[root.uuid];

                if ((transaction.type == PrivacyTransactionType::CompatibilityUnlock) ||
                    (transaction.type == PrivacyTransactionType::CompatibilityRelock))
                {
                    ++compatibilityExposuresByRoot[root.uuid];
                }
            }
        }

        if (!affectedAnyRoot || !recoveredEveryRoot)
        {
            unresolvedTransactionUuids.insert(transaction.uuid);
        }
    }

    QHash<QString, PrivacyRootInspectionResult> integrityResults;

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        if ((verifiedRootStates.value(root.uuid) == PrivacyRootRuntimeState::VerifiedAvailable) &&
            protectedRootUuids.contains(root.uuid) &&
            (unresolvedTransactionsByRoot.value(root.uuid) == 0))
        {
            PrivacyRootInspectionResult result;
            result.summary.rootUuid = root.uuid;
            result.summary.protectedItemCount = itemUuidsForRoot(snapshot, root.uuid).size();

            if (integrityInspector)
            {
                result = integrityInspector->inspect(root, snapshot);
            }

            const PrivacyRootRuntimeState finalState = rootVerifier
                                                     ? rootVerifier->verify(root)
                                                     : PrivacyRootRuntimeState::IdentityMismatch;

            if (finalState != PrivacyRootRuntimeState::VerifiedAvailable)
            {
                verifiedRootStates.insert(
                    root.uuid,
                    isValidRootState(finalState)
                    ? finalState
                    : PrivacyRootRuntimeState::IdentityMismatch);
                result.disposition = PrivacyIntegrityDisposition::Deferred;
            }

            integrityResults.insert(root.uuid, result);
        }
    }

    QWriteLocker locker(&d->lock);
    const bool unassignedProtectedItems =
        (assignedPrimaryItemUuids.size() != snapshot.items.size());

    d->snapshot = snapshot;
    d->rootStates.clear();
    d->rootEpochs.clear();
    d->rootSummaries.clear();
    d->albumRootUuids.clear();
    d->items.clear();
    d->imageIdsByItemUuid.clear();
    d->conflictingItemUuids.clear();
    d->proxyIssueItemsByRoot.clear();
    d->displayProxyIssueItemsByRoot.clear();
    d->exposedOriginalItemsByRoot.clear();
    d->originalIssueItemsByRoot.clear();
    d->conflictingRootUuids = conflictingRootUuids;
    d->compatibilityExposedItems.clear();
    d->protectedRootUuids = protectedRootUuids;
    d->hasUnassignedProtectedItems = false;
    d->report = PrivacyStartupReport();
    d->rootVerifier = rootVerifier;
    d->recovery = recovery;
    d->integrityInspector = integrityInspector;
    d->service.reset(snapshot.categories, snapshot.items);

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        d->setRootState(root.uuid, PrivacyRootRuntimeState::Recovering);

        if (root.kind == PrivacyStorageRootKind::AlbumRoot)
        {
            d->albumRootUuids.insert(root.albumRootId, root.uuid);
        }

        const PrivacyRootRuntimeState state = verifiedRootStates.value(
            root.uuid, PrivacyRootRuntimeState::IdentityMismatch);
        PrivacyRootIntegritySummary summary = integrityResults.value(root.uuid).summary;
        summary.rootUuid = root.uuid;
        summary.configuredPath = root.configuredPath;
        summary.protectedItemCount = itemUuidsForRoot(snapshot, root.uuid).size();
        summary.unresolvedTransactionCount = unresolvedTransactionsByRoot.value(root.uuid);
        summary.compatibilityExposureCount = compatibilityExposuresByRoot.value(root.uuid);
        summary.identityMismatch = (state == PrivacyRootRuntimeState::IdentityMismatch);

        if (conflictingRootUuids.contains(root.uuid))
        {
            d->setRootState(root.uuid, PrivacyRootRuntimeState::Recovering);
            d->report.diagnostics << QLatin1String("Conflicting privacy root mapping: ") +
                                     root.uuid;
        }
        else if ((root.kind == PrivacyStorageRootKind::AlbumRoot) &&
                 unassignedProtectedItems &&
                 (state == PrivacyRootRuntimeState::VerifiedAvailable))
        {
            d->setRootState(root.uuid, PrivacyRootRuntimeState::Recovering);
            d->report.diagnostics << QLatin1String("Protected item has no canonical public asset root");
        }
        else if ((state == PrivacyRootRuntimeState::VerifiedAvailable) &&
                 (unresolvedTransactionsByRoot.value(root.uuid) > 0))
        {
            d->setRootState(root.uuid, PrivacyRootRuntimeState::Recovering);
            d->report.diagnostics << QLatin1String("Root has unresolved privacy transactions: ") +
                                     root.uuid;
        }
        else if ((state == PrivacyRootRuntimeState::VerifiedAvailable) &&
                 protectedRootUuids.contains(root.uuid) &&
                 (integrityResults.value(root.uuid).disposition !=
                  PrivacyIntegrityDisposition::Verified))
        {
            d->setRootState(root.uuid, PrivacyRootRuntimeState::Recovering);
            d->report.diagnostics << QLatin1String("Startup integrity pass deferred for root: ") +
                                     root.uuid;
        }
        else
        {
            d->setRootState(root.uuid, state);
        }

        summary.state = d->rootStates.value(root.uuid);
        d->rootSummaries.insert(root.uuid, summary);
        d->proxyIssueItemsByRoot.insert(
            root.uuid, integrityResults.value(root.uuid).proxyIssueItemUuids);
        d->originalIssueItemsByRoot.insert(
            root.uuid, integrityResults.value(root.uuid).originalIssueItemUuids);
    }

    QHash<qlonglong, int> imageIdCounts;
    QHash<QString, int> itemUuidCounts;

    for (const PrivacyItem& item : snapshot.items)
    {
        ++imageIdCounts[item.imageId];
        ++itemUuidCounts[item.uuid];
    }

    for (const PrivacyItem& item : snapshot.items)
    {
        Private::ItemRuntime runtime;
        runtime.item              = item;
        runtime.expectedProxySize = item.expectedProxySize;
        runtime.mappingConflict   = ((imageIdCounts.value(item.imageId) != 1) ||
                                     (itemUuidCounts.value(item.uuid) != 1));
        d->items.insert(item.imageId, runtime);

        if (runtime.mappingConflict)
        {
            d->conflictingItemUuids.insert(item.uuid);
        }
        else
        {
            d->imageIdsByItemUuid.insert(item.uuid, item.imageId);
        }
    }

    QHash<QString, PrivacyContainer> containersByUuid;
    QSet<QString> conflictingContainerUuids;

    for (const PrivacyContainer& container : snapshot.containers)
    {
        if (containersByUuid.contains(container.uuid))
        {
            conflictingContainerUuids.insert(container.uuid);
        }
        else
        {
            containersByUuid.insert(container.uuid, container);
        }
    }

    QHash<QString, PrivacyStore> storesByUuid;
    QSet<QString> conflictingStoreUuids;

    for (const PrivacyStore& store : snapshot.stores)
    {
        if (storesByUuid.contains(store.uuid))
        {
            conflictingStoreUuids.insert(store.uuid);
        }
        else
        {
            storesByUuid.insert(store.uuid, store);
        }
    }

    QSet<QString> assignedPrimaryAssets;

    for (const PrivacyAsset& asset : snapshot.assets)
    {
        const qlonglong imageId = d->imageIdsByItemUuid.value(asset.itemUuid, -1);
        auto runtimeIt = d->items.find(imageId);

        if ((runtimeIt == d->items.end()) ||
            (asset.role != PrivacyAsset::PrimaryMediaRole) || (asset.ordinal != 0))
        {
            continue;
        }

        if (assignedPrimaryAssets.contains(asset.itemUuid))
        {
            runtimeIt->mappingConflict = true;
            d->conflictingItemUuids.insert(asset.itemUuid);

            continue;
        }

        assignedPrimaryAssets.insert(asset.itemUuid);
        runtimeIt->publicRootUuid    = asset.publicRootUuid;
        runtimeIt->publicRelativePath = asset.publicRelativePath;
        runtimeIt->expectedProxySize = asset.proxySize;

        if (!d->rootStates.contains(asset.publicRootUuid) ||
            conflictingContainerUuids.contains(asset.containerUuid) ||
            !containersByUuid.contains(asset.containerUuid))
        {
            runtimeIt->mappingConflict = true;
            d->conflictingItemUuids.insert(asset.itemUuid);

            continue;
        }

        const PrivacyContainer container = containersByUuid.value(asset.containerUuid);

        if (container.itemUuid != asset.itemUuid)
        {
            runtimeIt->mappingConflict = true;
        }
        else if ((container.kind == PrivacyContainerKind::CasualArchive) &&
                 !container.rootUuid.isEmpty())
        {
            runtimeIt->originalRootUuid = container.rootUuid;
            runtimeIt->originalInspectable =
                (container.state == PrivacyContainerState::Verified);
        }
        else if ((container.kind == PrivacyContainerKind::StrongObject) &&
                 !container.storeUuid.isEmpty() &&
                 !conflictingStoreUuids.contains(container.storeUuid) &&
                 storesByUuid.contains(container.storeUuid))
        {
            runtimeIt->originalRootUuid = storesByUuid.value(container.storeUuid).rootUuid;
            // Strong plaintext availability additionally requires a verified
            // mounted-store sentinel. That provider is not installed yet.

            runtimeIt->originalInspectable = false;
        }
        else
        {
            runtimeIt->mappingConflict = true;
        }

        if (runtimeIt->originalRootUuid.isEmpty() ||
            !d->rootStates.contains(runtimeIt->originalRootUuid))
        {
            runtimeIt->mappingConflict = true;
        }

        if (runtimeIt->mappingConflict)
        {
            d->conflictingItemUuids.insert(asset.itemUuid);
        }
    }

    for (auto it = d->items.begin() ; it != d->items.end() ; ++it)
    {
        if (!assignedPrimaryAssets.contains(it->item.uuid))
        {
            it->mappingConflict = true;
            d->conflictingItemUuids.insert(it->item.uuid);
        }
    }

    d->hasUnassignedProtectedItems = (unassignedProtectedItems ||
                                      !d->conflictingItemUuids.isEmpty());

    if (!d->conflictingItemUuids.isEmpty())
    {
        d->report.diagnostics << QLatin1String(
            "Conflicting protected item/root mappings require repair");

        for (auto rootIt = d->albumRootUuids.constBegin() ;
             rootIt != d->albumRootUuids.constEnd() ; ++rootIt)
        {
            if (d->rootStates.value(rootIt.value()) ==
                PrivacyRootRuntimeState::VerifiedAvailable)
            {
                d->setRootState(rootIt.value(), PrivacyRootRuntimeState::Recovering);
                PrivacyRootIntegritySummary summary = d->rootSummaries.value(rootIt.value());
                summary.state = PrivacyRootRuntimeState::Recovering;
                d->rootSummaries.insert(rootIt.value(), summary);
            }
        }
    }

    d->report.unresolvedTransactionCount = unresolvedTransactionUuids.size();

    for (const QString& transactionUuid : unresolvedTransactionUuids)
    {
        d->report.diagnostics << QLatin1String("Unresolved privacy transaction: ") +
                                 transactionUuid;
    }

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if (!transaction.isActive() ||
            !unresolvedTransactionUuids.contains(transaction.uuid) ||
            ((transaction.type != PrivacyTransactionType::CompatibilityUnlock) &&
             (transaction.type != PrivacyTransactionType::CompatibilityRelock)))
        {
            continue;
        }

        for (auto it = d->items.begin() ; it != d->items.end() ; ++it)
        {
            const bool affected = transaction.itemUuid.isEmpty()
                                ? (it->item.categoryUuid == transaction.categoryUuid)
                                : (it->item.uuid == transaction.itemUuid);

            if (affected)
            {
                d->compatibilityExposedItems.insert(it.key());
            }
        }
    }

    for (auto it = d->rootStates.constBegin() ; it != d->rootStates.constEnd() ; ++it)
    {
        switch (it.value())
        {
            case PrivacyRootRuntimeState::VerifiedAvailable:
            {
                ++d->report.verifiedRootCount;
                break;
            }

            case PrivacyRootRuntimeState::Offline:
            {
                ++d->report.offlineRootCount;
                break;
            }

            case PrivacyRootRuntimeState::IdentityMismatch:
            {
                ++d->report.mismatchedRootCount;
                break;
            }

            case PrivacyRootRuntimeState::Unknown:
            case PrivacyRootRuntimeState::Recovering:
            {
                ++d->report.recoveringRootCount;
                break;
            }
        }

        PrivacyRootIntegritySummary summary = d->rootSummaries.value(it.key());
        summary.state = it.value();
        summary.identityMismatch = (it.value() == PrivacyRootRuntimeState::IdentityMismatch);
        d->rootSummaries.insert(it.key(), summary);
        d->report.roots << summary;
    }

    d->report.state = ((d->report.offlineRootCount == 0) &&
                       (d->report.mismatchedRootCount == 0) &&
                       (d->report.recoveringRootCount == 0) &&
                       (d->report.unresolvedTransactionCount == 0))
                    ? PrivacyStartupState::Ready
                    : PrivacyStartupState::Degraded;
    d->initialized = true;

    return d->report;
}

void PrivacyRuntimeCoordinator::reset()
{
    QWriteLocker locker(&d->lock);
    d->service.reset({}, {});
    d->service.lockAll();
    d->snapshot = PrivacyRepositorySnapshot();
    d->rootStates.clear();
    d->rootEpochs.clear();
    d->rootSummaries.clear();
    d->albumRootUuids.clear();
    d->items.clear();
    d->imageIdsByItemUuid.clear();
    d->conflictingItemUuids.clear();
    d->proxyIssueItemsByRoot.clear();
    d->displayProxyIssueItemsByRoot.clear();
    d->exposedOriginalItemsByRoot.clear();
    d->originalIssueItemsByRoot.clear();
    d->conflictingRootUuids.clear();
    d->compatibilityExposedItems.clear();
    d->protectedRootUuids.clear();
    d->hasUnassignedProtectedItems = false;
    d->report = PrivacyStartupReport();
    d->rootVerifier.clear();
    d->recovery.clear();
    d->integrityInspector.clear();
    d->initialized = false;
}

PrivacyStartupReport PrivacyRuntimeCoordinator::report() const
{
    QReadLocker locker(&d->lock);

    return d->report;
}

PrivacyRootRuntimeState PrivacyRuntimeCoordinator::rootState(const QString& rootUuid) const
{
    QReadLocker locker(&d->lock);

    return d->rootStates.value(rootUuid, PrivacyRootRuntimeState::Unknown);
}

quint64 PrivacyRuntimeCoordinator::rootEpoch(const QString& rootUuid) const
{
    QReadLocker locker(&d->lock);

    return d->rootEpochs.value(rootUuid, 0);
}

PrivacyRootIntegritySummary PrivacyRuntimeCoordinator::rootSummary(
    const QString& rootUuid) const
{
    QReadLocker locker(&d->lock);

    return d->rootSummaries.value(rootUuid);
}

QString PrivacyRuntimeCoordinator::rootUuidForAlbumRootId(int albumRootId) const
{
    QReadLocker locker(&d->lock);

    return d->albumRootUuids.value(albumRootId);
}

PrivacyPublicSourceDisposition PrivacyRuntimeCoordinator::publicSourceDisposition(
    qlonglong imageId) const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized)
    {
        return PrivacyPublicSourceDisposition::Denied;
    }

    const auto itemIt = d->items.constFind(imageId);

    if (itemIt == d->items.constEnd())
    {
        return PrivacyPublicSourceDisposition::Unprotected;
    }

    if (d->compatibilityExposedItems.contains(imageId) ||
        itemIt->mappingConflict ||
        itemIt->publicRootUuid.isEmpty() ||
        (itemIt->expectedProxySize < 0) ||
        (d->rootStates.value(itemIt->publicRootUuid, PrivacyRootRuntimeState::Unknown) !=
         PrivacyRootRuntimeState::VerifiedAvailable))
    {
        return PrivacyPublicSourceDisposition::Denied;
    }

    return PrivacyPublicSourceDisposition::LockedProxy;
}

QString PrivacyRuntimeCoordinator::publicSourceCacheNamespace(qlonglong imageId) const
{
    QReadLocker locker(&d->lock);
    const auto itemIt = d->items.constFind(imageId);

    if (itemIt == d->items.constEnd())
    {
        return QString();
    }

    return (QLatin1String("privacy-locked:") + itemIt->item.uuid + QLatin1Char(':') +
            QString::number(itemIt->item.generation) + QLatin1Char(':') +
            QString::number(itemIt->item.presentationVersion) + QLatin1Char(':') +
            QString::number(d->rootEpochs.value(itemIt->publicRootUuid, 0)));
}

qlonglong PrivacyRuntimeCoordinator::expectedPublicProxySize(qlonglong imageId) const
{
    QReadLocker locker(&d->lock);

    return d->items.value(imageId).expectedProxySize;
}

PrivacyPublicProxyDisplayResult
PrivacyRuntimeCoordinator::validatePublicProxyForDisplay(
    qlonglong imageId, const QString& absolutePath)
{
    qlonglong expectedSize = -1;
    QByteArray expectedSha256;
    QByteArray expectedOriginalSha256;
    QString expectedAbsolutePath;
    QString itemUuid;
    QString publicRootUuid;
    QString publicRelativePath;
    qlonglong itemGeneration = -1;
    qlonglong expectedOriginalSize = -1;
    int presentationVersion = -1;
    quint64 rootEpoch = 0;

    {
        QReadLocker locker(&d->lock);

        if (!d->initialized || (imageId <= 0))
        {
            return PrivacyPublicProxyDisplayResult::Denied;
        }

        const auto itemIt = d->items.constFind(imageId);

        if ((itemIt == d->items.constEnd()) || itemIt->mappingConflict ||
            d->conflictingItemUuids.contains(itemIt->item.uuid) ||
            itemIt->publicRootUuid.isEmpty() ||
            (d->rootStates.value(itemIt->publicRootUuid,
                                 PrivacyRootRuntimeState::Unknown) !=
             PrivacyRootRuntimeState::VerifiedAvailable))
        {
            return PrivacyPublicProxyDisplayResult::Denied;
        }

        expectedSize = itemIt->expectedProxySize;
        expectedSha256 = QByteArray::fromHex(
            itemIt->item.expectedProxyHash.toLatin1());
        expectedOriginalSha256 = QByteArray::fromHex(
            itemIt->item.originalHash.toLatin1());
        itemUuid = itemIt->item.uuid;
        publicRootUuid = itemIt->publicRootUuid;
        publicRelativePath = itemIt->publicRelativePath;
        itemGeneration = itemIt->item.generation;
        expectedOriginalSize = itemIt->item.originalSize;
        presentationVersion = itemIt->item.presentationVersion;
        rootEpoch = d->rootEpochs.value(publicRootUuid);

        int rootCount = 0;

        for (const PrivacyStorageRoot& root : std::as_const(d->snapshot.storageRoots))
        {
            if (root.uuid == itemIt->publicRootUuid)
            {
                expectedAbsolutePath = QDir(root.configuredPath).absoluteFilePath(
                    itemIt->publicRelativePath);
                ++rootCount;
            }
        }

        if ((rootCount != 1) || itemIt->publicRelativePath.isEmpty())
        {
            return PrivacyPublicProxyDisplayResult::Denied;
        }
    }

    if ((QDir::cleanPath(absolutePath) !=
         QDir::cleanPath(expectedAbsolutePath)) ||
        (expectedSha256.size() !=
         QCryptographicHash::hashLength(QCryptographicHash::Sha256)))
    {
        return PrivacyPublicProxyDisplayResult::Denied;
    }

    QByteArray actualSha256;
    const bool verified = stablePublicProxySha256(
                              absolutePath, expectedSize,
                              expectedOriginalSize, &actualSha256) &&
                          (actualSha256 == expectedSha256);
    const bool exposedOriginal =
        (!verified &&
         (expectedOriginalSha256.size() ==
          QCryptographicHash::hashLength(QCryptographicHash::Sha256)) &&
         (actualSha256 == expectedOriginalSha256));

    QWriteLocker locker(&d->lock);
    const auto itemIt = d->items.constFind(imageId);

    if (!d->initialized || (itemIt == d->items.constEnd()) ||
        itemIt->mappingConflict ||
        d->conflictingItemUuids.contains(itemUuid) ||
        (itemIt->item.uuid != itemUuid) ||
        (itemIt->item.generation != itemGeneration) ||
        (itemIt->item.presentationVersion != presentationVersion) ||
        (QByteArray::fromHex(itemIt->item.expectedProxyHash.toLatin1()) !=
         expectedSha256) ||
        (QByteArray::fromHex(itemIt->item.originalHash.toLatin1()) !=
         expectedOriginalSha256) ||
        (itemIt->item.originalSize != expectedOriginalSize) ||
        (itemIt->expectedProxySize != expectedSize) ||
        (itemIt->publicRootUuid != publicRootUuid) ||
        (itemIt->publicRelativePath != publicRelativePath) ||
        (d->rootEpochs.value(publicRootUuid) != rootEpoch) ||
        (d->rootStates.value(publicRootUuid,
                             PrivacyRootRuntimeState::Unknown) !=
         PrivacyRootRuntimeState::VerifiedAvailable))
    {
        return PrivacyPublicProxyDisplayResult::Denied;
    }

    QSet<QString>& displayIssues = d->displayProxyIssueItemsByRoot[publicRootUuid];
    QSet<QString>& exposedOriginals = d->exposedOriginalItemsByRoot[publicRootUuid];
    const bool alreadyReported = displayIssues.contains(itemUuid);
    const bool alreadyReportedAsOriginal = exposedOriginals.contains(itemUuid);

    if (verified)
    {
        displayIssues.remove(itemUuid);
        exposedOriginals.remove(itemUuid);
        d->proxyIssueItemsByRoot[publicRootUuid].remove(itemUuid);
    }
    else
    {
        displayIssues.insert(itemUuid);

        if (exposedOriginal)
        {
            exposedOriginals.insert(itemUuid);
        }
        else
        {
            exposedOriginals.remove(itemUuid);
        }
    }

    d->refreshDisplayProxyIssueCount(publicRootUuid);

    if (verified)
    {
        return PrivacyPublicProxyDisplayResult::Verified;
    }

    if (exposedOriginal)
    {
        return alreadyReportedAsOriginal
             ? PrivacyPublicProxyDisplayResult::Denied
             : PrivacyPublicProxyDisplayResult::NewlyExposedOriginal;
    }

    return (alreadyReported && !alreadyReportedAsOriginal)
         ? PrivacyPublicProxyDisplayResult::Denied
         : PrivacyPublicProxyDisplayResult::NewlyFailedValidation;
}

bool PrivacyRuntimeCoordinator::setCategoryUnlocked(const QString& categoryUuid,
                                                    bool unlocked)
{
    {
        QReadLocker locker(&d->lock);

        if (!d->initialized)
        {
            return false;
        }
    }

    return d->service.setCategoryUnlocked(categoryUuid, unlocked);
}

bool PrivacyRuntimeCoordinator::isCategoryUnlocked(const QString& categoryUuid) const
{
    QReadLocker locker(&d->lock);

    return (d->initialized && d->service.isCategoryUnlocked(categoryUuid));
}

bool PrivacyRuntimeCoordinator::publishCategory(const PrivacyCategory& category)
{
    QWriteLocker locker(&d->lock);

    if (!d->initialized || !d->service.addCategory(category))
    {
        return false;
    }

    d->snapshot.categories << category;

    return true;
}

bool PrivacyRuntimeCoordinator::publishCategoryCreation(
    const PrivacyCategory& category, const PrivacyCredential& credential,
    const PrivacyStorageRoot& root, const PrivacyStore& store,
    const QList<PrivacyStoreBinding>& bindings)
{
    if (!category.isValid() || !credential.isValid() || !root.isValid() ||
        !store.isValid() || bindings.isEmpty() ||
        (category.backend != PrivacyBackend::Casual) ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (category.currentCredentialGeneration != credential.generation) ||
        (credential.categoryUuid != category.uuid) ||
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot) ||
        (store.categoryUuid != category.uuid) || (store.rootUuid != root.uuid) ||
        (store.lifecycleState != PrivacyStoreLifecycleState::Active) ||
        (store.configGeneration != credential.generation))
    {
        return false;
    }

    QSet<PrivacyStoreRole> roles;

    for (const PrivacyStoreBinding& binding : bindings)
    {
        if (!binding.isValid() || (binding.categoryUuid != category.uuid) ||
            (binding.storeUuid != store.uuid) || roles.contains(binding.role))
        {
            return false;
        }

        roles.insert(binding.role);
    }

    if ((roles.size() != 2) ||
        !roles.contains(PrivacyStoreRole::CredentialAuthority) ||
        !roles.contains(PrivacyStoreRole::Derivatives))
    {
        return false;
    }

    QSharedPointer<const PrivacyRootVerifier> verifier;

    {
        QReadLocker locker(&d->lock);

        if (!d->initialized)
        {
            return false;
        }

        verifier = d->rootVerifier;
    }

    if (!verifier ||
        (verifier->verify(root) != PrivacyRootRuntimeState::VerifiedAvailable))
    {
        return false;
    }

    QWriteLocker locker(&d->lock);

    if (!d->initialized)
    {
        return false;
    }

    bool categoryPresent = false;

    for (const PrivacyCategory& existing : std::as_const(d->snapshot.categories))
    {
        if (existing.uuid == category.uuid)
        {
            if (categoryPresent || (existing.name != category.name) ||
                (existing.backend != category.backend) ||
                (existing.presentationMode != category.presentationMode) ||
                (existing.unlockedThumbnailMode != category.unlockedThumbnailMode) ||
                (existing.tagVisibilityMode != category.tagVisibilityMode) ||
                (existing.lifecycleState != category.lifecycleState) ||
                (existing.currentCredentialGeneration !=
                 category.currentCredentialGeneration))
            {
                return false;
            }

            categoryPresent = true;
            continue;
        }

        if (existing.name.compare(category.name, Qt::CaseInsensitive) == 0)
        {
            return false;
        }
    }

    bool credentialPresent = false;

    for (const PrivacyCredential& existing : std::as_const(d->snapshot.credentials))
    {
        if ((existing.categoryUuid != credential.categoryUuid) ||
            (existing.generation != credential.generation))
        {
            continue;
        }

        if (credentialPresent ||
            (existing.encodingVersion != credential.encodingVersion) ||
            (existing.envelopeFormat != credential.envelopeFormat) ||
            (existing.envelopeBlob != credential.envelopeBlob) ||
            (existing.envelopeHashAlgorithm != credential.envelopeHashAlgorithm) ||
            (existing.envelopeHash != credential.envelopeHash))
        {
            return false;
        }

        credentialPresent = true;
    }

    bool rootPresent = false;

    for (const PrivacyStorageRoot& existing : std::as_const(d->snapshot.storageRoots))
    {
        if (existing.uuid != root.uuid)
        {
            continue;
        }

        if (rootPresent)
        {
            return false;
        }

        rootPresent = true;

        if ((existing.kind != root.kind) ||
            (existing.configuredPath != root.configuredPath) ||
            (existing.identityVersion != root.identityVersion) ||
            (existing.identityData != root.identityData) ||
            (existing.markerUuid != root.markerUuid))
        {
            return false;
        }
    }

    bool storePresent = false;

    for (const PrivacyStore& existing : std::as_const(d->snapshot.stores))
    {
        if (existing.uuid == store.uuid)
        {
            if (storePresent || (existing.categoryUuid != store.categoryUuid) ||
                (existing.rootUuid != store.rootUuid) ||
                (existing.format != store.format) ||
                (existing.formatVersion != store.formatVersion) ||
                (existing.cipherRelativePath != store.cipherRelativePath) ||
                (existing.configRelativePath != store.configRelativePath) ||
                (existing.configGeneration != store.configGeneration) ||
                (existing.lifecycleState != store.lifecycleState))
            {
                return false;
            }

            storePresent = true;
        }
    }

    QSet<PrivacyStoreRole> existingRoles;

    for (const PrivacyStoreBinding& existing :
         std::as_const(d->snapshot.storeBindings))
    {
        if (existing.categoryUuid != category.uuid)
        {
            continue;
        }

        if ((existing.storeUuid != store.uuid) ||
            existingRoles.contains(existing.role))
        {
            return false;
        }

        existingRoles.insert(existing.role);
    }

    if (!existingRoles.isEmpty() && (existingRoles != roles))
    {
        return false;
    }

    if ((categoryPresent != credentialPresent) ||
        (categoryPresent != storePresent) ||
        (categoryPresent != !existingRoles.isEmpty()))
    {
        return false;
    }

    if (!categoryPresent && !d->service.addCategory(category))
    {
        return false;
    }

    if (categoryPresent && (d->service.categoryEpoch(category.uuid) == 0))
    {
        return false;
    }

    if (!categoryPresent)
    {
        d->snapshot.categories << category;
    }

    if (!credentialPresent)
    {
        d->snapshot.credentials << credential;
    }

    if (!storePresent)
    {
        d->snapshot.stores << store;
    }

    if (existingRoles.isEmpty())
    {
        d->snapshot.storeBindings << bindings;
    }
    d->protectedRootUuids.insert(root.uuid);

    if (!rootPresent)
    {
        d->snapshot.storageRoots << root;
        d->setRootState(root.uuid, PrivacyRootRuntimeState::VerifiedAvailable);
        PrivacyRootIntegritySummary summary;
        summary.rootUuid = root.uuid;
        summary.configuredPath = root.configuredPath;
        summary.state = PrivacyRootRuntimeState::VerifiedAvailable;
        d->rootSummaries.insert(root.uuid, summary);
        d->report.roots << summary;
        ++d->report.verifiedRootCount;
    }
    else if (d->rootStates.value(root.uuid) !=
             PrivacyRootRuntimeState::VerifiedAvailable)
    {
        switch (d->rootStates.value(root.uuid))
        {
            case PrivacyRootRuntimeState::Offline:
                d->report.offlineRootCount = qMax(0, d->report.offlineRootCount - 1);
                break;

            case PrivacyRootRuntimeState::IdentityMismatch:
                d->report.mismatchedRootCount =
                    qMax(0, d->report.mismatchedRootCount - 1);
                break;

            case PrivacyRootRuntimeState::Unknown:
            case PrivacyRootRuntimeState::Recovering:
                d->report.recoveringRootCount =
                    qMax(0, d->report.recoveringRootCount - 1);
                break;

            case PrivacyRootRuntimeState::VerifiedAvailable:
                break;
        }

        d->setRootState(root.uuid, PrivacyRootRuntimeState::VerifiedAvailable);
        PrivacyRootIntegritySummary summary = d->rootSummaries.value(root.uuid);
        summary.rootUuid = root.uuid;
        summary.state = PrivacyRootRuntimeState::VerifiedAvailable;
        summary.identityMismatch = false;
        d->rootSummaries.insert(root.uuid, summary);
        ++d->report.verifiedRootCount;

        for (PrivacyRootIntegritySummary& reported : d->report.roots)
        {
            if (reported.rootUuid == root.uuid)
            {
                reported = summary;
            }
        }
    }

    d->report.state = ((d->report.offlineRootCount == 0) &&
                       (d->report.mismatchedRootCount == 0) &&
                       (d->report.recoveringRootCount == 0) &&
                       (d->report.unresolvedTransactionCount == 0))
                    ? PrivacyStartupState::Ready
                    : PrivacyStartupState::Degraded;

    return true;
}

bool PrivacyRuntimeCoordinator::publishProtectedItem(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets)
{
    QWriteLocker locker(&d->lock);
    ValidatedProtectedItemFacts facts;

    if (!d->initialized ||
        !validateProtectedItemFacts(item, container, assets, d->snapshot,
                                    d->rootStates, d->conflictingRootUuids,
                                    &facts) ||
        d->items.contains(item.imageId) ||
        d->imageIdsByItemUuid.contains(item.uuid) ||
        d->conflictingItemUuids.contains(item.uuid))
    {
        return false;
    }

    for (const PrivacyItem& existing : std::as_const(d->snapshot.items))
    {
        if ((existing.imageId == item.imageId) || (existing.uuid == item.uuid))
        {
            return false;
        }
    }

    for (const PrivacyContainer& existing : std::as_const(d->snapshot.containers))
    {
        if ((existing.uuid == container.uuid) || (existing.itemUuid == item.uuid))
        {
            return false;
        }
    }

    for (const PrivacyAsset& existing : std::as_const(d->snapshot.assets))
    {
        if (existing.itemUuid == item.uuid)
        {
            return false;
        }
    }

    for (const PrivacyDerivative& existing : std::as_const(d->snapshot.derivatives))
    {
        if (existing.itemUuid == item.uuid)
        {
            return false;
        }
    }

    if (!d->service.addItem(item))
    {
        return false;
    }

    Private::ItemRuntime runtime;
    runtime.item                  = item;
    runtime.publicRootUuid        = facts.publicRootUuid;
    runtime.publicRelativePath    = facts.publicRelativePath;
    runtime.originalRootUuid      = facts.originalRootUuid;
    runtime.expectedProxySize     = facts.expectedProxySize;
    runtime.originalInspectable  = facts.originalInspectable;

    d->snapshot.items << item;
    d->snapshot.containers << container;
    d->snapshot.assets << assets;
    d->items.insert(item.imageId, runtime);
    d->imageIdsByItemUuid.insert(item.uuid, item.imageId);

    for (const QString& rootUuid : std::as_const(facts.rootUuids))
    {
        d->proxyIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->displayProxyIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->exposedOriginalItemsByRoot[rootUuid].remove(item.uuid);
        d->originalIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->refreshDisplayProxyIssueCount(rootUuid);
    }

    d->refreshProtectedRootFacts(facts.rootUuids);

    return true;
}

bool PrivacyRuntimeCoordinator::publishProtectedItemForProtectRecovery(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets,
    const PrivacyTransaction& completedTransaction)
{
    QWriteLocker locker(&d->lock);
    ValidatedProtectedItemFacts facts;

    if (!d->initialized ||
        (completedTransaction.type != PrivacyTransactionType::ProtectItem) ||
        (completedTransaction.state != PrivacyTransactionState::Complete) ||
        (completedTransaction.generation != 2) ||
        (completedTransaction.itemUuid != item.uuid) ||
        (completedTransaction.categoryUuid != item.categoryUuid) ||
        !validateProtectedItemFacts(item, container, assets, d->snapshot,
                                    d->rootStates, d->conflictingRootUuids,
                                    &facts, true) ||
        !d->conflictingRootUuids.isEmpty() ||
        (d->conflictingItemUuids.size() != 1) ||
        !d->conflictingItemUuids.contains(item.uuid) ||
        d->compatibilityExposedItems.contains(item.imageId))
    {
        return false;
    }

    const PrivacyTransaction* activeProtect = nullptr;
    int activeProtectCount = 0;

    for (const PrivacyTransaction& transaction :
         std::as_const(d->snapshot.transactions))
    {
        if (transaction.uuid == completedTransaction.uuid)
        {
            activeProtect = &transaction;
            ++activeProtectCount;
        }
    }

    // Prepared Protect retains exact proxy bytes for cold staging. The
    // completed transaction deliberately retains only its compact journal
    // identity, so both payloads must exist but are not byte-identical.
    if ((activeProtectCount != 1) || !activeProtect ||
        !activeProtect->isActive() ||
        (activeProtect->type != PrivacyTransactionType::ProtectItem) ||
        (activeProtect->itemUuid != item.uuid) ||
        (activeProtect->categoryUuid != item.categoryUuid) ||
        (((activeProtect->state != PrivacyTransactionState::Created) ||
          (activeProtect->generation != 0)) &&
         ((activeProtect->state != PrivacyTransactionState::Prepared) ||
          (activeProtect->generation != 1))) ||
        (activeProtect->payloadFormatVersion !=
         completedTransaction.payloadFormatVersion) ||
        ((activeProtect->state == PrivacyTransactionState::Prepared) &&
         (activeProtect->payloadData.isEmpty() ||
          completedTransaction.payloadData.isEmpty())) ||
        (activeProtect->createdAt != completedTransaction.createdAt) ||
        (activeProtect->fromCredentialGeneration !=
         completedTransaction.fromCredentialGeneration) ||
        (activeProtect->toCredentialGeneration !=
         completedTransaction.toCredentialGeneration))
    {
        return false;
    }

    int partialItemIndex = -1;
    PrivacyItem partialItem;

    for (int i = 0 ; i < d->snapshot.items.size() ; ++i)
    {
        const PrivacyItem& candidate = d->snapshot.items.at(i);

        if ((candidate.imageId == item.imageId) || (candidate.uuid == item.uuid))
        {
            if (partialItemIndex >= 0)
            {
                return false;
            }

            partialItemIndex = i;
            partialItem = candidate;
        }
    }

    const auto runtimeIt = d->items.find(item.imageId);

    if ((partialItemIndex < 0) || !partialItem.isValid() ||
        (partialItem.imageId != item.imageId) ||
        (partialItem.uuid != item.uuid) ||
        (partialItem.categoryUuid != item.categoryUuid) ||
        (partialItem.originalHash != item.originalHash) ||
        (partialItem.originalSize != item.originalSize) ||
        (partialItem.originalWidth != item.originalWidth) ||
        (partialItem.originalHeight != item.originalHeight) ||
        (partialItem.originalCreationDate != item.originalCreationDate) ||
        !partialItem.expectedProxyHash.isEmpty() ||
        (partialItem.expectedProxySize != -1) ||
        (partialItem.presentationVersion != item.presentationVersion) ||
        (partialItem.generation != item.generation) ||
        (partialItem.transactionState !=
         static_cast<int>(PrivacyTransactionState::Created)) ||
        (runtimeIt == d->items.end()) || !runtimeIt->mappingConflict ||
        !sameItem(runtimeIt->item, partialItem) ||
        (d->imageIdsByItemUuid.value(item.uuid, -1) != item.imageId))
    {
        return false;
    }

    for (const PrivacyContainer& existing : std::as_const(d->snapshot.containers))
    {
        if ((existing.uuid == container.uuid) ||
            (existing.itemUuid == item.uuid))
        {
            return false;
        }
    }

    for (const PrivacyAsset& existing : std::as_const(d->snapshot.assets))
    {
        if (existing.itemUuid == item.uuid)
        {
            return false;
        }
    }

    for (const PrivacyDerivative& derivative :
         std::as_const(d->snapshot.derivatives))
    {
        if (derivative.itemUuid == item.uuid)
        {
            return false;
        }
    }

    for (const QString& rootUuid : std::as_const(facts.rootUuids))
    {
        const PrivacyRootIntegritySummary summary =
            d->rootSummaries.value(rootUuid);
        int activeTransactionCount = 0;
        bool exactProtectAffectsRoot = false;

        if ((d->rootStates.value(rootUuid,
                                 PrivacyRootRuntimeState::Unknown) !=
             PrivacyRootRuntimeState::Recovering) ||
            summary.identityMismatch ||
            (summary.unresolvedTransactionCount != 1))
        {
            return false;
        }

        for (const PrivacyTransaction& transaction :
             std::as_const(d->snapshot.transactions))
        {
            if (!transaction.isActive())
            {
                continue;
            }

            QList<PrivacyTransactionJournal> journals;

            for (const PrivacyTransactionJournal& journal :
                 std::as_const(d->snapshot.transactionJournals))
            {
                if (journal.transactionUuid == transaction.uuid)
                {
                    journals << journal;
                }
            }

            if (!transactionAffectsRoot(transaction, journals,
                                        d->snapshot, rootUuid))
            {
                continue;
            }

            ++activeTransactionCount;
            exactProtectAffectsRoot = exactProtectAffectsRoot ||
                (transaction.uuid == completedTransaction.uuid);
        }

        if ((activeTransactionCount != 1) || !exactProtectAffectsRoot)
        {
            return false;
        }
    }

    PrivacyRepositorySnapshot prospective = d->snapshot;
    prospective.items[partialItemIndex] = item;
    prospective.containers << container;
    prospective.assets << assets;

    for (PrivacyTransaction& transaction : prospective.transactions)
    {
        if (transaction.uuid == completedTransaction.uuid)
        {
            transaction = completedTransaction;
        }
    }

    QHash<QString, PrivacyRootInspectionResult> inspections;

    for (const QString& rootUuid : std::as_const(facts.rootUuids))
    {
        const PrivacyStorageRoot* exactRoot = nullptr;
        int rootCount = 0;

        for (const PrivacyStorageRoot& root : prospective.storageRoots)
        {
            if (root.uuid == rootUuid)
            {
                exactRoot = &root;
                ++rootCount;
            }
        }

        if ((rootCount != 1) || !exactRoot || !d->rootVerifier ||
            (d->rootVerifier->verify(*exactRoot) !=
             PrivacyRootRuntimeState::VerifiedAvailable) ||
            !d->integrityInspector)
        {
            return false;
        }

        const PrivacyRootInspectionResult inspection =
            d->integrityInspector->inspect(*exactRoot, prospective);

        if ((inspection.disposition != PrivacyIntegrityDisposition::Verified) ||
            !inspection.proxyIssueItemUuids.isEmpty() ||
            !inspection.originalIssueItemUuids.isEmpty())
        {
            return false;
        }

        inspections.insert(rootUuid, inspection);
    }

    d->snapshot = prospective;
    runtimeIt->item = item;
    runtimeIt->publicRootUuid = facts.publicRootUuid;
    runtimeIt->publicRelativePath = facts.publicRelativePath;
    runtimeIt->originalRootUuid = facts.originalRootUuid;
    runtimeIt->expectedProxySize = facts.expectedProxySize;
    runtimeIt->originalInspectable = facts.originalInspectable;
    runtimeIt->mappingConflict = false;
    d->conflictingItemUuids.remove(item.uuid);
    d->hasUnassignedProtectedItems = false;

    for (const QString& rootUuid : std::as_const(facts.rootUuids))
    {
        d->setRootState(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
        PrivacyRootIntegritySummary summary = inspections.value(rootUuid).summary;
        summary.rootUuid = rootUuid;
        summary.state = PrivacyRootRuntimeState::VerifiedAvailable;
        summary.protectedItemCount = itemUuidsForRoot(d->snapshot, rootUuid).size();
        summary.unresolvedTransactionCount = 0;
        summary.identityMismatch = false;
        d->rootSummaries.insert(rootUuid, summary);
        d->proxyIssueItemsByRoot.insert(
            rootUuid, inspections.value(rootUuid).proxyIssueItemUuids);
        d->displayProxyIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->exposedOriginalItemsByRoot[rootUuid].remove(item.uuid);
        d->originalIssueItemsByRoot.insert(
            rootUuid, inspections.value(rootUuid).originalIssueItemUuids);
        d->refreshDisplayProxyIssueCount(rootUuid);
    }

    d->refreshProtectedRootFacts(facts.rootUuids);
    d->report.unresolvedTransactionCount = 0;
    d->report.verifiedRootCount = 0;
    d->report.offlineRootCount = 0;
    d->report.mismatchedRootCount = 0;
    d->report.recoveringRootCount = 0;
    d->report.roots.clear();

    for (auto it = d->rootStates.constBegin() ; it != d->rootStates.constEnd() ; ++it)
    {
        switch (it.value())
        {
            case PrivacyRootRuntimeState::VerifiedAvailable:
                ++d->report.verifiedRootCount;
                break;
            case PrivacyRootRuntimeState::Offline:
                ++d->report.offlineRootCount;
                break;
            case PrivacyRootRuntimeState::IdentityMismatch:
                ++d->report.mismatchedRootCount;
                break;
            case PrivacyRootRuntimeState::Unknown:
            case PrivacyRootRuntimeState::Recovering:
                ++d->report.recoveringRootCount;
                break;
        }

        d->report.roots << d->rootSummaries.value(it.key());
    }

    for (const PrivacyTransaction& transaction :
         std::as_const(d->snapshot.transactions))
    {
        if (transaction.isActive())
        {
            ++d->report.unresolvedTransactionCount;
        }
    }

    d->report.state = ((d->report.offlineRootCount == 0) &&
                       (d->report.mismatchedRootCount == 0) &&
                       (d->report.recoveringRootCount == 0) &&
                       (d->report.unresolvedTransactionCount == 0))
                    ? PrivacyStartupState::Ready
                    : PrivacyStartupState::Degraded;
    return true;
}

bool PrivacyRuntimeCoordinator::hasProtectedItem(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets) const
{
    QReadLocker locker(&d->lock);
    ValidatedProtectedItemFacts facts;

    if (!d->initialized ||
        !validateProtectedItemFacts(item, container, assets, d->snapshot,
                                    d->rootStates, d->conflictingRootUuids,
                                    &facts) ||
        d->conflictingItemUuids.contains(item.uuid))
    {
        return false;
    }

    const auto runtimeIt = d->items.constFind(item.imageId);

    if ((runtimeIt == d->items.constEnd()) || runtimeIt->mappingConflict ||
        !sameItem(runtimeIt->item, item) ||
        (runtimeIt->publicRootUuid != facts.publicRootUuid) ||
        (runtimeIt->publicRelativePath != facts.publicRelativePath) ||
        (runtimeIt->originalRootUuid != facts.originalRootUuid) ||
        (runtimeIt->expectedProxySize != facts.expectedProxySize) ||
        (runtimeIt->originalInspectable != facts.originalInspectable) ||
        (d->imageIdsByItemUuid.value(item.uuid, -1) != item.imageId))
    {
        return false;
    }

    int matchingItemCount = 0;

    for (const PrivacyItem& existing : std::as_const(d->snapshot.items))
    {
        if ((existing.imageId == item.imageId) || (existing.uuid == item.uuid))
        {
            if (!sameItem(existing, item))
            {
                return false;
            }

            ++matchingItemCount;
        }
    }

    int matchingContainerCount = 0;

    for (const PrivacyContainer& existing : std::as_const(d->snapshot.containers))
    {
        if ((existing.uuid == container.uuid) || (existing.itemUuid == item.uuid))
        {
            if (!sameContainer(existing, container))
            {
                return false;
            }

            ++matchingContainerCount;
        }
    }

    QHash<QString, PrivacyAsset> suppliedAssets;

    for (const PrivacyAsset& asset : assets)
    {
        suppliedAssets.insert(assetIdentity(asset), asset);
    }

    int matchingAssetCount = 0;

    for (const PrivacyAsset& existing : std::as_const(d->snapshot.assets))
    {
        if (existing.itemUuid != item.uuid)
        {
            continue;
        }

        const auto suppliedIt = suppliedAssets.constFind(assetIdentity(existing));

        if ((suppliedIt == suppliedAssets.constEnd()) ||
            !sameAsset(existing, suppliedIt.value()))
        {
            return false;
        }

        ++matchingAssetCount;
    }

    PrivacyServiceItemState sessionState;

    return ((matchingItemCount == 1) &&
            (matchingContainerCount == 1) &&
            (matchingAssetCount == suppliedAssets.size()) &&
            d->service.sessionStateForItem(item.imageId, &sessionState) &&
            sessionState.protectedItem &&
            (sessionState.categoryUuid == item.categoryUuid) &&
            (sessionState.itemGeneration == item.generation));
}

bool PrivacyRuntimeCoordinator::removeProtectedItem(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets)
{
    return removeProtectedItemInternal(item, container, assets, {});
}

bool PrivacyRuntimeCoordinator::removeProtectedItemForUnprotectRecovery(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets,
    const QString& transactionUuid)
{
    return removeProtectedItemInternal(item, container, assets,
                                       transactionUuid);
}

bool PrivacyRuntimeCoordinator::removeProtectedItemInternal(
    const PrivacyItem& item,
    const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets,
    const QString& recoveryTransactionUuid)
{
    QWriteLocker locker(&d->lock);
    ValidatedProtectedItemFacts facts;
    const bool recoveringUnprotect = !recoveryTransactionUuid.isEmpty();

    if (!d->initialized ||
        !validateProtectedItemFacts(item, container, assets, d->snapshot,
                                    d->rootStates, d->conflictingRootUuids,
                                    &facts, recoveringUnprotect) ||
        d->conflictingItemUuids.contains(item.uuid) ||
        d->compatibilityExposedItems.contains(item.imageId))
    {
        return false;
    }

    if (recoveringUnprotect)
    {
        const PrivacyTransaction* recoveryTransaction = nullptr;
        int matchingTransactionCount = 0;

        for (const PrivacyTransaction& transaction :
             std::as_const(d->snapshot.transactions))
        {
            if (transaction.uuid == recoveryTransactionUuid)
            {
                recoveryTransaction = &transaction;
                ++matchingTransactionCount;
            }
        }

        if ((matchingTransactionCount != 1) || !recoveryTransaction ||
            !recoveryTransaction->isActive() ||
            (recoveryTransaction->type != PrivacyTransactionType::UnprotectItem) ||
            (recoveryTransaction->itemUuid != item.uuid) ||
            (recoveryTransaction->categoryUuid != item.categoryUuid))
        {
            return false;
        }

        for (const QString& rootUuid : std::as_const(facts.rootUuids))
        {
            const PrivacyRootIntegritySummary summary =
                d->rootSummaries.value(rootUuid);
            int activeTransactionCount = 0;
            bool recoveryTransactionAffectsRoot = false;

            if ((d->rootStates.value(rootUuid,
                                     PrivacyRootRuntimeState::Unknown) !=
                 PrivacyRootRuntimeState::Recovering) ||
                summary.identityMismatch ||
                (summary.unresolvedTransactionCount != 1))
            {
                return false;
            }

            for (const PrivacyTransaction& transaction :
                 std::as_const(d->snapshot.transactions))
            {
                if (!transaction.isActive())
                {
                    continue;
                }

                QList<PrivacyTransactionJournal> journals;

                for (const PrivacyTransactionJournal& journal :
                     std::as_const(d->snapshot.transactionJournals))
                {
                    if (journal.transactionUuid == transaction.uuid)
                    {
                        journals << journal;
                    }
                }

                if (!transactionAffectsRoot(transaction, journals,
                                            d->snapshot, rootUuid))
                {
                    continue;
                }

                ++activeTransactionCount;
                recoveryTransactionAffectsRoot =
                    recoveryTransactionAffectsRoot ||
                    (transaction.uuid == recoveryTransactionUuid);
            }

            if ((activeTransactionCount != 1) ||
                !recoveryTransactionAffectsRoot)
            {
                return false;
            }
        }
    }

    const auto runtimeIt = d->items.constFind(item.imageId);

    if ((runtimeIt == d->items.constEnd()) || runtimeIt->mappingConflict ||
        !sameItem(runtimeIt->item, item) ||
        (runtimeIt->publicRootUuid != facts.publicRootUuid) ||
        (runtimeIt->publicRelativePath != facts.publicRelativePath) ||
        (runtimeIt->originalRootUuid != facts.originalRootUuid) ||
        (runtimeIt->expectedProxySize != facts.expectedProxySize) ||
        (runtimeIt->originalInspectable != facts.originalInspectable) ||
        (d->imageIdsByItemUuid.value(item.uuid, -1) != item.imageId))
    {
        return false;
    }

    int itemIndex = -1;

    for (int i = 0 ; i < d->snapshot.items.size() ; ++i)
    {
        const PrivacyItem& existing = d->snapshot.items.at(i);

        if ((existing.imageId == item.imageId) || (existing.uuid == item.uuid))
        {
            if ((itemIndex >= 0) || !sameItem(existing, item))
            {
                return false;
            }

            itemIndex = i;
        }
    }

    int containerIndex = -1;

    for (int i = 0 ; i < d->snapshot.containers.size() ; ++i)
    {
        const PrivacyContainer& existing = d->snapshot.containers.at(i);

        if ((existing.uuid == container.uuid) || (existing.itemUuid == item.uuid))
        {
            if ((containerIndex >= 0) || !sameContainer(existing, container))
            {
                return false;
            }

            containerIndex = i;
        }
    }

    QHash<QString, PrivacyAsset> suppliedAssets;

    for (const PrivacyAsset& asset : assets)
    {
        suppliedAssets.insert(assetIdentity(asset), asset);
    }

    QList<int> assetIndexes;

    for (int i = 0 ; i < d->snapshot.assets.size() ; ++i)
    {
        const PrivacyAsset& existing = d->snapshot.assets.at(i);

        if (existing.itemUuid != item.uuid)
        {
            continue;
        }

        const auto suppliedIt = suppliedAssets.constFind(assetIdentity(existing));

        if ((suppliedIt == suppliedAssets.constEnd()) ||
            !sameAsset(existing, suppliedIt.value()))
        {
            return false;
        }

        assetIndexes << i;
    }

    if ((itemIndex < 0) || (containerIndex < 0) ||
        (assetIndexes.size() != suppliedAssets.size()))
    {
        return false;
    }

    for (const PrivacyDerivative& derivative : std::as_const(d->snapshot.derivatives))
    {
        if (derivative.itemUuid == item.uuid)
        {
            // Derivative deletion requires its own exact durable input. Do not
            // leave a dangling item fact or silently discard an unspecified one.
            return false;
        }
    }

    if (!d->service.removeItem(item))
    {
        return false;
    }

    for (auto it = assetIndexes.crbegin() ; it != assetIndexes.crend() ; ++it)
    {
        d->snapshot.assets.removeAt(*it);
    }

    d->snapshot.containers.removeAt(containerIndex);
    d->snapshot.items.removeAt(itemIndex);
    d->items.remove(item.imageId);
    d->imageIdsByItemUuid.remove(item.uuid);
    d->conflictingItemUuids.remove(item.uuid);
    d->compatibilityExposedItems.remove(item.imageId);

    for (const QString& rootUuid : std::as_const(facts.rootUuids))
    {
        d->proxyIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->displayProxyIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->exposedOriginalItemsByRoot[rootUuid].remove(item.uuid);
        d->originalIssueItemsByRoot[rootUuid].remove(item.uuid);
        d->refreshDisplayProxyIssueCount(rootUuid);
    }

    d->refreshProtectedRootFacts(facts.rootUuids);

    return true;
}

quint64 PrivacyRuntimeCoordinator::categoryEpoch(const QString& categoryUuid) const
{
    QReadLocker locker(&d->lock);

    return d->initialized ? d->service.categoryEpoch(categoryUuid) : 0;
}

bool PrivacyRuntimeCoordinator::setCategoryTagVisibilityMode(
    const QString& categoryUuid,
    PrivacyTagVisibilityMode mode,
    bool categoryAuthenticationVerified)
{
    {
        QReadLocker locker(&d->lock);

        if (!d->initialized)
        {
            return false;
        }
    }

    return d->service.setCategoryTagVisibilityMode(categoryUuid,
                                                   mode,
                                                   categoryAuthenticationVerified);
}

void PrivacyRuntimeCoordinator::lockAllCategories()
{
    {
        QReadLocker locker(&d->lock);

        if (!d->initialized)
        {
            return;
        }
    }

    d->service.lockAll();
}

bool PrivacyRuntimeCoordinator::compareAndSetItemGeneration(
    qlonglong imageId,
    qlonglong expectedGeneration,
    qlonglong newGeneration)
{
    QWriteLocker locker(&d->lock);
    auto itemIt = d->items.find(imageId);

    if (!d->initialized || (itemIt == d->items.end()) ||
        itemIt->mappingConflict ||
        (itemIt->item.generation != expectedGeneration) ||
        !d->service.compareAndSetItemGeneration(imageId,
                                                expectedGeneration,
                                                newGeneration))
    {
        return false;
    }

    itemIt->item.generation = newGeneration;

    for (PrivacyItem& snapshotItem : d->snapshot.items)
    {
        if ((snapshotItem.imageId == imageId) &&
            (snapshotItem.uuid == itemIt->item.uuid))
        {
            snapshotItem.generation = newGeneration;
        }
    }

    return true;
}

bool PrivacyRuntimeCoordinator::mayAccessManualTags(qlonglong imageId) const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized || (imageId <= 0))
    {
        return false;
    }

    const auto itemIt = d->items.constFind(imageId);

    if ((itemIt != d->items.constEnd()) &&
        (itemIt->mappingConflict ||
         d->conflictingItemUuids.contains(itemIt->item.uuid)))
    {
        return false;
    }

    return d->service.mayAccessManualTags(imageId);
}

QSet<QString> PrivacyRuntimeCoordinator::visibleManualTagCategoryUuids() const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized || !d->conflictingItemUuids.isEmpty())
    {
        return QSet<QString>();
    }

    return d->service.visibleManualTagCategoryUuids();
}

PrivacyAnalysisDisposition PrivacyRuntimeCoordinator::analysisDisposition(
    qlonglong imageId) const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized || (imageId <= 0))
    {
        return PrivacyAnalysisDisposition::Unavailable;
    }

    const auto itemIt = d->items.constFind(imageId);

    if ((itemIt != d->items.constEnd()) &&
        (itemIt->mappingConflict ||
         d->conflictingItemUuids.contains(itemIt->item.uuid)))
    {
        return PrivacyAnalysisDisposition::Unavailable;
    }

    PrivacyServiceItemState state;

    if (!d->service.sessionStateForItem(imageId, &state))
    {
        return PrivacyAnalysisDisposition::Unavailable;
    }

    return state.protectedItem
         ? PrivacyAnalysisDisposition::ProtectedExcluded
         : PrivacyAnalysisDisposition::Allowed;
}

bool PrivacyRuntimeCoordinator::stateForItem(qlonglong imageId,
                                             PrivacyActionItemState* state) const
{
    if (!state || (imageId <= 0))
    {
        return false;
    }

    *state = PrivacyActionItemState();
    Private::ItemRuntime runtime;
    PrivacyRootRuntimeState publicRootState = PrivacyRootRuntimeState::Unknown;
    PrivacyRootRuntimeState originalRootState = PrivacyRootRuntimeState::Unknown;
    bool proxyReady = false;
    bool originalReady = false;
    bool unresolved = false;
    bool hasRuntimeItem = false;

    {
        QReadLocker locker(&d->lock);

        if (!d->initialized)
        {
            return false;
        }

        const auto itemIt = d->items.constFind(imageId);

        if (itemIt == d->items.constEnd())
        {
            hasRuntimeItem = false;
        }
        else
        {
            hasRuntimeItem = true;
            runtime = itemIt.value();
        }

        if (hasRuntimeItem &&
            (runtime.mappingConflict ||
             d->conflictingItemUuids.contains(runtime.item.uuid)))
        {
            return false;
        }

        if (hasRuntimeItem)
        {
            publicRootState = d->rootStates.value(runtime.publicRootUuid,
                                                   PrivacyRootRuntimeState::Unknown);
            originalRootState = d->rootStates.value(runtime.originalRootUuid,
                                                     PrivacyRootRuntimeState::Unknown);
            proxyReady = ((runtime.expectedProxySize >= 0) &&
                          (publicRootState == PrivacyRootRuntimeState::VerifiedAvailable) &&
                          !d->proxyIssueItemsByRoot.value(runtime.publicRootUuid).contains(
                              runtime.item.uuid) &&
                          !d->displayProxyIssueItemsByRoot.value(
                              runtime.publicRootUuid).contains(
                              runtime.item.uuid));
            originalReady = (runtime.originalInspectable &&
                             (originalRootState ==
                              PrivacyRootRuntimeState::VerifiedAvailable) &&
                             !d->originalIssueItemsByRoot.value(runtime.originalRootUuid).contains(
                                 runtime.item.uuid));
            unresolved = (d->compatibilityExposedItems.contains(imageId) ||
                          (d->rootSummaries.value(runtime.publicRootUuid)
                               .unresolvedTransactionCount > 0) ||
                          (d->rootSummaries.value(runtime.originalRootUuid)
                               .unresolvedTransactionCount > 0));
        }
    }

    PrivacyServiceItemState sessionState;

    if (!d->service.sessionStateForItem(imageId, &sessionState))
    {
        return false;
    }

    if (!hasRuntimeItem)
    {
        return !sessionState.protectedItem;
    }

    if (!sessionState.protectedItem || sessionState.categoryUuid.isEmpty() ||
        (sessionState.categoryUuid != runtime.item.categoryUuid) ||
        (sessionState.itemGeneration < 0) ||
        (sessionState.access == PrivacyItemAccess::Unprotected))
    {
        return false;
    }

    state->protectedItem       = true;
    state->categoryUuid        = sessionState.categoryUuid;
    state->access              = sessionState.access;
    state->publicRootState     = publicRootState;
    state->originalRootState   = originalRootState;
    state->checkoutRootState   = originalRootState;
    state->proxyReady          = proxyReady;
    state->originalReady       = originalReady;
    // This is source-prerequisite readiness only. Checkout creation still
    // occurs later in the prepared-selection workflow and can fail closed.
    state->checkoutReady       = originalReady;
    state->unresolvedTransaction = unresolved;
    state->itemGeneration      = sessionState.itemGeneration;

    return state->isValid();
}

bool PrivacyRuntimeCoordinator::currentState(
    const QString& itemUuid,
    PrivacyLeaseCurrentState* state) const
{
    if (!state || !isCanonicalUuid(itemUuid))
    {
        return false;
    }

    *state = PrivacyLeaseCurrentState();
    Private::ItemRuntime runtime;
    quint64 publicRootEpoch = 0;
    quint64 originalRootEpoch = 0;
    bool publicReady = false;
    bool originalReady = false;
    bool unresolved = false;

    {
        QReadLocker locker(&d->lock);

        if (!d->initialized || d->conflictingItemUuids.contains(itemUuid))
        {
            return false;
        }

        const qlonglong imageId = d->imageIdsByItemUuid.value(itemUuid, -1);
        const auto itemIt = d->items.constFind(imageId);

        if ((itemIt == d->items.constEnd()) || itemIt->mappingConflict)
        {
            return false;
        }

        runtime = itemIt.value();
        publicRootEpoch = d->rootEpochs.value(runtime.publicRootUuid, 0);
        originalRootEpoch = d->rootEpochs.value(runtime.originalRootUuid, 0);
        publicReady = ((runtime.expectedProxySize >= 0) &&
                       (d->rootStates.value(runtime.publicRootUuid) ==
                        PrivacyRootRuntimeState::VerifiedAvailable) &&
                       !d->proxyIssueItemsByRoot.value(runtime.publicRootUuid).contains(
                           itemUuid) &&
                       !d->displayProxyIssueItemsByRoot.value(
                           runtime.publicRootUuid).contains(
                           itemUuid));
        originalReady = (runtime.originalInspectable &&
                         (d->rootStates.value(runtime.originalRootUuid) ==
                          PrivacyRootRuntimeState::VerifiedAvailable) &&
                         !d->originalIssueItemsByRoot.value(runtime.originalRootUuid).contains(
                             itemUuid));
        unresolved = (d->compatibilityExposedItems.contains(imageId) ||
                      (d->rootSummaries.value(runtime.publicRootUuid)
                           .unresolvedTransactionCount > 0) ||
                      (d->rootSummaries.value(runtime.originalRootUuid)
                           .unresolvedTransactionCount > 0));
    }

    const qlonglong imageId = runtime.item.imageId;
    PrivacyServiceItemState sessionState;

    if (!d->service.sessionStateForItem(imageId, &sessionState) ||
        !sessionState.protectedItem ||
        (sessionState.categoryUuid != runtime.item.categoryUuid) ||
        (sessionState.itemGeneration < 0) ||
        (sessionState.categoryEpoch == 0))
    {
        return false;
    }

    {
        QReadLocker locker(&d->lock);
        const auto currentItem = d->items.constFind(imageId);

        if (!d->initialized || (currentItem == d->items.constEnd()) ||
            currentItem->mappingConflict ||
            (currentItem->item.uuid != itemUuid) ||
            (currentItem->item.generation != sessionState.itemGeneration) ||
            (d->rootEpochs.value(runtime.publicRootUuid, 0) != publicRootEpoch) ||
            (d->rootEpochs.value(runtime.originalRootUuid, 0) != originalRootEpoch))
        {
            return false;
        }
    }

    state->itemUuid             = itemUuid;
    state->itemGeneration       = sessionState.itemGeneration;
    state->categoryEpoch        = sessionState.categoryEpoch;
    state->publicRootEpoch      = publicRootEpoch;
    state->storeRootEpoch       = originalRootEpoch;
    state->categoryUnlocked     =
        (sessionState.access == PrivacyItemAccess::Unlocked);
    state->publicRootAvailable  = publicReady;
    state->storeRootAvailable   = originalReady;
    state->unresolvedTransaction = unresolved;

    return state->isValid();
}

PrivacyRootRecoveryResult PrivacyRuntimeCoordinator::registerAlbumRoot(
    const PrivacyStorageRoot& root)
{
    if (!root.isValid() || (root.kind != PrivacyStorageRootKind::AlbumRoot))
    {
        return PrivacyRootRecoveryResult::UnknownRoot;
    }

    QString recoveryUuid;

    {
        QWriteLocker locker(&d->lock);

        if (!d->initialized)
        {
            return PrivacyRootRecoveryResult::UnknownRoot;
        }

        const PrivacyStorageRoot* existingByUuid = nullptr;
        const PrivacyStorageRoot* existingByAlbumRoot = nullptr;

        for (const PrivacyStorageRoot& existing : std::as_const(d->snapshot.storageRoots))
        {
            if (existing.uuid == root.uuid)
            {
                existingByUuid = &existing;
            }

            if ((existing.kind == PrivacyStorageRootKind::AlbumRoot) &&
                (existing.albumRootId == root.albumRootId))
            {
                existingByAlbumRoot = &existing;
            }
        }

        if (existingByUuid || existingByAlbumRoot)
        {
            if (!existingByUuid || !existingByAlbumRoot ||
                (existingByUuid != existingByAlbumRoot) ||
                (existingByUuid->identityVersion != root.identityVersion) ||
                (existingByUuid->identityData != root.identityData))
            {
                return PrivacyRootRecoveryResult::Deferred;
            }

            recoveryUuid = existingByUuid->uuid;
        }
        else
        {
            d->snapshot.storageRoots << root;
            d->albumRootUuids.insert(root.albumRootId, root.uuid);
            d->setRootState(root.uuid, PrivacyRootRuntimeState::Recovering);

            PrivacyRootIntegritySummary summary;
            summary.rootUuid = root.uuid;
            summary.state    = PrivacyRootRuntimeState::Recovering;
            d->rootSummaries.insert(root.uuid, summary);
            d->proxyIssueItemsByRoot.insert(root.uuid, QSet<QString>());
            d->displayProxyIssueItemsByRoot.insert(root.uuid, QSet<QString>());
            d->exposedOriginalItemsByRoot.insert(root.uuid, QSet<QString>());
            d->originalIssueItemsByRoot.insert(root.uuid, QSet<QString>());
            recoveryUuid = root.uuid;
        }
    }

    return recoverRoot(recoveryUuid);
}

bool PrivacyRuntimeCoordinator::unregisterUnreferencedAlbumRoot(
    const QString& rootUuid)
{
    if (!isCanonicalUuid(rootUuid))
    {
        return false;
    }

    QWriteLocker locker(&d->lock);
    int rootIndex = -1;

    for (int i = 0 ; i < d->snapshot.storageRoots.size() ; ++i)
    {
        const PrivacyStorageRoot& root = d->snapshot.storageRoots.at(i);

        if (root.uuid == rootUuid)
        {
            if (root.kind != PrivacyStorageRootKind::AlbumRoot)
            {
                return false;
            }

            rootIndex = i;
            break;
        }
    }

    if (!d->initialized)
    {
        return false;
    }

    if (rootIndex < 0)
    {
        return true;
    }

    for (const PrivacyAsset& asset : std::as_const(d->snapshot.assets))
    {
        if (asset.publicRootUuid == rootUuid)
        {
            return false;
        }
    }

    for (const PrivacyContainer& container : std::as_const(d->snapshot.containers))
    {
        if (container.rootUuid == rootUuid)
        {
            return false;
        }
    }

    for (const PrivacyStore& store : std::as_const(d->snapshot.stores))
    {
        if (store.rootUuid == rootUuid)
        {
            return false;
        }
    }

    for (const PrivacyTransactionJournal& journal :
         std::as_const(d->snapshot.transactionJournals))
    {
        if (journal.rootUuid == rootUuid)
        {
            return false;
        }
    }

    const PrivacyStorageRoot root = d->snapshot.storageRoots.takeAt(rootIndex);

    if (d->albumRootUuids.value(root.albumRootId) == rootUuid)
    {
        d->albumRootUuids.remove(root.albumRootId);
    }

    const PrivacyRootRuntimeState previousState = d->rootStates.take(rootUuid);
    d->rootEpochs.remove(rootUuid);
    d->rootSummaries.remove(rootUuid);
    d->proxyIssueItemsByRoot.remove(rootUuid);
    d->displayProxyIssueItemsByRoot.remove(rootUuid);
    d->exposedOriginalItemsByRoot.remove(rootUuid);
    d->originalIssueItemsByRoot.remove(rootUuid);
    d->conflictingRootUuids.remove(rootUuid);

    for (int i = d->report.roots.size() - 1 ; i >= 0 ; --i)
    {
        if (d->report.roots.at(i).rootUuid == rootUuid)
        {
            d->report.roots.removeAt(i);
        }
    }

    switch (previousState)
    {
        case PrivacyRootRuntimeState::VerifiedAvailable:
            d->report.verifiedRootCount = qMax(0, d->report.verifiedRootCount - 1);
            break;

        case PrivacyRootRuntimeState::Offline:
            d->report.offlineRootCount = qMax(0, d->report.offlineRootCount - 1);
            break;

        case PrivacyRootRuntimeState::IdentityMismatch:
            d->report.mismatchedRootCount = qMax(0, d->report.mismatchedRootCount - 1);
            break;

        case PrivacyRootRuntimeState::Unknown:
        case PrivacyRootRuntimeState::Recovering:
            d->report.recoveringRootCount = qMax(0, d->report.recoveringRootCount - 1);
            break;
    }

    return true;
}

bool PrivacyRuntimeCoordinator::beginRootRecovery(const QString& rootUuid)
{
    if (!isCanonicalUuid(rootUuid))
    {
        return false;
    }

    QWriteLocker locker(&d->lock);

    if (!d->rootStates.contains(rootUuid))
    {
        return false;
    }

    d->setRootState(rootUuid, PrivacyRootRuntimeState::Recovering);
    PrivacyRootIntegritySummary summary = d->rootSummaries.value(rootUuid);
    summary.rootUuid = rootUuid;
    summary.state = PrivacyRootRuntimeState::Recovering;
    summary.identityMismatch = false;
    clearArtifactCounts(&summary);
    d->rootSummaries.insert(rootUuid, summary);

    return true;
}

bool PrivacyRuntimeCoordinator::publishRootState(const QString& rootUuid,
                                                 PrivacyRootRuntimeState state)
{
    if (!isCanonicalUuid(rootUuid) || !isValidRootState(state) ||
        (state == PrivacyRootRuntimeState::Unknown))
    {
        return false;
    }

    QWriteLocker locker(&d->lock);

    if (!d->rootStates.contains(rootUuid))
    {
        return false;
    }

    d->setRootState(rootUuid, state);
    PrivacyRootIntegritySummary summary = d->rootSummaries.value(rootUuid);
    summary.rootUuid = rootUuid;
    summary.state = state;
    summary.identityMismatch = (state == PrivacyRootRuntimeState::IdentityMismatch);

    if (state != PrivacyRootRuntimeState::VerifiedAvailable)
    {
        clearArtifactCounts(&summary);
    }
    d->rootSummaries.insert(rootUuid, summary);

    return true;
}

bool PrivacyRuntimeCoordinator::publishRootStateIfEpoch(
    const QString& rootUuid,
    quint64 expectedEpoch,
    PrivacyRootRuntimeState state)
{
    if (!isCanonicalUuid(rootUuid) || (expectedEpoch == 0) ||
        !isValidRootState(state) || (state == PrivacyRootRuntimeState::Unknown))
    {
        return false;
    }

    QWriteLocker locker(&d->lock);

    if (!d->rootStates.contains(rootUuid) ||
        (d->rootStates.value(rootUuid) != PrivacyRootRuntimeState::Recovering) ||
        (d->rootEpochs.value(rootUuid) != expectedEpoch))
    {
        return false;
    }

    d->setRootState(rootUuid, state);
    PrivacyRootIntegritySummary summary = d->rootSummaries.value(rootUuid);
    summary.rootUuid = rootUuid;
    summary.state = state;
    summary.identityMismatch = (state == PrivacyRootRuntimeState::IdentityMismatch);

    if (state != PrivacyRootRuntimeState::VerifiedAvailable)
    {
        clearArtifactCounts(&summary);
    }
    d->rootSummaries.insert(rootUuid, summary);

    return true;
}

PrivacyRootRecoveryResult PrivacyRuntimeCoordinator::recoverRoot(const QString& rootUuid)
{
    if (!isCanonicalUuid(rootUuid))
    {
        return PrivacyRootRecoveryResult::UnknownRoot;
    }

    PrivacyStorageRoot root;
    PrivacyRepositorySnapshot snapshot;
    QSharedPointer<const PrivacyRootVerifier> verifier;
    QSharedPointer<const PrivacyTransactionRecovery> recovery;
    QSharedPointer<const PrivacyRootIntegrityInspector> inspector;
    quint64 recoveryEpoch = 0;
    bool rootHasProtectedData = false;
    bool rootHasMappingConflict = false;

    {
        QWriteLocker locker(&d->lock);

        if (!d->initialized || !d->rootStates.contains(rootUuid))
        {
            return PrivacyRootRecoveryResult::UnknownRoot;
        }

        for (const PrivacyStorageRoot& candidate : d->snapshot.storageRoots)
        {
            if (candidate.uuid == rootUuid)
            {
                root = candidate;
                break;
            }
        }

        if (root.uuid.isEmpty())
        {
            return PrivacyRootRecoveryResult::UnknownRoot;
        }

        if (d->conflictingRootUuids.contains(rootUuid))
        {
            d->setRootState(rootUuid, PrivacyRootRuntimeState::Recovering);

            return PrivacyRootRecoveryResult::Deferred;
        }

        d->setRootState(rootUuid, PrivacyRootRuntimeState::Recovering);
        recoveryEpoch = d->rootEpochs.value(rootUuid);
        PrivacyRootIntegritySummary summary = d->rootSummaries.value(rootUuid);
        summary.rootUuid = rootUuid;
        summary.state = PrivacyRootRuntimeState::Recovering;
        summary.identityMismatch = false;
        // Recovery adapters use the published unresolved count as part of
        // their exact single-transaction mutation guard. Recalculate it only
        // after every adapter has returned.
        clearArtifactCounts(&summary);
        d->rootSummaries.insert(rootUuid, summary);
        snapshot = d->snapshot;
        verifier = d->rootVerifier;
        recovery = d->recovery;
        inspector = d->integrityInspector;
        rootHasProtectedData = d->protectedRootUuids.contains(rootUuid);
    }

    const PrivacyRootRuntimeState verifiedState = verifier
                                                ? verifier->verify(root)
                                                : PrivacyRootRuntimeState::IdentityMismatch;

    if ((verifiedState == PrivacyRootRuntimeState::Offline) ||
        (verifiedState == PrivacyRootRuntimeState::IdentityMismatch))
    {
        if (!publishRootStateIfEpoch(rootUuid, recoveryEpoch, verifiedState))
        {
            return PrivacyRootRecoveryResult::StaleEpoch;
        }

        return (verifiedState == PrivacyRootRuntimeState::Offline)
             ? PrivacyRootRecoveryResult::PublishedOffline
             : PrivacyRootRecoveryResult::PublishedIdentityMismatch;
    }

    if (verifiedState != PrivacyRootRuntimeState::VerifiedAvailable)
    {
        return PrivacyRootRecoveryResult::Deferred;
    }

    int unresolvedCount = 0;
    int compatibilityCount = 0;
    QList<PrivacyTransaction> recoveredCompatibilityTransactions;
    QSet<QString> recoveredTransactionUuids;

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if (!transaction.isActive())
        {
            continue;
        }

        QList<PrivacyTransactionJournal> journals;

        for (const PrivacyTransactionJournal& journal : snapshot.transactionJournals)
        {
            if ((journal.transactionUuid == transaction.uuid) &&
                (journal.rootUuid == rootUuid))
            {
                journals << journal;
            }
        }

        if (!transactionAffectsRoot(transaction, journals, snapshot, rootUuid))
        {
            continue;
        }

        const PrivacyRecoveryDisposition disposition = recovery
                                                     ? recovery->recoverRoot(
                                                           root, transaction, journals)
                                                     : PrivacyRecoveryDisposition::Deferred;

        if (disposition != PrivacyRecoveryDisposition::Recovered)
        {
            ++unresolvedCount;

            if ((transaction.type == PrivacyTransactionType::CompatibilityUnlock) ||
                (transaction.type == PrivacyTransactionType::CompatibilityRelock))
            {
                ++compatibilityCount;
            }
        }
        else if ((transaction.type == PrivacyTransactionType::CompatibilityUnlock) ||
                 (transaction.type == PrivacyTransactionType::CompatibilityRelock))
        {
            recoveredCompatibilityTransactions << transaction;
            recoveredTransactionUuids.insert(transaction.uuid);
        }
        else
        {
            recoveredTransactionUuids.insert(transaction.uuid);
        }
    }

    {
        QWriteLocker locker(&d->lock);
        const PrivacyRootRuntimeState currentState = d->rootStates.value(
            rootUuid, PrivacyRootRuntimeState::Unknown);
        QSet<QString> removedActiveTransactionUuids;

        if ((currentState != PrivacyRootRuntimeState::Recovering) &&
            (currentState != PrivacyRootRuntimeState::VerifiedAvailable))
        {
            return PrivacyRootRecoveryResult::StaleEpoch;
        }

        for (int i = d->snapshot.transactions.size() - 1 ; i >= 0 ; --i)
        {
            const PrivacyTransaction& transaction =
                d->snapshot.transactions.at(i);

            if (transaction.isActive() &&
                recoveredTransactionUuids.contains(transaction.uuid))
            {
                removedActiveTransactionUuids.insert(transaction.uuid);
                d->snapshot.transactions.removeAt(i);
            }
        }

        for (int i = d->snapshot.transactionJournals.size() - 1 ; i >= 0 ; --i)
        {
            if (removedActiveTransactionUuids.contains(
                    d->snapshot.transactionJournals.at(i).transactionUuid))
            {
                d->snapshot.transactionJournals.removeAt(i);
            }
        }

        snapshot = d->snapshot;
        rootHasProtectedData = d->protectedRootUuids.contains(rootUuid);
        rootHasMappingConflict = false;

        for (auto itemIt = d->items.constBegin() ;
             itemIt != d->items.constEnd() ; ++itemIt)
        {
            if (itemIt->mappingConflict &&
                ((itemIt->publicRootUuid == rootUuid) ||
                 (itemIt->originalRootUuid == rootUuid)))
            {
                rootHasMappingConflict = true;
                break;
            }
        }

        recoveryEpoch = d->rootEpochs.value(rootUuid);
    }

    if (unresolvedCount > 0)
    {
        QWriteLocker locker(&d->lock);

        if ((d->rootEpochs.value(rootUuid) != recoveryEpoch) ||
            ((d->rootStates.value(rootUuid) !=
              PrivacyRootRuntimeState::Recovering) &&
             (d->rootStates.value(rootUuid) !=
              PrivacyRootRuntimeState::VerifiedAvailable)))
        {
            return PrivacyRootRecoveryResult::StaleEpoch;
        }

        d->setRootState(rootUuid, PrivacyRootRuntimeState::Recovering);
        PrivacyRootIntegritySummary summary = d->rootSummaries.value(rootUuid);
        summary.state = PrivacyRootRuntimeState::Recovering;
        summary.unresolvedTransactionCount = unresolvedCount;
        summary.compatibilityExposureCount = compatibilityCount;
        d->rootSummaries.insert(rootUuid, summary);

        return PrivacyRootRecoveryResult::Deferred;
    }

    if (rootHasMappingConflict)
    {
        return PrivacyRootRecoveryResult::Deferred;
    }

    PrivacyRootInspectionResult inspection;
    inspection.summary.rootUuid = rootUuid;
    inspection.summary.protectedItemCount = itemUuidsForRoot(snapshot, rootUuid).size();

    if (rootHasProtectedData)
    {
        if (!inspector)
        {
            return PrivacyRootRecoveryResult::Deferred;
        }

        inspection = inspector->inspect(root, snapshot);

        if (inspection.disposition != PrivacyIntegrityDisposition::Verified)
        {
            return PrivacyRootRecoveryResult::Deferred;
        }
    }
    else
    {
        inspection.disposition = PrivacyIntegrityDisposition::Verified;
    }

    const PrivacyRootRuntimeState finalVerifiedState = verifier
                                                    ? verifier->verify(root)
                                                    : PrivacyRootRuntimeState::IdentityMismatch;

    if (finalVerifiedState != PrivacyRootRuntimeState::VerifiedAvailable)
    {
        if ((finalVerifiedState == PrivacyRootRuntimeState::Offline) ||
            (finalVerifiedState == PrivacyRootRuntimeState::IdentityMismatch))
        {
            if (!publishRootStateIfEpoch(rootUuid, recoveryEpoch, finalVerifiedState))
            {
                return PrivacyRootRecoveryResult::StaleEpoch;
            }

            return (finalVerifiedState == PrivacyRootRuntimeState::Offline)
                 ? PrivacyRootRecoveryResult::PublishedOffline
                 : PrivacyRootRecoveryResult::PublishedIdentityMismatch;
        }

        return PrivacyRootRecoveryResult::Deferred;
    }

    {
        QWriteLocker locker(&d->lock);

        if ((d->rootEpochs.value(rootUuid) != recoveryEpoch) ||
            ((d->rootStates.value(rootUuid) !=
              PrivacyRootRuntimeState::Recovering) &&
             (d->rootStates.value(rootUuid) !=
              PrivacyRootRuntimeState::VerifiedAvailable)))
        {
            return PrivacyRootRecoveryResult::StaleEpoch;
        }

        inspection.summary.rootUuid = rootUuid;
        inspection.summary.state = PrivacyRootRuntimeState::VerifiedAvailable;
        inspection.summary.unresolvedTransactionCount = 0;
        inspection.summary.compatibilityExposureCount = 0;
        inspection.summary.failedProxyValidationCount =
            qMax(0, d->displayProxyIssueItemsByRoot.value(rootUuid).size() -
                    d->exposedOriginalItemsByRoot.value(rootUuid).size());
        inspection.summary.exposedOriginalAtProxyPathCount =
            d->exposedOriginalItemsByRoot.value(rootUuid).size();
        inspection.summary.identityMismatch = false;
        d->rootSummaries.insert(rootUuid, inspection.summary);
        d->proxyIssueItemsByRoot.insert(rootUuid, inspection.proxyIssueItemUuids);
        d->originalIssueItemsByRoot.insert(rootUuid, inspection.originalIssueItemUuids);

        for (const PrivacyTransaction& transaction : recoveredCompatibilityTransactions)
        {
            for (auto itemIt = d->items.constBegin() ; itemIt != d->items.constEnd() ; ++itemIt)
            {
                const bool affected = transaction.itemUuid.isEmpty()
                                    ? (itemIt->item.categoryUuid == transaction.categoryUuid)
                                    : (itemIt->item.uuid == transaction.itemUuid);

                if (affected && (itemIt->publicRootUuid == rootUuid))
                {
                    d->compatibilityExposedItems.remove(itemIt.key());
                }
            }
        }

        d->setRootState(rootUuid, PrivacyRootRuntimeState::VerifiedAvailable);
    }

    return PrivacyRootRecoveryResult::PublishedVerified;
}

PrivacyScanDisposition PrivacyRuntimeCoordinator::evaluate(const PrivacyScanRequest& request) const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized)
    {
        return PrivacyScanDisposition::RootRecovering;
    }

    const auto itemIt = d->items.constFind(request.imageId);

    if ((itemIt != d->items.constEnd()) &&
        d->compatibilityExposedItems.contains(request.imageId))
    {
        return PrivacyScanDisposition::CompatibilityOriginalExposed;
    }

    const QString rootUuid = d->albumRootUuids.value(request.albumRootId);

    if (!rootUuid.isEmpty())
    {
        const PrivacyScanDisposition rootDisposition =
            dispositionForRootState(d->rootStates.value(rootUuid, PrivacyRootRuntimeState::Unknown));

        if (rootDisposition != PrivacyScanDisposition::Unprotected)
        {
            return rootDisposition;
        }
    }

    if (itemIt == d->items.constEnd())
    {
        return PrivacyScanDisposition::Unprotected;
    }

    if (rootUuid.isEmpty() && !itemIt->publicRootUuid.isEmpty())
    {
        return PrivacyScanDisposition::PrivacyInspectionRequired;
    }

    if (!rootUuid.isEmpty() && !itemIt->publicRootUuid.isEmpty() &&
        (rootUuid != itemIt->publicRootUuid))
    {
        return PrivacyScanDisposition::PrivacyInspectionRequired;
    }

    if (!itemIt->publicRootUuid.isEmpty())
    {
        const PrivacyScanDisposition itemRootDisposition =
            dispositionForRootState(d->rootStates.value(itemIt->publicRootUuid,
                                                        PrivacyRootRuntimeState::Unknown));

        if (itemRootDisposition != PrivacyScanDisposition::Unprotected)
        {
            return itemRootDisposition;
        }
    }

    if (request.pathExists && (request.byteSize >= 0) &&
        (itemIt->expectedProxySize >= 0) &&
        (request.byteSize == itemIt->expectedProxySize))
    {
        return PrivacyScanDisposition::ProtectedProxyExpected;
    }

    return PrivacyScanDisposition::PrivacyInspectionRequired;
}

bool PrivacyRuntimeCoordinator::hasDeferredRoots() const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized)
    {
        return true;
    }

    for (auto it = d->albumRootUuids.constBegin() ; it != d->albumRootUuids.constEnd() ; ++it)
    {
        if (d->rootStates.value(it.value(), PrivacyRootRuntimeState::Unknown) !=
            PrivacyRootRuntimeState::VerifiedAvailable)
        {
            return true;
        }
    }

    return false;
}

bool PrivacyRuntimeCoordinator::rootContainsProtectedItems(int albumRootId) const
{
    QReadLocker locker(&d->lock);

    if (!d->initialized)
    {
        return true;
    }

    const QString rootUuid = d->albumRootUuids.value(albumRootId);

    return (!rootUuid.isEmpty() &&
            (d->hasUnassignedProtectedItems || d->protectedRootUuids.contains(rootUuid)));
}

QSharedPointer<const PrivacyRootVerifier> createDefaultPrivacyRootVerifier()
{
    return QSharedPointer<const PrivacyRootVerifier>(new CollectionPrivacyRootVerifier);
}

QSharedPointer<const PrivacyRootIntegrityInspector>
createDefaultPrivacyRootIntegrityInspector()
{
    return QSharedPointer<const PrivacyRootIntegrityInspector>(
               new CollectionPrivacyIntegrityInspector);
}

PrivacyStartupReport PrivacyStartupRecovery::run()
{
    PrivacyRepositorySnapshot snapshot;
    PrivacyRepository repository;
    QSharedPointer<PrivacyRuntimeCoordinator> runtime(new PrivacyRuntimeCoordinator);
    QSharedPointer<PrivacyCategorySessionOwner> categorySessions;
    QSharedPointer<const PrivacyTransactionRecovery> transactionRecovery;
    TransactionRecoveryFactory transactionRecoveryFactory;
    PrivacyStartupReport result;
    const bool unusedRootsPruned = repository.pruneUnreferencedAlbumRoots();

    {
        QReadLocker locker(&startupData->lock);
        transactionRecoveryFactory = startupData->transactionRecoveryFactory;
    }

    if (repository.loadSnapshot(&snapshot))
    {
        const QSharedPointer<const PrivacyRootVerifier> verifier =
            createDefaultPrivacyRootVerifier();
        const QSharedPointer<const PrivacyRootIntegrityInspector> integrity =
            createDefaultPrivacyRootIntegrityInspector();

        if (transactionRecoveryFactory)
        {
            transactionRecovery = transactionRecoveryFactory(*runtime);
        }

        result = runtime->initialize(snapshot, verifier, transactionRecovery,
                                     integrity);

        if (transactionRecoveryFactory && !transactionRecovery)
        {
            result.state = PrivacyStartupState::Degraded;
            result.diagnostics << QLatin1String(
                "Privacy transaction-recovery owner could not be initialized");
        }

        categorySessions = PrivacyCategorySessionOwner::create(runtime, verifier);

        if (!categorySessions)
        {
            result.state = PrivacyStartupState::Degraded;
            result.diagnostics << QLatin1String(
                "Privacy category-session owner could not be initialized");
        }
    }
    else
    {
        result.state = PrivacyStartupState::Degraded;
        result.diagnostics << QLatin1String("Privacy repository snapshot could not be validated");
    }

    if (!unusedRootsPruned)
    {
        result.state = PrivacyStartupState::Degraded;
        result.diagnostics << QLatin1String(
            "Unused privacy album-root registrations could not be reconciled");
    }

    PrivacyScanGate::setProvider(runtime);
    PrivacyAnalysisGate::setProvider(runtime);
    PrivacyManualTagVisibilityGate::setProvider(runtime);

    QSharedPointer<PrivacyRuntimeCoordinator> previousRuntime;
    QSharedPointer<PrivacyCategorySessionOwner> previousCategorySessions;

    {
        QWriteLocker locker(&startupData->lock);
        previousRuntime = startupData->coordinator;
        previousCategorySessions = startupData->categorySessions;
        startupData->coordinator = runtime;
        startupData->categorySessions = categorySessions;
        startupData->report      = result;
    }

    if (previousCategorySessions && (previousCategorySessions != categorySessions))
    {
        previousCategorySessions->shutdown();
    }

    if (previousRuntime && (previousRuntime != runtime))
    {
        previousRuntime->reset();
    }

    return result;
}

void PrivacyStartupRecovery::setTransactionRecoveryFactory(
    const TransactionRecoveryFactory& factory)
{
    QWriteLocker locker(&startupData->lock);
    startupData->transactionRecoveryFactory = factory;
}

void PrivacyStartupRecovery::reset()
{
    QSharedPointer<PrivacyRuntimeCoordinator> runtime(new PrivacyRuntimeCoordinator);
    PrivacyScanGate::setProvider(runtime);
    PrivacyAnalysisGate::setProvider(runtime);
    PrivacyManualTagVisibilityGate::setProvider(runtime);

    QSharedPointer<PrivacyRuntimeCoordinator> previousRuntime;
    QSharedPointer<PrivacyCategorySessionOwner> previousCategorySessions;

    {
        QWriteLocker locker(&startupData->lock);
        previousRuntime = startupData->coordinator;
        previousCategorySessions = startupData->categorySessions;
        startupData->coordinator = runtime;
        startupData->categorySessions.clear();
        startupData->report      = PrivacyStartupReport();
    }

    if (previousCategorySessions)
    {
        previousCategorySessions->shutdown();
    }

    if (previousRuntime && (previousRuntime != runtime))
    {
        previousRuntime->reset();
    }
}

PrivacyStartupReport PrivacyStartupRecovery::report()
{
    QReadLocker locker(&startupData->lock);

    return startupData->report;
}

QSharedPointer<PrivacyRuntimeCoordinator> PrivacyStartupRecovery::coordinator()
{
    QReadLocker locker(&startupData->lock);

    return startupData->coordinator;
}

QSharedPointer<PrivacyCategorySessionOwner> PrivacyStartupRecovery::categorySessions()
{
    QReadLocker locker(&startupData->lock);

    return startupData->categorySessions;
}

QSharedPointer<const PrivacyActionStateProvider>
PrivacyStartupRecovery::actionStateProvider()
{
    QReadLocker locker(&startupData->lock);

    return startupData->coordinator;
}

QSharedPointer<const PrivacyLeaseStateProvider>
PrivacyStartupRecovery::leaseStateProvider()
{
    QReadLocker locker(&startupData->lock);

    return startupData->coordinator;
}

QSharedPointer<const PrivacyManualTagVisibilityProvider>
PrivacyStartupRecovery::manualTagVisibilityProvider()
{
    QReadLocker locker(&startupData->lock);

    return startupData->coordinator;
}

void PrivacyManualTagVisibilityGate::setProvider(
    const QSharedPointer<const PrivacyManualTagVisibilityProvider>& provider)
{
    QWriteLocker locker(&manualTagGateData->lock);
    manualTagGateData->provider = provider;

    if (++manualTagGateData->generation == 0)
    {
        ++manualTagGateData->generation;
    }
}

void PrivacyManualTagVisibilityGate::resetProvider()
{
    setProvider(QSharedPointer<const PrivacyManualTagVisibilityProvider>());
}

bool PrivacyManualTagVisibilityGate::isInstalled()
{
    QReadLocker locker(&manualTagGateData->lock);

    return bool(manualTagGateData->provider);
}

bool PrivacyManualTagVisibilityGate::mayAccess(qlonglong imageId)
{
    QSharedPointer<const PrivacyManualTagVisibilityProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&manualTagGateData->lock);
        provider   = manualTagGateData->provider;
        generation = manualTagGateData->generation;
    }

    const bool allowed = (!provider || provider->mayAccessManualTags(imageId));

    {
        QReadLocker locker(&manualTagGateData->lock);

        if ((generation != manualTagGateData->generation) ||
            (provider != manualTagGateData->provider))
        {
            return false;
        }
    }

    return allowed;
}

QSet<QString> PrivacyManualTagVisibilityGate::visibleCategoryUuids()
{
    QSet<QString> categoryUuids;
    queryState(&categoryUuids);

    return categoryUuids;
}

bool PrivacyManualTagVisibilityGate::queryState(
    QSet<QString>* visibleCategoryUuids)
{
    if (!visibleCategoryUuids)
    {
        return true;
    }

    visibleCategoryUuids->clear();
    QSharedPointer<const PrivacyManualTagVisibilityProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&manualTagGateData->lock);
        provider   = manualTagGateData->provider;
        generation = manualTagGateData->generation;
    }

    if (!provider)
    {
        return false;
    }

    const QSet<QString> categoryUuids = provider->visibleManualTagCategoryUuids();

    {
        QReadLocker locker(&manualTagGateData->lock);

        if ((generation != manualTagGateData->generation) ||
            (provider != manualTagGateData->provider))
        {
            return true;
        }
    }

    *visibleCategoryUuids = categoryUuids;

    return true;
}

} // namespace Digikam

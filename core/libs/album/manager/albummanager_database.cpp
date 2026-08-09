/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface - Database helpers.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albummanager_p.h"

// Qt includes

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#   include <QTextCodec>
#else
#   include <QStringConverter>
#endif

#include <QFileInfo>
#include <QCache>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUuid>
#include <QWeakPointer>

// C++ includes

#include <functional>
#include <memory>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "loadingdescription.h"
#include "privacyruntime.h"
#include "privacycachetransition.h"
#include "privacycasualoriginalreader.h"
#include "privacycategorysessionowner.h"
#include "privacyderivativestore.h"
#include "privacypreparedaccessregistry.h"
#include "privacyrepository.h"
#include "loadingcacheinterface.h"
#include "privacysourceresolver.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "iteminfo.h"

namespace Digikam
{

namespace
{

constexpr qlonglong MaximumMaterializedOriginalBytes = 512LL * 1024LL * 1024LL;
constexpr qlonglong PreparedScratchMinimumReserveBytes = 1024LL * 1024LL * 1024LL;

class MaterializedOriginal final : public PrivacySourceLifetime
{
public:

    ~MaterializedOriginal()
    {
        if (!physicalPath.isEmpty() &&
            !physicalPath.startsWith(QLatin1String("/proc/self/fd/")))
        {
            QFile::remove(physicalPath);
        }
    }

public:

    qlonglong                         imageId = -1;
    QString                           categoryUuid;
    QString                           logicalPath;
    QString                           physicalPath;
    QString                           cacheNamespace;
    QSharedPointer<QTemporaryFile>    backing;
};

class StartupPrivacySourceProvider final : public PrivacySourceProvider
{
public:

    explicit StartupPrivacySourceProvider(
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
        const QSharedPointer<PrivacyCategorySessionOwner>& sessions,
        const std::function<void(qlonglong, bool)>& validationFailed)
        : m_runtime(runtime),
          m_sessions(sessions),
          m_clearBytes(128 * 1024),
          m_validationFailed(validationFailed)
    {
    }

    bool setPresentationAvailable(const QString& categoryUuid, bool available)
    {
        if (!available &&
            PrivacyPreparedAccessRegistry::hasActiveAccess(categoryUuid))
        {
            return false;
        }

        QSet<QString> changedPaths;
        QList<QSharedPointer<MaterializedOriginal> > originals;

        {
            QMutexLocker locker(&m_clearLock);

            if (categoryUuid.isEmpty())
            {
                m_allPresentationBlocked = !available;

                if (available)
                {
                    m_blockedCategories.clear();
                }

                for (const QSet<QString>& paths :
                     std::as_const(m_knownPathsByCategory))
                {
                    changedPaths.unite(paths);
                }
            }
            else
            {
                if (available)
                {
                    m_blockedCategories.remove(categoryUuid);
                }
                else
                {
                    m_blockedCategories.insert(categoryUuid);
                }

                changedPaths = m_knownPathsByCategory.value(categoryUuid);
            }

            if (!available)
            {
                m_clearBytes.clear();

                for (const QSharedPointer<MaterializedOriginal>& original :
                     std::as_const(m_originals))
                {
                    if (original &&
                        (categoryUuid.isEmpty() ||
                         (original->categoryUuid == categoryUuid)))
                    {
                        originals << original;
                    }
                }
            }
        }

        bool revoked = true;

        for (const QSharedPointer<MaterializedOriginal>& original :
             std::as_const(originals))
        {
            LoadingDescription description(
                original->logicalPath, PreviewSettings(), 0,
                LoadingDescription::NoColorConversion,
                LoadingDescription::PreviewParameters::Thumbnail);
            description.previewParameters.storageReference = original->imageId;
            description.resolveSource();
            const PrivacyCacheTransitionToken token =
                PrivacyCacheTransition::begin(description.thumbnailIdentifier());
            PrivacyCacheTransitionInventory inventory;
            inventory.direction = PrivacyCacheTransitionInventory::Unprotect;
            ThreadImageIOPrivacyCacheTransitionBackend backend;
            const PrivacyCacheTransition::Result purge =
                PrivacyCacheTransition::purge(token, inventory, &backend);

            if (!token.isValid() ||
                (purge.status != PrivacyCacheTransition::Complete) ||
                PrivacyPreparedAccessRegistry::hasActiveAccess(categoryUuid))
            {
                if (token.isValid())
                {
                    (void)PrivacyCacheTransition::rollback(token);
                }

                revoked = false;
                break;
            }

            {
                QMutexLocker locker(&m_clearLock);

                for (auto it = m_originals.begin(); it != m_originals.end(); )
                {
                    if (it.value() == original)
                    {
                        it = m_originals.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            if (!PrivacyCacheTransition::finish(token))
            {
                (void)PrivacyCacheTransition::rollback(token);
                revoked = false;
                break;
            }
        }

        if (!revoked)
        {
            QMutexLocker locker(&m_clearLock);

            if (categoryUuid.isEmpty())
            {
                m_allPresentationBlocked = false;
                m_blockedCategories.clear();
            }
            else
            {
                m_blockedCategories.remove(categoryUuid);
            }

            return false;
        }

        for (const QString& path : std::as_const(changedPaths))
        {
            LoadingCacheInterface::cleanFileCache(path);
            LoadingCacheInterface::fileChanged(path, true);
        }

        return true;
    }

    PrivacySourceResult resolve(const PrivacySourceRequest& request) const override
    {
        if (!m_runtime)
        {
            return PrivacySourceResult::denied(QLatin1String("privacy-runtime-unavailable"));
        }

        qlonglong imageId = request.itemReference.toLongLong();

        if ((imageId <= 0) && !request.logicalFilePath.isEmpty())
        {
            const ItemInfo info = ItemInfo::fromLocalFile(request.logicalFilePath);
            imageId             = info.id();
        }

        if (imageId <= 0)
        {
            const CollectionLocation location =
                CollectionManager::instance()->locationForPath(request.logicalFilePath);

            if (!location.isNull() && m_runtime->rootContainsProtectedItems(location.id()))
            {
                return PrivacySourceResult::denied(QLatin1String("privacy-item-unresolved"));
            }

            return PrivacySourceResult::notHandled();
        }

        const PrivacyPublicSourceDisposition disposition =
            m_runtime->publicSourceDisposition(imageId);

        if (disposition == PrivacyPublicSourceDisposition::Unprotected)
        {
            return PrivacySourceResult::notHandled();
        }

        const ItemInfo logicalInfo(imageId);

        if (logicalInfo.isNull() ||
            (QDir::cleanPath(logicalInfo.filePath()) !=
             QDir::cleanPath(request.logicalFilePath)))
        {
            return PrivacySourceResult::denied(QLatin1String("privacy-logical-source-mismatch"));
        }

        const QString cacheNamespace = m_runtime->publicSourceCacheNamespace(imageId);

        if ((disposition == PrivacyPublicSourceDisposition::LockedProxy) &&
            !cacheNamespace.isEmpty())
        {
            const PrivacyPublicProxyDisplayResult validation =
                m_runtime->validatePublicProxyForDisplay(
                    imageId, request.logicalFilePath);

            if (validation != PrivacyPublicProxyDisplayResult::Verified)
            {
                if (((validation ==
                      PrivacyPublicProxyDisplayResult::NewlyFailedValidation) ||
                     (validation ==
                      PrivacyPublicProxyDisplayResult::NewlyExposedOriginal)) &&
                    m_validationFailed)
                {
                    m_validationFailed(
                        imageId,
                        validation ==
                            PrivacyPublicProxyDisplayResult::NewlyExposedOriginal);
                }

                return PrivacySourceResult::denied(cacheNamespace);
            }

            PrivacyActionItemState itemState;

            if (m_runtime->stateForItem(imageId, &itemState) &&
                itemState.protectedItem && !itemState.categoryUuid.isEmpty())
            {
                QMutexLocker locker(&m_clearLock);
                m_knownPathsByCategory[itemState.categoryUuid].insert(
                    QDir::cleanPath(request.logicalFilePath));
            }

            if ((request.consumer == PrivacySourceRequest::Thumbnail) &&
                !request.detailThumbnail)
            {
                PrivacyClearThumbnailSource clearSource;

                if (m_runtime->clearThumbnailSource(imageId, &clearSource) &&
                    ((clearSource.mode ==
                      PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked) ||
                     ((clearSource.mode ==
                       PrivacyUnlockedThumbnailMode::FocusedClear) &&
                      PrivacySourceResolver::thumbnailRevealRequested(
                          request.logicalFilePath))))
                {
                    const QByteArray bytes = clearThumbnailBytes(
                        imageId, request.logicalFilePath, clearSource);

                    if (!bytes.isEmpty())
                    {
                        return PrivacySourceResult::resolvedMemory(
                            bytes, clearThumbnailCacheNamespace(clearSource));
                    }
                }
            }

            if ((request.consumer != PrivacySourceRequest::Thumbnail) &&
                m_runtime->isCategoryUnlocked(itemState.categoryUuid))
            {
                {
                    QMutexLocker locker(&m_clearLock);

                    if (m_allPresentationBlocked ||
                        m_blockedCategories.contains(itemState.categoryUuid))
                    {
                        return PrivacySourceResult::denied(cacheNamespace);
                    }
                }

                const bool preparedAccess =
                    (request.consumer == PrivacySourceRequest::PreparedAccess);

                if (!preparedAccess &&
                    ((request.assetRole != PrivacyAsset::PrimaryMediaRole) ||
                     (request.assetOrdinal != 0)))
                {
                    return PrivacySourceResult::denied(cacheNamespace);
                }

                const QSharedPointer<MaterializedOriginal> original =
                    originalSource(imageId, request.logicalFilePath, itemState,
                                   request.assetRole, request.assetOrdinal,
                                   !preparedAccess, request.isCancelled);

                if (!original)
                {
                    return PrivacySourceResult::denied(
                        QLatin1String("privacy-original-unavailable"));
                }

                PrivacySourceResult result = PrivacySourceResult::resolved(
                    original->physicalPath, original->cacheNamespace,
                    PrivacySourceResult::MemoryOnly);
                result.lifetimeOwner = original;
                return result;
            }

            return PrivacySourceResult::resolved(request.logicalFilePath,
                                                 cacheNamespace,
                                                 PrivacySourceResult::Persistent);
        }

        return PrivacySourceResult::denied(cacheNamespace.isEmpty()
                                           ? QLatin1String("privacy-source-denied")
                                           : cacheNamespace);
    }

private:

    static QString clearThumbnailCacheNamespace(
        const PrivacyClearThumbnailSource& source)
    {
        return (QLatin1String("privacy-clear:") + source.derivative.itemUuid +
                QLatin1Char(':') +
                QString::number(source.derivative.generation) +
                QLatin1Char(':') +
                QString::number(source.derivative.presentationVersion) +
                QLatin1Char(':') + source.derivative.derivativeHash +
                QLatin1Char(':') + QString::number(source.categoryEpoch));
    }

    static bool sameClearSource(const PrivacyClearThumbnailSource& left,
                                const PrivacyClearThumbnailSource& right)
    {
        return (left.categoryUuid == right.categoryUuid) &&
               (left.categoryEpoch == right.categoryEpoch) &&
               (left.mode == right.mode) &&
               (left.derivative.itemUuid == right.derivative.itemUuid) &&
               (left.derivative.storeUuid == right.derivative.storeUuid) &&
               (left.derivative.protectedRelativePath ==
                right.derivative.protectedRelativePath) &&
               (left.derivative.derivativeHash ==
                right.derivative.derivativeHash) &&
               (left.derivative.derivativeSize ==
                right.derivative.derivativeSize) &&
               (left.derivative.generation == right.derivative.generation) &&
               (left.derivative.presentationVersion ==
                right.derivative.presentationVersion);
    }

    QByteArray clearThumbnailBytes(
        qlonglong imageId, const QString& logicalPath,
        const PrivacyClearThumbnailSource& source) const
    {
        const QString cacheKey = clearThumbnailCacheNamespace(source);

        {
            QMutexLocker locker(&m_clearLock);

            if (m_allPresentationBlocked ||
                m_blockedCategories.contains(source.categoryUuid))
            {
                return {};
            }

            m_knownPathsByCategory[source.categoryUuid].insert(
                QDir::cleanPath(logicalPath));

            if (const QByteArray* const cached = m_clearBytes.object(cacheKey))
            {
                return *cached;
            }
        }

        if (!m_sessions)
        {
            return {};
        }

        QByteArray bytes;
        const PrivacyCategoryOperationStatus operation =
            m_sessions->runWithUnlockedStore(
                source.categoryUuid,
                [&bytes, &source](const PrivacyPassword&, const QString& root)
                {
                    if (!root.isEmpty())
                    {
                        bytes = PrivacyDerivativeStore().read(root,
                                                              source.derivative);
                    }
                });

        PrivacyClearThumbnailSource current;

        if ((operation != PrivacyCategoryOperationStatus::Completed) ||
            bytes.isEmpty() ||
            !m_runtime->clearThumbnailSource(imageId, &current) ||
            !sameClearSource(source, current))
        {
            return {};
        }

        QMutexLocker locker(&m_clearLock);

        if (m_allPresentationBlocked ||
            m_blockedCategories.contains(source.categoryUuid))
        {
            return {};
        }

        const int cost = qMax(1, (bytes.size() + 1023) / 1024);
        m_clearBytes.insert(cacheKey, new QByteArray(bytes), cost);

        return bytes;
    }

    static bool sameLeaseState(const PrivacyLeaseCurrentState& left,
                               const PrivacyLeaseCurrentState& right)
    {
        return ((left.itemUuid == right.itemUuid) &&
                (left.itemGeneration == right.itemGeneration) &&
                (left.categoryEpoch == right.categoryEpoch) &&
                (left.publicRootEpoch == right.publicRootEpoch) &&
                (left.storeRootEpoch == right.storeRootEpoch) &&
                (left.categoryUnlocked == right.categoryUnlocked) &&
                (left.publicRootAvailable == right.publicRootAvailable) &&
                (left.storeRootAvailable == right.storeRootAvailable) &&
                (left.unresolvedTransaction == right.unresolvedTransaction));
    }

    QString originalRuntimePath() const
    {
#if !defined(Q_OS_LINUX)
        return {};
#else
        QMutexLocker locker(&m_clearLock);

        std::unique_ptr<QTemporaryDir>& directoryOwner = m_originalRuntimeDir;

        if (directoryOwner && directoryOwner->isValid())
        {
            return directoryOwner->path();
        }

        const QString base = QStandardPaths::writableLocation(
            QStandardPaths::RuntimeLocation);
        const QString parent = QDir(base).filePath(
            QLatin1String("digikam-private"));

        if (base.isEmpty() ||
            !QDir().mkpath(parent) ||
            !QFile::setPermissions(parent, QFileDevice::ReadOwner |
                                           QFileDevice::WriteOwner |
                                           QFileDevice::ExeOwner))
        {
            return {};
        }

        std::unique_ptr<QTemporaryDir> directory(new QTemporaryDir(
            QDir(parent).filePath(QLatin1String("original-sources-XXXXXX"))));

        if (!directory->isValid() ||
            !QFile::setPermissions(directory->path(), QFileDevice::ReadOwner |
                                                      QFileDevice::WriteOwner |
                                                      QFileDevice::ExeOwner))
        {
            return {};
        }

        directoryOwner = std::move(directory);
        return directoryOwner->path();
#endif
    }

    QSharedPointer<MaterializedOriginal> originalSource(
        qlonglong imageId, const QString& logicalPath,
        const PrivacyActionItemState& actionState,
        int assetRole, int assetOrdinal,
        bool retainInProviderCache,
        const std::function<bool()>& isCancelled) const
    {
#if !defined(Q_OS_LINUX)
        Q_UNUSED(imageId);
        Q_UNUSED(logicalPath);
        Q_UNUSED(actionState);
        Q_UNUSED(assetRole);
        Q_UNUSED(assetOrdinal);
        Q_UNUSED(retainInProviderCache);
        Q_UNUSED(isCancelled);
        return {};
#else
        if (!m_sessions || !actionState.protectedItem ||
            (actionState.access != PrivacyItemAccess::Unlocked) ||
            !actionState.originalReady || actionState.unresolvedTransaction ||
            (isCancelled && isCancelled()))
        {
            return {};
        }

        PrivacyCasualOriginalSource prepared;
        PrivacyLeaseCurrentState before;
        QSharedPointer<QTemporaryFile> backing;
        QString linkPath;
        bool restored = false;
        const PrivacyCategoryOperationStatus operation =
            m_sessions->runWithUnlockedSecret(
                actionState.categoryUuid,
                [this, imageId, &logicalPath, &prepared, &before,
                 &backing, &linkPath, &restored,
                 assetRole, assetOrdinal,
                 retainInProviderCache,
                 &isCancelled](const PrivacyPassword& password)
                {
                    PrivacyRepositorySnapshot snapshot;
                    PrivacyCasualOriginalReader reader;

                    if ((isCancelled && isCancelled()) ||
                        !PrivacyRepository().loadRuntimeSnapshot(&snapshot) ||
                        !reader.prepareAsset(snapshot, imageId, logicalPath,
                                             assetRole, assetOrdinal,
                                             &prepared) ||
                        !m_runtime->currentState(prepared.itemUuid, &before) ||
                        !before.isValid() || !before.categoryUnlocked ||
                        !before.publicRootAvailable || !before.storeRootAvailable ||
                        before.unresolvedTransaction ||
                        (before.itemGeneration != prepared.itemGeneration))
                    {
                        return;
                    }

                    const QString cacheNamespace =
                        QLatin1String("privacy-original:v1:") + prepared.itemUuid +
                        QLatin1Char(':') + QString::number(prepared.itemGeneration) +
                        QLatin1Char(':') + QString::number(before.categoryEpoch) +
                        QLatin1Char(':') + QString::number(before.publicRootEpoch) +
                        QLatin1Char(':') + QString::number(before.storeRootEpoch) +
                        QLatin1Char(':') + QString::number(assetRole) +
                        QLatin1Char(':') + QString::number(assetOrdinal) +
                        QLatin1Char(':') + prepared.originalHash;

                    {
                        QMutexLocker locker(&m_clearLock);
                        const auto existing = m_originals.constFind(cacheNamespace);

                        if (retainInProviderCache &&
                            (existing != m_originals.constEnd()) && existing.value() &&
                            QFileInfo::exists(existing.value()->physicalPath))
                        {
                            restored = true;
                            return;
                        }

                        qlonglong retainedBytes = 0;

                        for (const QSharedPointer<MaterializedOriginal>& original :
                             std::as_const(m_originals))
                        {
                            retainedBytes += (original && original->backing)
                                           ? original->backing->size() : 0;
                        }

                        if (retainInProviderCache &&
                            ((prepared.originalSize > MaximumMaterializedOriginalBytes) ||
                            (retainedBytes > (MaximumMaterializedOriginalBytes -
                                              prepared.originalSize))))
                        {
                            return;
                        }
                    }

                    const QString runtimePath = retainInProviderCache
                        ? originalRuntimePath()
                        : QFileInfo(prepared.restore.archivePath).absolutePath();

                    if (runtimePath.isEmpty())
                    {
                        return;
                    }

                    if (!retainInProviderCache)
                    {
                        const QStorageInfo storage(runtimePath);
                        const qint64 reserve = qMax(
                            PreparedScratchMinimumReserveBytes,
                            storage.bytesTotal() / 20);

                        if (!storage.isValid() || !storage.isReady() ||
                            storage.isReadOnly() ||
                            (prepared.originalSize > storage.bytesAvailable()) ||
                            ((storage.bytesAvailable() - prepared.originalSize) <
                             reserve))
                        {
                            return;
                        }
                    }

                    backing.reset(new QTemporaryFile(
                        QDir(runtimePath).filePath(QLatin1String("source-XXXXXX"))));

                    if (!backing->open() ||
                        !backing->setPermissions(QFileDevice::ReadOwner |
                                                 QFileDevice::WriteOwner))
                    {
                        backing.clear();
                        return;
                    }

                    if (!retainInProviderCache)
                    {
                        const QString temporaryPath = backing->fileName();

                        if ((backing->handle() < 0) ||
                            !QFile::remove(temporaryPath))
                        {
                            backing.clear();
                            return;
                        }

                        linkPath = QLatin1String("/proc/self/fd/") +
                                   QString::number(backing->handle());
                    }

                    PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;

                    if (!reader.restore(prepared, password, backing.data(),
                                        &error, isCancelled) ||
                        !backing->flush() ||
                        (backing->size() != prepared.originalSize) ||
                        !backing->seek(0) ||
                        !backing->setPermissions(QFileDevice::ReadOwner) ||
                        (backing->handle() < 0))
                    {
                        backing.clear();
                        return;
                    }

                    QString suffix = QFileInfo(prepared.originalName).suffix().toLower();

                    if (!QRegularExpression(QLatin1String("^[a-z0-9]{1,16}$"))
                             .match(suffix).hasMatch())
                    {
                        suffix.clear();
                    }

                    if (retainInProviderCache)
                    {
                        const QString temporaryPath = backing->fileName();

                        if (!QFile::remove(temporaryPath))
                        {
                            backing.clear();
                            return;
                        }

                        linkPath = QDir(runtimePath).filePath(
                            QUuid::createUuid().toString(QUuid::WithoutBraces) +
                            (suffix.isEmpty()
                                 ? QString() : (QLatin1Char('.') + suffix)));

                        if (!QFile::link(QLatin1String("/proc/self/fd/") +
                                             QString::number(backing->handle()),
                                         linkPath))
                        {
                            backing.clear();
                            linkPath.clear();
                            return;
                        }
                    }

                    PrivacyLeaseCurrentState after;
                    restored = m_runtime->currentState(prepared.itemUuid, &after) &&
                               sameLeaseState(before, after);
                });

        if ((operation != PrivacyCategoryOperationStatus::Completed) || !restored)
        {
            QFile::remove(linkPath);
            return {};
        }

        const QString cacheNamespace =
            QLatin1String("privacy-original:v1:") + prepared.itemUuid +
            QLatin1Char(':') + QString::number(prepared.itemGeneration) +
            QLatin1Char(':') + QString::number(before.categoryEpoch) +
            QLatin1Char(':') + QString::number(before.publicRootEpoch) +
            QLatin1Char(':') + QString::number(before.storeRootEpoch) +
            QLatin1Char(':') + QString::number(assetRole) +
            QLatin1Char(':') + QString::number(assetOrdinal) +
            QLatin1Char(':') + prepared.originalHash;
        QMutexLocker locker(&m_clearLock);
        const auto existing = m_originals.constFind(cacheNamespace);

        if (retainInProviderCache &&
            (existing != m_originals.constEnd()) && existing.value())
        {
            QFile::remove(linkPath);
            return existing.value();
        }

        if (!backing || linkPath.isEmpty() || m_allPresentationBlocked ||
            m_blockedCategories.contains(prepared.categoryUuid))
        {
            QFile::remove(linkPath);
            return {};
        }

        qlonglong retainedBytes = 0;

        for (const QSharedPointer<MaterializedOriginal>& retained :
             std::as_const(m_originals))
        {
            retainedBytes += (retained && retained->backing)
                           ? retained->backing->size() : 0;
        }

        if (retainInProviderCache &&
            ((prepared.originalSize > MaximumMaterializedOriginalBytes) ||
            (retainedBytes > (MaximumMaterializedOriginalBytes -
                              prepared.originalSize))))
        {
            QFile::remove(linkPath);
            return {};
        }

        QSharedPointer<MaterializedOriginal> original(new MaterializedOriginal);
        original->imageId = imageId;
        original->categoryUuid = prepared.categoryUuid;
        original->logicalPath = QDir::cleanPath(logicalPath);
        original->physicalPath = linkPath;
        original->cacheNamespace = cacheNamespace;
        original->backing = backing;
        if (retainInProviderCache)
        {
            m_originals.insert(cacheNamespace, original);
        }

        m_knownPathsByCategory[prepared.categoryUuid].insert(original->logicalPath);
        return original;
#endif
    }

private:

    QSharedPointer<PrivacyRuntimeCoordinator> m_runtime;
    QSharedPointer<PrivacyCategorySessionOwner> m_sessions;
    mutable QMutex                            m_clearLock;
    mutable QCache<QString, QByteArray>        m_clearBytes;
    mutable std::unique_ptr<QTemporaryDir>     m_originalRuntimeDir;
    mutable QHash<QString, QSharedPointer<MaterializedOriginal> > m_originals;
    mutable QHash<QString, QSet<QString> >     m_knownPathsByCategory;
    mutable QSet<QString>                     m_blockedCategories;
    mutable bool                              m_allPresentationBlocked = false;
    std::function<void(qlonglong, bool)>       m_validationFailed;
};

} // namespace

bool AlbumManager::setDatabase(const DbEngineParameters& params, bool priority, const QString& suggestedAlbumRoot, bool ignoreDisappearedLocations)
{
    PrivacyPreparedAccessQuiesceGuard preparedAccessQuiesce;

    if (!preparedAccessQuiesce.isAcquired())
    {
        QMessageBox::warning(
            qApp ? qApp->activeWindow() : nullptr,
            qApp ? qApp->applicationName() : QLatin1String("digiKam"),
            i18n("The database cannot be changed while an operation is using "
                 "unlocked private media. Finish or cancel that operation, "
                 "then try again."));
        return false;
    }

    // This is to ensure that the setup does not overrule the command line.
    // TODO: there is a bug that setup is showing something different here.

    if (!priority && d->hasPriorizedDbPath)
    {
        // ignore change without priority

        return true;
    }

    if (!PrivacyStartupRecovery::reset())
    {
        QMessageBox::warning(
            qApp ? qApp->activeWindow() : nullptr,
            qApp ? qApp->applicationName() : QLatin1String("digiKam"),
            i18n("The database cannot be changed because private media could "
                 "not be safely closed or relocked. Resolve the active "
                 "private operation, then try again."));
        return false;
    }

    if (priority)
    {
        d->hasPriorizedDbPath = true;
    }

    d->changed = true;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    DatabaseServerStarter::instance()->stopServerManagerProcess();

    // Shutdown possibly running collection scans.
    // Must call restartCollectionScan further down.

    ScanController::instance()->cancelAllAndSuspendCollectionScan();
    PrivacySourceResolver::resetProvider();

    disconnect(CollectionManager::instance(), nullptr, this, nullptr);
    CollectionManager::instance()->setWatchDisabled();

    if (CoreDbAccess::databaseWatch())
    {
        disconnect(CoreDbAccess::databaseWatch(), nullptr, this, nullptr);
    }

    ItemAttributesWatch::shutDown();
    ItemAttributesWatch::cleanUp();
    d->albumWatch->clear();

    cleanUp();

    d->currentAlbums.clear();

    Q_EMIT signalAlbumCurrentChanged(d->currentAlbums);
    Q_EMIT signalAlbumsCleared();

    d->albumPathHash.clear();
    d->allAlbumsIdHash.clear();
    d->albumRootAlbumHash.clear();

    // deletes all child albums as well

    delete d->rootPAlbum;
    delete d->rootTAlbum;
    delete d->rootDAlbum;
    delete d->rootSAlbum;

    d->rootPAlbum = nullptr;
    d->rootTAlbum = nullptr;
    d->rootDAlbum = nullptr;
    d->rootSAlbum = nullptr;

    // -- Database initialization -------------------------------------------------

    // ensure, embedded database is loaded

    qCDebug(DIGIKAM_GENERAL_LOG) << params;

    QString databaseError;

    const ApplicationSettings* const settings = ApplicationSettings::instance();

    if      (params.internalServer && suggestedAlbumRoot.isEmpty())
    {
        if      (!QFileInfo::exists(params.internalServerPath()))
        {
            databaseError = i18n("The MySQL database directory was not found.");
        }
        else if (
                 (
                  !QFileInfo::exists(params.internalServerMysqlUpgradeCmd)                        &&
                  QStandardPaths::findExecutable(params.internalServerMysqlUpgradeCmd).isEmpty()
                 ) ||
                 (
                  !QFileInfo::exists(params.internalServerMysqlServerCmd)                         &&
                  QStandardPaths::findExecutable(params.internalServerMysqlServerCmd).isEmpty()
                 ) ||
                 (
                 !QFileInfo::exists(params.internalServerMysqlAdminCmd)                           &&
                  QStandardPaths::findExecutable(params.internalServerMysqlAdminCmd).isEmpty()
                 )
                )
        {
            databaseError = i18n("The MySQL binary tools was not found.");
        }
    }
    else if (params.isSQLite() && suggestedAlbumRoot.isEmpty() && !settings->getDatabaseDirSetAtCmd())
    {
        if (!QFileInfo::exists(params.databaseNameCore))
        {
            databaseError = i18n("The SQLite core database was not found.");
        }
    }

    if (!databaseError.isEmpty())
    {
        QString configPath = QStandardPaths::locate(QStandardPaths::GenericConfigLocation,
                                                    QLatin1String("digikamrc"));

        databaseError     += i18n("<p>If you want to start with a new configuration and "
                                  "with a first run wizard, delete the file:<br>%1</p>",
                                  QDir::toNativeSeparators(configPath));

        return showDatabaseSetupPage(databaseError, priority, suggestedAlbumRoot);
    }

    if (params.internalServer)
    {
        DatabaseServerError result = DatabaseServerStarter::instance()->startServerManagerProcess(params);

        if (result.getErrorType() != DatabaseServerError::NoErrors)
        {
            databaseError = i18n("An error occurred during the internal server start."
                                 "<p>Details:<br>%1</p>", result.getErrorText());

            return showDatabaseSetupPage(databaseError, priority, suggestedAlbumRoot);
        }
    }

    CoreDbAccess::setParameters(params, CoreDbAccess::MainApplication);

    DbEngineGuiErrorHandler* const handler = new DbEngineGuiErrorHandler(CoreDbAccess::parameters());
    CoreDbAccess::initDbEngineErrorHandler(handler);

    QApplication::restoreOverrideCursor();

    if (!handler->checkDatabaseConnection())
    {
        databaseError = i18n("Failed to open the database.");

        if (!showDatabaseSetupPage(databaseError, priority, suggestedAlbumRoot))
        {
            if (params.isSQLite())
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                      i18n("<p>digiKam will attempt to start now, "
                                           "but it will <b>not</b> be functional.</p>"));

                CoreDbAccess::setParameters(DbEngineParameters(), CoreDbAccess::DatabaseSlave);

                return true;
            }

            return false;
        }

        return true;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    d->albumWatch->setDbEngineParameters(params);

    ScanController::Advice advice = ScanController::instance()->databaseInitialization();
    QString errorMsg              = CoreDbAccess().lastError();

    QApplication::restoreOverrideCursor();

    switch (advice)
    {
        case ScanController::Success:
        {
            break;
        }

        case ScanController::ContinueWithoutDatabase:
        {
            if (errorMsg.isEmpty())
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                      i18n("<p>Failed to open the database.</p>"
                                           "<p>You cannot use digiKam without a working database. "
                                           "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                           "Please check the database settings in the <b>configuration menu</b>.</p>"
                                          ));
            }
            else
            {
                QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                      i18n("<p>Failed to open the database. Error message from database:</p>"
                                           "<p><b>%1</b></p>"
                                           "<p>You cannot use digiKam without a working database. "
                                           "digiKam will attempt to start now, but it will <b>not</b> be functional. "
                                           "Please check the database settings in the <b>configuration menu</b>.</p>",
                                           errorMsg));
            }

            return true;
        }

        case ScanController::AbortImmediately:
        {
            if (!errorMsg.isEmpty())
            {
                databaseError = errorMsg;
            }
            else
            {
                databaseError = i18n("Failed to initialize the database.");
            }

            return showDatabaseSetupPage(databaseError, priority, suggestedAlbumRoot);
        }
    }

    // Privacy recovery must become authoritative before any thumbnail,
    // similarity, or collection scanner can consume public collection bytes.
    // The coordinator is GUI-independent; its consolidated report is consumed
    // only after DigikamApp exists.

    const PrivacyStartupReport privacyReport = PrivacyStartupRecovery::run();
    const QSharedPointer<PrivacyCategorySessionOwner> privacySessions =
        PrivacyStartupRecovery::categorySessions();
    const QSharedPointer<StartupPrivacySourceProvider> privacySource(
        new StartupPrivacySourceProvider(
                PrivacyStartupRecovery::coordinator(), privacySessions,
                [guardedManager = QPointer<AlbumManager>(this)](
                    qlonglong imageId, bool exposedOriginal)
                {
                    if (!guardedManager)
                    {
                        return;
                    }

                    QMetaObject::invokeMethod(
                        guardedManager.data(),
                        [guardedManager, imageId, exposedOriginal]()
                        {
                            if (guardedManager)
                            {
                                Q_EMIT guardedManager->
                                    signalPrivacyPublicProxyValidationFailed(
                                        imageId, exposedOriginal);
                            }
                        },
                        Qt::QueuedConnection);
                }));

    if (privacySessions)
    {
        const QWeakPointer<StartupPrivacySourceProvider> weakSource(privacySource);
        privacySessions->setPresentationAvailabilityCallback(
            [weakSource](const QString& categoryUuid, bool available)
            {
                const QSharedPointer<StartupPrivacySourceProvider> source =
                    weakSource.toStrongRef();

                if (source)
                {
                    return source->setPresentationAvailable(categoryUuid, available);
                }

                return false;
            });
    }

    PrivacySourceResolver::setProvider(privacySource);

    if (privacyReport.state == PrivacyStartupState::Degraded)
    {
        qCWarning(DIGIKAM_GENERAL_LOG)
            << "Privacy startup recovery is degraded; affected roots remain scan-gated"
            << privacyReport.diagnostics;
    }

    // -- Locale Checking ---------------------------------------------------------

    QString currLocale = CoreDbAccess().db()->getDatabaseEncoding();
    QString dbLocale   = CoreDbAccess().db()->getSetting(QLatin1String("Locale"));

    if      (dbLocale.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No locale found in database";
        CoreDbAccess().db()->setSetting(QLatin1String("Locale"), currLocale);
    }
    else if (dbLocale != currLocale)
    {
        // TODO it would be better to replace all yes/no confirmation dialogs with ones that has custom
        // buttons that denote the actions directly, i.e.:  ["Ignore and Continue"]  ["Adjust locale"]

        int result = QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                                 i18n("Your database character set has changed since this "
                                      "album was last opened.\n"
                                      "Old character set: %1, new character set: %2\n"
                                      "If you have recently changed your database character set, "
                                      "you need not be concerned.\n"
                                      "Please note that if you switched to a database character set "
                                      "that does not support some of the filenames in your collection, "
                                      "these files may no longer be found in the collection. "
                                      "If you are sure that you want to "
                                      "continue, click 'Yes'. "
                                      "Otherwise, click 'No' and correct your "
                                      "database character set setting before restarting digiKam.",
                                      dbLocale, currLocale),
                                  QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            return false;
        }

        CoreDbAccess().db()->setSetting(QLatin1String("Locale"), currLocale);
    }

    // -- UUID Checking ---------------------------------------------------------

    QList<CollectionLocation> disappearedLocations = CollectionManager::instance()->checkHardWiredLocations();

    for (const CollectionLocation& loc : std::as_const(disappearedLocations))
    {
        QString locDescription;
        QStringList candidateIds, candidateDescriptions;
        CollectionManager::instance()->migrationCandidates(loc, &locDescription, &candidateIds, &candidateDescriptions);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Migration candidates for" << locDescription
                                     << ":" << candidateIds << candidateDescriptions;

        QDialog* const dialog         = new QDialog;
        QWidget* const widget         = new QWidget(dialog);
        QGridLayout* const mainLayout = new QGridLayout;
        mainLayout->setColumnStretch(1, 1);

        QLabel* const deviceIconLabel = new QLabel;
        deviceIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("drive-harddisk")).pixmap(64));
        mainLayout->addWidget(deviceIconLabel, 0, 0);

        QLabel* const mainLabel       = new QLabel(i18n("<p>The collection </p><p><b>%1</b><br>(%2)</p><p> is currently "
                                                        "not found on your system.<br> Please choose the most "
                                                        "appropriate  option to handle this situation:</p>",
                                                   loc.label(), QDir::toNativeSeparators(locDescription)));
        mainLabel->setWordWrap(true);
        mainLayout->addWidget(mainLabel, 0, 1);

        QGroupBox* const groupBox     = new QGroupBox;
        mainLayout->addWidget(groupBox, 1, 0, 1, 2);

        QGridLayout* const layout     = new QGridLayout;
        layout->setColumnStretch(1, 1);

        QRadioButton* migrateButton   = nullptr;
        QComboBox* migrateChoices     = nullptr;

        if (!candidateIds.isEmpty())
        {
            migrateButton              = new QRadioButton;
            QLabel* const migrateLabel = new QLabel(i18n("<p>The collection is still available, but the identifier changed.<br>"
                                                         "This can be caused by restoring a backup, changing the partition layout "
                                                         "or the file system settings.<br>"
                                                         "The collection is now located at this place:</p>"));
            migrateLabel->setWordWrap(true);

            migrateChoices             = new QComboBox;

            for (int i = 0 ; i < candidateIds.size() ; ++i)
            {
                migrateChoices->addItem(QDir::toNativeSeparators(candidateDescriptions.at(i)), candidateIds.at(i));
            }

            layout->addWidget(migrateButton,  0, 0, Qt::AlignTop);
            layout->addWidget(migrateLabel,   0, 1);
            layout->addWidget(migrateChoices, 1, 1);
        }

        QRadioButton* const isRemovableButton = new QRadioButton;
        QLabel* const isRemovableLabel        = new QLabel(i18n("The collection is located on a storage device which is not "
                                                                "always attached. Mark the collection as a removable collection."));
        isRemovableLabel->setWordWrap(true);
        layout->addWidget(isRemovableButton, 2, 0, Qt::AlignTop);
        layout->addWidget(isRemovableLabel,  2, 1);

        QRadioButton* const solveManuallyButton = new QRadioButton;
        QLabel* const solveManuallyLabel        = new QLabel(i18n("Take no action now. I would like to solve the problem "
                                                                  "later using the setup dialog"));
        solveManuallyLabel->setWordWrap(true);
        layout->addWidget(solveManuallyButton, 3, 0, Qt::AlignTop);
        layout->addWidget(solveManuallyLabel,  3, 1);

        groupBox->setLayout(layout);
        widget->setLayout(mainLayout);

        QVBoxLayout* const vbx                  = new QVBoxLayout(dialog);
        QDialogButtonBox* const buttons         = new QDialogButtonBox(QDialogButtonBox::Ok, dialog);
        vbx->addWidget(widget);
        vbx->addWidget(buttons);
        dialog->setLayout(vbx);
        dialog->setWindowTitle(i18nc("@title:window", "Collection not Found"));

        connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
                dialog, SLOT(accept()));

        // Default option: If there is only one candidate, default to migration.
        // Otherwise default to do nothing now.

        if (migrateButton && (candidateIds.size() == 1))
        {
            migrateButton->setChecked(true);
        }
        else
        {
            solveManuallyButton->setChecked(true);
        }

        if (!ignoreDisappearedLocations && dialog->exec())
        {
            if      (migrateButton && migrateButton->isChecked())
            {
                CollectionManager::instance()->migrateToVolume(loc, migrateChoices->itemData(migrateChoices->currentIndex()).toString());
            }
            else if (isRemovableButton->isChecked())
            {
                CollectionManager::instance()->changeType(loc, CollectionLocation::VolumeRemovable);
            }
        }

        delete dialog;
    }

    // -- ---------------------------------------------------------

    // check that we have one album root

    if (CollectionManager::instance()->allLocations().isEmpty())
    {
        if (suggestedAlbumRoot.isEmpty())
        {
            Setup::execSinglePage(Setup::CollectionsPage);
        }
        else
        {
            QUrl albumRoot(QUrl::fromLocalFile(suggestedAlbumRoot));
            CollectionManager::instance()->addLocation(albumRoot, albumRoot.fileName());

            // Not needed? See bug #188959
/*
            ScanController::instance()->completeCollectionScan();
*/
        }
    }

    // -- ---------------------------------------------------------

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ThumbnailLoadThread::initializeThumbnailDatabase(CoreDbAccess::parameters().thumbnailParameters(),
                                                     new ThumbsDbInfoProvider());

    DbEngineGuiErrorHandler* const thumbnailsDBHandler = new DbEngineGuiErrorHandler(ThumbsDbAccess::parameters());
    ThumbsDbAccess::initDbEngineErrorHandler(thumbnailsDBHandler);

    // Activate the similarity database.

    SimilarityDbAccess::setParameters(params.similarityParameters());

    DbEngineGuiErrorHandler* const similarityHandler   = new DbEngineGuiErrorHandler(SimilarityDbAccess::parameters());
    SimilarityDbAccess::initDbEngineErrorHandler(similarityHandler);

    if (SimilarityDbAccess::checkReadyForUse(nullptr))
    {
        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database ready for use";
    }
    else
    {
        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Failed to initialize similarity database";
    }

    QApplication::restoreOverrideCursor();

    // still suspended from above

    ScanController::instance()->restartCollectionScan();

    return true;
}

void AlbumManager::checkDatabaseDirsAfterFirstRun(const QString& dbPath, const QString& albumPath)
{
    // for bug #193522

    QDir               newDir(dbPath);
    QDir               albumDir(albumPath);
    DbEngineParameters newParams = DbEngineParameters::parametersForSQLiteDefaultFile(newDir.path());
    QFileInfo          digikam4DB(newParams.SQLiteDatabaseFile());

    if (!digikam4DB.exists())
    {
        QFileInfo digikam3DB(newDir, QLatin1String("digikam3.db"));
        QFileInfo digikamVeryOldDB(newDir, QLatin1String("digikam.db"));

        if (digikam3DB.exists() || digikamVeryOldDB.exists())
        {
            QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                     i18nc("@title:window", "Database Folder"),
                     i18n("<p>You have chosen the folder \"%1\" as the place to store the database. "
                          "A database file from an older version of digiKam is found in this folder.</p> "
                          "<p>Would you like to upgrade the old database file - confirming "
                          "that this database file was indeed created for the pictures located in the folder \"%2\" - "
                          "or ignore the old file and start with a new database?</p> ",
                          QDir::toNativeSeparators(newDir.path()),
                          QDir::toNativeSeparators(albumDir.path())),
                      QMessageBox::Yes | QMessageBox::No,
                      qApp->activeWindow());

            msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Upgrade Database"));
            msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
            msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Create New Database"));
            msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
            msgBox->setDefaultButton(QMessageBox::Yes);

            int result = msgBox->exec();

            if      (result == QMessageBox::Yes)
            {
                // CoreDbSchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                // Restore this situation.

                KSharedConfigPtr config = KSharedConfig::openConfig();
                KConfigGroup group      = config->group(QLatin1String("Album Settings"));
                group.writeEntry("Album Path", albumDir.path());
                group.sync();
            }
            else if (result == QMessageBox::No)
            {
                moveToBackup(digikam3DB);
                moveToBackup(digikamVeryOldDB);
            }

            delete msgBox;
        }
    }
}

void AlbumManager::changeDatabase(const DbEngineParameters& newParams)
{
    ScanController::instance()->shutDown();

    // if there is no file at the new place, copy old one

    DbEngineParameters params = CoreDbAccess::parameters();

    // New database type SQLITE

    if (newParams.isSQLite())
    {
        const bool dbNameChanged = (params.getCoreDatabaseNameOrDir() != newParams.getCoreDatabaseNameOrDir());

        QDir newDir(newParams.getCoreDatabaseNameOrDir());
        QFileInfo newFile(newDir, QLatin1String("digikam4.db"));

        if      (!newFile.exists() && dbNameChanged)
        {
            QFileInfo digikam3DB(newDir, QLatin1String("digikam3.db"));
            QFileInfo digikamVeryOldDB(newDir, QLatin1String("digikam.db"));

            if (digikam3DB.exists() || digikamVeryOldDB.exists())
            {
                int result = -1;

                if (params.isSQLite())
                {
                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                             i18nc("@title:window", "New Database Folder"),
                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                  "A database file from an older version of digiKam is found in this folder.</p> "
                                  "<p>Would you like to upgrade the old database file, start with a new database, "
                                  "or copy the current database to this location and continue using it?</p> ",
                                  QDir::toNativeSeparators(newDir.path())),
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                             qApp->activeWindow());

                    msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Upgrade Database"));
                    msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                    msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Create New Database"));
                    msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox->button(QMessageBox::Cancel)->setText(i18nc("@action:button", "Copy Current Database"));
                    msgBox->button(QMessageBox::Cancel)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                    msgBox->setDefaultButton(QMessageBox::Yes);

                    result = msgBox->exec();
                    delete msgBox;
                }
                else
                {
                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                             i18nc("@title:window", "New Database Folder"),
                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                                  "A database file from an older version of digiKam is found in this folder.</p> "
                                  "<p>Would you like to upgrade the old database file or start with a new database?</p>",
                                  QDir::toNativeSeparators(newDir.path())),
                             QMessageBox::Yes | QMessageBox::No,
                             qApp->activeWindow());

                    msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Upgrade Database"));
                    msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                    msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Create New Database"));
                    msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox->setDefaultButton(QMessageBox::Yes);

                    result = msgBox->exec();
                    delete msgBox;
                }

                if      (result == QMessageBox::Yes)
                {
                    // CoreDbSchemaUpdater expects Album Path to point to the album root of the 0.9 db file.
                    // Restore this situation.

                    KSharedConfigPtr config = KSharedConfig::openConfig();
                    KConfigGroup group      = config->group(QLatin1String("Album Settings"));
                    group.writeEntry(QLatin1String("Album Path"), newDir.path());
                    group.sync();
                }
                else if (result == QMessageBox::No)
                {
                    moveToBackup(digikam3DB);
                    moveToBackup(digikamVeryOldDB);
                }
                else if (result == QMessageBox::Cancel)
                {
                    QFileInfo oldFile(params.SQLiteDatabaseFile());
                    copyToNewLocation(oldFile, newFile, i18n("Failed to copy the old database file (\"%1\") "
                                                             "to its new location (\"%2\"). "
                                                             "Trying to upgrade old databases.",
                                                             QDir::toNativeSeparators(oldFile.filePath()),
                                                             QDir::toNativeSeparators(newFile.filePath())));
                }
            }
            else
            {
                int result = QMessageBox::Yes;

                if (params.isSQLite())
                {
                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                             i18nc("@title:window", "New Database Folder"),
                             i18n("<p>You have chosen the folder \"%1\" as the new place to store the database.</p>"
                                  "<p>Would you like to copy the current database to this location "
                                  "and continue using it, or start with a new database?</p> ",
                                  QDir::toNativeSeparators(newDir.path())),
                             QMessageBox::Yes | QMessageBox::No,
                             qApp->activeWindow());

                    msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Create New Database"));
                    msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
                    msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Copy Current Database"));
                    msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                    msgBox->setDefaultButton(QMessageBox::Yes);

                    result = msgBox->exec();
                    delete msgBox;
                }

                if (result == QMessageBox::No)
                {
                    QFileInfo oldFile(params.SQLiteDatabaseFile());
                    copyToNewLocation(oldFile, newFile);
                }
            }
        }
        else if (dbNameChanged)
        {
            int result = QMessageBox::No;

            if (params.isSQLite())
            {
                QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                         i18nc("@title:window", "New Database Folder"),
                         i18n("<p>You have chosen the folder \"%1\" as the new place to store the database. "
                              "There is already a database file in this location.</p> "
                              "<p>Would you like to use this existing file as the new database, or remove it "
                              "and copy the current database to this place?</p> ",
                              QDir::toNativeSeparators(newDir.path())),
                         QMessageBox::Yes | QMessageBox::No,
                         qApp->activeWindow());

                msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Copy Current Database"));
                msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Use Existing File"));
                msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
                msgBox->setDefaultButton(QMessageBox::Yes);

                result = msgBox->exec();
                delete msgBox;
            }

            if (result == QMessageBox::Yes)
            {
                // first backup

                if (moveToBackup(newFile))
                {
                    QFileInfo oldFile(params.SQLiteDatabaseFile());

                    // then copy

                    copyToNewLocation(oldFile, newFile);
                }
            }
        }
    }

    ScanController::instance()->restart();

    if (setDatabase(newParams, false))
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        startScan();
        QApplication::restoreOverrideCursor();
        ScanController::instance()->completeCollectionScan();
    }
}

bool AlbumManager::databaseEqual(const DbEngineParameters& parameters) const
{
    DbEngineParameters params = CoreDbAccess::parameters();

    return (params == parameters);
}

bool AlbumManager::moveToBackup(const QFileInfo& info)
{
    if (info.exists())
    {
        QFileInfo backup(info.dir(), info.fileName() +
                                     QLatin1String("-backup-") +
                                     QDateTime::currentDateTime().toString(Qt::ISODate));

        bool ret = QDir().rename(info.filePath(), backup.filePath());

        if (!ret)
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18n("Failed to backup the existing database file (\"%1\"). "
                                       "Refusing to replace file without backup, using the existing file.",
                                       QDir::toNativeSeparators(info.filePath())));
            return false;
        }
    }

    return true;
}

bool AlbumManager::copyToNewLocation(const QFileInfo& oldFile,
                                     const QFileInfo& newFile,
                                     const QString& otherMessage)
{
    QString message = otherMessage;

    if (message.isNull())
    {
        message = i18n("Failed to copy the old database file (\"%1\") "
                       "to its new location (\"%2\"). "
                       "Starting with an empty database.",
                       QDir::toNativeSeparators(oldFile.filePath()),
                       QDir::toNativeSeparators(newFile.filePath()));
    }

    bool ret = QFile::copy(oldFile.filePath(), newFile.filePath());

    if (!ret)
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), message);

        return false;
    }

    return true;
}

bool AlbumManager::showDatabaseSetupPage(const QString& error, bool priority, const QString& suggestedAlbumRoot)
{
    QApplication::restoreOverrideCursor();

    QString errorMsg = error + i18n("<p><b>Please check the database settings in this dialog.</b></p>");

    // We cannot use Setup::execSinglePage() as this already requires a core database.

    QPointer<QDialog> setup                  = new QDialog(qApp->activeWindow());
    QVBoxLayout* const layout                = new QVBoxLayout(setup);
    QLabel* const errorLabel                 = new QLabel(errorMsg, setup);
    errorLabel->setWordWrap(true);
    errorLabel->setTextFormat(Qt::RichText);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    errorLabel->setStyleSheet(QLatin1String("QLabel { background-color: #FFDDDD; color: black; }"));
    DatabaseSettingsWidget* const dbsettings = new DatabaseSettingsWidget(setup);
    QDialogButtonBox* const buttons          = new QDialogButtonBox(QDialogButtonBox::Ok    |
                                                                    QDialogButtonBox::Reset |
                                                                    QDialogButtonBox::Cancel, setup);
    buttons->button(QDialogButtonBox::Reset)->setText(i18nc("@action:button", "New database"));
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    layout->addWidget(errorLabel);
    layout->addWidget(dbsettings);
    layout->addStretch(10);
    layout->addWidget(buttons);

    bool newDatabase = false;

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            setup, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            setup, SLOT(reject()));

    connect(buttons->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, [&newDatabase, setup]()        // clazy:exclude=lambda-in-connect
        {
            newDatabase = true;
            setup->accept();
        }
    );

    ApplicationSettings* const settings = ApplicationSettings::instance();
    dbsettings->setParametersFromSettings(settings);

    if (setup->exec() != QDialog::Accepted)
    {
        delete setup;

        return false;
    }

    DbEngineParameters dbParams = dbsettings->getDbEngineParameters();
    settings->setDbEngineParameters(dbParams);
    settings->saveSettings();

    delete setup;

    if (newDatabase)
    {
        if (dbParams.internalServer)
        {
            DatabaseServerError result = DatabaseServerStarter::instance()->startServerManagerProcess(dbParams);

            if (result.getErrorType() != DatabaseServerError::NoErrors)
            {
                return false;
            }
        }

        CoreDbAccess::setParameters(dbParams);

        if (!CoreDbAccess::checkReadyForUse())
        {
            return false;
        }
    }

    return (setDatabase(dbParams, priority, suggestedAlbumRoot));
}

} // namespace Digikam

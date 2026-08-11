/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacymigrationcoordinator.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSize>
#include <QTimeZone>
#include <QUuid>

#ifdef Q_OS_UNIX

#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>

#endif

// Local includes

#include "privacyrepository.h"
#include "privacyproxygenerator.h"
#include "privacystrongobjectbackend.h"

namespace Digikam
{

namespace
{

PrivacyMigrationItemResult itemFailure(PrivacyMigrationStatus status,
                                       const QString& itemUuid,
                                       const QString& detail,
                                       const QString& transactionUuid = {})
{
    PrivacyMigrationItemResult result;
    result.status = status;
    result.itemUuid = itemUuid;
    result.detail = detail;
    result.transactionUuid = transactionUuid;
    return result;
}

QByteArray encodePayload(const QString& kind, const QString& itemUuid,
                         const QString& sourceCategoryUuid,
                         const QString& targetCategoryUuid,
                         const QString& sourceContainerUuid,
                         const QString& compatTransactionUuid,
                         const QString& stage)
{
    QJsonObject object;
    object.insert(QLatin1String("formatVersion"), 1);
    object.insert(QLatin1String("kind"), kind);
    object.insert(QLatin1String("itemUuid"), itemUuid);
    object.insert(QLatin1String("sourceCategoryUuid"), sourceCategoryUuid);
    object.insert(QLatin1String("targetCategoryUuid"), targetCategoryUuid);
    object.insert(QLatin1String("sourceContainerUuid"), sourceContainerUuid);
    object.insert(QLatin1String("compatTransactionUuid"),
                  compatTransactionUuid);
    object.insert(QLatin1String("stage"), stage);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool payloadStage(const PrivacyTransaction& transaction, const QString& kind,
                  QString* const stage)
{
    if (!stage || (transaction.payloadFormatVersion != 1))
    {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(transaction.payloadData, &parseError);

    if ((parseError.error != QJsonParseError::NoError) || !document.isObject())
    {
        return false;
    }

    const QJsonObject object = document.object();

    if ((object.value(QLatin1String("kind")).toString() != kind) ||
        (object.value(QLatin1String("itemUuid")).toString() !=
         transaction.itemUuid))
    {
        return false;
    }

    *stage = object.value(QLatin1String("stage")).toString();
    return !stage->isEmpty();
}

bool fsyncDirectory(const QString& path)
{
#ifdef Q_OS_UNIX
    const int descriptor = ::open(QFile::encodeName(path).constData(),
                                  O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool synced = (::fsync(descriptor) == 0);
    ::close(descriptor);
    return synced;
#else
    Q_UNUSED(path);
    return true;
#endif
}

bool fileFacts(const QString& path, PrivacyInventoryAsset* const asset)
{
    if (!asset)
    {
        return false;
    }

#ifdef Q_OS_UNIX
    struct stat status = {};

    if (::lstat(QFile::encodeName(path).constData(), &status) != 0)
    {
        return false;
    }

    asset->evidence.type = PrivacyInventoryFileType::Regular;
    asset->evidence.identityComplete = true;
    asset->evidence.deviceId = static_cast<quint64>(status.st_dev);
    asset->evidence.inode = static_cast<quint64>(status.st_ino);
    asset->evidence.linkCount = static_cast<quint64>(status.st_nlink);
    asset->evidence.byteSize = static_cast<qlonglong>(status.st_size);
    return S_ISREG(status.st_mode);
#else
    const QFileInfo info(path);

    if (!info.isFile() || info.isSymLink())
    {
        return false;
    }

    asset->evidence.type = PrivacyInventoryFileType::Regular;
    asset->evidence.identityComplete = true;
    asset->evidence.byteSize = info.size();
    return true;
#endif
}

QByteArray hashFileBytes(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    if (!hash.addData(&file))
    {
        return {};
    }

    return hash.result();
}

QByteArray encodePortableMode(quint32 mode)
{
    const quint32 portable = mode & 07777;
    QByteArray bytes(4, Qt::Uninitialized);
    qToBigEndian(portable, bytes.data());
    return bytes;
}

quint32 fileMode(const QString& path)
{
#ifdef Q_OS_UNIX
    struct stat status = {};

    if (::lstat(QFile::encodeName(path).constData(), &status) != 0)
    {
        return 0;
    }

    return static_cast<quint32>(status.st_mode & 0xffffU);
#else
    Q_UNUSED(path);
    return 0100644;
#endif
}

QDateTime fileModificationTime(const QString& path)
{
#ifdef Q_OS_UNIX
    struct stat status = {};

    if (::lstat(QFile::encodeName(path).constData(), &status) != 0)
    {
        return {};
    }

    return QDateTime::fromMSecsSinceEpoch(
        (static_cast<qint64>(status.st_mtim.tv_sec) * 1000) +
        (status.st_mtim.tv_nsec / 1000000), QTimeZone::UTC);
#else
    return QFileInfo(path).lastModified();
#endif
}

bool writeFileAtomic(const QString& path, const QByteArray& bytes)
{
    const QString temporary = path + QLatin1String(".migration-tmp");

    if (QFileInfo(temporary).isSymLink() || QFileInfo(path).isSymLink())
    {
        return false;
    }

    QFile file(temporary);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        (file.write(bytes) != bytes.size()) || !file.flush())
    {
        return false;
    }

    file.close();

    if (!QFile::remove(path) || !QFile::rename(temporary, path) ||
        !fsyncDirectory(QFileInfo(path).absolutePath()))
    {
        QFile::remove(temporary);
        return false;
    }

    return true;
}

QString expectedArchivePath(const QList<PrivacyMigrationAssetInput>& assets)
{
    for (const PrivacyMigrationAssetInput& asset : assets)
    {
        if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
            (asset.ordinal == 0))
        {
            const QFileInfo info(asset.publicRelativePath);
            const QString parent = info.path();

            if (parent == QLatin1String("."))
            {
                return info.completeBaseName() +
                       QLatin1String(".digikam-private.zip");
            }

            return parent + QLatin1Char('/') + info.completeBaseName() +
                   QLatin1String(".digikam-private.zip");
        }
    }

    return QString();
}

} // namespace

bool PrivacyCoreDbMigrationPersistence::loadSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadSnapshot(snapshot);
}

bool PrivacyCoreDbMigrationPersistence::beginMigration(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginMigration(transaction, journal);
}

bool PrivacyCoreDbMigrationPersistence::compareAndUpdateTransaction(
    const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState,
    qlonglong expectedGeneration)
{
    return PrivacyRepository().compareAndUpdateTransaction(
        transaction, expectedState, expectedGeneration);
}

bool PrivacyCoreDbMigrationPersistence::publishMigration(
    const PrivacyItem& item, const PrivacyContainer& container,
    const QList<PrivacyAsset>& assets, const QString& sourceContainerUuid,
    const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState, qlonglong expectedGeneration)
{
    return PrivacyRepository().publishMigration(
        item, container, assets, sourceContainerUuid, transaction,
        expectedState, expectedGeneration);
}

bool PrivacyCoreDbMigrationPersistence::removeContainerAndAssets(
    const QString& containerUuid, const QString& itemUuid)
{
    return PrivacyRepository().removeContainerAndAssets(containerUuid, itemUuid);
}

PrivacyMigrationCoordinator::PrivacyMigrationCoordinator(
    PrivacyMigrationPersistence& persistence,
    PrivacyStillItemTransactionEngine& stillEngine,
    PrivacyRuntimeCoordinator& runtime,
    PrivacyCasualArchiveEngine& archiveEngine)
    : m_persistence(persistence),
      m_stillEngine(stillEngine),
      m_runtime(runtime),
      m_archiveEngine(archiveEngine)
{
}

PrivacyMigrationBatchResult PrivacyMigrationCoordinator::migrateBatch(
    const QList<PrivacyMigrationRequest>& requests,
    const BatchProgress& progress)
{
    PrivacyMigrationBatchResult batch;
    batch.requestedCount = requests.size();

    for (const PrivacyMigrationRequest& request : requests)
    {
        PrivacyMigrationItemResult item = migrateOne(request, false);
        batch.items << item;
        ++batch.processedCount;

        if (item.succeeded())
        {
            ++batch.succeededCount;
        }
        else
        {
            ++batch.remainingCount;
        }

        if (progress)
        {
            progress(batch.processedCount, batch.requestedCount);
        }
    }

    return batch;
}

PrivacyMigrationBatchResult PrivacyMigrationCoordinator::recover(
    const QList<PrivacyMigrationRequest>& requests)
{
    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return {};
    }

    PrivacyMigrationBatchResult batch;

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if (!transaction.isActive() ||
            (transaction.type != PrivacyTransactionType::MigrateBackend))
        {
            continue;
        }

        const PrivacyMigrationRequest* request = nullptr;

        for (const PrivacyMigrationRequest& candidate : requests)
        {
            if (candidate.itemUuid == transaction.itemUuid)
            {
                request = &candidate;
                break;
            }
        }

        if (!request)
        {
            PrivacyMigrationItemResult unavailable = itemFailure(
                PrivacyMigrationStatus::InvalidRequest,
                transaction.itemUuid,
                QStringLiteral(
                    "the migration request is required for recovery"),
                transaction.uuid);
            batch.items << unavailable;
            ++batch.processedCount;
            ++batch.remainingCount;
            continue;
        }

        PrivacyMigrationItemResult item = migrateOne(*request, true);
        batch.items << item;
        ++batch.processedCount;

        if (item.succeeded())
        {
            ++batch.succeededCount;
        }
        else
        {
            ++batch.remainingCount;
        }
    }

    batch.requestedCount = batch.processedCount;
    return batch;
}

PrivacyMigrationItemResult PrivacyMigrationCoordinator::migrateOne(
    const PrivacyMigrationRequest& request, bool resume)
{
    if ((request.imageId <= 0) || request.itemUuid.isEmpty() ||
        !request.publicRoot.isValid())
    {
        return itemFailure(PrivacyMigrationStatus::InvalidRequest,
                           request.itemUuid,
                           QStringLiteral("the migration request is incomplete"));
    }

    const bool toProtected = !request.targetCategoryUuid.isEmpty();
    const bool fromProtected = !request.sourceCategoryUuid.isEmpty();

    if (!toProtected && fromProtected)
    {
        return migrateToUnprotected(request);
    }

    if (toProtected && !fromProtected)
    {
        return migrateToProtected(request);
    }

    if (toProtected && fromProtected)
    {
        return migrateProtectedToProtected(request, resume);
    }

    return itemFailure(PrivacyMigrationStatus::InvalidRequest,
                       request.itemUuid,
                       QStringLiteral("migration targets are ambiguous"));
}

PrivacyMigrationItemResult PrivacyMigrationCoordinator::migrateToProtected(
    const PrivacyMigrationRequest& request)
{
    if (!request.targetPassword || !request.targetPassword->isValid())
    {
        return itemFailure(PrivacyMigrationStatus::InvalidRequest,
                           request.itemUuid,
                           QStringLiteral("the target category password is required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                           request.itemUuid,
                           QStringLiteral("the privacy catalogue could not be read"));
    }

    const PrivacyContainer* container = nullptr;

    for (const PrivacyContainer& candidate : snapshot.containers)
    {
        if (candidate.itemUuid == request.itemUuid)
        {
            container = &candidate;
            break;
        }
    }

    if (container)
    {
        return itemFailure(PrivacyMigrationStatus::AlreadyActive,
                           request.itemUuid,
                           QStringLiteral("the item is already protected"));
    }

    const QString transactionUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString containerUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    PrivacyStillProtectRequest protectRequest;

    if (!buildTargetProtectRequest(request, containerUuid, transactionUuid,
                                   &protectRequest))
    {
        return itemFailure(PrivacyMigrationStatus::InvalidRequest,
                           request.itemUuid,
                           QStringLiteral("the target protect request is invalid"));
    }

    const PrivacyStillItemTransactionResult result =
        m_stillEngine.protect(protectRequest, *request.targetPassword);

    if (result.status != PrivacyStillItemTransactionStatus::Protected)
    {
        return itemFailure(PrivacyMigrationStatus::Failed,
                           request.itemUuid, result.detail);
    }

    PrivacyMigrationItemResult migrated;
    migrated.status = PrivacyMigrationStatus::Migrated;
    migrated.itemUuid = request.itemUuid;
    migrated.transactionUuid = transactionUuid;
    return migrated;
}

PrivacyMigrationItemResult PrivacyMigrationCoordinator::migrateToUnprotected(
    const PrivacyMigrationRequest& request)
{
    if (!request.sourcePassword || !request.sourcePassword->isValid())
    {
        return itemFailure(PrivacyMigrationStatus::InvalidRequest,
                           request.itemUuid,
                           QStringLiteral("the source category password is required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                           request.itemUuid,
                           QStringLiteral("the privacy catalogue could not be read"));
    }

    const PrivacyItem* item = nullptr;

    for (const PrivacyItem& candidate : snapshot.items)
    {
        if (candidate.uuid == request.itemUuid)
        {
            item = &candidate;
            break;
        }
    }

    if (!item)
    {
        return itemFailure(PrivacyMigrationStatus::AlreadyActive,
                           request.itemUuid,
                           QStringLiteral("the item is not protected"));
    }

    PrivacyStillUnprotectRequest unprotectRequest;
    unprotectRequest.imageId = request.imageId;
    unprotectRequest.categoryUuid = request.sourceCategoryUuid;
    unprotectRequest.transactionUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    unprotectRequest.publicRoot = request.publicRoot;
    unprotectRequest.rootExpectation = request.rootExpectation;
    unprotectRequest.freshAuthenticationConfirmed = true;
    unprotectRequest.vaultPlaintextRoot =
        request.sourceVaultPlaintextRoot;
    unprotectRequest.strongStoreUuid = request.sourceStrongStoreUuid;

    const PrivacyStillItemTransactionResult result =
        m_stillEngine.unprotect(unprotectRequest, *request.sourcePassword);

    if (result.status != PrivacyStillItemTransactionStatus::Unprotected)
    {
        return itemFailure(PrivacyMigrationStatus::Failed,
                           request.itemUuid, result.detail);
    }

    PrivacyMigrationItemResult migrated;
    migrated.status = PrivacyMigrationStatus::Migrated;
    migrated.itemUuid = request.itemUuid;
    migrated.transactionUuid = unprotectRequest.transactionUuid;
    return migrated;
}

PrivacyMigrationItemResult PrivacyMigrationCoordinator::
    migrateProtectedToProtected(const PrivacyMigrationRequest& request,
                                bool resume)
{
    if (!request.sourcePassword || !request.sourcePassword->isValid() ||
        !request.targetPassword || !request.targetPassword->isValid())
    {
        return itemFailure(PrivacyMigrationStatus::InvalidRequest,
                           request.itemUuid,
                           QStringLiteral("both category passwords are required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                           request.itemUuid,
                           QStringLiteral("the privacy catalogue could not be read"));
    }

    const PrivacyItem* item = nullptr;

    for (const PrivacyItem& candidate : snapshot.items)
    {
        if (candidate.uuid == request.itemUuid)
        {
            item = &candidate;
            break;
        }
    }

    if (!item || (item->categoryUuid != request.sourceCategoryUuid))
    {
        return itemFailure(PrivacyMigrationStatus::AlreadyActive,
                           request.itemUuid,
                           QStringLiteral("the source mapping is no longer current"));
    }

    PrivacyContainer sourceContainer;
    bool foundContainer = false;

    for (const PrivacyContainer& candidate : snapshot.containers)
    {
        if (candidate.itemUuid == request.itemUuid)
        {
            sourceContainer = candidate;
            foundContainer = true;
            break;
        }
    }

    if (!foundContainer)
    {
        return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                           request.itemUuid,
                           QStringLiteral("the source container is missing"));
    }

    // Find an existing migration transaction for this item.
    const PrivacyTransaction* migration = nullptr;

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.isActive() &&
            (candidate.type == PrivacyTransactionType::MigrateBackend) &&
            (candidate.itemUuid == request.itemUuid))
        {
            migration = &candidate;
            break;
        }
    }

    QString migrationTransactionUuid;
    QString compatTransactionUuid;
    QString stage = QLatin1String("created");

    if (!resume)
    {
        if (migration)
        {
            return itemFailure(PrivacyMigrationStatus::AlreadyActive,
                               request.itemUuid,
                               QStringLiteral("a migration is already active"),
                               migration->uuid);
        }

        migrationTransactionUuid =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        const QDateTime now = QDateTime::currentDateTimeUtc();
        PrivacyTransaction transaction;
        transaction.uuid = migrationTransactionUuid;
        transaction.categoryUuid = request.targetCategoryUuid;
        transaction.itemUuid = request.itemUuid;
        transaction.type = PrivacyTransactionType::MigrateBackend;
        transaction.state = PrivacyTransactionState::Created;
        transaction.generation = 0;
        transaction.payloadFormatVersion = 1;
        transaction.payloadData = encodePayload(
            QLatin1String("protected-to-protected"), request.itemUuid,
            request.sourceCategoryUuid, request.targetCategoryUuid,
            sourceContainer.uuid, {}, QLatin1String("created"));
        transaction.createdAt = now;
        transaction.updatedAt = now;
        PrivacyTransactionJournal journal;
        journal.transactionUuid = migrationTransactionUuid;
        journal.rootUuid = request.publicRoot.uuid;
        journal.journalRelativePath =
            PrivacyTransactionJournalCodec::relativeJournalPath(
                migrationTransactionUuid);
        journal.journalFormatVersion =
            PrivacyTransactionJournalCodec::FormatVersion;
        journal.stage = static_cast<int>(PrivacyJournalStage::Created);
        journal.updatedAt = now;

        if (!m_persistence.beginMigration(transaction, journal))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               QStringLiteral("the migration transaction could not be begun"));
        }

    }
    else
    {
        if (!migration)
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               QStringLiteral("no pending migration exists"));
        }

        migrationTransactionUuid = migration->uuid;

        if (!payloadStage(*migration,
                          QLatin1String("protected-to-protected"), &stage))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               QStringLiteral("the migration payload is invalid"),
                               migrationTransactionUuid);
        }

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(migration->payloadData, &parseError);

        if ((parseError.error == QJsonParseError::NoError) &&
            document.isObject())
        {
            compatTransactionUuid =
                document.object().value(
                    QLatin1String("compatTransactionUuid")).toString();
        }
    }

    const auto updateStage = [&](const QString& nextStage,
                                 const QString& compatUuid,
                                 PrivacyTransactionState expectedState,
                                 qlonglong expectedGeneration,
                                 QString* const detail) -> bool
    {
        PrivacyRepositorySnapshot fresh;

        if (!m_persistence.loadSnapshot(&fresh))
        {
            *detail = QStringLiteral("the privacy catalogue could not be re-read");
            return false;
        }

        const PrivacyTransaction* current = nullptr;

        for (const PrivacyTransaction& candidate : fresh.transactions)
        {
            if (candidate.uuid == migrationTransactionUuid)
            {
                current = &candidate;
                break;
            }
        }

        if (!current)
        {
            *detail = QStringLiteral("the migration transaction disappeared");
            return false;
        }

        PrivacyTransaction updated = *current;
        updated.state = PrivacyTransactionState::Applying;
        updated.generation = expectedGeneration + 1;
        updated.payloadData = encodePayload(
            QLatin1String("protected-to-protected"), request.itemUuid,
            request.sourceCategoryUuid, request.targetCategoryUuid,
            sourceContainer.uuid, compatUuid, nextStage);
        updated.updatedAt = QDateTime::currentDateTimeUtc();

        return m_persistence.compareAndUpdateTransaction(
            updated, expectedState, expectedGeneration);
    };

    // Any active Compatibility exposure for this item must be resolved before
    // we proceed; before target publication that means a safe rollback.
    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.isActive() &&
            (candidate.type == PrivacyTransactionType::CompatibilityUnlock) &&
            (candidate.itemUuid == request.itemUuid))
        {
            if ((stage == QLatin1String("target-published")) &&
                !compatTransactionUuid.isEmpty() &&
                (candidate.uuid == compatTransactionUuid))
            {
                // The exposure is intentionally consumed by the migration.
                break;
            }

            QString rollbackDetail;

            if (!rollbackCompatibility(request, candidate.uuid,
                                        &rollbackDetail))
            {
                return itemFailure(PrivacyMigrationStatus::RolledBack,
                                   request.itemUuid, rollbackDetail,
                                   migrationTransactionUuid);
            }

            PrivacyRepositorySnapshot rolledBack;
            m_persistence.loadSnapshot(&rolledBack);
            const PrivacyTransaction* failedMigration = nullptr;

            for (const PrivacyTransaction& candidate :
                 rolledBack.transactions)
            {
                if (candidate.uuid == migrationTransactionUuid)
                {
                    failedMigration = &candidate;
                    break;
                }
            }

            if (!failedMigration ||
                !m_persistence.compareAndUpdateTransaction(
                    [&]()
                    {
                        PrivacyTransaction failed = *failedMigration;
                        failed.state = PrivacyTransactionState::Error;
                        failed.generation =
                            failedMigration->generation + 1;
                        failed.updatedAt =
                            QDateTime::currentDateTimeUtc();
                        return failed;
                    }(),
                    failedMigration->state,
                    failedMigration->generation))
            {
                return itemFailure(PrivacyMigrationStatus::RolledBack,
                                   request.itemUuid,
                                   QStringLiteral(
                                       "the migration could not be marked failed"),
                                   migrationTransactionUuid);
            }

            return itemFailure(PrivacyMigrationStatus::RolledBack,
                               request.itemUuid,
                               QStringLiteral(
                                   "the migration rolled back to the source proxy"),
                               migrationTransactionUuid);
        }
    }

    if (stage == QLatin1String("created"))
    {
        PrivacyCompatibilityUnlockRequest unlockRequest;
        unlockRequest.imageId = request.imageId;
        unlockRequest.categoryUuid = request.sourceCategoryUuid;
        unlockRequest.itemUuid = request.itemUuid;
        unlockRequest.transactionUuid =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        unlockRequest.groupUuid =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        unlockRequest.publicRoot = request.publicRoot;
        unlockRequest.rootExpectation = request.rootExpectation;
        unlockRequest.vaultPlaintextRoot =
            request.sourceVaultPlaintextRoot;
        unlockRequest.strongStoreUuid = request.sourceStrongStoreUuid;
        const PrivacyStillItemTransactionResult unlocked =
            m_stillEngine.compatibilityUnlock(
                unlockRequest, *request.sourcePassword);

        if (unlocked.status !=
            PrivacyStillItemTransactionStatus::CompatibilityUnlocked)
        {
            return itemFailure(PrivacyMigrationStatus::Failed,
                               request.itemUuid, unlocked.detail,
                               migrationTransactionUuid);
        }

        compatTransactionUuid = unlockRequest.transactionUuid;
        QString detail;

        if (!updateStage(QLatin1String("exposed"), compatTransactionUuid,
                         PrivacyTransactionState::Created, 0, &detail))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid, detail,
                               migrationTransactionUuid);
        }

        stage = QLatin1String("exposed");
    }

    if (stage == QLatin1String("exposed"))
    {
        const QString targetContainerUuid =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        PrivacyItem targetItem;
        PrivacyContainer targetContainer;
        QList<PrivacyAsset> targetAssets;
        QByteArray proxyBytes;
        QString detail;

        if (!publishTarget(request, *item, sourceContainer,
                           targetContainerUuid, migrationTransactionUuid,
                           &targetItem, &targetContainer, &targetAssets,
                           &proxyBytes, &detail))
        {
            QString rollbackDetail;
            rollbackCompatibility(request, compatTransactionUuid,
                                  &rollbackDetail);
            updateStage(QLatin1String("created"), compatTransactionUuid,
                        PrivacyTransactionState::Applying, 1,
                        &rollbackDetail);

            return itemFailure(PrivacyMigrationStatus::RolledBack,
                               request.itemUuid,
                               detail.isEmpty() ? rollbackDetail : detail,
                               migrationTransactionUuid);
        }

        PrivacyRepositorySnapshot fresh;
        m_persistence.loadSnapshot(&fresh);
        const PrivacyTransaction* currentMigration = nullptr;

        for (const PrivacyTransaction& candidate : fresh.transactions)
        {
            if (candidate.uuid == migrationTransactionUuid)
            {
                currentMigration = &candidate;
                break;
            }
        }

        if (!currentMigration)
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               QStringLiteral("the migration transaction disappeared"),
                               migrationTransactionUuid);
        }

        PrivacyTransaction published = *currentMigration;
        published.state = PrivacyTransactionState::Applying;
        published.generation = currentMigration->generation + 1;
        published.payloadData = encodePayload(
            QLatin1String("protected-to-protected"), request.itemUuid,
            request.sourceCategoryUuid, request.targetCategoryUuid,
            sourceContainer.uuid, compatTransactionUuid,
            QLatin1String("mapping-published"));
        published.updatedAt = QDateTime::currentDateTimeUtc();

        if (!m_persistence.publishMigration(
                targetItem, targetContainer, targetAssets,
                sourceContainer.uuid, published,
                currentMigration->state, currentMigration->generation))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               QStringLiteral("the target mapping could not be published"),
                               migrationTransactionUuid);
        }

        stage = QLatin1String("mapping-published");
    }

    if (stage == QLatin1String("mapping-published"))
    {
        QString detail;

        if (!retireSourceObject(request, sourceContainer, &detail) ||
            !installTargetProxy(request, sourceContainer, &detail))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid, detail,
                               migrationTransactionUuid);
        }

        PrivacyRepositorySnapshot fresh;
        m_persistence.loadSnapshot(&fresh);
        const PrivacyTransaction* currentMigration = nullptr;

        for (const PrivacyTransaction& candidate : fresh.transactions)
        {
            if (candidate.uuid == migrationTransactionUuid)
            {
                currentMigration = &candidate;
                break;
            }
        }

        if (!currentMigration ||
            !updateStage(QLatin1String("target-published"),
                         compatTransactionUuid,
                         currentMigration->state,
                         currentMigration->generation, &detail))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               currentMigration
                                   ? detail
                                   : QStringLiteral("the migration transaction disappeared"),
                               migrationTransactionUuid);
        }

        stage = QLatin1String("target-published");
    }

    if (stage == QLatin1String("target-published"))
    {
        QString detail;

        if (!retireSourceObject(request, sourceContainer, &detail) ||
            !finalizeCompatibility(request, compatTransactionUuid, &detail))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid, detail,
                               migrationTransactionUuid);
        }

        PrivacyRepositorySnapshot finalSnapshot;
        m_persistence.loadSnapshot(&finalSnapshot);
        const PrivacyTransaction* finalMigration = nullptr;

        for (const PrivacyTransaction& candidate :
             finalSnapshot.transactions)
        {
            if (candidate.uuid == migrationTransactionUuid)
            {
                finalMigration = &candidate;
                break;
            }
        }

        if (!finalMigration ||
            !m_persistence.compareAndUpdateTransaction(
                [&]()
                {
                    PrivacyTransaction completed = *finalMigration;
                    completed.state = PrivacyTransactionState::Complete;
                    completed.generation =
                        finalMigration->generation + 1;
                    completed.updatedAt =
                        QDateTime::currentDateTimeUtc();
                    return completed;
                }(),
                finalMigration->state, finalMigration->generation))
        {
            return itemFailure(PrivacyMigrationStatus::RecoveryRequired,
                               request.itemUuid,
                               QStringLiteral("the migration could not be completed"),
                               migrationTransactionUuid);
        }
    }

    PrivacyMigrationItemResult migrated;
    migrated.status = PrivacyMigrationStatus::Migrated;
    migrated.itemUuid = request.itemUuid;
    migrated.transactionUuid = migrationTransactionUuid;
    return migrated;
}

bool PrivacyMigrationCoordinator::buildTargetProtectRequest(
    const PrivacyMigrationRequest& request,
    const QString& containerUuid,
    const QString& transactionUuid,
    PrivacyStillProtectRequest* const protectRequest) const
{
    if (!protectRequest || request.assets.isEmpty())
    {
        return false;
    }

    PrivacyAssetInventoryBridgeItemResult bridgeItem;
    bridgeItem.imageId = request.imageId;
    bridgeItem.inventory.status = PrivacyInventoryStatus::Ready;

    for (const PrivacyMigrationAssetInput& input : request.assets)
    {
        PrivacyInventoryAsset inventoryAsset;
        inventoryAsset.role = static_cast<PrivacyInventoryAssetRole>(
            input.role);
        inventoryAsset.ordinal = input.ordinal;
        inventoryAsset.location.root.uuid = request.publicRoot.uuid;
        inventoryAsset.location.root.absolutePath =
            request.publicRoot.configuredPath;
        inventoryAsset.location.relativePath = input.publicRelativePath;

        const QString absolutePath = QDir(
            request.publicRoot.configuredPath).filePath(
                input.publicRelativePath);

        if (!fileFacts(absolutePath, &inventoryAsset))
        {
            return false;
        }

        bridgeItem.inventory.requiredAssets << inventoryAsset;
    }

    protectRequest->imageId = request.imageId;
    protectRequest->categoryUuid = request.targetCategoryUuid;
    protectRequest->itemUuid = request.itemUuid;
    protectRequest->containerUuid = containerUuid;
    protectRequest->transactionUuid = transactionUuid;
    protectRequest->preflight.bridge.status = PrivacyInventoryStatus::Ready;
    protectRequest->preflight.bridge.items << bridgeItem;
    protectRequest->associatedAssetsAcknowledged = true;
    protectRequest->publicRoot = request.publicRoot;
    protectRequest->rootExpectation = request.rootExpectation;
    protectRequest->originalPixelSize = QSize(1, 1);
    protectRequest->originalCreationDate = QDateTime::currentDateTimeUtc();
    protectRequest->vaultPlaintextRoot =
        request.targetVaultPlaintextRoot;
    protectRequest->strongStoreUuid = request.targetStrongStoreUuid;
    return true;
}

bool PrivacyMigrationCoordinator::publishTarget(
    const PrivacyMigrationRequest& request, const PrivacyItem& sourceItem,
    const PrivacyContainer& sourceContainer,
    const QString& targetContainerUuid, const QString& migrationTransactionUuid,
    PrivacyItem* const targetItem, PrivacyContainer* const targetContainer,
    QList<PrivacyAsset>* const targetAssets, QByteArray* const targetProxyBytes,
    QString* const detail)
{
    if (!targetItem || !targetContainer || !targetAssets ||
        !targetProxyBytes)
    {
        *detail = QStringLiteral("the target publication outputs are missing");
        return false;
    }

    struct ExposedMember
    {
        PrivacyMigrationAssetInput input;
        QString  absolutePath;
        QByteArray hash;
        qlonglong size = -1;
        quint32   mode = 0;
        QDateTime modificationTime;
    };

    QList<ExposedMember> members;
    bool foundPrimary = false;

    for (const PrivacyMigrationAssetInput& input : request.assets)
    {
        const QString absolutePath = QDir(
            request.publicRoot.configuredPath).filePath(
                input.publicRelativePath);
        const QByteArray hash = hashFileBytes(absolutePath);
        const quint32 mode = fileMode(absolutePath);
        const QDateTime modificationTime = fileModificationTime(absolutePath);
        const qlonglong size = QFileInfo(absolutePath).size();

        if (hash.isEmpty() || !mode || !modificationTime.isValid() ||
            (size < 0))
        {
            *detail = QStringLiteral(
                "an exposed source member could not be verified");
            return false;
        }

        ExposedMember member;
        member.input = input;
        member.absolutePath = absolutePath;
        member.hash = hash;
        member.size = size;
        member.mode = mode;
        member.modificationTime = modificationTime;
        members << member;

        if ((input.role == PrivacyAsset::PrimaryMediaRole) &&
            (input.ordinal == 0))
        {
            foundPrimary = true;
        }
    }

    if (!foundPrimary)
    {
        *detail = QStringLiteral("the primary exposed member is missing");
        return false;
    }

    PrivacyRepositorySnapshot targetSnapshot;

    if (!m_persistence.loadSnapshot(&targetSnapshot))
    {
        *detail = QStringLiteral("the privacy catalogue could not be read");
        return false;
    }

    QString targetRecoverySetUuid;

    for (const PrivacyCategory& category : targetSnapshot.categories)
    {
        if (category.uuid == request.targetCategoryUuid)
        {
            targetRecoverySetUuid = category.recoverySetUuid;
            break;
        }
    }

    if (targetRecoverySetUuid.isEmpty())
    {
        *detail = QStringLiteral("the target category is missing");
        return false;
    }

    const bool targetStrong =
        (request.targetBackend == PrivacyBackend::Strong);
    const QString targetArchiveRelative =
        expectedArchivePath(request.assets);
    const QString targetObjectRelative =
        targetStrong
            ? QLatin1String("originals/") + targetContainerUuid
            : targetArchiveRelative;
    qlonglong containerSize = -1;
    QByteArray containerHash;

    if (targetStrong)
    {
        if (request.targetVaultPlaintextRoot.isEmpty() ||
            request.targetStrongStoreUuid.isEmpty())
        {
            *detail = QStringLiteral(
                "the target Strong vault context is missing");
            return false;
        }

        QList<PrivacyStrongObjectMember> strongMembers;

        for (const ExposedMember& member : members)
        {
            PrivacyStrongObjectMember strongMember;
            strongMember.sourcePath = member.absolutePath;
            strongMember.protectedRelativePath =
                targetObjectRelative + QLatin1Char('/') +
                QString::number(member.input.ordinal) + QLatin1Char('-') +
                member.input.originalName;
            strongMember.originalName = member.input.originalName;
            strongMember.expectedSize = member.size;
            strongMember.expectedSha256 = member.hash;
            strongMembers << strongMember;
        }

        const QString stagedRelative =
            QLatin1String("originals/.staging-") +
            migrationTransactionUuid;
        QString backendDetail;
        const PrivacyStrongObjectStageResult staged =
            PrivacyStrongObjectBackend::stageObjects(
                request.targetVaultPlaintextRoot, stagedRelative,
                strongMembers, &backendDetail);

        if (!staged.valid ||
            !PrivacyStrongObjectBackend::publishObjects(
                request.targetVaultPlaintextRoot, stagedRelative,
                targetObjectRelative, strongMembers, staged.totalSize,
                staged.totalSha256, &backendDetail) ||
            !PrivacyStrongObjectBackend::verifyObjects(
                request.targetVaultPlaintextRoot, targetObjectRelative,
                strongMembers, staged.totalSize, staged.totalSha256,
                &backendDetail))
        {
            *detail = backendDetail;
            return false;
        }

        containerSize = staged.totalSize;
        containerHash = staged.totalSha256;
    }
    else
    {
        if (targetArchiveRelative.isEmpty())
        {
            *detail = QStringLiteral("the target archive path is invalid");
            return false;
        }

        PrivacyCasualArchiveRequest archiveRequest;
        archiveRequest.finalArchivePath = QDir(
            request.publicRoot.configuredPath).filePath(
                targetArchiveRelative);
        archiveRequest.categoryUuid = request.targetCategoryUuid;
        archiveRequest.containerUuid = targetContainerUuid;
        archiveRequest.itemUuid = request.itemUuid;
        archiveRequest.recoverySetUuid = targetRecoverySetUuid;

        for (const ExposedMember& member : members)
        {
            PrivacyCasualArchiveMember archiveMember;
            archiveMember.sourcePath = member.absolutePath;
            archiveMember.protectedRelativePath =
                PrivacyCasualArchiveEngine::expectedMemberPath(
                    member.input.role, member.input.ordinal,
                    member.input.originalName);
            archiveMember.originalName = member.input.originalName;
            archiveMember.role = member.input.role;
            archiveMember.ordinal = member.input.ordinal;
            archiveMember.originalCreationDate = member.modificationTime;
            archiveMember.originalModificationDate =
                member.modificationTime;
            archiveMember.portableAttributes =
                encodePortableMode(member.mode);
            archiveMember.expectedSize = member.size;
            archiveMember.expectedSha256 = member.hash;
            archiveRequest.members << archiveMember;
        }

        PrivacyCasualArchiveError archiveError =
            PrivacyCasualArchiveError::None;
        PrivacyCasualArchiveStage stage;
        const bool samePathArchive =
            (sourceContainer.kind == PrivacyContainerKind::CasualArchive) &&
            (sourceContainer.objectRelativePath == targetArchiveRelative);

        if (samePathArchive)
        {
            const QByteArray oldHash = hashFileBytes(
                archiveRequest.finalArchivePath);

            if (oldHash.isEmpty())
            {
                *detail = QStringLiteral("the source archive is missing");
                return false;
            }

            PrivacyCasualArchiveStage rewritten = m_archiveEngine.rewriteArchive(
                archiveRequest, *request.sourcePassword,
                *request.targetPassword, {}, &archiveError);

            if (!rewritten.isValid() ||
                !m_archiveEngine.publishReplacement(
                    &rewritten, oldHash, &archiveError))
            {
                *detail = QStringLiteral(
                    "the target archive could not be rewritten");
                return false;
            }

            containerHash = hashFileBytes(
                archiveRequest.finalArchivePath);
            containerSize = QFileInfo(
                archiveRequest.finalArchivePath).size();
        }
        else
        {
            PrivacyCasualArchiveStage staged = m_archiveEngine.stageArchive(
                archiveRequest, *request.targetPassword, {},
                &archiveError);

            if (!staged.isValid() ||
                !m_archiveEngine.publishNew(&staged, &archiveError))
            {
                *detail = QStringLiteral(
                    "the target archive could not be published");
                return false;
            }

            containerHash = staged.archiveSha256();
            containerSize = staged.archiveSize();
        }
    }

    const ExposedMember* primary = nullptr;

    for (const ExposedMember& member : members)
    {
        if ((member.input.role == PrivacyAsset::PrimaryMediaRole) &&
            (member.input.ordinal == 0))
        {
            primary = &member;
            break;
        }
    }

    PrivacyStillProxyRequest proxyRequest;
    proxyRequest.sourcePath = primary->absolutePath;
    proxyRequest.publicFileName = primary->input.originalName;
    proxyRequest.presentation = PrivacyStillProxyPresentation::Generic;
    const PrivacyStillProxyResult proxy =
        PrivacyStillProxyGenerator().generate(proxyRequest);

    if (!proxy.isValid() || proxy.encodedBytes.isEmpty())
    {
        *detail = QStringLiteral("the target proxy could not be generated");
        return false;
    }

    qlonglong targetCredentialGeneration = -1;

    for (const PrivacyCategory& category : targetSnapshot.categories)
    {
        if (category.uuid == request.targetCategoryUuid)
        {
            targetCredentialGeneration =
                category.currentCredentialGeneration;
            break;
        }
    }

    if (targetCredentialGeneration < 0)
    {
        *detail = QStringLiteral("the target category is missing");
        return false;
    }

    PrivacyItem item = sourceItem;
    item.categoryUuid = request.targetCategoryUuid;
    item.expectedProxyHash =
        QString::fromLatin1(proxy.sha256.toHex());
    item.expectedProxySize = proxy.encodedBytes.size();
    item.presentationVersion = 1;
    item.generation = sourceItem.generation + 1;
    item.transactionState =
        static_cast<int>(PrivacyTransactionState::Complete);

    PrivacyContainer container;
    container.uuid = targetContainerUuid;
    container.itemUuid = request.itemUuid;
    container.kind = targetStrong
        ? PrivacyContainerKind::StrongObject
        : PrivacyContainerKind::CasualArchive;
    container.rootUuid = targetStrong
        ? QString() : request.publicRoot.uuid;
    container.storeUuid = targetStrong
        ? request.targetStrongStoreUuid : QString();
    container.objectRelativePath = targetObjectRelative;
    container.protectedSize = containerSize;
    container.protectedHashAlgorithm = QLatin1String("sha256");
    container.protectedHash =
        QString::fromLatin1(containerHash.toHex());
    container.formatVersion = 1;
    container.credentialGeneration = targetCredentialGeneration;
    container.state = PrivacyContainerState::Verified;
    container.createdAt = QDateTime::currentDateTimeUtc();
    container.updatedAt = container.createdAt;

    QList<PrivacyAsset> assets;

    for (const ExposedMember& member : members)
    {
        PrivacyAsset asset;
        asset.itemUuid = request.itemUuid;
        asset.role = member.input.role;
        asset.ordinal = member.input.ordinal;
        asset.originalName = member.input.originalName;
        asset.publicRootUuid = request.publicRoot.uuid;
        asset.publicRelativePath = member.input.publicRelativePath;
        asset.containerUuid = targetContainerUuid;
        asset.protectedRelativePath = targetStrong
            ? targetObjectRelative + QLatin1Char('/') +
              QString::number(member.input.ordinal) + QLatin1Char('-') +
              member.input.originalName
            : PrivacyCasualArchiveEngine::expectedMemberPath(
                  member.input.role, member.input.ordinal,
                  member.input.originalName);
        asset.hashAlgorithm = QLatin1String("sha256");
        asset.originalHash =
            QString::fromLatin1(member.hash.toHex());
        asset.originalSize = member.size;
        asset.originalCreationDate = member.modificationTime;
        asset.originalModificationDate = member.modificationTime;
        asset.portableAttributes = encodePortableMode(member.mode);

        if ((member.input.role == PrivacyAsset::PrimaryMediaRole) &&
            (member.input.ordinal == 0))
        {
            asset.proxyHashAlgorithm = QLatin1String("sha256");
            asset.proxyHash = item.expectedProxyHash;
            asset.proxySize = item.expectedProxySize;
            asset.proxyPresentationVersion = item.presentationVersion;
            asset.proxyGeneration = item.generation;
        }

        assets << asset;
    }

    *targetItem = item;
    *targetContainer = container;
    *targetAssets = assets;
    *targetProxyBytes = proxy.encodedBytes;
    m_pendingProxyBytes = proxy.encodedBytes;
    return true;
}

bool PrivacyMigrationCoordinator::retireSourceObject(
    const PrivacyMigrationRequest& request,
    const PrivacyContainer& sourceContainer, QString* const detail)
{
    if ((sourceContainer.kind == PrivacyContainerKind::CasualArchive) &&
        (request.targetBackend == PrivacyBackend::Casual) &&
        (sourceContainer.objectRelativePath ==
         expectedArchivePath(request.assets)))
    {
        // The source archive was rewritten in place into the target archive.
        return true;
    }

    if (sourceContainer.kind == PrivacyContainerKind::StrongObject)
    {
        QString backendDetail;

        if (!PrivacyStrongObjectBackend::removeObjects(
                request.sourceVaultPlaintextRoot,
                sourceContainer.objectRelativePath, &backendDetail))
        {
            *detail = backendDetail;
            return false;
        }
    }
    else
    {
        const QString archivePath = QDir(
            request.publicRoot.configuredPath).filePath(
                sourceContainer.objectRelativePath);

        if (QFileInfo::exists(archivePath) &&
            !QFile::remove(archivePath))
        {
            *detail = QStringLiteral(
                "the source archive could not be removed");
            return false;
        }

        fsyncDirectory(QFileInfo(archivePath).absolutePath());
    }

    return true;
}

bool PrivacyMigrationCoordinator::installTargetProxy(
    const PrivacyMigrationRequest& request,
    const PrivacyContainer&, QString* const detail)
{
    if (m_pendingProxyBytes.isEmpty())
    {
        *detail = QStringLiteral("the target proxy bytes are missing");
        return false;
    }

    for (const PrivacyMigrationAssetInput& input : request.assets)
    {
        const QString path = QDir(
            request.publicRoot.configuredPath).filePath(
                input.publicRelativePath);

        if ((input.role == PrivacyAsset::PrimaryMediaRole) &&
            (input.ordinal == 0))
        {
            if (!writeFileAtomic(path, m_pendingProxyBytes))
            {
                *detail = QStringLiteral(
                    "the target proxy could not be installed");
                return false;
            }
        }
        else if (QFileInfo::exists(path) && !QFile::remove(path))
        {
            *detail = QStringLiteral(
                "an associated public path could not be removed");
            return false;
        }
        else
        {
            fsyncDirectory(QFileInfo(path).absolutePath());
        }
    }

    m_pendingProxyBytes.clear();
    return true;
}

bool PrivacyMigrationCoordinator::finalizeCompatibility(
    const PrivacyMigrationRequest& request,
    const QString& compatTransactionUuid, QString* const detail)
{
    if (compatTransactionUuid.isEmpty())
    {
        *detail = QStringLiteral("the compatibility transaction is missing");
        return false;
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        *detail = QStringLiteral("the privacy catalogue could not be read");
        return false;
    }

    const PrivacyTransaction* compatibility = nullptr;

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.uuid == compatTransactionUuid)
        {
            compatibility = &candidate;
            break;
        }
    }

    if (!compatibility)
    {
        *detail = QStringLiteral("the compatibility transaction is missing");
        return false;
    }

    if (compatibility->state != PrivacyTransactionState::Complete)
    {
        PrivacyTransaction completed = *compatibility;
        completed.state = PrivacyTransactionState::Complete;
        completed.generation = compatibility->generation + 1;
        completed.updatedAt = QDateTime::currentDateTimeUtc();

        if (!m_persistence.compareAndUpdateTransaction(
                completed, compatibility->state,
                compatibility->generation))
        {
            *detail = QStringLiteral(
                "the compatibility transaction could not be finalized");
            return false;
        }
    }

    m_runtime.publishCompatibilityExposure(request.imageId,
                                           request.itemUuid, false);
    return true;
}

bool PrivacyMigrationCoordinator::rollbackCompatibility(
    const PrivacyMigrationRequest& request,
    const QString& compatTransactionUuid, QString* const detail)
{
    const PrivacyStillItemTransactionResult result =
        m_stillEngine.recover(request.publicRoot, compatTransactionUuid);

    if ((result.status !=
         PrivacyStillItemTransactionStatus::CompatibilityRelocked) &&
        (result.status !=
         PrivacyStillItemTransactionStatus::CompatibilityUnlocked))
    {
        *detail = result.detail;
        return false;
    }

    return true;
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <functional>
#include <memory>

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacycasualarchive.h"
#include "privacystillitemtransaction.h"

namespace Digikam
{

enum class PrivacyMigrationKind
{
    ToProtected = 1,
    ToUnprotected = 2,
    ProtectedToProtected = 3
};

struct DIGIKAM_DATABASE_EXPORT PrivacyMigrationAssetInput
{
    int    role = 0;
    int    ordinal = -1;
    QString publicRelativePath;
    QString originalName;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyMigrationRequest
{
    qlonglong imageId = -1;
    QString   itemUuid;
    QString   sourceCategoryUuid;
    QString   targetCategoryUuid;
    PrivacyBackend targetBackend = PrivacyBackend::Casual;
    PrivacyStorageRoot publicRoot;
    PrivacyJournalRootExpectation rootExpectation;
    QList<PrivacyMigrationAssetInput> assets;
    QString sourceVaultPlaintextRoot;
    QString sourceStrongStoreUuid;
    QString targetVaultPlaintextRoot;
    QString targetStrongStoreUuid;
    std::shared_ptr<const PrivacyPassword> sourcePassword;
    std::shared_ptr<const PrivacyPassword> targetPassword;
};

enum class PrivacyMigrationStatus
{
    Migrated = 1,
    Failed,
    RolledBack,
    RecoveryRequired,
    AlreadyActive,
    InvalidRequest
};

struct DIGIKAM_DATABASE_EXPORT PrivacyMigrationItemResult
{
    bool succeeded() const
    {
        return (status == PrivacyMigrationStatus::Migrated);
    }

    PrivacyMigrationStatus status = PrivacyMigrationStatus::InvalidRequest;
    QString itemUuid;
    QString transactionUuid;
    QString detail;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyMigrationBatchResult
{
    bool succeeded() const
    {
        return (remainingCount == 0);
    }

    int requestedCount = 0;
    int processedCount = 0;
    int remainingCount = 0;
    int succeededCount = 0;
    QList<PrivacyMigrationItemResult> items;
};

/**
 * Durable per-item migration seams. Each item runs under one
 * MigrateBackend transaction; the source protected container is retired only
 * after the target container and P1 mapping are verified.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyMigrationPersistence
{
public:

    virtual ~PrivacyMigrationPersistence() = default;

    virtual bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const = 0;
    virtual bool beginMigration(const PrivacyTransaction& transaction,
                                const PrivacyTransactionJournal& journal) = 0;
    virtual bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) = 0;
    virtual bool publishMigration(
        const PrivacyItem& item,
        const PrivacyContainer& container,
        const QList<PrivacyAsset>& assets,
        const QString& sourceContainerUuid,
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) = 0;
    virtual bool removeContainerAndAssets(const QString& containerUuid,
                                          const QString& itemUuid) = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCoreDbMigrationPersistence final
    : public PrivacyMigrationPersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const override;
    bool beginMigration(const PrivacyTransaction& transaction,
                        const PrivacyTransactionJournal& journal) override;
    bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) override;
    bool publishMigration(const PrivacyItem& item,
                          const PrivacyContainer& container,
                          const QList<PrivacyAsset>& assets,
                          const QString& sourceContainerUuid,
                          const PrivacyTransaction& transaction,
                          PrivacyTransactionState expectedState,
                          qlonglong expectedGeneration) override;
    bool removeContainerAndAssets(const QString& containerUuid,
                                  const QString& itemUuid) override;
};

/**
 * Executes one item/associated-asset set at a time across unprotected,
 * Casual and Strong states or categories. Protected-to-protected migration
 * exposes the source through Compatibility Unlock (source container
 * retained), publishes the target mapping, and only then retires the source
 * container and protected object. Any interruption before target publication
 * rolls back to the source proxy; after publication, recovery retires the
 * source and finalizes the migration.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyMigrationCoordinator
{
public:

    using BatchProgress = std::function<void(int, int)>;

    PrivacyMigrationCoordinator(PrivacyMigrationPersistence& persistence,
                                PrivacyStillItemTransactionEngine& stillEngine,
                                PrivacyRuntimeCoordinator& runtime,
                                PrivacyCasualArchiveEngine& archiveEngine);

    PrivacyMigrationBatchResult migrateBatch(
        const QList<PrivacyMigrationRequest>& requests,
        const BatchProgress& progress = {});
    PrivacyMigrationBatchResult recover();

private:

    struct Bundle
    {
        PrivacyItem item;
        PrivacyContainer sourceContainer;
        QList<PrivacyAsset> sourceAssets;
        PrivacyCategory sourceCategory;
        PrivacyCategory targetCategory;
    };

    PrivacyMigrationItemResult migrateOne(const PrivacyMigrationRequest& request,
                                          bool resume);
    PrivacyMigrationItemResult migrateToProtected(
        const PrivacyMigrationRequest& request);
    PrivacyMigrationItemResult migrateToUnprotected(
        const PrivacyMigrationRequest& request);
    PrivacyMigrationItemResult migrateProtectedToProtected(
        const PrivacyMigrationRequest& request, bool resume);
    bool buildTargetProtectRequest(
        const PrivacyMigrationRequest& request,
        const QString& containerUuid,
        const QString& transactionUuid,
        PrivacyStillProtectRequest* protectRequest) const;
    bool publishTarget(const PrivacyMigrationRequest& request,
                       const PrivacyItem& sourceItem,
                       const PrivacyContainer& sourceContainer,
                       const QString& targetContainerUuid,
                       const QString& migrationTransactionUuid,
                       PrivacyItem* targetItem,
                       PrivacyContainer* targetContainer,
                       QList<PrivacyAsset>* targetAssets,
                       QByteArray* targetProxyBytes,
                       QString* detail);
    bool retireSourceObject(const PrivacyMigrationRequest& request,
                            const PrivacyContainer& sourceContainer,
                            QString* detail);
    bool installTargetProxy(const PrivacyMigrationRequest& request,
                            const PrivacyContainer& sourceContainer,
                            QString* detail);
    bool finalizeCompatibility(const PrivacyMigrationRequest& request,
                               const QString& compatTransactionUuid,
                               QString* detail);
    bool rollbackCompatibility(const PrivacyMigrationRequest& request,
                               const QString& compatTransactionUuid,
                               QString* detail);

    PrivacyMigrationPersistence&     m_persistence;
    PrivacyStillItemTransactionEngine& m_stillEngine;
    PrivacyRuntimeCoordinator&       m_runtime;
    PrivacyCasualArchiveEngine&      m_archiveEngine;
    QByteArray                       m_pendingProxyBytes;
};

} // namespace Digikam

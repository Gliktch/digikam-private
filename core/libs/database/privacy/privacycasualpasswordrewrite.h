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

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacycategorysession.h"
#include "privacycasualarchive.h"

namespace Digikam
{

enum class PrivacyCasualPasswordRewriteStatus
{
    Rewritten = 1,
    RecoveryRequired,
    AlreadyActive,
    InvalidRequest,
    AuthenticationFailed,
    ArchiveFailure,
    StoreFailure,
    JournalFailure,
    PersistenceFailure
};

struct DIGIKAM_DATABASE_EXPORT PrivacyCasualPasswordRewriteResult
{
    bool succeeded() const
    {
        return (status == PrivacyCasualPasswordRewriteStatus::Rewritten);
    }

    PrivacyCasualPasswordRewriteStatus status =
        PrivacyCasualPasswordRewriteStatus::InvalidRequest;
    QString transactionUuid;
    QString detail;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyCasualPasswordRewriteSpaceCheck
{
    bool     valid = false;
    qlonglong largestArchiveBytes = -1;
    qlonglong requiredBytes = -1;
    qlonglong availableBytes = -1;
    bool     insufficient = false;
    QString  detail;
};

/**
 * Durable seams for one atomic Casual category password rewrite. The
 * ChangePassword transaction is begun before any archive changes; every
 * Casual archive is rewritten to a verified sibling and its container
 * credential generation is CAS-advanced to N+1 before the category store is
 * rewrapped, so an interruption resumes only the remaining archives. The new
 * credential and category/store generations are published atomically at the
 * end, and the config backup is retained until publication.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyCasualPasswordRewritePersistence
{
public:

    virtual ~PrivacyCasualPasswordRewritePersistence() = default;

    virtual bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const = 0;
    virtual bool beginRewrap(const PrivacyTransaction& transaction,
                             const PrivacyTransactionJournal& journal) = 0;
    virtual bool publishRewrap(const QString& categoryUuid,
                               qlonglong categoryGeneration,
                               const PrivacyCredential& credential,
                               const QString& storeUuid,
                               qlonglong storeGeneration,
                               const PrivacyTransaction& transaction,
                               PrivacyTransactionState expectedState,
                               qlonglong expectedGeneration) = 0;
    virtual bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) = 0;
    virtual bool updateContainerCredentialGeneration(
        const QString& containerUuid,
        qlonglong expectedGeneration,
        qlonglong generation) = 0;
};

class DIGIKAM_DATABASE_EXPORT
    PrivacyCoreDbCasualPasswordRewritePersistence final
    : public PrivacyCasualPasswordRewritePersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const override;
    bool beginRewrap(const PrivacyTransaction& transaction,
                     const PrivacyTransactionJournal& journal) override;
    bool publishRewrap(const QString& categoryUuid,
                       qlonglong categoryGeneration,
                       const PrivacyCredential& credential,
                       const QString& storeUuid,
                       qlonglong storeGeneration,
                       const PrivacyTransaction& transaction,
                       PrivacyTransactionState expectedState,
                       qlonglong expectedGeneration) override;
    bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) override;
    bool updateContainerCredentialGeneration(
        const QString& containerUuid,
        qlonglong expectedGeneration,
        qlonglong generation) override;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCasualPasswordRewriteEngine
{
public:

    PrivacyCasualPasswordRewriteEngine(
        PrivacyCasualPasswordRewritePersistence& persistence,
        PrivacyCategoryStoreBackend& storeBackend,
        PrivacyCasualArchiveEngine& archiveEngine);

    PrivacyCasualPasswordRewriteResult rewrap(
        const QString& categoryUuid,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword);

    PrivacyCasualPasswordRewriteResult recover(
        const QString& categoryUuid,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword);

    /** Non-blocking preflight for the archive rewrite phase. The rewrite
     * keeps the old archive until the atomic exchange, so the required free
     * space is about twice the largest pending archive plus a small margin. */
    PrivacyCasualPasswordRewriteSpaceCheck checkSpace(
        const QString& categoryUuid) const;

    static qlonglong requiredSpaceForLargestArchive(
        qlonglong largestArchiveBytes);

private:

    struct Bundle
    {
        PrivacyCategory     category;
        PrivacyCredential   credential;
        PrivacyStore        store;
        PrivacyStorageRoot  root;
    };

    struct PendingContainer
    {
        PrivacyContainer container;
        PrivacyItem      item;
        QList<PrivacyAsset> assets;
    };

    bool loadBundle(const PrivacyRepositorySnapshot& snapshot,
                    const QString& categoryUuid,
                    Bundle* bundle,
                    PrivacyCasualPasswordRewriteResult* result) const;
    QList<PendingContainer> pendingContainers(
        const PrivacyRepositorySnapshot& snapshot,
        const Bundle& bundle) const;
    PrivacyCasualPasswordRewriteResult runRewrap(
        const Bundle& bundle, const QString& transactionUuid,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword);
    PrivacyCasualPasswordRewriteResult complete(
        const Bundle& bundle, const QByteArray& newConfig,
        const PrivacyTransaction& transaction,
        const QString& backupRelative,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration);

    PrivacyCasualPasswordRewritePersistence& m_persistence;
    PrivacyCategoryStoreBackend&             m_storeBackend;
    PrivacyCasualArchiveEngine&              m_archiveEngine;
};

} // namespace Digikam

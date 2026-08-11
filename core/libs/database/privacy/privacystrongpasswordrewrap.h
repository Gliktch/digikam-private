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

#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacycategorysession.h"

namespace Digikam
{

enum class PrivacyStrongPasswordRewrapStatus
{
    Rewrapped = 1,
    RecoveryRequired,
    AlreadyActive,
    InvalidRequest,
    AuthenticationFailed,
    StoreFailure,
    JournalFailure,
    PersistenceFailure
};

struct DIGIKAM_DATABASE_EXPORT PrivacyStrongPasswordRewrapResult
{
    bool succeeded() const
    {
        return (status == PrivacyStrongPasswordRewrapStatus::Rewrapped);
    }

    PrivacyStrongPasswordRewrapStatus status =
        PrivacyStrongPasswordRewrapStatus::InvalidRequest;
    QString transactionUuid;
    QString detail;
};

/**
 * Durable seams for one atomic Strong category password rewrap. The
 * transaction is created before the config backup, moved to Applying before
 * `gocryptfs -passwd`, and published (new credential + category/store
 * generation) atomically only after the new password is proven against the
 * same vault. The prior config backup is retained until publication.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyStrongPasswordRewrapPersistence
{
public:

    virtual ~PrivacyStrongPasswordRewrapPersistence() = default;

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
};

class DIGIKAM_DATABASE_EXPORT
    PrivacyCoreDbStrongPasswordRewrapPersistence final
    : public PrivacyStrongPasswordRewrapPersistence
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
};

class DIGIKAM_DATABASE_EXPORT PrivacyStrongPasswordRewrapEngine
{
public:

    PrivacyStrongPasswordRewrapEngine(
        PrivacyStrongPasswordRewrapPersistence& persistence,
        PrivacyCategoryStoreBackend& storeBackend);

    PrivacyStrongPasswordRewrapResult rewrap(
        const QString& categoryUuid,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword);

    PrivacyStrongPasswordRewrapResult recover(
        const QString& categoryUuid,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword);

private:

    struct Bundle
    {
        PrivacyCategory     category;
        PrivacyCredential   credential;
        PrivacyStore        store;
        PrivacyStorageRoot  root;
    };

    bool loadBundle(const PrivacyRepositorySnapshot& snapshot,
                    const QString& categoryUuid,
                    Bundle* bundle,
                    PrivacyStrongPasswordRewrapResult* result) const;
    PrivacyStrongPasswordRewrapResult runRewrap(
        const Bundle& bundle, const QString& transactionUuid,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword);
    PrivacyStrongPasswordRewrapResult complete(
        const Bundle& bundle, const QByteArray& newConfig,
        const PrivacyTransaction& transaction,
        const QString& backupDirectory,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration);

    PrivacyStrongPasswordRewrapPersistence& m_persistence;
    PrivacyCategoryStoreBackend&            m_storeBackend;
};

} // namespace Digikam

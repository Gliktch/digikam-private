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

// Qt includes

#include <QList>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "privacytransactionjournal.h"

namespace Digikam
{

enum class PrivacyExternalCheckoutStatus
{
    Ready                 = 1,
    CompletedUnchanged    = 2,
    ChangesPending        = 3,
    AuthenticationRequired = 4,
    InvalidRequest        = 5,
    ItemUnavailable       = 6,
    RootUnavailable       = 7,
    JournalFailure        = 8,
    PersistenceFailure    = 9,
    CheckoutFailure       = 10,
    RecoveryRequired      = 11
};

class DIGIKAM_DATABASE_EXPORT PrivacyExternalCheckoutAssetSource
{
public:

    int role = 0;
    int ordinal = -1;
    std::function<bool(int, QString*)> producer;
};

class DIGIKAM_DATABASE_EXPORT PrivacyExternalCheckoutRequest
{
public:

    qlonglong imageId = -1;
    QString categoryUuid;
    QString transactionUuid;
    PrivacyStorageRoot root;
    PrivacyJournalRootExpectation rootExpectation;
    QList<PrivacyExternalCheckoutAssetSource> sources;
};

class DIGIKAM_DATABASE_EXPORT PrivacyExternalCheckoutAsset
{
public:

    int role = 0;
    int ordinal = -1;
    QUrl logicalUrl;
    QUrl checkoutUrl;
};

class DIGIKAM_DATABASE_EXPORT PrivacyExternalCheckoutResult
{
public:

    bool succeeded() const;

public:

    PrivacyExternalCheckoutStatus status =
        PrivacyExternalCheckoutStatus::InvalidRequest;
    QString transactionUuid;
    QString itemUuid;
    QList<PrivacyExternalCheckoutAsset> assets;
    QString detail;
};

class DIGIKAM_DATABASE_EXPORT PrivacyExternalCheckoutPersistence
{
public:

    virtual ~PrivacyExternalCheckoutPersistence() = default;

    virtual bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const = 0;
    virtual bool beginExternalCheckout(
        const PrivacyTransaction& transaction,
        const PrivacyTransactionJournal& journal) = 0;
    virtual bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) = 0;
    virtual bool compareAndUpdateJournal(
        const PrivacyTransactionJournal& journal,
        int expectedStage) = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCoreDbExternalCheckoutPersistence final
    : public PrivacyExternalCheckoutPersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* snapshot) const override;
    bool beginExternalCheckout(
        const PrivacyTransaction& transaction,
        const PrivacyTransactionJournal& journal) override;
    bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) override;
    bool compareAndUpdateJournal(
        const PrivacyTransactionJournal& journal,
        int expectedStage) override;
};

/**
 * Owns the durable state machine for one private writable checkout. Launching,
 * UI prompts and explicit changed-result decisions remain with the application
 * owner; this engine never exposes an original at its collection path.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyExternalCheckoutTransactionEngine
{
public:

    explicit PrivacyExternalCheckoutTransactionEngine(
        PrivacyExternalCheckoutPersistence& persistence);
    ~PrivacyExternalCheckoutTransactionEngine();

    PrivacyExternalCheckoutResult create(
        const PrivacyExternalCheckoutRequest& request);
    PrivacyExternalCheckoutResult resumeAuthenticatedCreate(
        const PrivacyExternalCheckoutRequest& request);
    PrivacyExternalCheckoutResult authorizeLaunch(
        const PrivacyStorageRoot& root,
        const PrivacyJournalRootExpectation& rootExpectation,
        const QString& transactionUuid);
    PrivacyExternalCheckoutResult reconcile(
        const PrivacyStorageRoot& root,
        const PrivacyJournalRootExpectation& rootExpectation,
        const QString& transactionUuid);
    PrivacyExternalCheckoutResult recover(
        const PrivacyStorageRoot& root,
        const PrivacyJournalRootExpectation& rootExpectation,
        const QString& transactionUuid);

private:

    class Private;
    QScopedPointer<Private> d;
};

} // namespace Digikam

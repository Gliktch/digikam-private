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
#include <QSharedPointer>

// Local includes

#include "digikam_export.h"
#include "privacyexternalcheckouttransaction.h"
#include "privacyprotectpreflight.h"
#include "privacyruntime.h"
#include "privacystillitemtransaction.h"

namespace Digikam
{

class ItemInfo;

enum class PrivacyStillItemActionAvailability
{
    Unavailable,
    Protectable,
    Unprotectable,
    ResumeProtectable,
    ResumeUnprotectable,
    ProtectedUnavailable
};

enum class PrivacyCompatibilityActionAvailability
{
    Unavailable,
    Unlockable,
    Relockable,
    ReconciliationRequired
};

class PrivacyStillItemActionContext
{
public:

    PrivacyStillItemActionAvailability availability =
        PrivacyStillItemActionAvailability::Unavailable;
    PrivacyCategory protectedCategory;
    QList<PrivacyCategory> protectCategories;
    PrivacyRootRuntimeState publicRootState = PrivacyRootRuntimeState::Unknown;
    QString recoveryTransactionUuid;
    PrivacyCompatibilityActionAvailability compatibilityAvailability =
        PrivacyCompatibilityActionAvailability::Unavailable;
    QString compatibilityUnlockTransactionUuid;
};

class PrivacyCompatibilityCategoryContext
{
public:

    PrivacyCompatibilityActionAvailability availability =
        PrivacyCompatibilityActionAvailability::Unavailable;
    int protectedItemCount = 0;
    int activeExposureCount = 0;
};

class PrivacyThreadImageIOStillItemTransactionOwner final
    : public PrivacyTransactionRecovery
{
public:

    using ProtectAcknowledgement =
        std::function<bool(const PrivacyProtectPreflightResult&)>;
    using CompatibilityProgress = std::function<void(int, int)>;

    static DIGIKAM_GUI_EXPORT QSharedPointer<const PrivacyTransactionRecovery> create(
        PrivacyRuntimeCoordinator& runtime);
    static QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner> current();

    ~PrivacyThreadImageIOStillItemTransactionOwner() override;

    PrivacyStillItemActionContext actionContextForImage(qlonglong imageId) const;
    PrivacyCompatibilityCategoryContext compatibilityContextForCategory(
        const QString& categoryUuid) const;
    bool categoryIsUnlocked(const QString& categoryUuid) const;

    PrivacyStillItemTransactionResult protect(
        const ItemInfo& info,
        const QString& categoryUuid,
        const QString& passwordText,
        const ProtectAcknowledgement& acknowledgeAssetSet = {});
    PrivacyStillItemTransactionResult unprotect(
        const ItemInfo& info,
        const QString& passwordText);
    PrivacyStillItemTransactionResult compatibilityUnlock(
        const ItemInfo& info,
        const QString& passwordText);
    PrivacyStillItemTransactionResult compatibilityRelock(
        const ItemInfo& info,
        const QString& unlockTransactionUuid);
    PrivacyExternalCheckoutResult prepareExternalOpen(
        const ItemInfo& info,
        const QString& passwordText);
    PrivacyExternalCheckoutResult finishExternalCheckout(
        const QString& transactionUuid) const;
    PrivacyCompatibilityBatchResult compatibilityUnlockCategory(
        const QString& categoryUuid,
        const QString& passwordText,
        const CompatibilityProgress& progress = {});
    PrivacyCompatibilityBatchResult compatibilityRelockCategory(
        const QString& categoryUuid,
        const CompatibilityProgress& progress = {}) const;
    PrivacyCompatibilityBatchResult compatibilityRelockAll(
        const CompatibilityProgress& progress = {}) const;
    PrivacyStillItemTransactionResult resume(
        qlonglong imageId,
        const QString& transactionUuid,
        const QString& passwordText);

    PrivacyRecoveryDisposition recoverRoot(
        const PrivacyStorageRoot& root,
        const PrivacyTransaction& transaction,
        const QList<PrivacyTransactionJournal>& journals) const override;
    bool loadReconciledSnapshot(
        PrivacyRepositorySnapshot* snapshot) const override;
    bool prepareForShutdown() const override;

private:

    explicit PrivacyThreadImageIOStillItemTransactionOwner(
        PrivacyRuntimeCoordinator& runtime);

    class Private;
    QScopedPointer<Private> d;

    Q_DISABLE_COPY(PrivacyThreadImageIOStillItemTransactionOwner)
};

} // namespace Digikam

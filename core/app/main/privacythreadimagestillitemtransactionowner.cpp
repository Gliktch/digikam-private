/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacythreadimagestillitemtransactionowner.h"

// Local includes

#include "privacystillitemtransaction.h"
#include "privacythreadimagestillitemcachegate.h"

namespace Digikam
{

class Q_DECL_HIDDEN PrivacyThreadImageIOStillItemTransactionOwner::Private
{
public:

    explicit Private(PrivacyRuntimeCoordinator& runtime)
        : engine(persistence, runtime, cacheGate)
    {
    }

public:

    PrivacyCoreDbStillItemPersistence       persistence;
    PrivacyThreadImageIOStillItemCacheGate  cacheGate;
    mutable PrivacyStillItemTransactionEngine engine;
};

QSharedPointer<const PrivacyTransactionRecovery>
PrivacyThreadImageIOStillItemTransactionOwner::create(
    PrivacyRuntimeCoordinator& runtime)
{
    return QSharedPointer<const PrivacyTransactionRecovery>(
        new PrivacyThreadImageIOStillItemTransactionOwner(runtime));
}

PrivacyThreadImageIOStillItemTransactionOwner::
    PrivacyThreadImageIOStillItemTransactionOwner(
        PrivacyRuntimeCoordinator& runtime)
    : d(new Private(runtime))
{
}

PrivacyThreadImageIOStillItemTransactionOwner::
    ~PrivacyThreadImageIOStillItemTransactionOwner() = default;

PrivacyRecoveryDisposition
PrivacyThreadImageIOStillItemTransactionOwner::recoverRoot(
    const PrivacyStorageRoot& root,
    const PrivacyTransaction& transaction,
    const QList<PrivacyTransactionJournal>& journals) const
{
    if ((root.kind != PrivacyStorageRootKind::AlbumRoot) ||
        ((transaction.type != PrivacyTransactionType::ProtectItem) &&
         (transaction.type != PrivacyTransactionType::UnprotectItem)) ||
        (journals.size() != 1) ||
        (journals.constFirst().transactionUuid != transaction.uuid) ||
        (journals.constFirst().rootUuid != root.uuid))
    {
        return PrivacyRecoveryDisposition::Deferred;
    }

    const PrivacyStillItemTransactionResult result =
        d->engine.recover(root, transaction.uuid);

    if (result.succeeded())
    {
        return PrivacyRecoveryDisposition::Recovered;
    }

    switch (result.status)
    {
        case PrivacyStillItemTransactionStatus::AuthenticationRequired:
        case PrivacyStillItemTransactionStatus::CacheTransitionFailure:
        case PrivacyStillItemTransactionStatus::CleanupPending:
        {
            return PrivacyRecoveryDisposition::Deferred;
        }

        default:
        {
            return PrivacyRecoveryDisposition::Failed;
        }
    }
}

} // namespace Digikam

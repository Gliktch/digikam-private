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

#include <QByteArray>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacytransactionjournal.h"

namespace Digikam
{

enum class PrivacyPublicTransitionMode
{
    InstallAbsent   = 1,
    ExchangePresent = 2
};

enum class PrivacyPublicTransitionFactKind
{
    Original = 1,
    Proxy    = 2
};

enum class PrivacyPublicTransitionError
{
    None,
    InvalidRequest,
    JournalRejected,
    JournalAdvanceFailed,
    RootIdentityMismatch,
    UnsafePath,
    HardlinkReconciliationRequired,
    MissingExpectedFile,
    UnexpectedExistingFile,
    FileFactMismatch,
    IoFailure,
    AtomicPublicationUnavailable,
    PublicationConflict,
    DurabilityUncertain,
    ReconciliationRequired,
    RollbackSucceeded,
    RollbackUncertain,
    FaultInjected
};

enum class PrivacyPublicTransitionFaultPoint
{
    AfterRootOpened,
    AfterJournalValidated,
    AfterInitialVerification,
    AfterStagedFsync,
    AfterApplyingJournal,
    BeforeNamespaceMutation,
    AfterNamespaceMutation,
    AfterParentFsync,
    AfterInstalledVerification,
    AfterDisplacedVerification,
    BeforeRollback,
    AfterRollback,
    AfterRollbackFsync,
    AfterPublicStateJournal
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPublicTransitionRequest
{
    QString                         absoluteRootPath;
    PrivacyJournalRootExpectation  rootExpectation;
    PrivacyJournalRecord           journalRecord;
    QByteArray                     authoritativeJournalSha256;
    QString                         itemUuid;
    int                             role    = 0;
    int                             ordinal = -1;
    PrivacyPublicTransitionMode     mode =
        static_cast<PrivacyPublicTransitionMode>(0);
    PrivacyPublicTransitionFactKind currentFact =
        static_cast<PrivacyPublicTransitionFactKind>(0);
    PrivacyPublicTransitionFactKind installedFact =
        static_cast<PrivacyPublicTransitionFactKind>(0);
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPublicTransitionResult
{
    PrivacyPublicTransitionError error = PrivacyPublicTransitionError::None;
    QString                      detail;
    QByteArray                   applyingJournalSha256;
    QByteArray                   finalJournalSha256;
    QString                      displacedRelativePath;
    bool                         namespaceMutated  = false;
    bool                         installedVerified = false;
    bool                         displacedVerified = false;

    bool succeeded() const
    {
        return (error == PrivacyPublicTransitionError::None);
    }
};

/**
 * Performs one journal-governed public-name mutation. It never creates
 * archives, proxies or transaction records and never removes a displaced
 * public file. On exchange, the displaced file remains at the journaled,
 * transaction-owned stagedRelativePath for a later reconciler/cleanup step.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPublicTransitionEngine
{
public:

    using FaultHook = std::function<bool(PrivacyPublicTransitionFaultPoint)>;

    PrivacyPublicTransitionEngine() = default;

    void setFaultHook(const FaultHook& hook);
    PrivacyPublicTransitionResult execute(
        const PrivacyPublicTransitionRequest& request) const;

    static QString expectedStageFileName(const QString& transactionUuid,
                                         int role, int ordinal);

private:

    FaultHook m_faultHook;
};

} // namespace Digikam

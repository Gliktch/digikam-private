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
#include <QSharedPointer>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "privacycontracts.h"
#include "privacyservice.h"

namespace Digikam
{

enum class PrivacyActionPolicyDisposition
{
    UnprotectedPassThrough       = 1,
    ReadyWithoutPixels           = 2,
    ReadyWithProxy               = 3,
    ReadyWithInternalOriginal    = 4,
    ReadyWithWritableCheckout    = 5,
    UnlockRequired               = 6,
    CompatibilityUnlockRequired = 7,
    ProtectedMutationRequired    = 8,
    RootUnavailable              = 9,
    ArtifactInspectionRequired   = 10,
    NeedsReconciliation          = 11,
    Denied                       = 12
};

/**
 * One immutable state sample used by the non-interactive policy classifier.
 * A broker must sample again when the chosen operation is prepared, and every
 * resulting lease is revalidated immediately before I/O.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyActionItemState
{
public:

    bool isValid() const;

public:

    bool                    protectedItem = false;
    QString                 categoryUuid;
    PrivacyItemAccess       access = PrivacyItemAccess::Unprotected;
    PrivacyRootRuntimeState publicRootState = PrivacyRootRuntimeState::Unknown;
    PrivacyRootRuntimeState originalRootState = PrivacyRootRuntimeState::Unknown;
    PrivacyRootRuntimeState checkoutRootState = PrivacyRootRuntimeState::Unknown;
    bool                    proxyReady = false;
    bool                    originalReady = false;
    bool                    checkoutReady = false;
    bool                    unresolvedTransaction = false;
    qlonglong               itemGeneration = -1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyActionStateProvider
{
public:

    PrivacyActionStateProvider()          = default;
    virtual ~PrivacyActionStateProvider() = default;

    virtual bool stateForItem(qlonglong imageId,
                              PrivacyActionItemState* state) const = 0;

private:

    Q_DISABLE_COPY(PrivacyActionStateProvider)
};

class DIGIKAM_DATABASE_EXPORT PrivacyActionPolicyItem
{
public:

    bool isValid() const;

public:

    PrivacyActionItem              logicalItem;
    PrivacyActionPolicyDisposition disposition = PrivacyActionPolicyDisposition::Denied;
    QString                        categoryUuid;
    bool                           mayUseProxy = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyActionPolicyResult
{
public:

    bool isValid() const;
    bool isImmediatelyReady() const;

public:

    int                           contractVersion = 1;
    QList<PrivacyActionPolicyItem> items;
    QStringList                   affectedCategoryUuids;
    int                           protectedItemCount = 0;
    int                           lockedItemCount = 0;
    int                           unavailableItemCount = 0;
    int                           inspectionItemCount = 0;
    int                           reconciliationItemCount = 0;
    int                           deniedItemCount = 0;
    bool                          canContinueWithProxy = false;
    bool                          canExcludeAffected = false;
    bool                          canUnlockCategories = false;
    bool                          canUseCompatibilityUnlock = false;
    bool                          requiresFreshAuthentication = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyActionPolicy
{
public:

    static PrivacyActionPolicyResult classify(
        const PrivacyActionRequest& request,
        const PrivacyActionStateProvider& stateProvider);

    static bool actionAllowsProxyFallback(PrivacyActionKind kind);
};

/**
 * Process-wide, thread-safe entry point for consumers that need the current
 * runtime policy provider. A missing or concurrently replaced provider returns
 * an invalid result so callers can fail closed; isolated upstream consumers can
 * use isInstalled() to preserve normal behavior when privacy was never started.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyActionGate
{
public:

    static void setProvider(
        const QSharedPointer<const PrivacyActionStateProvider>& provider);
    static void resetProvider();
    static bool isInstalled();
    static PrivacyActionPolicyResult classify(const PrivacyActionRequest& request);
};

} // namespace Digikam

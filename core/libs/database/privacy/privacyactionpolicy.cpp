/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyactionpolicy.h"

// Qt includes

#include <QGlobalStatic>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSet>
#include <QUuid>
#include <QWriteLocker>

namespace Digikam
{

namespace
{

class PrivacyActionGateData
{
public:

    QReadWriteLock                                  lock;
    QSharedPointer<const PrivacyActionStateProvider> provider;
    quint64                                         generation = 0;
};

Q_GLOBAL_STATIC(PrivacyActionGateData, actionGateData)

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

bool isKnownRootState(PrivacyRootRuntimeState state)
{
    return ((state == PrivacyRootRuntimeState::Unknown)           ||
            (state == PrivacyRootRuntimeState::Recovering)        ||
            (state == PrivacyRootRuntimeState::VerifiedAvailable) ||
            (state == PrivacyRootRuntimeState::Offline)           ||
            (state == PrivacyRootRuntimeState::IdentityMismatch));
}

bool isReadyDisposition(PrivacyActionPolicyDisposition disposition)
{
    return ((disposition == PrivacyActionPolicyDisposition::UnprotectedPassThrough) ||
            (disposition == PrivacyActionPolicyDisposition::ReadyWithoutPixels)      ||
            (disposition == PrivacyActionPolicyDisposition::ReadyWithProxy)          ||
            (disposition == PrivacyActionPolicyDisposition::ReadyWithInternalOriginal) ||
            (disposition == PrivacyActionPolicyDisposition::ReadyWithWritableCheckout));
}

bool requestIsProtectedMutation(const PrivacyActionRequest& request)
{
    return ((request.mutationPolicy == PrivacyMutationPolicy::CommitProtectedAsset) ||
            (request.mutationPolicy == PrivacyMutationPolicy::DestructiveMutation)  ||
            (request.actionKind == PrivacyActionKind::MoveRenameDelete));
}

PrivacyActionPolicyDisposition classifyProtectedItem(
    const PrivacyActionRequest& request,
    const PrivacyActionItemState& state)
{
    if (state.unresolvedTransaction)
    {
        return PrivacyActionPolicyDisposition::NeedsReconciliation;
    }

    // V1 deliberately excludes every protected item from automated face,
    // similarity and AI processing, even while its category is unlocked or a
    // Compatibility Unlock has exposed the public original. This also keeps
    // private pixels and feature vectors out of shared long-running analysis
    // stores. Manual database-only tagging uses NoPixels under its own action
    // contract; it must not be represented as Analysis.

    if (request.actionKind == PrivacyActionKind::Analysis)
    {
        return PrivacyActionPolicyDisposition::Denied;
    }

    if (requestIsProtectedMutation(request))
    {
        return PrivacyActionPolicyDisposition::ProtectedMutationRequired;
    }

    switch (request.requestedSource)
    {
        case PrivacyRequestedSource::NoPixels:
        {
            return PrivacyActionPolicyDisposition::ReadyWithoutPixels;
        }

        case PrivacyRequestedSource::PublicProxy:
        {
            if (state.publicRootState != PrivacyRootRuntimeState::VerifiedAvailable)
            {
                return PrivacyActionPolicyDisposition::RootUnavailable;
            }

            if (!state.proxyReady)
            {
                return PrivacyActionPolicyDisposition::ArtifactInspectionRequired;
            }

            return PrivacyActionPolicyDisposition::ReadyWithProxy;
        }

        case PrivacyRequestedSource::InternalOriginal:
        {
            if (state.originalRootState != PrivacyRootRuntimeState::VerifiedAvailable)
            {
                return PrivacyActionPolicyDisposition::RootUnavailable;
            }

            if (!state.originalReady)
            {
                return PrivacyActionPolicyDisposition::ArtifactInspectionRequired;
            }

            return (state.access == PrivacyItemAccess::Unlocked)
                 ? PrivacyActionPolicyDisposition::ReadyWithInternalOriginal
                 : PrivacyActionPolicyDisposition::UnlockRequired;
        }

        case PrivacyRequestedSource::WritableCheckout:
        {
            if ((state.originalRootState != PrivacyRootRuntimeState::VerifiedAvailable) ||
                (state.checkoutRootState != PrivacyRootRuntimeState::VerifiedAvailable))
            {
                return PrivacyActionPolicyDisposition::RootUnavailable;
            }

            if (!state.originalReady || !state.checkoutReady)
            {
                return PrivacyActionPolicyDisposition::ArtifactInspectionRequired;
            }

            return (state.access == PrivacyItemAccess::Unlocked)
                 ? PrivacyActionPolicyDisposition::ReadyWithWritableCheckout
                 : PrivacyActionPolicyDisposition::UnlockRequired;
        }

        case PrivacyRequestedSource::PublicOriginal:
        {
            // Public-path plaintext is never an implicit consequence of a
            // normal category unlock. It always enters the acknowledged,
            // journalled Compatibility Unlock workflow.

            if ((state.originalRootState != PrivacyRootRuntimeState::VerifiedAvailable) ||
                (state.publicRootState != PrivacyRootRuntimeState::VerifiedAvailable))
            {
                return PrivacyActionPolicyDisposition::RootUnavailable;
            }

            if (!state.originalReady || !state.proxyReady)
            {
                return PrivacyActionPolicyDisposition::ArtifactInspectionRequired;
            }

            return PrivacyActionPolicyDisposition::CompatibilityUnlockRequired;
        }
    }

    return PrivacyActionPolicyDisposition::Denied;
}

} // namespace

bool PrivacyActionItemState::isValid() const
{
    if (!protectedItem)
    {
        return (categoryUuid.isEmpty() &&
                (access == PrivacyItemAccess::Unprotected) &&
                !proxyReady && !originalReady && !checkoutReady &&
                !unresolvedTransaction && (itemGeneration < 0));
    }

    return (isCanonicalUuid(categoryUuid) &&
            ((access == PrivacyItemAccess::Locked) ||
             (access == PrivacyItemAccess::Unlocked)) &&
            isKnownRootState(publicRootState) &&
            isKnownRootState(originalRootState) &&
            isKnownRootState(checkoutRootState) &&
            (!proxyReady ||
             (publicRootState == PrivacyRootRuntimeState::VerifiedAvailable)) &&
            (!originalReady ||
             (originalRootState == PrivacyRootRuntimeState::VerifiedAvailable)) &&
            (!checkoutReady ||
             (checkoutRootState == PrivacyRootRuntimeState::VerifiedAvailable)) &&
            (itemGeneration >= 0));
}

bool PrivacyActionPolicyItem::isValid() const
{
    if (!logicalItem.isValid())
    {
        return false;
    }

    if (disposition == PrivacyActionPolicyDisposition::UnprotectedPassThrough)
    {
        return (categoryUuid.isEmpty() && !mayUseProxy);
    }

    if (disposition == PrivacyActionPolicyDisposition::Denied)
    {
        return (categoryUuid.isEmpty() || isCanonicalUuid(categoryUuid));
    }

    return ((disposition >= PrivacyActionPolicyDisposition::ReadyWithoutPixels) &&
            (disposition < PrivacyActionPolicyDisposition::Denied) &&
            isCanonicalUuid(categoryUuid));
}

bool PrivacyActionPolicyResult::isValid() const
{
    if ((contractVersion != 1) || items.isEmpty() ||
        (protectedItemCount < 0) || (lockedItemCount < 0) ||
        (unavailableItemCount < 0) || (inspectionItemCount < 0) ||
        (reconciliationItemCount < 0) ||
        (deniedItemCount < 0) ||
        (lockedItemCount > protectedItemCount) ||
        (unavailableItemCount > protectedItemCount) ||
        (inspectionItemCount > protectedItemCount) ||
        (reconciliationItemCount > protectedItemCount) ||
        (deniedItemCount > protectedItemCount))
    {
        return false;
    }

    int countedProtected = 0;
    int countedUnavailable = 0;
    int countedInspection = 0;
    int countedReconciliation = 0;
    int countedDenied = 0;
    QSet<qlonglong> itemIds;
    QSet<QString> categories;

    for (const PrivacyActionPolicyItem& item : items)
    {
        if (!item.isValid() || itemIds.contains(item.logicalItem.imageId))
        {
            return false;
        }

        itemIds.insert(item.logicalItem.imageId);

        if (item.disposition != PrivacyActionPolicyDisposition::UnprotectedPassThrough)
        {
            ++countedProtected;
            if (!item.categoryUuid.isEmpty())
            {
                categories.insert(item.categoryUuid);
            }
        }

        countedUnavailable +=
            (item.disposition == PrivacyActionPolicyDisposition::RootUnavailable) ? 1 : 0;
        countedInspection +=
            (item.disposition == PrivacyActionPolicyDisposition::ArtifactInspectionRequired) ? 1 : 0;
        countedReconciliation +=
            (item.disposition == PrivacyActionPolicyDisposition::NeedsReconciliation) ? 1 : 0;
        countedDenied +=
            (item.disposition == PrivacyActionPolicyDisposition::Denied) ? 1 : 0;
    }

    QSet<QString> reportedCategories;

    for (const QString& categoryUuid : affectedCategoryUuids)
    {
        if (!isCanonicalUuid(categoryUuid) || reportedCategories.contains(categoryUuid))
        {
            return false;
        }

        reportedCategories.insert(categoryUuid);
    }

    return ((countedProtected == protectedItemCount) &&
            (countedUnavailable == unavailableItemCount) &&
            (countedInspection == inspectionItemCount) &&
            (countedReconciliation == reconciliationItemCount) &&
            (countedDenied == deniedItemCount) &&
            (reportedCategories == categories));
}

bool PrivacyActionPolicyResult::isImmediatelyReady() const
{
    if (!isValid())
    {
        return false;
    }

    for (const PrivacyActionPolicyItem& item : items)
    {
        if (!isReadyDisposition(item.disposition))
        {
            return false;
        }
    }

    return true;
}

PrivacyActionPolicyResult PrivacyActionPolicy::classify(
    const PrivacyActionRequest& request,
    const PrivacyActionStateProvider& stateProvider)
{
    PrivacyActionPolicyResult result;

    if (!request.isValid())
    {
        return result;
    }

    QSet<QString> categories;
    bool allAffectedCanUseProxy = true;
    bool hasUnlockRequired = false;
    bool hasCompatibilityRequired = false;
    bool hasReadyItem = false;
    bool hasBlockedItem = false;

    for (const PrivacyActionItem& logicalItem : request.items)
    {
        PrivacyActionPolicyItem policyItem;
        policyItem.logicalItem = logicalItem;
        PrivacyActionItemState state;

        if (!stateProvider.stateForItem(logicalItem.imageId, &state) || !state.isValid())
        {
            policyItem.disposition = PrivacyActionPolicyDisposition::Denied;
            result.items << policyItem;
            ++result.protectedItemCount;
            ++result.deniedItemCount;
            allAffectedCanUseProxy = false;
            hasBlockedItem = true;
            continue;
        }

        if (!state.protectedItem)
        {
            policyItem.disposition = PrivacyActionPolicyDisposition::UnprotectedPassThrough;
            result.items << policyItem;
            hasReadyItem = true;
            continue;
        }

        ++result.protectedItemCount;
        categories.insert(state.categoryUuid);
        policyItem.categoryUuid = state.categoryUuid;
        policyItem.mayUseProxy = (actionAllowsProxyFallback(request.actionKind) &&
                                  state.proxyReady &&
                                  (state.publicRootState ==
                                   PrivacyRootRuntimeState::VerifiedAvailable));
        policyItem.disposition = classifyProtectedItem(request, state);

        if (state.access == PrivacyItemAccess::Locked)
        {
            ++result.lockedItemCount;
        }

        if (policyItem.disposition == PrivacyActionPolicyDisposition::RootUnavailable)
        {
            ++result.unavailableItemCount;
        }
        else if (policyItem.disposition == PrivacyActionPolicyDisposition::ArtifactInspectionRequired)
        {
            ++result.inspectionItemCount;
        }
        else if (policyItem.disposition == PrivacyActionPolicyDisposition::NeedsReconciliation)
        {
            ++result.reconciliationItemCount;
        }
        else if (policyItem.disposition == PrivacyActionPolicyDisposition::Denied)
        {
            ++result.deniedItemCount;
        }

        hasUnlockRequired = (hasUnlockRequired ||
                             (policyItem.disposition ==
                              PrivacyActionPolicyDisposition::UnlockRequired));
        hasCompatibilityRequired = (hasCompatibilityRequired ||
                                    (policyItem.disposition ==
                                     PrivacyActionPolicyDisposition::CompatibilityUnlockRequired));
        allAffectedCanUseProxy = (allAffectedCanUseProxy && policyItem.mayUseProxy);
        hasReadyItem = (hasReadyItem || isReadyDisposition(policyItem.disposition));
        hasBlockedItem = (hasBlockedItem || !isReadyDisposition(policyItem.disposition));
        result.items << policyItem;
    }

    result.affectedCategoryUuids = categories.values();
    result.affectedCategoryUuids.sort();
    result.canContinueWithProxy = ((result.protectedItemCount > 0) &&
                                   allAffectedCanUseProxy);
    result.canExcludeAffected = (hasReadyItem && hasBlockedItem);
    result.canUnlockCategories = hasUnlockRequired;
    result.canUseCompatibilityUnlock = hasCompatibilityRequired;
    result.requiresFreshAuthentication =
        (requestIsProtectedMutation(request) && (result.protectedItemCount > 0));

    return result;
}

bool PrivacyActionPolicy::actionAllowsProxyFallback(PrivacyActionKind kind)
{
    switch (kind)
    {
        case PrivacyActionKind::Preview:
        case PrivacyActionKind::ExternalOpen:
        case PrivacyActionKind::OpenInFileManager:
        case PrivacyActionKind::Export:
        case PrivacyActionKind::Print:
        case PrivacyActionKind::DragOrClipboard:
        case PrivacyActionKind::Slideshow:
        case PrivacyActionKind::BatchProcess:
        {
            return true;
        }

        case PrivacyActionKind::InternalEdit:
        case PrivacyActionKind::MetadataWrite:
        case PrivacyActionKind::MoveRenameDelete:
        case PrivacyActionKind::Analysis:
        {
            return false;
        }
    }

    return false;
}

void PrivacyActionGate::setProvider(
    const QSharedPointer<const PrivacyActionStateProvider>& provider)
{
    QWriteLocker locker(&actionGateData->lock);
    actionGateData->provider = provider;

    if (++actionGateData->generation == 0)
    {
        ++actionGateData->generation;
    }
}

void PrivacyActionGate::resetProvider()
{
    setProvider(QSharedPointer<const PrivacyActionStateProvider>());
}

bool PrivacyActionGate::isInstalled()
{
    QReadLocker locker(&actionGateData->lock);

    return !actionGateData->provider.isNull();
}

PrivacyActionPolicyResult PrivacyActionGate::classify(
    const PrivacyActionRequest& request)
{
    QSharedPointer<const PrivacyActionStateProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&actionGateData->lock);
        provider   = actionGateData->provider;
        generation = actionGateData->generation;
    }

    if (!provider)
    {
        return PrivacyActionPolicyResult();
    }

    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(request, *provider);

    {
        QReadLocker locker(&actionGateData->lock);

        if ((generation != actionGateData->generation) ||
            (provider != actionGateData->provider))
        {
            return PrivacyActionPolicyResult();
        }
    }

    return result;
}

bool PrivacyActionGate::mayMutatePublicItem(qlonglong imageId,
                                            const QString& publicPath,
                                            PrivacyActionKind actionKind)
{
    if (!isInstalled())
    {
        return true;
    }

    PrivacyActionItem item;
    item.imageId    = imageId;
    item.publicPath = publicPath;

    PrivacyActionRequest request;
    request.actionKind       = actionKind;
    request.consumerIdentity = QLatin1String("digikam-public-item-mutation");
    request.items             = { item };
    request.requestedSource   = PrivacyRequestedSource::NoPixels;
    request.mutationPolicy    = PrivacyMutationPolicy::CommitProtectedAsset;

    const PrivacyActionPolicyResult result = classify(request);

    return (result.isValid() && result.isImmediatelyReady());
}

} // namespace Digikam

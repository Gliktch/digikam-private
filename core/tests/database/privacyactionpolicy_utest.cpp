/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QHash>
#include <QTest>

// Local includes

#include "privacyactionpolicy.h"

using namespace Digikam;

namespace
{

const QString categoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");

class FakeActionStateProvider final : public PrivacyActionStateProvider
{
public:

    bool stateForItem(qlonglong imageId,
                      PrivacyActionItemState* state) const override
    {
        if (!state || !states.contains(imageId))
        {
            return false;
        }

        *state = states.value(imageId);

        return true;
    }

public:

    QHash<qlonglong, PrivacyActionItemState> states;
};

PrivacyActionItemState unprotectedState()
{
    return PrivacyActionItemState();
}

PrivacyActionItemState protectedState(PrivacyItemAccess access = PrivacyItemAccess::Locked)
{
    PrivacyActionItemState state;
    state.protectedItem    = true;
    state.categoryUuid     = categoryUuid;
    state.access           = access;
    state.publicRootState  = PrivacyRootRuntimeState::VerifiedAvailable;
    state.originalRootState = PrivacyRootRuntimeState::VerifiedAvailable;
    state.checkoutRootState = PrivacyRootRuntimeState::VerifiedAvailable;
    state.proxyReady       = true;
    state.originalReady    = true;
    state.checkoutReady    = true;
    state.itemGeneration   = 4;

    return state;
}

PrivacyActionRequest makeRequest(PrivacyActionKind action,
                                 PrivacyRequestedSource source,
                                 PrivacyMutationPolicy mutation,
                                 const QList<qlonglong>& imageIds)
{
    PrivacyActionRequest request;
    request.actionKind       = action;
    request.consumerIdentity = QLatin1String("synthetic-consumer");
    request.requestedSource  = source;
    request.mutationPolicy   = mutation;

    for (qlonglong imageId : imageIds)
    {
        PrivacyActionItem item;
        item.imageId    = imageId;
        item.publicPath = QLatin1String("/synthetic/collection/item-") +
                          QString::number(imageId) + QLatin1String(".jpg");
        request.items << item;
    }

    return request;
}

} // namespace

class PrivacyActionPolicyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testUnprotectedPassThrough();
    void testMixedLockedExportChoices();
    void testMixedProtectedStatesCanExcludeBlocked();
    void testUnlockedSourcesReady();
    void testCompatibilityAlwaysExplicit();
    void testUnavailableOriginalCanOfferProxy();
    void testMissingArtifactIsNotOfflineRoot();
    void testReconciliationAndMutationPrecedence();
    void testAnalysisExcludesProtectedItemsInEveryState();
    void testProviderFailureFailsClosed();
};

void PrivacyActionPolicyTest::testUnprotectedPassThrough()
{
    FakeActionStateProvider provider;
    provider.states.insert(1, unprotectedState());
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Export,
                    PrivacyRequestedSource::WritableCheckout,
                    PrivacyMutationPolicy::MayCreateOutputs, { 1 }), provider);

    QVERIFY(result.isValid());
    QVERIFY(result.isImmediatelyReady());
    QCOMPARE(result.protectedItemCount, 0);
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::UnprotectedPassThrough);
}

void PrivacyActionPolicyTest::testMixedLockedExportChoices()
{
    FakeActionStateProvider provider;
    provider.states.insert(1, unprotectedState());
    provider.states.insert(2, protectedState());
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Export,
                    PrivacyRequestedSource::WritableCheckout,
                    PrivacyMutationPolicy::MayCreateOutputs, { 1, 2 }), provider);

    QVERIFY(result.isValid());
    QVERIFY(!result.isImmediatelyReady());
    QCOMPARE(result.protectedItemCount, 1);
    QCOMPARE(result.lockedItemCount, 1);
    QCOMPARE(result.affectedCategoryUuids, QStringList { categoryUuid });
    QCOMPARE(result.items.at(1).disposition,
             PrivacyActionPolicyDisposition::UnlockRequired);
    QVERIFY(result.items.at(1).mayUseProxy);
    QVERIFY(result.canContinueWithProxy);
    QVERIFY(result.canExcludeAffected);
    QVERIFY(result.canUnlockCategories);
    QVERIFY(!result.canUseCompatibilityUnlock);
}

void PrivacyActionPolicyTest::testMixedProtectedStatesCanExcludeBlocked()
{
    FakeActionStateProvider provider;
    provider.states.insert(2, protectedState(PrivacyItemAccess::Unlocked));
    provider.states.insert(3, protectedState(PrivacyItemAccess::Locked));
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::ExternalOpen,
                    PrivacyRequestedSource::WritableCheckout,
                    PrivacyMutationPolicy::MayCreateOutputs, { 2, 3 }), provider);

    QVERIFY(result.isValid());
    QCOMPARE(result.items.at(0).disposition,
             PrivacyActionPolicyDisposition::ReadyWithWritableCheckout);
    QCOMPARE(result.items.at(1).disposition,
             PrivacyActionPolicyDisposition::UnlockRequired);
    QVERIFY(result.canExcludeAffected);
    QVERIFY(result.canUnlockCategories);
}

void PrivacyActionPolicyTest::testUnlockedSourcesReady()
{
    FakeActionStateProvider provider;
    provider.states.insert(2, protectedState(PrivacyItemAccess::Unlocked));

    PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Preview,
                    PrivacyRequestedSource::InternalOriginal,
                    PrivacyMutationPolicy::ReadOnly, { 2 }), provider);
    QVERIFY(result.isValid());
    QVERIFY(result.isImmediatelyReady());
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::ReadyWithInternalOriginal);

    result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::ExternalOpen,
                    PrivacyRequestedSource::WritableCheckout,
                    PrivacyMutationPolicy::MayCreateOutputs, { 2 }), provider);
    QVERIFY(result.isImmediatelyReady());
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::ReadyWithWritableCheckout);
}

void PrivacyActionPolicyTest::testCompatibilityAlwaysExplicit()
{
    FakeActionStateProvider provider;
    provider.states.insert(2, protectedState(PrivacyItemAccess::Unlocked));
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::OpenInFileManager,
                    PrivacyRequestedSource::PublicOriginal,
                    PrivacyMutationPolicy::ReadOnly, { 2 }), provider);

    QVERIFY(result.isValid());
    QVERIFY(!result.isImmediatelyReady());
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::CompatibilityUnlockRequired);
    QVERIFY(result.canUseCompatibilityUnlock);
}

void PrivacyActionPolicyTest::testUnavailableOriginalCanOfferProxy()
{
    FakeActionStateProvider provider;
    PrivacyActionItemState state = protectedState();
    state.originalRootState = PrivacyRootRuntimeState::Offline;
    state.originalReady     = false;
    provider.states.insert(2, state);
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Print,
                    PrivacyRequestedSource::WritableCheckout,
                    PrivacyMutationPolicy::ReadOnly, { 2 }), provider);

    QVERIFY(result.isValid());
    QCOMPARE(result.unavailableItemCount, 1);
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::RootUnavailable);
    QVERIFY(result.canContinueWithProxy);
    QVERIFY(!result.canUnlockCategories);
}

void PrivacyActionPolicyTest::testMissingArtifactIsNotOfflineRoot()
{
    FakeActionStateProvider provider;
    PrivacyActionItemState state = protectedState();
    state.proxyReady = false;
    provider.states.insert(2, state);
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Preview,
                    PrivacyRequestedSource::PublicProxy,
                    PrivacyMutationPolicy::ReadOnly, { 2 }), provider);

    QVERIFY(result.isValid());
    QCOMPARE(result.unavailableItemCount, 0);
    QCOMPARE(result.inspectionItemCount, 1);
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::ArtifactInspectionRequired);
}

void PrivacyActionPolicyTest::testReconciliationAndMutationPrecedence()
{
    FakeActionStateProvider provider;
    PrivacyActionItemState state = protectedState(PrivacyItemAccess::Unlocked);
    state.unresolvedTransaction = true;
    provider.states.insert(2, state);

    PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::MoveRenameDelete,
                    PrivacyRequestedSource::NoPixels,
                    PrivacyMutationPolicy::DestructiveMutation, { 2 }), provider);
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::NeedsReconciliation);
    QCOMPARE(result.reconciliationItemCount, 1);

    state.unresolvedTransaction = false;
    provider.states.insert(2, state);
    result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::MoveRenameDelete,
                    PrivacyRequestedSource::NoPixels,
                    PrivacyMutationPolicy::DestructiveMutation, { 2 }), provider);
    QCOMPARE(result.items.constFirst().disposition,
             PrivacyActionPolicyDisposition::ProtectedMutationRequired);
    QVERIFY(result.requiresFreshAuthentication);
}

void PrivacyActionPolicyTest::testAnalysisExcludesProtectedItemsInEveryState()
{
    FakeActionStateProvider provider;
    provider.states.insert(2, protectedState());

    const QList<PrivacyRequestedSource> sources = {
        PrivacyRequestedSource::NoPixels,
        PrivacyRequestedSource::PublicProxy,
        PrivacyRequestedSource::InternalOriginal,
        PrivacyRequestedSource::WritableCheckout,
        PrivacyRequestedSource::PublicOriginal
    };

    for (PrivacyItemAccess access : { PrivacyItemAccess::Locked,
                                      PrivacyItemAccess::Unlocked })
    {
        provider.states.insert(2, protectedState(access));

        for (PrivacyRequestedSource source : sources)
        {
            const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
                makeRequest(PrivacyActionKind::Analysis, source,
                            PrivacyMutationPolicy::ReadOnly, { 2 }), provider);

            QVERIFY(result.isValid());
            QCOMPARE(result.items.constFirst().disposition,
                     PrivacyActionPolicyDisposition::Denied);
            QCOMPARE(result.deniedItemCount, 1);
            QVERIFY(!result.items.constFirst().mayUseProxy);
            QVERIFY(!result.canContinueWithProxy);
            QVERIFY(!result.canUnlockCategories);
            QVERIFY(!result.canUseCompatibilityUnlock);
        }
    }

    provider.states.insert(1, unprotectedState());
    const PrivacyActionPolicyResult mixed = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Analysis,
                    PrivacyRequestedSource::InternalOriginal,
                    PrivacyMutationPolicy::ReadOnly, { 1, 2 }), provider);

    QVERIFY(mixed.isValid());
    QCOMPARE(mixed.items.at(0).disposition,
             PrivacyActionPolicyDisposition::UnprotectedPassThrough);
    QCOMPARE(mixed.items.at(1).disposition,
             PrivacyActionPolicyDisposition::Denied);
    QCOMPARE(mixed.deniedItemCount, 1);
    QVERIFY(mixed.canExcludeAffected);
}

void PrivacyActionPolicyTest::testProviderFailureFailsClosed()
{
    FakeActionStateProvider provider;
    const PrivacyActionPolicyResult result = PrivacyActionPolicy::classify(
        makeRequest(PrivacyActionKind::Export,
                    PrivacyRequestedSource::WritableCheckout,
                    PrivacyMutationPolicy::ReadOnly, { 99 }), provider);

    QVERIFY(result.isValid());
    QVERIFY(!result.isImmediatelyReady());
    QCOMPARE(result.protectedItemCount, 1);
    QCOMPARE(result.deniedItemCount, 1);
    QVERIFY(result.affectedCategoryUuids.isEmpty());
    QVERIFY(!result.canContinueWithProxy);
}

QTEST_GUILESS_MAIN(PrivacyActionPolicyTest)

#include "privacyactionpolicy_utest.moc"

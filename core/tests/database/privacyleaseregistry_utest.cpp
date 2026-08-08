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

// C++ includes

#include <functional>

// Local includes

#include "privacyleaseregistry.h"

using namespace Digikam;

namespace
{

const QString itemUuidA = QLatin1String("20000000-0000-0000-0000-000000000001");
const QString itemUuidB = QLatin1String("20000000-0000-0000-0000-000000000002");

class FakeLeaseStateProvider final : public PrivacyLeaseStateProvider
{
public:

    bool currentState(const QString& itemUuid,
                      PrivacyLeaseCurrentState* state) const override
    {
        ++callCount;

        if (beforeSample)
        {
            beforeSample(callCount);
        }

        if (!state || !states.contains(itemUuid))
        {
            return false;
        }

        *state = states.value(itemUuid);

        if (afterSample)
        {
            afterSample(callCount);
        }

        return true;
    }

public:

    QHash<QString, PrivacyLeaseCurrentState> states;
    mutable int callCount = 0;
    std::function<void(int)> beforeSample;
    std::function<void(int)> afterSample;
};

PrivacyLeaseCurrentState readyState(const QString& itemUuid)
{
    PrivacyLeaseCurrentState state;
    state.itemUuid            = itemUuid;
    state.itemGeneration      = 3;
    state.categoryEpoch       = 7;
    state.publicRootEpoch     = 11;
    state.storeRootEpoch      = 13;
    state.categoryUnlocked    = true;
    state.publicRootAvailable = true;
    state.storeRootAvailable  = true;

    return state;
}

} // namespace

class PrivacyLeaseRegistryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testIssueAndValidate();
    void testGenerationAndAvailabilityChanges();
    void testTamperAndExplicitRevocation();
    void testIssueFailsClosed();
    void testStateChangesDuringIssueAndValidation();
};

void PrivacyLeaseRegistryTest::testIssueAndValidate()
{
    QSharedPointer<FakeLeaseStateProvider> provider(new FakeLeaseStateProvider);
    provider->states.insert(itemUuidA, readyState(itemUuidA));
    PrivacyLeaseRegistry registry(provider);

    const PrivacyLeaseToken publicLease = registry.issue(itemUuidA, false);
    QVERIFY(publicLease.isValid());
    QCOMPARE(publicLease.storeRootEpoch, quint64(0));
    QCOMPARE(registry.validate(publicLease), PrivacyLeaseValidation::Valid);

    const PrivacyLeaseToken storeLease = registry.issue(itemUuidA, true);
    QVERIFY(storeLease.isValid());
    QCOMPARE(storeLease.storeRootEpoch, quint64(13));
    QCOMPARE(registry.validate(storeLease), PrivacyLeaseValidation::Valid);
    QCOMPARE(registry.activeLeaseCount(), 2);
}

void PrivacyLeaseRegistryTest::testGenerationAndAvailabilityChanges()
{
    QSharedPointer<FakeLeaseStateProvider> provider(new FakeLeaseStateProvider);
    provider->states.insert(itemUuidA, readyState(itemUuidA));
    PrivacyLeaseRegistry registry(provider);
    const PrivacyLeaseToken lease = registry.issue(itemUuidA, true);
    QVERIFY(lease.isValid());

    PrivacyLeaseCurrentState state = provider->states.value(itemUuidA);
    ++state.categoryEpoch;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::StateChanged);

    state = readyState(itemUuidA);
    state.publicRootAvailable = false;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::RootUnavailable);

    state = readyState(itemUuidA);
    ++state.publicRootEpoch;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::StateChanged);

    state = readyState(itemUuidA);
    state.storeRootAvailable = false;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::RootUnavailable);

    state = readyState(itemUuidA);
    ++state.storeRootEpoch;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::StateChanged);

    state = readyState(itemUuidA);
    ++state.itemGeneration;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::StateChanged);

    state = readyState(itemUuidA);
    state.categoryUnlocked = false;
    ++state.categoryEpoch;
    provider->states.insert(itemUuidA, state);
    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::StateChanged);
}

void PrivacyLeaseRegistryTest::testTamperAndExplicitRevocation()
{
    QSharedPointer<FakeLeaseStateProvider> provider(new FakeLeaseStateProvider);
    provider->states.insert(itemUuidA, readyState(itemUuidA));
    provider->states.insert(itemUuidB, readyState(itemUuidB));
    PrivacyLeaseRegistry registry(provider);
    PrivacyLeaseToken leaseA1 = registry.issue(itemUuidA, false);
    const PrivacyLeaseToken leaseA2 = registry.issue(itemUuidA, true);
    const PrivacyLeaseToken leaseB = registry.issue(itemUuidB, false);

    ++leaseA1.itemGeneration;
    QCOMPARE(registry.validate(leaseA1), PrivacyLeaseValidation::Revoked);
    --leaseA1.itemGeneration;
    QCOMPARE(registry.validate(leaseA1), PrivacyLeaseValidation::Valid);

    QVERIFY(registry.revoke(leaseA1.uuid));
    QVERIFY(!registry.revoke(leaseA1.uuid));
    QCOMPARE(registry.validate(leaseA1), PrivacyLeaseValidation::Revoked);
    QCOMPARE(registry.revokeItem(itemUuidA), 1);
    QCOMPARE(registry.validate(leaseA2), PrivacyLeaseValidation::Revoked);
    QCOMPARE(registry.activeLeaseCount(), 1);
    registry.revokeAll();
    QCOMPARE(registry.validate(leaseB), PrivacyLeaseValidation::Revoked);
    QCOMPARE(registry.activeLeaseCount(), 0);
}

void PrivacyLeaseRegistryTest::testIssueFailsClosed()
{
    QSharedPointer<FakeLeaseStateProvider> provider(new FakeLeaseStateProvider);
    PrivacyLeaseRegistry registry(provider);
    QVERIFY(!registry.issue(itemUuidA, false).isValid());
    QVERIFY(!registry.issue(QLatin1String("not-a-uuid"), false).isValid());

    PrivacyLeaseCurrentState state = readyState(itemUuidA);
    state.publicRootAvailable = false;
    provider->states.insert(itemUuidA, state);
    QVERIFY(!registry.issue(itemUuidA, false).isValid());

    state = readyState(itemUuidA);
    state.storeRootAvailable = false;
    provider->states.insert(itemUuidA, state);
    QVERIFY(registry.issue(itemUuidA, false).isValid());
    QVERIFY(!registry.issue(itemUuidA, true).isValid());

    state = readyState(itemUuidB);
    provider->states.insert(itemUuidA, state);
    QVERIFY(!registry.issue(itemUuidA, false).isValid());

    state = readyState(itemUuidA);
    state.categoryUnlocked = false;
    provider->states.insert(itemUuidA, state);
    QVERIFY(registry.issue(itemUuidA, false).isValid());
    QVERIFY(!registry.issue(itemUuidA, true).isValid());

    state = readyState(itemUuidA);
    state.unresolvedTransaction = true;
    provider->states.insert(itemUuidA, state);
    QVERIFY(!registry.issue(itemUuidA, false).isValid());
}

void PrivacyLeaseRegistryTest::testStateChangesDuringIssueAndValidation()
{
    QSharedPointer<FakeLeaseStateProvider> provider(new FakeLeaseStateProvider);
    provider->states.insert(itemUuidA, readyState(itemUuidA));
    PrivacyLeaseRegistry registry(provider);

    FakeLeaseStateProvider* const providerPointer = provider.data();
    provider->beforeSample = [providerPointer](int callCount)
    {
        if (callCount == 2)
        {
            PrivacyLeaseCurrentState changed = providerPointer->states.value(itemUuidA);
            ++changed.categoryEpoch;
            providerPointer->states.insert(itemUuidA, changed);
        }
    };

    QVERIFY(!registry.issue(itemUuidA, true).isValid());
    QCOMPARE(registry.activeLeaseCount(), 0);

    provider->beforeSample = {};
    provider->callCount = 0;
    provider->states.insert(itemUuidA, readyState(itemUuidA));
    const PrivacyLeaseToken lease = registry.issue(itemUuidA, true);
    QVERIFY(lease.isValid());

    provider->afterSample = [&registry, lease](int)
    {
        registry.revoke(lease.uuid);
    };

    QCOMPARE(registry.validate(lease), PrivacyLeaseValidation::Revoked);
}

QTEST_GUILESS_MAIN(PrivacyLeaseRegistryTest)

#include "privacyleaseregistry_utest.moc"

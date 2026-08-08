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

#include "privacyanalysisgate.h"

using namespace Digikam;

namespace
{

class FakeAnalysisProvider final : public PrivacyAnalysisGateProvider
{
public:

    PrivacyAnalysisDisposition analysisDisposition(qlonglong imageId) const override
    {
        if (onEvaluate)
        {
            onEvaluate(imageId);
        }

        return dispositions.value(imageId, PrivacyAnalysisDisposition::Unavailable);
    }

public:

    QHash<qlonglong, PrivacyAnalysisDisposition> dispositions;
    std::function<void(qlonglong)> onEvaluate;
};

} // namespace

class PrivacyAnalysisGateTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init();
    void cleanup();
    void testAbsentProviderFailsClosed();
    void testMixedSelectionSummary();
    void testInvalidProviderDecisionFailsClosed();
    void testProviderReplacementFailsClosed();
};

void PrivacyAnalysisGateTest::init()
{
    PrivacyAnalysisGate::resetProvider();
}

void PrivacyAnalysisGateTest::cleanup()
{
    PrivacyAnalysisGate::resetProvider();
}

void PrivacyAnalysisGateTest::testAbsentProviderFailsClosed()
{
    QCOMPARE(PrivacyAnalysisGate::evaluate(1), PrivacyAnalysisDisposition::Unavailable);
    QVERIFY(!PrivacyAnalysisGate::mayAnalyze(1));

    const PrivacyAnalysisSelectionResult result = PrivacyAnalysisGate::filter({ 1, 2, -1 });
    QCOMPARE(result.requestedCount, 3);
    QVERIFY(result.allowedImageIds.isEmpty());
    QCOMPARE(result.protectedExcludedCount, 0);
    QCOMPARE(result.unavailableExcludedCount, 3);
    QCOMPARE(result.excludedCount(), 3);
    QCOMPARE(result.notice(), PrivacyAnalysisNotice::AnalysisUnavailable);
}

void PrivacyAnalysisGateTest::testMixedSelectionSummary()
{
    const QSharedPointer<FakeAnalysisProvider> provider(new FakeAnalysisProvider);
    provider->dispositions.insert(1, PrivacyAnalysisDisposition::Allowed);
    provider->dispositions.insert(2, PrivacyAnalysisDisposition::ProtectedExcluded);
    provider->dispositions.insert(3, PrivacyAnalysisDisposition::Unavailable);
    PrivacyAnalysisGate::setProvider(provider);

    const PrivacyAnalysisSelectionResult result = PrivacyAnalysisGate::filter({ 1, 2, 3, -1 });
    QCOMPARE(result.requestedCount, 4);
    QCOMPARE(result.allowedImageIds, QList<qlonglong> { 1 });
    QCOMPARE(result.protectedExcludedCount, 1);
    QCOMPARE(result.unavailableExcludedCount, 2);
    QCOMPARE(result.excludedCount(), 3);
    QCOMPARE(result.notice(), PrivacyAnalysisNotice::ProtectedItemsAndUnavailable);

    const PrivacyAnalysisSelectionResult protectedOnly = PrivacyAnalysisGate::filter({ 1, 2 });
    QCOMPARE(protectedOnly.notice(), PrivacyAnalysisNotice::ProtectedItemsExcluded);
}

void PrivacyAnalysisGateTest::testInvalidProviderDecisionFailsClosed()
{
    const QSharedPointer<FakeAnalysisProvider> provider(new FakeAnalysisProvider);
    provider->dispositions.insert(1, static_cast<PrivacyAnalysisDisposition>(99));
    PrivacyAnalysisGate::setProvider(provider);

    QCOMPARE(PrivacyAnalysisGate::evaluate(1), PrivacyAnalysisDisposition::Unavailable);
}

void PrivacyAnalysisGateTest::testProviderReplacementFailsClosed()
{
    const QSharedPointer<FakeAnalysisProvider> first(new FakeAnalysisProvider);
    const QSharedPointer<FakeAnalysisProvider> second(new FakeAnalysisProvider);
    first->dispositions.insert(1, PrivacyAnalysisDisposition::Allowed);
    first->dispositions.insert(2, PrivacyAnalysisDisposition::Allowed);
    second->dispositions.insert(1, PrivacyAnalysisDisposition::Allowed);
    second->dispositions.insert(2, PrivacyAnalysisDisposition::Allowed);
    first->onEvaluate = [second](qlonglong)
    {
        PrivacyAnalysisGate::setProvider(second);
    };
    PrivacyAnalysisGate::setProvider(first);

    QCOMPARE(PrivacyAnalysisGate::evaluate(1), PrivacyAnalysisDisposition::Unavailable);

    PrivacyAnalysisGate::setProvider(first);
    const PrivacyAnalysisSelectionResult result = PrivacyAnalysisGate::filter({ 1, 2 });
    QVERIFY(result.allowedImageIds.isEmpty());
    QCOMPARE(result.protectedExcludedCount, 0);
    QCOMPARE(result.unavailableExcludedCount, 2);
    QCOMPARE(result.notice(), PrivacyAnalysisNotice::AnalysisUnavailable);
}

QTEST_GUILESS_MAIN(PrivacyAnalysisGateTest)

#include "privacyanalysisgate_utest.moc"

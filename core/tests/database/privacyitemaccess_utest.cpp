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

#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTimeZone>

// Local includes

#include "dinfointerface.h"
#include "dfileoperations.h"

using namespace Digikam;

namespace
{

DItemAccessRequest validRequest()
{
    DItemAccessRequest request;
    request.consumerIdentity = QLatin1String("synthetic-access-test");
    request.logicalUrls = {
        QUrl::fromLocalFile(QLatin1String("/synthetic/one.jpg")),
        QUrl::fromLocalFile(QLatin1String("/synthetic/two.jpg"))
    };
    request.purpose         = DItemAccessPurpose::Export;
    request.mutation        = DItemAccessMutation::MayCreateOutputs;
    request.requestedSource = DItemAccessSource::InternalOriginal;
    request.consumerScope   = DItemAccessConsumerScope::SameProcess;
    request.allowPlaceholderFallback = true;
    request.allowPartialSelection     = true;
    return request;
}

class SyntheticSourceHandle final : public DItemAccessSourceHandle
{
public:

    SyntheticSourceHandle(
        const DItemAccessEntry& entry,
        const QList<DItemAssociatedAccessEntry>& associatedEntries)
        : DItemAccessSourceHandle(entry, associatedEntries)
    {
    }
};

class SyntheticAccessHandle final : public DItemAccessHandle
{
public:

    explicit SyntheticAccessHandle(const QList<DItemAccessEntry>& entries)
        : DItemAccessHandle(entries, {}, false)
    {
    }
};

} // namespace

class PrivacyItemAccessTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRequestValidation();
    void testPassThroughLifetime();
    void testCanceledSelection();
    void testSourceAcquisitionCancellation();
    void testFileFactsValidationAndApplication();
    void testAssociatedSourceValidation();
    void testDeferredPlanHasNoIoUrl();
};

void PrivacyItemAccessTest::testRequestValidation()
{
    DItemAccessRequest request = validRequest();
    QVERIFY(request.isValid());

    request.consumerIdentity.clear();
    QVERIFY(!request.isValid());
    request = validRequest();
    request.logicalUrls << request.logicalUrls.constFirst();
    QVERIFY(!request.isValid());
    request = validRequest();
    request.logicalUrls[0] = QUrl(QLatin1String("https://example.invalid/one.jpg"));
    QVERIFY(!request.isValid());
    request = validRequest();
    request.requestedSource = static_cast<DItemAccessSource>(0);
    QVERIFY(!request.isValid());
    request = validRequest();
    request.consumerScope = DItemAccessConsumerScope::DetachedProcess;
    QVERIFY(!request.isValid());
    request.requestedSource = DItemAccessSource::PublicRepresentation;
    QVERIFY(request.isValid());
    request.requestedSource = DItemAccessSource::PublicOriginal;
    QVERIFY(!request.isValid());
    request.purpose = DItemAccessPurpose::ExternalOpen;
    request.mutation = DItemAccessMutation::MayModifyInputs;
    request.allowPlaceholderFallback = false;
    QVERIFY(request.isValid());
    request.allowPlaceholderFallback = true;
    QVERIFY(!request.isValid());
    request.requestedSource = DItemAccessSource::PublicRepresentation;
    QVERIFY(!request.isValid());
    request.consumerScope = DItemAccessConsumerScope::SameProcess;
    request.purpose = DItemAccessPurpose::Export;
    request.mutation = DItemAccessMutation::MayModifyInputs;
    request.requestedSource = DItemAccessSource::InternalOriginal;
    QVERIFY(!request.isValid());
    request.requestedSource = DItemAccessSource::WritableCheckout;
    request.allowPlaceholderFallback = false;
    QVERIFY(!request.isValid());
    request.purpose = DItemAccessPurpose::ExternalOpen;
    request.consumerScope = DItemAccessConsumerScope::DetachedProcess;
    QVERIFY(request.isValid());

    DItemAccessEntry snapshotEntry;
    snapshotEntry.logicalUrl = request.logicalUrls.constFirst();
    snapshotEntry.physicalUrl =
        QUrl::fromLocalFile(QLatin1String("/run/user/1000/private-proxy"));
    snapshotEntry.placeholder = true;
    snapshotEntry.sameProcessOnly = true;
    QVERIFY(snapshotEntry.isValid());

    DItemAssociatedAccessEntry associated;
    associated.logicalUrl =
        QUrl::fromLocalFile(QLatin1String("/synthetic/one.xmp"));
    associated.physicalUrl =
        QUrl::fromLocalFile(QLatin1String("/proc/self/fd/42"));
    associated.role = static_cast<int>(DItemAssociatedRole::XmpSidecar);
    associated.ordinal = 0;
    associated.sameProcessOnly = true;
    QVERIFY(associated.isValid());
}

void PrivacyItemAccessTest::testPassThroughLifetime()
{
    const DItemAccessRequest request = validRequest();
    const QSharedPointer<DItemAccessHandle> handle =
        DItemAccessHandle::passThrough(request);

    QVERIFY(handle);
    QVERIFY(handle->isValid());
    QVERIFY(!handle->isCanceled());
    QCOMPARE(handle->entries().size(), request.logicalUrls.size());
    QCOMPARE(handle->physicalUrls(), request.logicalUrls);
    QVERIFY(handle->excludedLogicalUrls().isEmpty());

    for (const QUrl& url : request.logicalUrls)
    {
        QCOMPARE(handle->logicalUrlFor(url), url);
        QVERIFY(handle->validateAccess(url));
        const QSharedPointer<DItemAccessSourceHandle> source =
            handle->acquireSource(url);
        QVERIFY(source);
        QVERIFY(source->isValid());
        QVERIFY(source->validateAccess());
        QCOMPARE(source->entry().logicalUrl, url);
        QVERIFY(source->associatedEntries().isEmpty());
    }

    for (const DItemAccessEntry& entry : handle->entries())
    {
        QVERIFY(!entry.sameProcessOnly);
    }

    QVERIFY(!handle->validateAccess(
        QUrl::fromLocalFile(QLatin1String("/synthetic/other.jpg"))));
}

void PrivacyItemAccessTest::testCanceledSelection()
{
    const DItemAccessRequest request = validRequest();
    const QSharedPointer<DItemAccessHandle> handle =
        DItemAccessHandle::canceled(request);

    QVERIFY(handle);
    QVERIFY(handle->isValid());
    QVERIFY(handle->isCanceled());
    QVERIFY(handle->entries().isEmpty());
    QCOMPARE(handle->excludedLogicalUrls(), request.logicalUrls);
    QVERIFY(!handle->validateAccess(request.logicalUrls.constFirst()));
}

void PrivacyItemAccessTest::testSourceAcquisitionCancellation()
{
    const DItemAccessRequest request = validRequest();
    const QSharedPointer<DItemAccessHandle> handle =
        DItemAccessHandle::passThrough(request);
    QSharedPointer<DItemAccessCancellationToken> cancellation(
        new DItemAccessCancellationToken);

    QVERIFY(handle);
    QVERIFY(!cancellation->isCanceled());
    cancellation->cancel();
    QVERIFY(cancellation->isCanceled());
    QVERIFY(!handle->acquireSource(request.logicalUrls.constFirst(),
                                   cancellation));
}

void PrivacyItemAccessTest::testFileFactsValidationAndApplication()
{
    DItemAccessFileFacts facts;
    QVERIFY(facts.isValid());

    facts.available = true;
    QVERIFY(!facts.isValid());
    facts.modificationDate = QDateTime::fromMSecsSinceEpoch(
        1700000000000LL, QTimeZone::UTC);
    facts.permissions = QFileDevice::ReadOwner  |
                        QFileDevice::WriteOwner |
                        QFileDevice::ReadGroup;
    QVERIFY(facts.isValid());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QLatin1String("export.jpg"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("synthetic"), qint64(9));
    file.close();

    QVERIFY(DFileOperations::setPermissionsAndModificationTime(
        path, facts.permissions, facts.modificationDate));
    const QFileInfo info(path);
    const QFileDevice::Permissions comparable =
        QFileDevice::ReadOwner  | QFileDevice::WriteOwner |
        QFileDevice::ExeOwner   | QFileDevice::ReadGroup  |
        QFileDevice::WriteGroup | QFileDevice::ExeGroup   |
        QFileDevice::ReadOther  | QFileDevice::WriteOther |
        QFileDevice::ExeOther;
    QCOMPARE(info.permissions() & comparable, facts.permissions);
    QCOMPARE(info.lastModified().toSecsSinceEpoch(),
             facts.modificationDate.toSecsSinceEpoch());
}

void PrivacyItemAccessTest::testAssociatedSourceValidation()
{
    DItemAccessEntry primary;
    primary.logicalUrl = QUrl::fromLocalFile(
        QLatin1String("/synthetic/one.jpg"));
    primary.physicalUrl = QUrl::fromLocalFile(
        QLatin1String("/proc/self/fd/40"));
    primary.sameProcessOnly = true;

    DItemAssociatedAccessEntry sidecar;
    sidecar.logicalUrl = QUrl::fromLocalFile(
        QLatin1String("/synthetic/one.jpg.xmp"));
    sidecar.physicalUrl = QUrl::fromLocalFile(
        QLatin1String("/proc/self/fd/41"));
    sidecar.role = static_cast<int>(DItemAssociatedRole::XmpSidecar);
    sidecar.ordinal = 0;
    sidecar.sameProcessOnly = true;

    SyntheticSourceHandle valid(primary, { sidecar });
    QVERIFY(valid.isValid());
    QVERIFY(valid.validateAccess());

    DItemAssociatedAccessEntry duplicateLogical = sidecar;
    duplicateLogical.physicalUrl = QUrl::fromLocalFile(
        QLatin1String("/proc/self/fd/42"));
    SyntheticSourceHandle duplicateLogicalHandle(
        primary, { sidecar, duplicateLogical });
    QVERIFY(!duplicateLogicalHandle.isValid());

    DItemAssociatedAccessEntry duplicatePhysical = sidecar;
    duplicatePhysical.logicalUrl = QUrl::fromLocalFile(
        QLatin1String("/synthetic/one.json"));
    SyntheticSourceHandle duplicatePhysicalHandle(
        primary, { sidecar, duplicatePhysical });
    QVERIFY(!duplicatePhysicalHandle.isValid());

    primary.deferred = true;
    SyntheticSourceHandle deferred(primary, { sidecar });
    QVERIFY(!deferred.isValid());
}

void PrivacyItemAccessTest::testDeferredPlanHasNoIoUrl()
{
    DItemAccessEntry plan;
    plan.logicalUrl = QUrl::fromLocalFile(
        QLatin1String("/synthetic/private.jpg"));
    plan.deferred = true;
    plan.sameProcessOnly = true;

    SyntheticAccessHandle handle({ plan });
    QVERIFY(handle.isValid());
    QVERIFY(handle.physicalUrls().isEmpty());
    QVERIFY(handle.entries().constFirst().physicalUrl.isEmpty());
    QVERIFY(handle.logicalUrlFor(plan.logicalUrl).isEmpty());
    QVERIFY(!handle.validateAccess(plan.logicalUrl));
    QVERIFY(!handle.acquireSource(plan.logicalUrl));
}

QTEST_GUILESS_MAIN(PrivacyItemAccessTest)

#include "privacyitemaccess_utest.moc"

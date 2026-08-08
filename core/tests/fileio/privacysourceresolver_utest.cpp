/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-08-08
 * Description : unit tests for privacy-aware image source resolution
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ includes

#include <atomic>
#include <limits>
#include <thread>
#include <vector>

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QRect>
#include <QSemaphore>
#include <QSet>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QUrlQuery>

// Local includes

#include "loadingdescription.h"
#include "dbengineparameters.h"
#ifndef DIGIKAM_PRIVACY_TRANSITION_UNIT_ONLY
#include "previewloadthread.h"
#endif
#include "privacycachetransition.h"
#include "privacysourceresolver.h"
#include "thumbsdb.h"
#include "thumbsdbaccess.h"
#include "thumbnailinfo.h"
#include "thumbnailloadthread.h"

using namespace Digikam;

namespace
{

class FixedProvider : public PrivacySourceProvider
{
public:

    explicit FixedProvider(const PrivacySourceResult& result,
                           const QString& handledPath = QLatin1String("/logical/item.jpg"))
        : m_result     (result),
          m_handledPath(handledPath)
    {
    }

    PrivacySourceResult resolve(const PrivacySourceRequest& request) const override
    {
        if (request.logicalFilePath != m_handledPath)
        {
            return PrivacySourceResult::notHandled();
        }

        QMutexLocker locker(&m_mutex);
        m_lastRequest = request;

        return m_result;
    }

    void setResult(const PrivacySourceResult& result)
    {
        QMutexLocker locker(&m_mutex);
        m_result = result;
    }

    PrivacySourceRequest lastRequest() const
    {
        QMutexLocker locker(&m_mutex);

        return m_lastRequest;
    }

private:

    PrivacySourceResult          m_result;
    QString                     m_handledPath;
    mutable QMutex              m_mutex;
    mutable PrivacySourceRequest m_lastRequest;
};

class BlockingProviderState
{
public:

    QSemaphore        entered;
    QSemaphore        release;
    std::atomic<bool> destroyed = false;
};

class BlockingProvider : public PrivacySourceProvider
{
public:

    explicit BlockingProvider(const QSharedPointer<BlockingProviderState>& state)
        : m_state(state)
    {
    }

    ~BlockingProvider() override
    {
        m_state->destroyed.store(true);
    }

    PrivacySourceResult resolve(const PrivacySourceRequest&) const override
    {
        m_state->entered.release();
        m_state->release.acquire();

        return PrivacySourceResult::resolved(QLatin1String("/runtime/original.jpg"),
                                             QLatin1String("category-a:unlocked:blocking"));
    }

private:

    QSharedPointer<BlockingProviderState> m_state;
};

QString persistentAddress(const ThumbnailIdentifier& identifier, const QRect& rect)
{
    QString address = identifier.filePath + QLatin1Char('|') +
                      QString::number(identifier.id) + QLatin1Char('|') +
                      identifier.cacheNamespace;

    if (!identifier.cacheNamespace.isEmpty())
    {
        address += QLatin1Char('|') +
                   QString::number(identifier.sourceResolverGeneration);
    }

    if (!rect.isNull())
    {
        address += QLatin1Char('|') +
                   QString::number(rect.x()) + QLatin1Char(',') +
                   QString::number(rect.y()) + QLatin1Char('-') +
                   QString::number(rect.width()) + QLatin1Char('x') +
                   QString::number(rect.height());
    }

    return address;
}

class FakeTransitionBackend : public PrivacyCacheTransitionBackend
{
public:

    void evictRamCaches(const QString& logicalFilePath) override
    {
        evictedPaths << logicalFilePath;
    }

    bool removePersistentThumbnail(const ThumbnailIdentifier& identifier,
                                   const QRect& detailRect) override
    {
        const QString address = persistentAddress(identifier, detailRect);
        removedAddresses << address;
        persistentRows.remove(address);

        return removalSucceeds;
    }

public:

    QSet<QString> persistentRows;
    QStringList   removedAddresses;
    QStringList   evictedPaths;
    bool          removalSucceeds = true;
};

} // namespace

class PrivacySourceResolverTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init();
    void cleanup();

#ifndef DIGIKAM_PRIVACY_TRANSITION_UNIT_ONLY
    void testLegacyCacheKeyStability();
#endif
    void testPrivacyNamespaceSeparation();
    void testPhysicalSourceAndFailClosedResolution();
    void testSourceSnapshotInvalidationAndCachePolicy();
    void testProviderSnapshotLifetime();
    void testIdenticalProviderReinstallInvalidatesHandledSnapshot();
    void testThreadSafeProviderReset();
    void testOrdinarySourceUseGuardDrainsAndBlocksNewcomers();
    void testProviderChangeWhileBeginWaitsRemovesBarrier();
#ifndef DIGIKAM_PRIVACY_TRANSITION_UNIT_ONLY
    void testSynchronousPreviewHonorsSourceUseBarrier();
#endif
    void testCrossPathTransitionOverlapRejected();
    void testProtectTransitionBlocksAndPurgesInFlightWrite();
    void testUnprotectDeletesOnlyExactNamespacedPrimary();
    void testRepeatedPurgeAndIncompleteInventoryFailClosed();
    void testInvalidOwnershipInventoryDoesNotDelete();
    void testActualLegacyDetailInventoryFailsClosed();
    void testProviderReplacementMakesFinishFailClosed();
};

void PrivacySourceResolverTest::init()
{
    PrivacySourceResolver::resetProvider();
}

void PrivacySourceResolverTest::cleanup()
{
    PrivacySourceResolver::resetProvider();
}

#ifndef DIGIKAM_PRIVACY_TRANSITION_UNIT_ONLY

void PrivacySourceResolverTest::testLegacyCacheKeyStability()
{
    const QString path = QLatin1String("/logical/item.jpg");

    LoadingDescription full(path);
    full.resolveSource();
    QCOMPARE(full.filePath, path);
    QCOMPARE(full.effectiveFilePath(), path);
    QCOMPARE(full.cacheKey(), path);
    QVERIFY(full.privacyCacheNamespace().isEmpty());
    QVERIFY(!full.isSourceDenied());
    QVERIFY(full.persistentCacheAllowed());
    QVERIFY(full.sourceResolutionIsCurrent());

    LoadingDescription unresolvedFull(path);
    QVERIFY(unresolvedFull == full);

    LoadingDescription preview(path, PreviewSettings(), 256);
    preview.resolveSource();
    QCOMPARE(preview.cacheKey(), path + QLatin1String("-previewImage-256"));

    LoadingDescription thumbnail(path, PreviewSettings(), 128,
                                 LoadingDescription::NoColorConversion,
                                 LoadingDescription::PreviewParameters::Thumbnail);
    thumbnail.previewParameters.storageReference = 42;
    thumbnail.resolveSource();
    QCOMPARE(thumbnail.cacheKey(), path + QLatin1String("-thumbnail-128"));

    LoadingDescription detail(path, PreviewSettings(), 128,
                              LoadingDescription::NoColorConversion,
                              LoadingDescription::PreviewParameters::DetailThumbnail);
    detail.previewParameters.storageReference = 42;
    detail.previewParameters.extraParameter   = QRect(1, 2, 3, 4);
    detail.resolveSource();
    QCOMPARE(detail.cacheKey(), path + QLatin1String("-thumbnail-1,2-3x4-128"));

    LoadingDescription globalRaw(path);
    globalRaw.rawDecodingHint = LoadingDescription::RawDecodingGlobalSettings;
    globalRaw.resolveSource();
    QCOMPARE(globalRaw.cacheKey(), path + QLatin1String("-globalraw"));

    const QStringList expectedLookupKeys = {
        path + QLatin1String("-previewImage-256"),
        path + QLatin1String("-previewImage"),
        path
    };
    QCOMPARE(preview.lookupCacheKeys(), expectedLookupKeys);
}

#endif

void PrivacySourceResolverTest::testPrivacyNamespaceSeparation()
{
    const QString path = QLatin1String("/logical/item.jpg");

    QSharedPointer<FixedProvider> firstProvider(
        new FixedProvider(PrivacySourceResult::resolved(QLatin1String("/runtime/original.jpg"),
                                                        QLatin1String("category-a:unlocked:7"))));
    PrivacySourceResolver::setProvider(firstProvider);

    LoadingDescription first(path, PreviewSettings(), 256);
    first.resolveSource();

    QSharedPointer<FixedProvider> secondProvider(
        new FixedProvider(PrivacySourceResult::resolved(path,
                                                        QLatin1String("category-a:locked:8"))));
    PrivacySourceResolver::setProvider(secondProvider);

    LoadingDescription second(path, PreviewSettings(), 256);
    second.resolveSource();

    QCOMPARE(first.filePath, second.filePath);
    QCOMPARE(first.effectiveFilePath(), QLatin1String("/runtime/original.jpg"));
    QCOMPARE(second.effectiveFilePath(), path);
    QVERIFY(first.cacheKey() != second.cacheKey());
    QVERIFY(first != second);
    QVERIFY(!first.persistentCacheAllowed());
    QVERIFY(!second.persistentCacheAllowed());

    for (const QString& key : first.lookupCacheKeys())
    {
        const QString prefix = QLatin1String("digikam-private-cache:/v1/");
        QVERIFY(key.startsWith(prefix));
        QCOMPARE(key.indexOf(QLatin1Char('/'), prefix.size()),
                 prefix.size() + 64);
    }
}

void PrivacySourceResolverTest::testPhysicalSourceAndFailClosedResolution()
{
    const QString path = QLatin1String("/logical/item.jpg");
    QSharedPointer<FixedProvider> provider(
        new FixedProvider(PrivacySourceResult::resolved(QLatin1String("/runtime/original.jpg"),
                                                        QLatin1String("category-a:unlocked:9"))));
    PrivacySourceResolver::setProvider(provider);

    LoadingDescription resolved(path, PreviewSettings(), 128,
                                LoadingDescription::NoColorConversion,
                                LoadingDescription::PreviewParameters::Thumbnail);
    resolved.previewParameters.storageReference = 73;
    resolved.resolveSource();

    QCOMPARE(resolved.filePath, path);
    QCOMPARE(resolved.previewParameters.storageReference.toLongLong(), 73LL);
    QCOMPARE(resolved.effectiveFilePath(), QLatin1String("/runtime/original.jpg"));
    QCOMPARE(provider->lastRequest().consumer, PrivacySourceRequest::Thumbnail);
    QCOMPARE(provider->lastRequest().itemReference.toLongLong(), 73LL);

    const ThumbnailIdentifier identifier = resolved.thumbnailIdentifier();
    QCOMPARE(identifier.filePath, path);
    QCOMPARE(identifier.sourceFilePath, QLatin1String("/runtime/original.jpg"));
    QVERIFY(!identifier.sourceAccessDenied);
    QVERIFY(!identifier.persistentCacheAllowed);

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::denied(
                                  QLatin1String("category-a:offline:10")))));

    LoadingDescription denied(path);
    denied.resolveSource();
    QVERIFY(denied.isSourceDenied());
    QVERIFY(denied.effectiveFilePath().isEmpty());
    QVERIFY(denied.cacheKey() != path);

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::resolved(
                                  QLatin1String("/runtime/original.jpg"), QString()))));

    LoadingDescription malformed(path);
    malformed.resolveSource();
    QVERIFY(malformed.isSourceDenied());
    QVERIFY(malformed.effectiveFilePath().isEmpty());
    QVERIFY(!malformed.privacyCacheNamespace().isEmpty());

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::resolved(
                                  QLatin1String("relative/original.jpg"),
                                  QLatin1String("category-a:invalid-path:10")))));

    LoadingDescription relativePath(path);
    relativePath.resolveSource();
    QVERIFY(relativePath.isSourceDenied());

    QString nulPath = QLatin1String("/runtime/original.jpg");
    nulPath.append(QChar::Null);
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::resolved(
                                  nulPath,
                                  QLatin1String("category-a:invalid-nul-path:10")))));

    LoadingDescription embeddedNulPath(path);
    embeddedNulPath.resolveSource();
    QVERIFY(embeddedNulPath.isSourceDenied());

    PrivacySourceResult invalidPolicy = PrivacySourceResult::resolved(
                                            QLatin1String("/runtime/original.jpg"),
                                            QLatin1String("category-a:invalid-policy:10"));
    invalidPolicy.cachePolicy = static_cast<PrivacySourceResult::CachePolicy>(999);
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(invalidPolicy)));

    LoadingDescription malformedPolicy(path);
    malformedPolicy.resolveSource();
    QVERIFY(malformedPolicy.isSourceDenied());
    QVERIFY(!malformedPolicy.persistentCacheAllowed());

    PrivacySourceResult spoofedGeneration = PrivacySourceResult::resolved(
                                                QLatin1String("/runtime/original.jpg"),
                                                QLatin1String("category-a:spoofed-generation:10"));
    spoofedGeneration.resolverGeneration = std::numeric_limits<quint64>::max();
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(spoofedGeneration)));

    PrivacySourceRequest directRequest;
    directRequest.logicalFilePath = path;
    const PrivacySourceResult normalizedGeneration =
        PrivacySourceResolver::resolve(directRequest);
    QVERIFY(normalizedGeneration.resolverGeneration !=
            spoofedGeneration.resolverGeneration);

    LoadingDescription untouched(QLatin1String("/ordinary/item.jpg"));
    untouched.resolveSource();
    QCOMPARE(untouched.cacheKey(), QLatin1String("/ordinary/item.jpg"));
    QCOMPARE(untouched.effectiveFilePath(), QLatin1String("/ordinary/item.jpg"));
}

void PrivacySourceResolverTest::testSourceSnapshotInvalidationAndCachePolicy()
{
    const QString path = QLatin1String("/logical/item.jpg");
    const PrivacySourceResult unlocked = PrivacySourceResult::resolved(
                                             QLatin1String("/runtime/original.jpg"),
                                             QLatin1String("category-a:unlocked:12"));

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(unlocked)));

    LoadingDescription queued(path, PreviewSettings(), 256);
    queued.resolveSource();
    QVERIFY(queued.sourceResolutionIsCurrent());
    QVERIFY(!queued.persistentCacheAllowed());

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(unlocked)));
    QVERIFY(!queued.sourceResolutionIsCurrent());

    LoadingDescription logicalCopy = queued;
    logicalCopy.resetSourceResolution();
    QVERIFY(queued != logicalCopy);
    QVERIFY(queued.equalsIgnoringSourceResolution(logicalCopy));

    PrivacySourceResolver::resetProvider();
    QVERIFY(!queued.sourceResolutionIsCurrent());

    LoadingDescription ordinary(path);
    ordinary.resolveSource();
    QVERIFY(ordinary.sourceResolutionIsCurrent());
    QVERIFY(ordinary.persistentCacheAllowed());

    PrivacySourceResolver::resetProvider();

    LoadingDescription nextOrdinary(path);
    nextOrdinary.resolveSource();
    QCOMPARE(nextOrdinary.cacheKey(), ordinary.cacheKey());
    QVERIFY(ordinary != nextOrdinary);
    QVERIFY(!ordinary.sourceResolutionIsCurrent());

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(unlocked)));
    QVERIFY(!nextOrdinary.sourceResolutionIsCurrent());

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::resolved(
                                  path,
                                  QLatin1String("category-a:locked:13"),
                                  PrivacySourceResult::Persistent))));

    LoadingDescription lockedProxy(path);
    lockedProxy.resolveSource();
    QVERIFY(lockedProxy.persistentCacheAllowed());

    LoadingDescription lockedDetail(path, PreviewSettings(), 128,
                                    LoadingDescription::NoColorConversion,
                                    LoadingDescription::PreviewParameters::DetailThumbnail);
    lockedDetail.previewParameters.storageReference = 42;
    lockedDetail.previewParameters.extraParameter   = QRect(1, 2, 3, 4);
    lockedDetail.resolveSource();
    QVERIFY(lockedDetail.persistentCacheAllowed());
    QVERIFY(!lockedDetail.thumbnailIdentifier().persistentCacheAllowed);

    PrivacySourceResolver::resetProvider();

    LoadingDescription ordinaryDetail(path, PreviewSettings(), 128,
                                      LoadingDescription::NoColorConversion,
                                      LoadingDescription::PreviewParameters::DetailThumbnail);
    ordinaryDetail.previewParameters.storageReference = 42;
    ordinaryDetail.previewParameters.extraParameter   = QRect(1, 2, 3, 4);
    ordinaryDetail.resolveSource();
    QVERIFY(ordinaryDetail.thumbnailIdentifier().persistentCacheAllowed);
}

void PrivacySourceResolverTest::testProviderSnapshotLifetime()
{
    QSharedPointer<BlockingProviderState> state(new BlockingProviderState);
    PrivacySourceResolver::setProvider(
        QSharedPointer<BlockingProvider>(new BlockingProvider(state)));

    PrivacySourceResult result;
    std::thread worker([&result]()
    {
        PrivacySourceRequest request;
        request.logicalFilePath = QLatin1String("/logical/item.jpg");
        result = PrivacySourceResolver::resolve(request);
    });

    state->entered.acquire();
    PrivacySourceResolver::resetProvider();
    const bool destroyedWhileCallbackActive = state->destroyed.load();
    state->release.release();
    worker.join();

    QVERIFY(!destroyedWhileCallbackActive);
    QVERIFY(state->destroyed.load());
    QCOMPARE(result.disposition, PrivacySourceResult::Denied);
    QVERIFY(result.physicalFilePath.isEmpty());
    QVERIFY(!result.cacheNamespace.isEmpty());
}

void PrivacySourceResolverTest::testIdenticalProviderReinstallInvalidatesHandledSnapshot()
{
    const QString path = QLatin1String("/logical/item.jpg");
    const PrivacySourceResult unlocked = PrivacySourceResult::resolved(
                                             QLatin1String("/runtime/original.jpg"),
                                             QLatin1String("category-a:unlocked:14"));

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(unlocked)));

    LoadingDescription oldSnapshot(path, PreviewSettings(), 256,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    oldSnapshot.previewParameters.storageReference = 42;
    oldSnapshot.resolveSource();
    const QString oldCacheKey = oldSnapshot.cacheKey();
    QVERIFY(oldSnapshot.sourceResolutionIsCurrent());

    PrivacySourceResolver::resetProvider();
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(new FixedProvider(unlocked)));

    QVERIFY(!oldSnapshot.sourceResolutionIsCurrent());

    LoadingDescription newSnapshot(path, PreviewSettings(), 256,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    newSnapshot.previewParameters.storageReference = 42;
    newSnapshot.resolveSource();
    QVERIFY(newSnapshot.sourceResolutionIsCurrent());
    QVERIFY(oldSnapshot != newSnapshot);
    QVERIFY(oldCacheKey != newSnapshot.cacheKey());

    const ThumbnailIdentifier oldIdentifier = oldSnapshot.thumbnailIdentifier();
    const ThumbnailIdentifier newIdentifier = newSnapshot.thumbnailIdentifier();
    QVERIFY(oldIdentifier.sourceResolverGeneration !=
            newIdentifier.sourceResolverGeneration);
}

void PrivacySourceResolverTest::testThreadSafeProviderReset()
{
    const PrivacySourceRequest request = {
        QLatin1String("/logical/item.jpg"),
        91,
        PrivacySourceRequest::Preview
    };
    std::atomic<bool> valid(true);
    std::atomic<bool> start(false);
    std::vector<std::thread> workers;

    for (int worker = 0 ; worker < 4 ; ++worker)
    {
        workers.emplace_back([&request, &valid, &start]()
        {
            while (!start.load())
            {
                std::this_thread::yield();
            }

            for (int i = 0 ; i < 4000 ; ++i)
            {
                const PrivacySourceResult result = PrivacySourceResolver::resolve(request);

                if (result.disposition == PrivacySourceResult::NotHandled)
                {
                    if (!result.physicalFilePath.isEmpty()                          ||
                        !result.cacheNamespace.isEmpty()                            ||
                        (result.cachePolicy != PrivacySourceResult::Persistent))
                    {
                        valid.store(false);
                    }
                }
                else if (result.disposition == PrivacySourceResult::Resolved)
                {
                    if ((result.physicalFilePath != QLatin1String("/runtime/original.jpg")) ||
                        (result.cacheNamespace != QLatin1String("category-a:unlocked:11")) ||
                        (result.cachePolicy != PrivacySourceResult::MemoryOnly))
                    {
                        valid.store(false);
                    }
                }
                else if (result.disposition == PrivacySourceResult::Denied)
                {
                    if (!result.physicalFilePath.isEmpty()                         ||
                        result.cacheNamespace.isEmpty()                            ||
                        (result.cachePolicy != PrivacySourceResult::MemoryOnly))
                    {
                        valid.store(false);
                    }
                }
                else
                {
                    valid.store(false);
                }
            }
        });
    }

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::resolved(
                                  QLatin1String("/runtime/original.jpg"),
                                  QLatin1String("category-a:unlocked:11")))));
    start.store(true);

    for (int i = 0 ; i < 1000 ; ++i)
    {
        PrivacySourceResolver::setProvider(
            QSharedPointer<FixedProvider>(
                new FixedProvider(PrivacySourceResult::resolved(
                                      QLatin1String("/runtime/original.jpg"),
                                      QLatin1String("category-a:unlocked:11")))));
        PrivacySourceResolver::resetProvider();
    }

    for (std::thread& worker : workers)
    {
        worker.join();
    }

    QVERIFY(valid.load());

    const PrivacySourceResult afterReset = PrivacySourceResolver::resolve(request);
    QCOMPARE(afterReset.disposition, PrivacySourceResult::NotHandled);
}

void PrivacySourceResolverTest::testOrdinarySourceUseGuardDrainsAndBlocksNewcomers()
{
    const QString path = QLatin1String("/ordinary/item.jpg");
    LoadingDescription ordinary(path, PreviewSettings(), 128,
                                LoadingDescription::NoColorConversion,
                                LoadingDescription::PreviewParameters::Thumbnail);
    ordinary.previewParameters.storageReference = 42;
    ordinary.resolveSource();
    QVERIFY(ordinary.sourceResolutionIsCurrent());
    QVERIFY(ordinary.privacyCacheNamespace().isEmpty());

    PrivacySourceUseGuard incumbent(ordinary);
    QVERIFY(incumbent.isAcquired());

    PrivacyCacheTransitionToken token;
    PrivacyCacheTransitionToken repeatedToken;
    std::atomic<bool> beginStarted(false);
    std::atomic<bool> beginReturned(false);
    std::atomic<bool> repeatedStarted(false);
    std::atomic<bool> repeatedReturned(false);
    std::thread beginWorker([&]()
    {
        beginStarted.store(true);
        token = PrivacyCacheTransition::begin(ordinary.thumbnailIdentifier());
        beginReturned.store(true);
    });

    while (!beginStarted.load())
    {
        std::this_thread::yield();
    }

    PrivacySourceRequest request;
    request.logicalFilePath = path;
    request.itemReference   = 42;
    request.consumer        = PrivacySourceRequest::Thumbnail;
    PrivacySourceResult blocked;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 5000)
    {
        blocked = PrivacySourceResolver::resolve(request);

        if (blocked.disposition == PrivacySourceResult::Denied)
        {
            break;
        }

        std::this_thread::yield();
    }

    std::thread repeatedWorker([&]()
    {
        repeatedStarted.store(true);
        repeatedToken = PrivacyCacheTransition::begin(ordinary.thumbnailIdentifier());
        repeatedReturned.store(true);
    });

    while (!repeatedStarted.load())
    {
        std::this_thread::yield();
    }

    PrivacySourceUseGuard newcomer(ordinary);
    const bool blockedObserved =
        (blocked.disposition == PrivacySourceResult::Denied);
    const bool firstReturnedBeforeRelease = beginReturned.load();
    const bool repeatedReturnedBeforeRelease = repeatedReturned.load();
    const bool newcomerAcquired = newcomer.isAcquired();

    incumbent.release();
    beginWorker.join();
    repeatedWorker.join();

    QVERIFY(blockedObserved);
    QVERIFY(!firstReturnedBeforeRelease);
    QVERIFY(!repeatedReturnedBeforeRelease);
    QVERIFY(!newcomerAcquired);
    QVERIFY(token.isValid());
    QVERIFY(repeatedToken.isValid());
    QCOMPARE(repeatedToken.logicalFilePath(), token.logicalFilePath());
    QVERIFY(PrivacyCacheTransition::rollback(token));
    QVERIFY(PrivacyCacheTransition::rollback(repeatedToken));
}

void PrivacySourceResolverTest::testProviderChangeWhileBeginWaitsRemovesBarrier()
{
    const QString path = QLatin1String("/logical/item.jpg");
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::notHandled())));
    LoadingDescription description(path, PreviewSettings(), 128,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = 42;
    description.resolveSource();
    PrivacySourceUseGuard incumbent(description);
    QVERIFY(incumbent.isAcquired());

    PrivacyCacheTransitionToken token;
    std::atomic<bool> beginStarted(false);
    std::thread beginWorker([&]()
    {
        beginStarted.store(true);
        token = PrivacyCacheTransition::begin(description.thumbnailIdentifier());
    });

    while (!beginStarted.load())
    {
        std::this_thread::yield();
    }

    PrivacySourceRequest request;
    request.logicalFilePath = path;
    request.itemReference   = 42;
    request.consumer        = PrivacySourceRequest::Thumbnail;
    PrivacySourceResult blocked;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 5000)
    {
        blocked = PrivacySourceResolver::resolve(request);

        if (blocked.disposition == PrivacySourceResult::Denied)
        {
            break;
        }

        std::this_thread::yield();
    }

    const bool barrierObserved =
        (blocked.disposition == PrivacySourceResult::Denied);
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::notHandled())));
    incumbent.release();
    beginWorker.join();

    QVERIFY(barrierObserved);
    QVERIFY(!token.isValid());

    LoadingDescription current(path, PreviewSettings(), 128,
                               LoadingDescription::NoColorConversion,
                               LoadingDescription::PreviewParameters::Thumbnail);
    current.previewParameters.storageReference = 42;
    current.resolveSource();
    const PrivacyCacheTransitionToken currentToken =
        PrivacyCacheTransition::begin(current.thumbnailIdentifier());
    QVERIFY(currentToken.isValid());
    QVERIFY(PrivacyCacheTransition::rollback(currentToken));
}

#ifndef DIGIKAM_PRIVACY_TRANSITION_UNIT_ONLY

void PrivacySourceResolverTest::testSynchronousPreviewHonorsSourceUseBarrier()
{
    const QString path = QLatin1String("/definitely/missing/private-source.jpg");
    LoadingDescription transitionDescription(
        path, PreviewSettings(), 128,
        LoadingDescription::NoColorConversion,
        LoadingDescription::PreviewParameters::Thumbnail);
    transitionDescription.previewParameters.storageReference = 42;
    transitionDescription.resolveSource();
    LoadingDescription preview(path, PreviewSettings(), 128);
    preview.resolveSource();

    const PrivacyCacheTransitionToken token =
        PrivacyCacheTransition::begin(transitionDescription.thumbnailIdentifier());
    QVERIFY(token.isValid());

    const DImg image = PreviewLoadThread::loadSynchronously(preview);
    QVERIFY(image.isNull());
    QVERIFY(PrivacyCacheTransition::rollback(token));
}

#endif

void PrivacySourceResolverTest::testCrossPathTransitionOverlapRejected()
{
    LoadingDescription first(QLatin1String("/ordinary/first.jpg"),
                             PreviewSettings(), 128,
                             LoadingDescription::NoColorConversion,
                             LoadingDescription::PreviewParameters::Thumbnail);
    first.previewParameters.storageReference = 41;
    first.resolveSource();
    LoadingDescription second(QLatin1String("/ordinary/second.jpg"),
                              PreviewSettings(), 128,
                              LoadingDescription::NoColorConversion,
                              LoadingDescription::PreviewParameters::Thumbnail);
    second.previewParameters.storageReference = 42;
    second.resolveSource();

    const PrivacyCacheTransitionToken firstToken =
        PrivacyCacheTransition::begin(first.thumbnailIdentifier());
    QVERIFY(firstToken.isValid());

    const PrivacyCacheTransitionToken repeated =
        PrivacyCacheTransition::begin(first.thumbnailIdentifier());
    QVERIFY(repeated.isValid());
    QCOMPARE(repeated.logicalFilePath(), firstToken.logicalFilePath());

    QVERIFY(!PrivacyCacheTransition::begin(second.thumbnailIdentifier()).isValid());
    QVERIFY(PrivacyCacheTransition::rollback(firstToken));

    const PrivacyCacheTransitionToken secondToken =
        PrivacyCacheTransition::begin(second.thumbnailIdentifier());
    QVERIFY(secondToken.isValid());
    QVERIFY(PrivacyCacheTransition::rollback(secondToken));
}

void PrivacySourceResolverTest::testProtectTransitionBlocksAndPurgesInFlightWrite()
{
    const QString path = QLatin1String("/logical/item.jpg");
    const QRect faceRect(10, 20, 30, 40);
    QSharedPointer<FixedProvider> provider(
        new FixedProvider(PrivacySourceResult::notHandled()));
    PrivacySourceResolver::setProvider(provider);
    LoadingDescription oldDescription(path, PreviewSettings(), 128,
                                      LoadingDescription::NoColorConversion,
                                      LoadingDescription::PreviewParameters::Thumbnail);
    oldDescription.previewParameters.storageReference = 42;
    oldDescription.resolveSource();
    const ThumbnailIdentifier oldIdentifier = oldDescription.thumbnailIdentifier();
    QVERIFY(oldIdentifier.sourceResolutionApplied);

    PrivacyPersistentCacheWriteGuard writer(path);
    QVERIFY(writer.isAcquired());
    PrivacyCacheTransitionToken token;
    std::atomic<bool> beginStarted(false);
    std::atomic<bool> beginReturned(false);
    std::thread beginWorker([&]()
    {
        beginStarted.store(true);
        token = PrivacyCacheTransition::begin(oldIdentifier);
        beginReturned.store(true);
    });

    while (!beginStarted.load())
    {
        std::this_thread::yield();
    }

    PrivacySourceRequest request;
    request.logicalFilePath = path;
    request.itemReference   = 42;
    request.consumer        = PrivacySourceRequest::Thumbnail;
    PrivacySourceResult blocked;

    for (int i = 0 ; i < 10000 ; ++i)
    {
        blocked = PrivacySourceResolver::resolve(request);

        if (blocked.disposition == PrivacySourceResult::Denied)
        {
            break;
        }

        std::this_thread::yield();
    }

    const bool blockedBeforeRelease =
        (blocked.disposition == PrivacySourceResult::Denied);
    const bool returnedBeforeRelease = beginReturned.load();
    writer.release();
    beginWorker.join();

    QVERIFY(blockedBeforeRelease);
    QVERIFY(!returnedBeforeRelease);
    QVERIFY(token.isValid());
    QVERIFY(PrivacyCacheTransition::isActive(token));
    QVERIFY(!oldDescription.sourceResolutionIsCurrent());

    QVERIFY(!blocked.cacheNamespace.isEmpty());

    FakeTransitionBackend backend;
    backend.persistentRows.insert(persistentAddress(oldIdentifier, QRect()));
    backend.persistentRows.insert(persistentAddress(oldIdentifier, faceRect));

    PrivacyCacheTransitionInventory inventory;
    inventory.detailAndFaceRectangles << faceRect;
    inventory.detailAndFaceInventoryComplete = true;
    inventory.legacyPrimaryAliasInventoryComplete = true;

    const PrivacyCacheTransition::Result purge =
        PrivacyCacheTransition::purge(token, inventory, &backend);
    QCOMPARE(purge.status, PrivacyCacheTransition::Complete);
    QCOMPARE(backend.evictedPaths.size(), 1);
    QCOMPARE(backend.evictedPaths.constFirst(), path);
    QVERIFY(backend.persistentRows.isEmpty());
    const quint64 priorGeneration = PrivacySourceResolver::currentGeneration();
    provider->setResult(PrivacySourceResult::resolved(
                            path,
                            QLatin1String("category-a:locked:1"),
                            PrivacySourceResult::Persistent));
    QVERIFY(PrivacyCacheTransition::finish(token));
    QVERIFY(PrivacySourceResolver::currentGeneration() > priorGeneration);
    QVERIFY(PrivacyCacheTransition::finish(token));
    QVERIFY(!PrivacyCacheTransition::isActive(token));
}

void PrivacySourceResolverTest::testUnprotectDeletesOnlyExactNamespacedPrimary()
{
    const QString path = QLatin1String("/logical/item.jpg");
    const QRect cropRect(1, 2, 50, 60);
    QSharedPointer<FixedProvider> provider(
        new FixedProvider(PrivacySourceResult::resolved(
                              QLatin1String("/runtime/original.jpg"),
                              QLatin1String("category-a:unlocked:2"))));
    PrivacySourceResolver::setProvider(provider);

    LoadingDescription unlocked(path, PreviewSettings(), 128,
                                LoadingDescription::NoColorConversion,
                                LoadingDescription::PreviewParameters::Thumbnail);
    unlocked.previewParameters.storageReference = 42;
    unlocked.resolveSource();
    const ThumbnailIdentifier unlockedIdentifier = unlocked.thumbnailIdentifier();
    const PrivacyCacheTransitionToken token =
        PrivacyCacheTransition::begin(unlockedIdentifier);
    QVERIFY(token.isValid());

    ThumbnailIdentifier legacy(path);
    legacy.id = 42;

    ThumbnailIdentifier unrelated(QLatin1String("/logical/ordinary.jpg"));
    unrelated.id = 77;

    FakeTransitionBackend backend;
    backend.persistentRows.insert(persistentAddress(unlockedIdentifier, QRect()));
    backend.persistentRows.insert(persistentAddress(unlockedIdentifier, cropRect));
    backend.persistentRows.insert(persistentAddress(legacy, QRect()));
    backend.persistentRows.insert(persistentAddress(legacy, cropRect));

    const QString unrelatedAddress = persistentAddress(unrelated, QRect());
    backend.persistentRows.insert(unrelatedAddress);

    PrivacyCacheTransitionInventory inventory;
    inventory.direction = PrivacyCacheTransitionInventory::Unprotect;

    QCOMPARE(PrivacyCacheTransition::purge(token, inventory, &backend).status,
             PrivacyCacheTransition::Complete);
    QCOMPARE(backend.persistentRows.size(), 4);
    QVERIFY(backend.persistentRows.contains(unrelatedAddress));
    QVERIFY(backend.persistentRows.contains(persistentAddress(legacy, QRect())));
    QVERIFY(backend.persistentRows.contains(persistentAddress(legacy, cropRect)));
    QVERIFY(backend.persistentRows.contains(
                persistentAddress(unlockedIdentifier, cropRect)));

    provider->setResult(PrivacySourceResult::resolved(
                            path,
                            QLatin1String("category-a:locked:2"),
                            PrivacySourceResult::Persistent));
    QVERIFY(PrivacyCacheTransition::finish(token));
}

void PrivacySourceResolverTest::testRepeatedPurgeAndIncompleteInventoryFailClosed()
{
    const QString path = QLatin1String("/logical/item.jpg");
    LoadingDescription description(path, PreviewSettings(), 128,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = 42;
    description.resolveSource();
    const PrivacyCacheTransitionToken token =
        PrivacyCacheTransition::begin(description.thumbnailIdentifier());
    QVERIFY(token.isValid());

    FakeTransitionBackend backend;
    PrivacyCacheTransitionInventory incomplete;
    QCOMPARE(PrivacyCacheTransition::purge(token, incomplete, &backend).status,
             PrivacyCacheTransition::IncompleteOwnershipInventory);
    QVERIFY(PrivacyCacheTransition::isActive(token));

    PrivacyCacheTransitionInventory complete;
    complete.detailAndFaceInventoryComplete = true;
    complete.legacyPrimaryAliasInventoryComplete = true;
    QCOMPARE(PrivacyCacheTransition::purge(token, complete, &backend).status,
             PrivacyCacheTransition::Complete);
    QCOMPARE(PrivacyCacheTransition::purge(token, complete, &backend).status,
             PrivacyCacheTransition::Complete);
    backend.removalSucceeds = false;
    QCOMPARE(PrivacyCacheTransition::purge(token, complete, &backend).status,
             PrivacyCacheTransition::PersistentPurgeFailed);
    // A failed idempotent retry cannot revoke a previously completed purge.

    QVERIFY(PrivacyCacheTransition::finish(token));
    QVERIFY(PrivacyCacheTransition::finish(token));
}

void PrivacySourceResolverTest::testInvalidOwnershipInventoryDoesNotDelete()
{
    const QString path = QLatin1String("/logical/item.jpg");
    LoadingDescription description(path, PreviewSettings(), 128,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = 42;
    description.resolveSource();
    const PrivacyCacheTransitionToken token =
        PrivacyCacheTransition::begin(description.thumbnailIdentifier());
    QVERIFY(token.isValid());

    ThumbnailIdentifier unrelated(QLatin1String("/logical/ordinary.jpg"));
    unrelated.id = 99;

    FakeTransitionBackend backend;
    const QString unrelatedAddress = persistentAddress(unrelated, QRect());
    backend.persistentRows.insert(unrelatedAddress);

    PrivacyCacheTransitionInventory invalid;
    invalid.direction =
        static_cast<PrivacyCacheTransitionInventory::Direction>(99);
    QCOMPARE(PrivacyCacheTransition::purge(token, invalid, &backend).status,
             PrivacyCacheTransition::InvalidInventory);
    QCOMPARE(backend.persistentRows.size(), 1);
    QVERIFY(backend.persistentRows.contains(unrelatedAddress));
    QVERIFY(PrivacyCacheTransition::isActive(token));

    PrivacyCacheTransitionInventory valid;
    valid.detailAndFaceInventoryComplete = true;
    valid.legacyPrimaryAliasInventoryComplete = true;
    QCOMPARE(PrivacyCacheTransition::purge(token, valid, &backend).status,
             PrivacyCacheTransition::Complete);
    QCOMPARE(backend.persistentRows.size(), 1);
    QVERIFY(backend.persistentRows.contains(unrelatedAddress));

    QVERIFY(PrivacyCacheTransition::finish(token));

    LoadingDescription rollbackDescription(path, PreviewSettings(), 128,
                                           LoadingDescription::NoColorConversion,
                                           LoadingDescription::PreviewParameters::Thumbnail);
    rollbackDescription.previewParameters.storageReference = 42;
    rollbackDescription.resolveSource();
    const PrivacyCacheTransitionToken rollbackToken =
        PrivacyCacheTransition::begin(rollbackDescription.thumbnailIdentifier());
    QVERIFY(rollbackToken.isValid());
    QVERIFY(PrivacyCacheTransition::rollback(rollbackToken));
    QVERIFY(PrivacyCacheTransition::rollback(rollbackToken));
    QVERIFY(!PrivacyCacheTransition::isActive(rollbackToken));
}

void PrivacySourceResolverTest::testActualLegacyDetailInventoryFailsClosed()
{
    QTemporaryDir databaseDirectory;
    QVERIFY(databaseDirectory.isValid());

    DbEngineParameters parameters;
    parameters.databaseType = DbEngineParameters::SQLiteDatabaseType();
    parameters.setThumbsDatabasePath(
        databaseDirectory.filePath(QLatin1String("thumbnails.db")));
    parameters.legacyAndDefaultChecks();
    ThumbnailLoadThread::initializeThumbnailDatabase(parameters);
    QVERIFY(ThumbsDbAccess::isInitialized());

    const QString path = QLatin1String("/logical/item.jpg");
    const QRect expectedRect(3, 4, 50, 60);
    LoadingDescription description(path, PreviewSettings(), 128,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = 42;
    description.resolveSource();
    const PrivacyCacheTransitionToken token =
        PrivacyCacheTransition::begin(description.thumbnailIdentifier());
    QVERIFY(token.isValid());

    const auto detailIdentifier = [](const QString& ownerPath,
                                     const QString& rectangle)
    {
        QUrl url = QUrl::fromLocalFile(ownerPath);
        url.setScheme(QLatin1String("detail"));
        QUrlQuery query;
        query.addQueryItem(QLatin1String("rect"), rectangle);
        url.setQuery(query);
        return url.toString();
    };
    const auto insertIdentifier = [](const QString& identifier)
    {
        ThumbsDbAccess access;
        ThumbsDbInfo info;
        info.type = DatabaseThumbnail::PGF;
        info.modificationDate = QDateTime::currentDateTimeUtc();
        info.data = QByteArray("thumbnail");
        QVariant id;

        if (access.db()->insertThumbnail(info, &id) !=
            BdEngineBackend::NoErrors)
        {
            return false;
        }

        return (access.db()->insertCustomIdentifier(identifier, id.toInt()) ==
                BdEngineBackend::NoErrors);
    };

    QVERIFY(insertIdentifier(detailIdentifier(
        path, QLatin1String("3,4-50x60"))));
    QVERIFY(insertIdentifier(detailIdentifier(
        QLatin1String("/logical/unrelated.jpg"), QLatin1String("broken"))));

    QList<QRect> rectangles;
    QVERIFY(ThumbnailLoadThread::privacyLegacyDetailRectangles(token,
                                                               &rectangles));
    QCOMPARE(rectangles, QList<QRect>({ expectedRect }));

    // This row is not reachable through face/catalogue inventory. Direct table
    // enumeration must still see it and reject its malformed target identity.
    QVERIFY(insertIdentifier(detailIdentifier(path, QLatin1String("broken"))));
    QVERIFY(!ThumbnailLoadThread::privacyLegacyDetailRectangles(token,
                                                                &rectangles));
    QVERIFY(rectangles.isEmpty());
    QVERIFY(PrivacyCacheTransition::rollback(token));
    ThumbsDbAccess::cleanUpDatabase();
}

void PrivacySourceResolverTest::testProviderReplacementMakesFinishFailClosed()
{
    const QString path = QLatin1String("/logical/item.jpg");
    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::notHandled())));
    LoadingDescription description(path, PreviewSettings(), 128,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = 42;
    description.resolveSource();
    const PrivacyCacheTransitionToken token =
        PrivacyCacheTransition::begin(description.thumbnailIdentifier());
    QVERIFY(token.isValid());

    FakeTransitionBackend backend;
    PrivacyCacheTransitionInventory inventory;
    inventory.detailAndFaceInventoryComplete = true;
    inventory.legacyPrimaryAliasInventoryComplete = true;
    QCOMPARE(PrivacyCacheTransition::purge(token, inventory, &backend).status,
             PrivacyCacheTransition::Complete);

    PrivacySourceResolver::setProvider(
        QSharedPointer<FixedProvider>(
            new FixedProvider(PrivacySourceResult::resolved(
                                  path,
                                  QLatin1String("replacement-provider")))));
    QVERIFY(!PrivacyCacheTransition::finish(token));
    QVERIFY(PrivacyCacheTransition::isActive(token));
}

QTEST_GUILESS_MAIN(PrivacySourceResolverTest)

#include "privacysourceresolver_utest.moc"

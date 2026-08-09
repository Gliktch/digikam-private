/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycategorysessionowner.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QDir>
#include <QReadWriteLock>
#include <QStandardPaths>

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "coredbchangesets.h"
#include "privacygocryptfscategorystore.h"
#include "privacyprocessrunner.h"

namespace Digikam
{

class Q_DECL_HIDDEN PrivacyCategorySessionOwner::Private
{
public:

    Private(const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
            const QSharedPointer<const PrivacyRootVerifier>& rootVerifier)
        : runtime(runtime),
          rootVerifier(rootVerifier),
          storeBackend(processRunner, mountProbe, toolPaths(), runtimeRoot()),
          coordinator(repository, storeBackend, *rootVerifier, *runtime)
    {
    }

    static PrivacyGocryptfsToolPaths toolPaths()
    {
        PrivacyGocryptfsToolPaths paths;
        paths.gocryptfs = QStandardPaths::findExecutable(QLatin1String("gocryptfs"));
        paths.gocryptfsXray = QStandardPaths::findExecutable(
                                  QLatin1String("gocryptfs-xray"));
        paths.fusermount = QStandardPaths::findExecutable(QLatin1String("fusermount3"));

        return paths;
    }

    static QString runtimeRoot()
    {
        return QDir(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation))
            .filePath(QLatin1String("digikam-private/category-stores"));
    }

    void publishManualTagVisibilityChange(const QString& categoryUuid = QString())
    {
        PrivacyRepositorySnapshot snapshot;

        if (!repository.loadSnapshot(&snapshot))
        {
            return;
        }

        QList<qlonglong> imageIds;
        QSet<int> tagIds;
        CoreDbAccess access;

        for (const PrivacyItem& item : std::as_const(snapshot.items))
        {
            if (!categoryUuid.isEmpty() && (item.categoryUuid != categoryUuid))
            {
                continue;
            }

            imageIds << item.imageId;

            const QList<int> itemTagIds = access.db()->getItemTagIDs(item.imageId);
            tagIds.unite(QSet<int>(itemTagIds.cbegin(), itemTagIds.cend()));
        }

        if (!imageIds.isEmpty())
        {
            std::sort(imageIds.begin(), imageIds.end());
            QList<int> affectedTagIds = tagIds.values();
            std::sort(affectedTagIds.begin(), affectedTagIds.end());
            access.backend()->recordChangeset(ImageTagChangeset(
                imageIds, affectedTagIds, ImageTagChangeset::VisibilityChanged));
        }
    }

public:

    mutable QReadWriteLock                         lifecycleLock;
    bool                                           closed = false;
    QSharedPointer<PrivacyRuntimeCoordinator>      runtime;
    QSharedPointer<const PrivacyRootVerifier>      rootVerifier;
    QProcessPrivacyProcessRunner                   processRunner;
    ProcMountInfoPrivacyMountStateProbe            mountProbe;
    PrivacyCoreDbCategorySessionRepository         repository;
    PrivacyGocryptfsCategoryStoreBackend           storeBackend;
    PrivacyCategorySessionCoordinator              coordinator;
};

QSharedPointer<PrivacyCategorySessionOwner> PrivacyCategorySessionOwner::create(
    const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
    const QSharedPointer<const PrivacyRootVerifier>& rootVerifier)
{
    if (!runtime || !rootVerifier)
    {
        return {};
    }

    return QSharedPointer<PrivacyCategorySessionOwner>(
               new PrivacyCategorySessionOwner(runtime, rootVerifier));
}

PrivacyCategorySessionOwner::PrivacyCategorySessionOwner(
    const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
    const QSharedPointer<const PrivacyRootVerifier>& rootVerifier)
    : d(new Private(runtime, rootVerifier))
{
}

PrivacyCategorySessionOwner::~PrivacyCategorySessionOwner()
{
    shutdown();
}

PrivacyCategorySessionResult PrivacyCategorySessionOwner::createCategory(
    const PrivacyCategoryCreateRequest& request, const QString& passwordText)
{
    QReadLocker locker(&d->lifecycleLock);

    if (d->closed)
    {
        return { PrivacyCategorySessionStatus::TransactionBlocked };
    }

    return d->coordinator.createCategory(request, passwordText);
}

PrivacyCategorySessionResult PrivacyCategorySessionOwner::unlockCategory(
    const QString& categoryUuid, const QString& passwordText)
{
    QReadLocker locker(&d->lifecycleLock);

    if (d->closed)
    {
        return { PrivacyCategorySessionStatus::TransactionBlocked };
    }

    const PrivacyCategorySessionResult result =
        d->coordinator.unlockCategory(categoryUuid, passwordText);

    if (result.succeeded())
    {
        d->publishManualTagVisibilityChange(categoryUuid);
    }

    return result;
}

PrivacyCategorySessionResult PrivacyCategorySessionOwner::lockCategory(
    const QString& categoryUuid)
{
    QReadLocker locker(&d->lifecycleLock);

    if (d->closed)
    {
        return { PrivacyCategorySessionStatus::TransactionBlocked };
    }

    const PrivacyCategorySessionResult result = d->coordinator.lockCategory(categoryUuid);

    if (result.succeeded())
    {
        d->publishManualTagVisibilityChange(categoryUuid);
    }

    return result;
}

QList<PrivacyCategorySessionResult> PrivacyCategorySessionOwner::lockAllCategories()
{
    QReadLocker locker(&d->lifecycleLock);

    if (d->closed)
    {
        PrivacyCategorySessionResult result;
        result.status = PrivacyCategorySessionStatus::TransactionBlocked;

        return { result };
    }

    const QList<PrivacyCategorySessionResult> results =
        d->coordinator.lockAllCategories();

    if (std::any_of(results.cbegin(), results.cend(),
                    [](const PrivacyCategorySessionResult& result)
                    {
                        return result.succeeded();
                    }))
    {
        d->publishManualTagVisibilityChange();
    }

    return results;
}

PrivacyCategoryOperationStatus PrivacyCategorySessionOwner::runWhileUnlocked(
    const QString& categoryUuid, const std::function<void()>& operation)
{
    QReadLocker locker(&d->lifecycleLock);

    return d->closed
         ? PrivacyCategoryOperationStatus::TransactionBlocked
         : d->coordinator.runWhileUnlocked(categoryUuid, operation);
}

PrivacyCategoryOperationStatus PrivacyCategorySessionOwner::runWithUnlockedSecret(
    const QString& categoryUuid,
    const std::function<void(const PrivacyPassword&)>& operation)
{
    QReadLocker locker(&d->lifecycleLock);

    return d->closed
         ? PrivacyCategoryOperationStatus::TransactionBlocked
         : d->coordinator.runWithUnlockedSecret(categoryUuid, operation);
}

PrivacyCategorySessionResult
PrivacyCategorySessionOwner::runWithFreshlyAuthenticatedSecret(
    const QString& categoryUuid, const QString& passwordText,
    const std::function<void(const PrivacyPassword&)>& operation,
    const QString& allowedActiveItemTransactionUuid)
{
    QReadLocker locker(&d->lifecycleLock);

    if (d->closed)
    {
        return { PrivacyCategorySessionStatus::TransactionBlocked };
    }

    return d->coordinator.runWithFreshlyAuthenticatedSecret(
               categoryUuid, passwordText, operation,
               allowedActiveItemTransactionUuid);
}

PrivacyCategorySessionResult
PrivacyCategorySessionOwner::setCategoryTagVisibilityMode(
    const QString& categoryUuid, PrivacyTagVisibilityMode mode,
    const QString& passwordText)
{
    QReadLocker locker(&d->lifecycleLock);

    if (d->closed)
    {
        return { PrivacyCategorySessionStatus::TransactionBlocked };
    }

    const PrivacyCategorySessionResult result =
        d->coordinator.setCategoryTagVisibilityMode(categoryUuid, mode,
                                                    passwordText);

    if (result.succeeded())
    {
        d->publishManualTagVisibilityChange(categoryUuid);
    }

    return result;
}

bool PrivacyCategorySessionOwner::ownsSecret(const QString& categoryUuid) const
{
    QReadLocker locker(&d->lifecycleLock);

    return (!d->closed && d->coordinator.ownsSecret(categoryUuid));
}

void PrivacyCategorySessionOwner::shutdown()
{
    QWriteLocker locker(&d->lifecycleLock);
    d->closed = true;

    // Retry retained sessions when an earlier backend unmount failed. The
    // coordinator keeps those sessions specifically so a later attempt can
    // finish without losing the lease or authentication secret.
    d->coordinator.lockAllCategories();
}

} // namespace Digikam

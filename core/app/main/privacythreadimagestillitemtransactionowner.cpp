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

// C++ includes

#include <algorithm>
#include <cerrno>
#include <utility>

// POSIX includes

#if defined(Q_OS_UNIX)
#   include <fcntl.h>
#   include <signal.h>
#   include <sys/stat.h>
#   include <unistd.h>
#endif

// Qt includes

#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QProcess>
#include <QRecursiveMutex>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QThread>
#include <QUuid>

// Local includes

#include "collectionlocation.h"
#include "collectionmanager.h"
#include "iteminfo.h"
#include "metaenginesettings.h"
#include "privacycategorysessionowner.h"
#include "privacycontracts.h"
#include "privacyrepository.h"
#include "privacystillitemtransaction.h"
#include "privacythreadimagestillitemcachegate.h"

namespace Digikam
{

namespace
{

class StillItemOwnerData
{
public:

    QMutex mutex;
    QWeakPointer<PrivacyThreadImageIOStillItemTransactionOwner> current;
};

Q_GLOBAL_STATIC(StillItemOwnerData, ownerData)

class ScopeExit final
{
public:

    explicit ScopeExit(std::function<void()> callback)
        : callback(std::move(callback))
    {
    }

    ~ScopeExit()
    {
        callback();
    }

private:

    std::function<void()> callback;

    Q_DISABLE_COPY(ScopeExit)
};

QString newUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toLower();
}

bool launchCompatibilityGuard(
    const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& expectation,
    qint64* const processId, QString* const detail)
{
#if !defined(Q_OS_LINUX)
    Q_UNUSED(root);
    Q_UNUSED(expectation);
    Q_UNUSED(processId);

    if (detail)
    {
        *detail = QStringLiteral(
            "The independent Compatibility guard requires Linux");
    }

    return false;
#else
    if (!processId || !detail || !root.isValid() ||
        (root.kind != PrivacyStorageRootKind::AlbumRoot))
    {
        return false;
    }

    const QString program = QDir(QCoreApplication::applicationDirPath())
                                .filePath(
                                    QStringLiteral("digikam-private-guard"));

    if (!QFileInfo(program).isExecutable())
    {
        *detail = QStringLiteral(
            "The bundled Compatibility guard executable is unavailable");
        return false;
    }

    const QString runtimePath = QStandardPaths::writableLocation(
        QStandardPaths::RuntimeLocation);
    QTemporaryFile readyFile(QDir(runtimePath).filePath(
        QStringLiteral("digikam-private-guard-XXXXXX.ready")));

    if (runtimePath.isEmpty() || !QFileInfo(runtimePath).isDir() ||
        !readyFile.open() ||
        !readyFile.setPermissions(QFileDevice::ReadOwner |
                                  QFileDevice::WriteOwner))
    {
        *detail = QStringLiteral(
            "A secure Compatibility guard handshake could not be created");
        return false;
    }

    const QString readyPath = readyFile.fileName();
    const QString readyToken = newUuid();
    readyFile.setAutoRemove(false);
    readyFile.close();

    QProcess guard;
    guard.setProgram(program);
    guard.setArguments({
        QStringLiteral("--parent-pid"),
        QString::number(QCoreApplication::applicationPid()),
        QStringLiteral("--root-path"), root.configuredPath,
        QStringLiteral("--root-uuid"), root.uuid,
        QStringLiteral("--root-marker-uuid"), root.markerUuid,
        QStringLiteral("--root-identity"),
        QString::fromLatin1(root.identityData.toBase64()),
        QStringLiteral("--album-root-id"),
        QString::number(root.albumRootId),
        QStringLiteral("--root-device"),
        QString::number(expectation.device),
        QStringLiteral("--root-inode"),
        QString::number(expectation.inode),
        QStringLiteral("--ready-file"), readyPath,
        QStringLiteral("--ready-token"), readyToken,
        QStringLiteral("--all-compatibility")
    });
    guard.setWorkingDirectory(QCoreApplication::applicationDirPath());
    guard.setStandardOutputFile(QProcess::nullDevice());
    guard.setStandardErrorFile(QProcess::nullDevice());
    qint64 launchedProcessId = 0;

    if (!guard.startDetached(&launchedProcessId) || (launchedProcessId <= 0))
    {
        QFile::remove(readyPath);
        *detail = guard.errorString().isEmpty()
                ? QStringLiteral("The independent Compatibility guard did not start")
                : guard.errorString();
        return false;
    }

    bool armed = false;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 2000)
    {
        QFile acknowledgement(readyPath);

        if (acknowledgement.open(QIODevice::ReadOnly) &&
            (acknowledgement.readAll() == readyToken.toUtf8()))
        {
            armed = true;
            break;
        }

        QThread::msleep(10);
    }

    QFile::remove(readyPath);

    if (!armed)
    {
        *detail = QStringLiteral(
            "The independent Compatibility guard did not acknowledge readiness");
        return false;
    }

    *processId = launchedProcessId;
    return true;
#endif
}

bool processIsRunning(qint64 processId)
{
#if !defined(Q_OS_LINUX)
    Q_UNUSED(processId);
    return false;
#else
    if ((processId <= 0) ||
        ((::kill(static_cast<pid_t>(processId), 0) != 0) &&
         (errno != EPERM)))
    {
        return false;
    }

    const QString expected = QFileInfo(
        QDir(QCoreApplication::applicationDirPath()).filePath(
            QStringLiteral("digikam-private-guard"))).canonicalFilePath();
    const QString actual = QFileInfo(
        QStringLiteral("/proc/%1/exe").arg(processId)).symLinkTarget();

    return (!expected.isEmpty() && (QDir::cleanPath(actual) == expected));
#endif
}

PrivacyStillItemTransactionResult actionFailure(
    PrivacyStillItemTransactionStatus status, const QString& detail)
{
    PrivacyStillItemTransactionResult result;
    result.status = status;
    result.detail = detail;
    return result;
}

PrivacyCompatibilityBatchResult batchFailure(
    PrivacyStillItemTransactionStatus status, const QString& detail,
    int requestedCount = 0)
{
    PrivacyCompatibilityBatchResult result;
    result.status = status;
    result.detail = detail;
    result.requestedCount = requestedCount;
    return result;
}

const PrivacyCategory* categoryForUuid(const PrivacyRepositorySnapshot& snapshot,
                                       const QString& categoryUuid)
{
    const auto it = std::find_if(
        snapshot.categories.cbegin(), snapshot.categories.cend(),
        [&categoryUuid](const PrivacyCategory& category)
        {
            return (category.uuid == categoryUuid);
        });

    return (it == snapshot.categories.cend()) ? nullptr : &*it;
}

const PrivacyStorageRoot* rootForUuid(const PrivacyRepositorySnapshot& snapshot,
                                      const QString& rootUuid)
{
    const auto it = std::find_if(
        snapshot.storageRoots.cbegin(), snapshot.storageRoots.cend(),
        [&rootUuid](const PrivacyStorageRoot& root)
        {
            return (root.uuid == rootUuid);
        });

    return (it == snapshot.storageRoots.cend()) ? nullptr : &*it;
}

bool rootExpectation(const PrivacyStorageRoot& root,
                     PrivacyJournalRootExpectation* const expectation)
{
    if (!expectation || !root.isValid() ||
        (root.kind != PrivacyStorageRootKind::AlbumRoot))
    {
        return false;
    }

    const CollectionLocation location = CollectionManager::instance()
                                            ->locationForAlbumRootId(
                                                root.albumRootId);
    const QString currentPath = QDir::cleanPath(location.albumRootPath());

    if (location.isNull() || !location.isAvailable() ||
        (currentPath != root.configuredPath) ||
        !PrivacyRootIdentityCodec::matchesAlbumRootV1(
            root.identityData, root.albumRootId, location.identifier))
    {
        return false;
    }

#if defined(Q_OS_UNIX)

    const QByteArray path = QFile::encodeName(currentPath);
    const int descriptor = ::open(path.constData(),
                                  O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);

    if (descriptor < 0)
    {
        return false;
    }

    struct stat status = {};
    const bool valid = (::fstat(descriptor, &status) == 0) &&
                       S_ISDIR(status.st_mode) &&
                       (status.st_dev != 0) && (status.st_ino != 0);
    ::close(descriptor);

    if (!valid)
    {
        return false;
    }

    expectation->rootUuid = root.uuid;
    expectation->markerUuid = root.markerUuid;
    expectation->device = static_cast<quint64>(status.st_dev);
    expectation->inode = static_cast<quint64>(status.st_ino);
    expectation->identitySha256 = QCryptographicHash::hash(
        root.identityData, QCryptographicHash::Sha256);
    return true;

#else

    Q_UNUSED(root);
    return false;

#endif
}

QString categorySessionFailureDetail(PrivacyCategorySessionStatus status)
{
    switch (status)
    {
        case PrivacyCategorySessionStatus::InvalidPassword:
        case PrivacyCategorySessionStatus::AuthenticationFailed:
        {
            return QStringLiteral("The category password was not accepted");
        }

        case PrivacyCategorySessionStatus::StoreOffline:
        {
            return QStringLiteral("The category store is offline");
        }

        case PrivacyCategorySessionStatus::StoreIdentityMismatch:
        {
            return QStringLiteral("The category store identity does not match");
        }

        case PrivacyCategorySessionStatus::TransactionBlocked:
        {
            return QStringLiteral("Another category operation is already active");
        }

        case PrivacyCategorySessionStatus::CategoryLocked:
        {
            return QStringLiteral("The category is locked");
        }

        default:
        {
            return QStringLiteral("The category could not be authenticated");
        }
    }
}

PrivacyStillItemTransactionStatus categorySessionTransactionStatus(
    PrivacyCategorySessionStatus status)
{
    switch (status)
    {
        case PrivacyCategorySessionStatus::InvalidPassword:
        case PrivacyCategorySessionStatus::AuthenticationFailed:
        case PrivacyCategorySessionStatus::CategoryLocked:
        {
            return PrivacyStillItemTransactionStatus::AuthenticationRequired;
        }

        default:
        {
            return PrivacyStillItemTransactionStatus::CategoryUnavailable;
        }
    }
}

bool currentActionComposition(
    PrivacyRuntimeCoordinator* const expectedRuntime,
    QSharedPointer<PrivacyRuntimeCoordinator>* const runtime,
    QSharedPointer<PrivacyCategorySessionOwner>* const sessions = nullptr)
{
    if (!expectedRuntime || !runtime)
    {
        return false;
    }

    const QSharedPointer<PrivacyRuntimeCoordinator> before =
        PrivacyStartupRecovery::coordinator();
    const QSharedPointer<PrivacyCategorySessionOwner> categorySessions =
        sessions ? PrivacyStartupRecovery::categorySessions()
                 : QSharedPointer<PrivacyCategorySessionOwner>();
    const QSharedPointer<PrivacyRuntimeCoordinator> after =
        PrivacyStartupRecovery::coordinator();

    if (!before || (before != after) || (before.data() != expectedRuntime) ||
        (sessions && !categorySessions))
    {
        return false;
    }

    *runtime = before;

    if (sessions)
    {
        *sessions = categorySessions;
    }

    return true;
}

} // namespace

class Q_DECL_HIDDEN PrivacyThreadImageIOStillItemTransactionOwner::Private
{
public:

    explicit Private(PrivacyRuntimeCoordinator& runtime)
        : runtime(runtime),
          engine(persistence, runtime, cacheGate)
    {
    }

public:

    PrivacyRuntimeCoordinator&               runtime;
    PrivacyCoreDbStillItemPersistence       persistence;
    PrivacyThreadImageIOStillItemCacheGate  cacheGate;
    mutable QRecursiveMutex                  transactionMutex;
    mutable PrivacyStillItemTransactionEngine engine;
    mutable QHash<QString, qint64>             compatibilityGuardProcesses;
    mutable QString                          authenticatedTransactionUuid;
    mutable const PrivacyPassword*           authenticatedPassword = nullptr;
    mutable bool                             freshAuthenticationConfirmed = false;
    mutable PrivacyStillItemTransactionResult* authenticatedResult = nullptr;
};

QSharedPointer<const PrivacyTransactionRecovery>
PrivacyThreadImageIOStillItemTransactionOwner::create(
    PrivacyRuntimeCoordinator& runtime)
{
    const QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner> owner(
        new PrivacyThreadImageIOStillItemTransactionOwner(runtime));

    {
        QMutexLocker locker(&ownerData->mutex);
        ownerData->current = owner;
    }

    return owner;
}

QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner>
PrivacyThreadImageIOStillItemTransactionOwner::current()
{
    QMutexLocker locker(&ownerData->mutex);
    return ownerData->current.toStrongRef();
}

PrivacyThreadImageIOStillItemTransactionOwner::
    PrivacyThreadImageIOStillItemTransactionOwner(
        PrivacyRuntimeCoordinator& runtime)
    : d(new Private(runtime))
{
    d->engine.setCompatibilityGuardArmHook(
        [this](const PrivacyStorageRoot& root,
           const PrivacyJournalRootExpectation& expectation,
           const QString&, QString* const detail)
        {
            const qint64 existing =
                d->compatibilityGuardProcesses.value(root.uuid);

            if (processIsRunning(existing))
            {
                return true;
            }

            qint64 processId = 0;

            if (!launchCompatibilityGuard(root, expectation,
                                          &processId, detail))
            {
                d->compatibilityGuardProcesses.remove(root.uuid);
                return false;
            }

            d->compatibilityGuardProcesses.insert(root.uuid, processId);
            return true;
        });
}

PrivacyThreadImageIOStillItemTransactionOwner::
    ~PrivacyThreadImageIOStillItemTransactionOwner() = default;

PrivacyStillItemActionContext
PrivacyThreadImageIOStillItemTransactionOwner::actionContextForImage(
    qlonglong imageId) const
{
    PrivacyRepositorySnapshot snapshot;
    PrivacyStillItemActionContext result;
    QSharedPointer<PrivacyRuntimeCoordinator> runtime;

    if ((imageId <= 0) ||
        !currentActionComposition(&d->runtime, &runtime) ||
        !d->persistence.loadSnapshot(&snapshot))
    {
        return result;
    }

    const PrivacyItem* item = nullptr;

    for (const PrivacyItem& candidate : std::as_const(snapshot.items))
    {
        if (candidate.imageId == imageId)
        {
            if (item)
            {
                return result;
            }

            item = &candidate;
        }
    }

    if (item)
    {
        const PrivacyCategory* const category = categoryForUuid(
            snapshot, item->categoryUuid);

        if (category)
        {
            result.protectedCategory = *category;
        }

        const PrivacyTransaction* createdRecovery = nullptr;
        const PrivacyTransaction* activeCompatibility = nullptr;
        int activeItemTransactionCount = 0;

        for (const PrivacyTransaction& transaction :
             std::as_const(snapshot.transactions))
        {
            if (transaction.isActive() &&
                (transaction.itemUuid == item->uuid))
            {
                ++activeItemTransactionCount;

                if ((transaction.state == PrivacyTransactionState::Created) &&
                    ((transaction.type == PrivacyTransactionType::ProtectItem) ||
                     (transaction.type == PrivacyTransactionType::UnprotectItem)))
                {
                    createdRecovery = &transaction;
                }

                if (transaction.type ==
                    PrivacyTransactionType::CompatibilityUnlock)
                {
                    activeCompatibility = &transaction;
                }
            }
        }

        if (category && createdRecovery &&
            (activeItemTransactionCount == 1) &&
            (createdRecovery->categoryUuid == category->uuid) &&
            (category->backend == PrivacyBackend::Casual) &&
            (category->lifecycleState ==
             PrivacyCategoryLifecycleState::Active))
        {
            const PrivacyTransactionJournal* recoveryJournal = nullptr;
            int recoveryJournalCount = 0;

            for (const PrivacyTransactionJournal& journal :
                 std::as_const(snapshot.transactionJournals))
            {
                if (journal.transactionUuid == createdRecovery->uuid)
                {
                    ++recoveryJournalCount;
                    recoveryJournal = &journal;
                }
            }

            int rootActiveTransactionCount = 0;

            if (recoveryJournal && (recoveryJournalCount == 1))
            {
                for (const PrivacyTransaction& transaction :
                     std::as_const(snapshot.transactions))
                {
                    bool affectsRecoveryRoot = false;

                    if (!transaction.isActive())
                    {
                        continue;
                    }

                    for (const PrivacyTransactionJournal& journal :
                         std::as_const(snapshot.transactionJournals))
                    {
                        if ((journal.transactionUuid == transaction.uuid) &&
                            (journal.rootUuid == recoveryJournal->rootUuid))
                        {
                            affectsRecoveryRoot = true;
                            break;
                        }
                    }

                    rootActiveTransactionCount += affectsRecoveryRoot ? 1 : 0;
                }

                result.publicRootState = runtime->rootState(
                    recoveryJournal->rootUuid);
            }

            if (recoveryJournal && (recoveryJournalCount == 1) &&
                (rootActiveTransactionCount == 1) &&
                (result.publicRootState ==
                 PrivacyRootRuntimeState::Recovering))
            {
                result.protectedCategory = *category;
                result.recoveryTransactionUuid = createdRecovery->uuid;
                result.availability =
                    (createdRecovery->type ==
                     PrivacyTransactionType::ProtectItem)
                        ? PrivacyStillItemActionAvailability::ResumeProtectable
                        : PrivacyStillItemActionAvailability::ResumeUnprotectable;
                return result;
            }
        }

        const PrivacyAsset* primary = nullptr;
        int assetCount = 0;

        for (const PrivacyAsset& asset : std::as_const(snapshot.assets))
        {
            if (asset.itemUuid != item->uuid)
            {
                continue;
            }

            ++assetCount;

            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                if (primary)
                {
                    return result;
                }

                primary = &asset;
            }
        }

        if (primary)
        {
            result.publicRootState = runtime->rootState(
                primary->publicRootUuid);
        }

        if ((activeItemTransactionCount == 0) && category && primary &&
            (assetCount > 0) &&
            (category->backend == PrivacyBackend::Casual) &&
            (category->lifecycleState ==
             PrivacyCategoryLifecycleState::Active) &&
            (result.publicRootState ==
             PrivacyRootRuntimeState::VerifiedAvailable))
        {
            result.compatibilityAvailability =
                PrivacyCompatibilityActionAvailability::Unlockable;
        }

        if ((activeItemTransactionCount == 1) && category && primary &&
            activeCompatibility &&
            (activeCompatibility->categoryUuid == category->uuid) &&
            (activeCompatibility->itemUuid == item->uuid))
        {
            if ((activeCompatibility->type ==
                 PrivacyTransactionType::CompatibilityUnlock) &&
                (activeCompatibility->state ==
                 PrivacyTransactionState::Exposed) &&
                (result.publicRootState ==
                 PrivacyRootRuntimeState::VerifiedAvailable))
            {
                result.compatibilityAvailability =
                    PrivacyCompatibilityActionAvailability::Relockable;
                result.compatibilityUnlockTransactionUuid =
                    activeCompatibility->uuid;
            }
            else if ((activeCompatibility->type ==
                      PrivacyTransactionType::CompatibilityUnlock) &&
                     (activeCompatibility->state ==
                      PrivacyTransactionState::NeedsReconciliation))
            {
                result.compatibilityAvailability =
                    PrivacyCompatibilityActionAvailability::
                        ReconciliationRequired;
                result.compatibilityUnlockTransactionUuid =
                    activeCompatibility->uuid;
            }
        }

        if ((activeItemTransactionCount == 0) && category &&
            (category->backend == PrivacyBackend::Casual) &&
            (category->lifecycleState ==
             PrivacyCategoryLifecycleState::Active) && primary &&
            (assetCount == 1))
        {
            result.protectedCategory = *category;
            result.availability =
                (result.publicRootState ==
                 PrivacyRootRuntimeState::VerifiedAvailable)
                    ? PrivacyStillItemActionAvailability::Unprotectable
                    : PrivacyStillItemActionAvailability::ProtectedUnavailable;
        }

        if (activeItemTransactionCount > 0)
        {
            result.protectedCategory = category ? *category : PrivacyCategory();
            result.availability =
                PrivacyStillItemActionAvailability::ProtectedUnavailable;
        }

        return result;
    }

    if (runtime->publicSourceDisposition(imageId) !=
        PrivacyPublicSourceDisposition::Unprotected)
    {
        return result;
    }

    for (const PrivacyCategory& category : std::as_const(snapshot.categories))
    {
        if ((category.backend == PrivacyBackend::Casual) &&
            (category.lifecycleState == PrivacyCategoryLifecycleState::Active))
        {
            result.protectCategories << category;
        }
    }

    std::sort(result.protectCategories.begin(), result.protectCategories.end(),
              [](const PrivacyCategory& left, const PrivacyCategory& right)
              {
                  const int nameOrder = left.name.compare(right.name,
                                                          Qt::CaseInsensitive);
                  return (nameOrder == 0) ? (left.uuid < right.uuid)
                                          : (nameOrder < 0);
              });
    result.availability = PrivacyStillItemActionAvailability::Protectable;
    return result;
}

PrivacyCompatibilityCategoryContext
PrivacyThreadImageIOStillItemTransactionOwner::compatibilityContextForCategory(
    const QString& categoryUuid) const
{
    PrivacyCompatibilityCategoryContext result;
    QSharedPointer<PrivacyRuntimeCoordinator> runtime;

    if (!currentActionComposition(&d->runtime, &runtime))
    {
        return result;
    }

    Q_UNUSED(runtime);

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return result;
    }

    const PrivacyCategory* category = nullptr;
    int categoryCount = 0;
    bool otherActiveTransaction = false;
    bool reconciliationRequired = false;

    for (const PrivacyCategory& candidate : std::as_const(snapshot.categories))
    {
        if (candidate.uuid == categoryUuid)
        {
            category = &candidate;
            ++categoryCount;
        }
    }

    for (const PrivacyItem& item : std::as_const(snapshot.items))
    {
        result.protectedItemCount += (item.categoryUuid == categoryUuid) ? 1 : 0;
    }

    for (const PrivacyTransaction& transaction :
         std::as_const(snapshot.transactions))
    {
        if (!transaction.isActive() ||
            (transaction.categoryUuid != categoryUuid))
        {
            continue;
        }

        if (transaction.type == PrivacyTransactionType::CompatibilityUnlock)
        {
            ++result.activeExposureCount;
            reconciliationRequired = reconciliationRequired ||
                (transaction.state ==
                 PrivacyTransactionState::NeedsReconciliation);
        }
        else
        {
            otherActiveTransaction = true;
        }
    }

    if ((categoryCount != 1) || !category ||
        (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        return result;
    }

    if (result.activeExposureCount > 0)
    {
        result.availability = reconciliationRequired
            ? PrivacyCompatibilityActionAvailability::ReconciliationRequired
            : PrivacyCompatibilityActionAvailability::Relockable;
    }
    else if (!otherActiveTransaction && (result.protectedItemCount > 0))
    {
        result.availability =
            PrivacyCompatibilityActionAvailability::Unlockable;
    }

    return result;
}

bool PrivacyThreadImageIOStillItemTransactionOwner::categoryIsUnlocked(
    const QString& categoryUuid) const
{
    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;
    return currentActionComposition(&d->runtime, &runtime, &sessions) &&
           sessions->ownsSecret(categoryUuid);
}

PrivacyStillItemTransactionResult
PrivacyThreadImageIOStillItemTransactionOwner::protect(
    const ItemInfo& info, const QString& categoryUuid,
    const QString& passwordText,
    const ProtectAcknowledgement& acknowledgeAssetSet)
{
    if (info.isNull() || (info.id() <= 0) ||
        ((info.category() != DatabaseItem::Image) &&
         (info.category() != DatabaseItem::Video)) ||
        !info.isLocationAvailable())
    {
        return actionFailure(PrivacyStillItemTransactionStatus::InvalidRequest,
                             QStringLiteral("Select one available photo or video"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;

    if (!currentActionComposition(&d->runtime, &runtime, &sessions))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                             QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot initialSnapshot;

    if (!d->persistence.loadSnapshot(&initialSnapshot))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::PersistenceFailure,
                             QStringLiteral("The privacy catalogue could not be read"));
    }

    const PrivacyCategory* const requestedCategory = categoryForUuid(
        initialSnapshot, categoryUuid);
    const bool alreadyMapped = std::any_of(
        initialSnapshot.items.cbegin(), initialSnapshot.items.cend(),
        [&info](const PrivacyItem& item)
        {
            return (item.imageId == info.id());
        });

    if (!requestedCategory ||
        (requestedCategory->backend != PrivacyBackend::Casual) ||
        (requestedCategory->lifecycleState !=
         PrivacyCategoryLifecycleState::Active) ||
        alreadyMapped ||
        (runtime->publicSourceDisposition(info.id()) !=
         PrivacyPublicSourceDisposition::Unprotected))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral("The item or requested Casual category is unavailable"));
    }

    if (!sessions->ownsSecret(categoryUuid))
    {
        const PrivacyCategorySessionResult unlock =
            sessions->unlockCategory(categoryUuid, passwordText);

        if (!unlock.succeeded())
        {
            return actionFailure(
                categorySessionTransactionStatus(unlock.status),
                categorySessionFailureDetail(unlock.status));
        }
    }

    PrivacyStillItemTransactionResult result = actionFailure(
        PrivacyStillItemTransactionStatus::CategoryUnavailable,
        QStringLiteral("The category became unavailable"));

    const PrivacyCategoryOperationStatus operationStatus =
        sessions->runWithUnlockedSecret(
            categoryUuid,
            [this, &info, &categoryUuid, &acknowledgeAssetSet, &result,
             &runtime]
            (const PrivacyPassword& password)
            {
                PrivacyAssetInventoryBridgeRequest bridgeRequest;
                bridgeRequest.imageIds << info.id();
                bridgeRequest.configuredSidecarExtensions =
                    MetaEngineSettings::instance()->settings().sidecarExtensions;
                const PrivacyProtectPreflightResult preflight =
                    PrivacyProtectPreflight::build(bridgeRequest, runtime);

                if ((preflight.bridge.status != PrivacyInventoryStatus::Ready) ||
                    (preflight.bridge.items.size() != 1) ||
                    !preflight.bridge.items.constFirst().inventory.isReady())
                {
                    PrivacyProtectPreflight::discardNewlyCreatedRoots(preflight,
                                                                      runtime);
                    result = actionFailure(
                        PrivacyStillItemTransactionStatus::PreflightRejected,
                        QStringLiteral("The exact associated-file check did not pass"));
                    return;
                }

                const PrivacyAssetInventoryResult& inventory =
                    preflight.bridge.items.constFirst().inventory;
                const auto primaryAsset = std::find_if(
                    inventory.requiredAssets.cbegin(),
                    inventory.requiredAssets.cend(),
                    [](const PrivacyInventoryAsset& asset)
                    {
                        return ((asset.role ==
                                 PrivacyInventoryAssetRole::PrimaryMedia) &&
                                (asset.ordinal == 0));
                    });

                if ((primaryAsset == inventory.requiredAssets.cend()) ||
                    inventory.requiredAssets.isEmpty())
                {
                    PrivacyProtectPreflight::discardNewlyCreatedRoots(preflight,
                                                                      runtime);
                    result = actionFailure(
                        PrivacyStillItemTransactionStatus::PreflightRejected,
                        QStringLiteral(
                            "The associated-file set has no exact primary item"));
                    return;
                }

                const bool acknowledgementRequired =
                    (inventory.requiredAssets.size() > 1) ||
                    !inventory.exposureWarnings.isEmpty();

                if (acknowledgementRequired &&
                    (!acknowledgeAssetSet ||
                     !acknowledgeAssetSet(preflight)))
                {
                    PrivacyProtectPreflight::discardNewlyCreatedRoots(preflight,
                                                                      runtime);
                    result = actionFailure(
                        PrivacyStillItemTransactionStatus::AcknowledgementRequired,
                        QStringLiteral(
                            "The associated-file set or related copies were not acknowledged"));
                    return;
                }

                PrivacyRepositorySnapshot snapshot;

                if (!d->persistence.loadSnapshot(&snapshot))
                {
                    PrivacyProtectPreflight::discardNewlyCreatedRoots(preflight,
                                                                      runtime);
                    result = actionFailure(
                        PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral("The privacy catalogue could not be read"));
                    return;
                }

                const QString rootUuid = primaryAsset->location.root.uuid;
                const PrivacyStorageRoot* const root = rootForUuid(snapshot,
                                                                   rootUuid);
                PrivacyJournalRootExpectation expectation;

                if (!root ||
                    (runtime->rootState(root->uuid) !=
                     PrivacyRootRuntimeState::VerifiedAvailable) ||
                    !rootExpectation(*root, &expectation))
                {
                    PrivacyProtectPreflight::discardNewlyCreatedRoots(preflight,
                                                                      runtime);
                    result = actionFailure(
                        PrivacyStillItemTransactionStatus::RootUnavailable,
                        QStringLiteral("The collection root is not safely available"));
                    return;
                }

                PrivacyStillProtectRequest request;
                request.imageId = info.id();
                request.categoryUuid = categoryUuid;
                request.itemUuid = newUuid();
                request.containerUuid = newUuid();
                request.transactionUuid = newUuid();
                request.preflight = preflight;
                request.associatedAssetsAcknowledged = true;
                request.publicRoot = *root;
                request.rootExpectation = expectation;
                request.originalPixelSize = info.dimensions();
                request.originalCreationDate = info.dateTime();

                {
                    QMutexLocker locker(&d->transactionMutex);
                    result = d->engine.protect(request, password);
                }

                if (!result.succeeded())
                {
                    PrivacyProtectPreflight::discardNewlyCreatedRoots(preflight,
                                                                      runtime);
                }
            });

    if (operationStatus != PrivacyCategoryOperationStatus::Completed)
    {
        result = actionFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            (operationStatus == PrivacyCategoryOperationStatus::CategoryLocked)
                ? QStringLiteral("The category was locked before protection began")
                : QStringLiteral("Another category operation is already active"));
    }

    return result;
}

PrivacyStillItemTransactionResult
PrivacyThreadImageIOStillItemTransactionOwner::unprotect(
    const ItemInfo& info, const QString& passwordText)
{
    if (info.isNull() || (info.id() <= 0) ||
        ((info.category() != DatabaseItem::Image) &&
         (info.category() != DatabaseItem::Video)) ||
        !info.isLocationAvailable())
    {
        return actionFailure(PrivacyStillItemTransactionStatus::InvalidRequest,
                             QStringLiteral("Select one available protected photo or video"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;

    if (!currentActionComposition(&d->runtime, &runtime, &sessions))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                             QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::PersistenceFailure,
                             QStringLiteral("The privacy catalogue could not be read"));
    }

    const PrivacyItem* item = nullptr;

    for (const PrivacyItem& candidate : std::as_const(snapshot.items))
    {
        if (candidate.imageId == info.id())
        {
            if (item)
            {
                return actionFailure(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("The image has conflicting privacy mappings"));
            }

            item = &candidate;
        }
    }

    if (!item)
    {
        return actionFailure(PrivacyStillItemTransactionStatus::InvalidRequest,
                             QStringLiteral("The selected image is not protected"));
    }

    const PrivacyCategory* const category = categoryForUuid(snapshot,
                                                             item->categoryUuid);

    if (!category || (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                             QStringLiteral("The protecting Casual category is unavailable"));
    }

    const PrivacyAsset* primary = nullptr;

    for (const PrivacyAsset& asset : std::as_const(snapshot.assets))
    {
        if ((asset.itemUuid == item->uuid) &&
            (asset.role == PrivacyAsset::PrimaryMediaRole) &&
            (asset.ordinal == 0))
        {
            if (primary)
            {
                return actionFailure(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("The protected item has conflicting primary assets"));
            }

            primary = &asset;
        }
    }

    const PrivacyStorageRoot* const root = primary
                                         ? rootForUuid(snapshot,
                                                       primary->publicRootUuid)
                                         : nullptr;
    PrivacyJournalRootExpectation expectation;

    if (!root ||
        (runtime->rootState(root->uuid) !=
         PrivacyRootRuntimeState::VerifiedAvailable) ||
        !rootExpectation(*root, &expectation))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::RootUnavailable,
                             QStringLiteral("The collection root is not safely available"));
    }

    PrivacyStillItemTransactionResult result = actionFailure(
        PrivacyStillItemTransactionStatus::AuthenticationRequired,
        QStringLiteral("Fresh category authentication is required"));
    const PrivacyCategorySessionResult authentication =
        sessions->runWithFreshlyAuthenticatedSecret(
            category->uuid, passwordText,
            [this, &info, &category, &root, &expectation, &result]
            (const PrivacyPassword& password)
            {
                PrivacyStillUnprotectRequest request;
                request.imageId = info.id();
                request.categoryUuid = category->uuid;
                request.transactionUuid = newUuid();
                request.publicRoot = *root;
                request.rootExpectation = expectation;
                request.freshAuthenticationConfirmed = true;
                QMutexLocker locker(&d->transactionMutex);
                result = d->engine.unprotect(request, password);
            });

    if (!authentication.succeeded())
    {
        result = actionFailure(
            categorySessionTransactionStatus(authentication.status),
            categorySessionFailureDetail(authentication.status));
    }

    return result;
}

PrivacyStillItemTransactionResult
PrivacyThreadImageIOStillItemTransactionOwner::compatibilityUnlock(
    const ItemInfo& info, const QString& passwordText)
{
    if (info.isNull() || (info.id() <= 0) ||
        ((info.category() != DatabaseItem::Image) &&
         (info.category() != DatabaseItem::Video)) ||
        !info.isLocationAvailable())
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::InvalidRequest,
            QStringLiteral("Select one available protected photo or video"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;

    if (!currentActionComposition(&d->runtime, &runtime, &sessions))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::PersistenceFailure,
            QStringLiteral("The privacy catalogue could not be read"));
    }

    const PrivacyItem* item = nullptr;

    for (const PrivacyItem& candidate : std::as_const(snapshot.items))
    {
        if (candidate.imageId == info.id())
        {
            if (item)
            {
                return actionFailure(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("The image has conflicting privacy mappings"));
            }

            item = &candidate;
        }
    }

    const PrivacyCategory* const category = item
        ? categoryForUuid(snapshot, item->categoryUuid)
        : nullptr;
    const PrivacyAsset* primary = nullptr;
    int assetCount = 0;
    int activeTransactionCount = 0;

    if (item)
    {
        for (const PrivacyAsset& asset : std::as_const(snapshot.assets))
        {
            if (asset.itemUuid != item->uuid)
            {
                continue;
            }

            ++assetCount;

            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                if (primary)
                {
                    return actionFailure(
                        PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral(
                            "The protected item has conflicting primary assets"));
                }

                primary = &asset;
            }
        }

        for (const PrivacyTransaction& transaction :
             std::as_const(snapshot.transactions))
        {
            if (transaction.isActive() &&
                (transaction.itemUuid == item->uuid))
            {
                ++activeTransactionCount;
            }
        }
    }

    const PrivacyStorageRoot* const root = primary
        ? rootForUuid(snapshot, primary->publicRootUuid)
        : nullptr;
    PrivacyJournalRootExpectation expectation;

    if (!item || !category || !primary || (assetCount <= 0) ||
        (activeTransactionCount != 0) ||
        (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState !=
         PrivacyCategoryLifecycleState::Active) ||
        !root ||
        (runtime->rootState(root->uuid) !=
         PrivacyRootRuntimeState::VerifiedAvailable) ||
        !rootExpectation(*root, &expectation))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral(
                "The protected item is not available for Compatibility Unlock"));
    }

    if (!sessions->ownsSecret(category->uuid))
    {
        const PrivacyCategorySessionResult unlock =
            sessions->unlockCategory(category->uuid, passwordText);

        if (!unlock.succeeded())
        {
            return actionFailure(
                categorySessionTransactionStatus(unlock.status),
                categorySessionFailureDetail(unlock.status));
        }
    }

    PrivacyStillItemTransactionResult result = actionFailure(
        PrivacyStillItemTransactionStatus::CategoryUnavailable,
        QStringLiteral("The category became unavailable"));
    const QString itemUuid = item->uuid;
    const QString categoryUuid = category->uuid;
    const PrivacyStorageRoot publicRoot = *root;
    const PrivacyCategoryOperationStatus operationStatus =
        sessions->runWithUnlockedSecret(
            categoryUuid,
            [this, &info, &result, itemUuid, categoryUuid, publicRoot,
             expectation]
            (const PrivacyPassword& password)
            {
                PrivacyCompatibilityUnlockRequest request;
                request.imageId = info.id();
                request.categoryUuid = categoryUuid;
                request.itemUuid = itemUuid;
                request.transactionUuid = newUuid();
                request.groupUuid = newUuid();
                request.publicRoot = publicRoot;
                request.rootExpectation = expectation;
                QMutexLocker locker(&d->transactionMutex);
                result = d->engine.compatibilityUnlock(request, password);
            });

    if (operationStatus != PrivacyCategoryOperationStatus::Completed)
    {
        result = actionFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            (operationStatus == PrivacyCategoryOperationStatus::CategoryLocked)
                ? QStringLiteral(
                      "The category was locked before Compatibility Unlock began")
                : QStringLiteral(
                      "Another category operation is already active"));
    }

    return result;
}

PrivacyStillItemTransactionResult
PrivacyThreadImageIOStillItemTransactionOwner::compatibilityRelock(
    const ItemInfo& info, const QString& unlockTransactionUuid)
{
    if (info.isNull() || (info.id() <= 0) ||
        unlockTransactionUuid.isEmpty())
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::InvalidRequest,
            QStringLiteral("The Compatibility Relock request is invalid"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;

    if (!currentActionComposition(&d->runtime, &runtime))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::PersistenceFailure,
            QStringLiteral("The privacy catalogue could not be read"));
    }

    const PrivacyItem* item = nullptr;
    const PrivacyTransaction* unlock = nullptr;
    const PrivacyAsset* primary = nullptr;
    int activeTransactionCount = 0;

    for (const PrivacyItem& candidate : std::as_const(snapshot.items))
    {
        if (candidate.imageId == info.id())
        {
            if (item)
            {
                return actionFailure(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral("The image has conflicting privacy mappings"));
            }

            item = &candidate;
        }
    }

    if (item)
    {
        for (const PrivacyTransaction& transaction :
             std::as_const(snapshot.transactions))
        {
            if (transaction.isActive() &&
                (transaction.itemUuid == item->uuid))
            {
                ++activeTransactionCount;

                if (transaction.uuid == unlockTransactionUuid)
                {
                    unlock = &transaction;
                }
            }
        }

        for (const PrivacyAsset& asset : std::as_const(snapshot.assets))
        {
            if ((asset.itemUuid == item->uuid) &&
                (asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                if (primary)
                {
                    return actionFailure(
                        PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral(
                            "The protected item has conflicting primary assets"));
                }

                primary = &asset;
            }
        }
    }

    const PrivacyCategory* const category = item
        ? categoryForUuid(snapshot, item->categoryUuid)
        : nullptr;
    const PrivacyStorageRoot* const root = primary
        ? rootForUuid(snapshot, primary->publicRootUuid)
        : nullptr;
    PrivacyJournalRootExpectation expectation;

    if (!item || !category || !unlock || !primary ||
        (activeTransactionCount != 1) ||
        (unlock->type != PrivacyTransactionType::CompatibilityUnlock) ||
        (unlock->state != PrivacyTransactionState::Exposed) ||
        (unlock->categoryUuid != category->uuid) ||
        (category->backend != PrivacyBackend::Casual) || !root ||
        (runtime->rootState(root->uuid) !=
         PrivacyRootRuntimeState::VerifiedAvailable) ||
        !rootExpectation(*root, &expectation))
    {
        return actionFailure(
            PrivacyStillItemTransactionStatus::RecoveryRequired,
            QStringLiteral(
                "The exact Compatibility Unlock exposure is unavailable"));
    }

    QMutexLocker locker(&d->transactionMutex);
    PrivacyCompatibilityExposureGuardEngine::relock(
        *root, expectation, unlock->uuid);
    return d->engine.recover(*root, unlock->uuid);
}

PrivacyCompatibilityBatchResult
PrivacyThreadImageIOStillItemTransactionOwner::compatibilityUnlockCategory(
    const QString& categoryUuid, const QString& passwordText,
    const CompatibilityProgress& progress)
{
    if (QUuid(categoryUuid).toString(QUuid::WithoutBraces) != categoryUuid)
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::InvalidRequest,
            QStringLiteral("The privacy category identifier is invalid"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;

    if (!currentActionComposition(&d->runtime, &runtime, &sessions))
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::PersistenceFailure,
            QStringLiteral("The privacy catalogue could not be read"));
    }

    const PrivacyCategory* category = nullptr;
    int categoryCount = 0;
    QList<PrivacyItem> items;

    for (const PrivacyCategory& candidate : std::as_const(snapshot.categories))
    {
        if (candidate.uuid == categoryUuid)
        {
            category = &candidate;
            ++categoryCount;
        }
    }

    for (const PrivacyItem& item : std::as_const(snapshot.items))
    {
        if (item.categoryUuid == categoryUuid)
        {
            items.append(item);
        }
    }

    if ((categoryCount != 1) || !category || items.isEmpty() ||
        (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral(
                "The category is not available for Compatibility Unlock"),
            items.size());
    }

    for (const PrivacyTransaction& transaction :
         std::as_const(snapshot.transactions))
    {
        if (transaction.isActive() &&
            (transaction.categoryUuid == categoryUuid))
        {
            return batchFailure(
                PrivacyStillItemTransactionStatus::RecoveryRequired,
                QStringLiteral(
                    "The category already has an operation requiring attention"),
                items.size());
        }
    }

    std::sort(items.begin(), items.end(),
              [](const PrivacyItem& left, const PrivacyItem& right)
              {
                  return (left.uuid < right.uuid);
              });
    const QString groupUuid = newUuid();
    QList<PrivacyCompatibilityUnlockRequest> requests;

    for (const PrivacyItem& item : std::as_const(items))
    {
        if (item.imageId <= 0)
        {
            return batchFailure(
                PrivacyStillItemTransactionStatus::PersistenceFailure,
                QStringLiteral("A protected item has no catalogue identity"),
                items.size());
        }

        const PrivacyAsset* primary = nullptr;
        QString publicRootUuid;
        int assetCount = 0;

        for (const PrivacyAsset& asset : std::as_const(snapshot.assets))
        {
            if (asset.itemUuid != item.uuid)
            {
                continue;
            }

            ++assetCount;

            if (publicRootUuid.isEmpty())
            {
                publicRootUuid = asset.publicRootUuid;
            }
            else if (publicRootUuid != asset.publicRootUuid)
            {
                return batchFailure(
                    PrivacyStillItemTransactionStatus::PersistenceFailure,
                    QStringLiteral(
                        "A protected asset set spans conflicting public roots"),
                    items.size());
            }

            if ((asset.role == PrivacyAsset::PrimaryMediaRole) &&
                (asset.ordinal == 0))
            {
                if (primary)
                {
                    return batchFailure(
                        PrivacyStillItemTransactionStatus::PersistenceFailure,
                        QStringLiteral(
                            "A protected item has conflicting primary assets"),
                        items.size());
                }

                primary = &asset;
            }
        }

        const PrivacyStorageRoot* const root = primary
            ? rootForUuid(snapshot, primary->publicRootUuid)
            : nullptr;
        PrivacyJournalRootExpectation expectation;

        if (!primary || (assetCount <= 0) || !root ||
            (runtime->rootState(root->uuid) !=
             PrivacyRootRuntimeState::VerifiedAvailable) ||
            !rootExpectation(*root, &expectation))
        {
            return batchFailure(
                PrivacyStillItemTransactionStatus::RootUnavailable,
                QStringLiteral(
                    "Every collection root must be safely available before "
                    "category Compatibility Unlock begins"),
                items.size());
        }

        PrivacyCompatibilityUnlockRequest request;
        request.imageId = item.imageId;
        request.categoryUuid = categoryUuid;
        request.itemUuid = item.uuid;
        request.transactionUuid = newUuid();
        request.groupUuid = groupUuid;
        request.publicRoot = *root;
        request.rootExpectation = expectation;
        requests.append(request);
    }

    if (!sessions->ownsSecret(categoryUuid))
    {
        const PrivacyCategorySessionResult unlocked =
            sessions->unlockCategory(categoryUuid, passwordText);

        if (!unlocked.succeeded())
        {
            return batchFailure(
                categorySessionTransactionStatus(unlocked.status),
                categorySessionFailureDetail(unlocked.status), requests.size());
        }
    }

    PrivacyCompatibilityBatchResult result = batchFailure(
        PrivacyStillItemTransactionStatus::CategoryUnavailable,
        QStringLiteral("The category became unavailable"), requests.size());
    const PrivacyCategoryOperationStatus operationStatus =
        sessions->runWithUnlockedSecret(
            categoryUuid,
            [this, &requests, &result, &progress]
            (const PrivacyPassword& password)
            {
                QMutexLocker locker(&d->transactionMutex);
                result = d->engine.compatibilityUnlockBatch(
                    requests, password, progress);
            });

    if (operationStatus != PrivacyCategoryOperationStatus::Completed)
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            (operationStatus == PrivacyCategoryOperationStatus::CategoryLocked)
                ? QStringLiteral(
                      "The category was locked before Compatibility Unlock began")
                : QStringLiteral(
                      "Another category operation is already active"),
            requests.size());
    }

    return result;
}

PrivacyCompatibilityBatchResult
PrivacyThreadImageIOStillItemTransactionOwner::compatibilityRelockCategory(
    const QString& categoryUuid, const CompatibilityProgress& progress) const
{
    if (QUuid(categoryUuid).toString(QUuid::WithoutBraces) != categoryUuid)
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::InvalidRequest,
            QStringLiteral("The privacy category identifier is invalid"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;

    if (!currentActionComposition(&d->runtime, &runtime))
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::CategoryUnavailable,
            QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::PersistenceFailure,
            QStringLiteral("The privacy catalogue could not be read"));
    }

    QList<PrivacyTransaction> transactions;

    for (const PrivacyTransaction& transaction :
         std::as_const(snapshot.transactions))
    {
        if (transaction.isActive() &&
            (transaction.categoryUuid == categoryUuid) &&
            (transaction.type == PrivacyTransactionType::CompatibilityUnlock))
        {
            transactions.append(transaction);
        }
    }

    std::sort(transactions.begin(), transactions.end(),
              [](const PrivacyTransaction& left,
                 const PrivacyTransaction& right)
              {
                  return (left.uuid < right.uuid);
              });

    if (transactions.isEmpty())
    {
        PrivacyCompatibilityBatchResult complete;
        complete.status =
            PrivacyStillItemTransactionStatus::CompatibilityRelocked;
        return complete;
    }

    QList<PrivacyCompatibilityRelockRequest> requests;
    QList<PrivacyStillItemTransactionResult> unavailable;
    bool reconciliationRequired = false;

    for (const PrivacyTransaction& transaction : std::as_const(transactions))
    {
        const PrivacyTransactionJournal* journal = nullptr;
        int journalCount = 0;

        for (const PrivacyTransactionJournal& candidate :
             std::as_const(snapshot.transactionJournals))
        {
            if (candidate.transactionUuid == transaction.uuid)
            {
                journal = &candidate;
                ++journalCount;
            }
        }

        const PrivacyStorageRoot* const root =
            ((journalCount == 1) && journal)
                ? rootForUuid(snapshot, journal->rootUuid)
                : nullptr;
        PrivacyJournalRootExpectation expectation;

        if (!root ||
            (runtime->rootState(root->uuid) !=
             PrivacyRootRuntimeState::VerifiedAvailable) ||
            !rootExpectation(*root, &expectation))
        {
            PrivacyStillItemTransactionResult item = actionFailure(
                PrivacyStillItemTransactionStatus::RootUnavailable,
                QStringLiteral(
                    "The collection root remains offline or unverified"));
            item.transactionUuid = transaction.uuid;
            item.itemUuid = transaction.itemUuid;
            unavailable.append(item);
            reconciliationRequired = reconciliationRequired ||
                (transaction.state ==
                 PrivacyTransactionState::NeedsReconciliation);
            continue;
        }

        PrivacyCompatibilityRelockRequest request;
        request.transactionUuid = transaction.uuid;
        request.publicRoot = *root;
        request.rootExpectation = expectation;
        requests.append(request);
    }

    PrivacyCompatibilityBatchResult available;

    if (!requests.isEmpty())
    {
        CompatibilityProgress availableProgress;

        if (progress)
        {
            availableProgress =
                [progress, total = transactions.size()](int completed, int)
                {
                    progress(completed, total);
                };
        }

        QMutexLocker locker(&d->transactionMutex);
        available = d->engine.compatibilityRelockBatch(
            requests, availableProgress);
    }

    PrivacyCompatibilityBatchResult result;
    result.requestedCount = transactions.size();
    result.processedCount = available.processedCount + unavailable.size();
    result.remainingExposureCount = available.remainingExposureCount +
                                    unavailable.size();
    result.itemResults = available.itemResults;
    result.itemResults.append(unavailable);

    if (progress && !unavailable.isEmpty())
    {
        progress(result.processedCount, result.requestedCount);
    }

    if (result.remainingExposureCount == 0)
    {
        result.status =
            PrivacyStillItemTransactionStatus::CompatibilityRelocked;
    }
    else
    {
        result.status = reconciliationRequired ||
            (available.status ==
             PrivacyStillItemTransactionStatus::ReconciliationRequired)
                ? PrivacyStillItemTransactionStatus::ReconciliationRequired
                : PrivacyStillItemTransactionStatus::RecoveryRequired;
        result.detail = QStringLiteral(
            "%1 of %2 Compatibility exposure(s) remain targeted; reconnect "
            "offline storage and try again")
                            .arg(result.remainingExposureCount)
                            .arg(result.requestedCount);
    }

    return result;
}

PrivacyCompatibilityBatchResult
PrivacyThreadImageIOStillItemTransactionOwner::compatibilityRelockAll(
    const CompatibilityProgress& progress) const
{
    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return batchFailure(
            PrivacyStillItemTransactionStatus::PersistenceFailure,
            QStringLiteral("The privacy catalogue could not be read"));
    }

    QSet<QString> categoryUuids;
    int total = 0;

    for (const PrivacyTransaction& transaction :
         std::as_const(snapshot.transactions))
    {
        if (transaction.isActive() &&
            (transaction.type == PrivacyTransactionType::CompatibilityUnlock))
        {
            categoryUuids.insert(transaction.categoryUuid);
            ++total;
        }
    }

    PrivacyCompatibilityBatchResult aggregate;
    aggregate.requestedCount = total;

    if (total == 0)
    {
        aggregate.status =
            PrivacyStillItemTransactionStatus::CompatibilityRelocked;
        return aggregate;
    }

    QStringList orderedCategories = categoryUuids.values();
    std::sort(orderedCategories.begin(), orderedCategories.end());
    bool reconciliationRequired = false;

    for (const QString& categoryUuid : std::as_const(orderedCategories))
    {
        CompatibilityProgress categoryProgress;

        if (progress)
        {
            categoryProgress =
                [progress, offset = aggregate.processedCount, total](int completed,
                                                                    int)
                {
                    progress(offset + completed, total);
                };
        }

        const PrivacyCompatibilityBatchResult category =
            compatibilityRelockCategory(categoryUuid, categoryProgress);
        aggregate.processedCount += category.processedCount;
        aggregate.remainingExposureCount += category.remainingExposureCount;
        aggregate.itemResults.append(category.itemResults);
        reconciliationRequired = reconciliationRequired ||
            (category.status ==
             PrivacyStillItemTransactionStatus::ReconciliationRequired);
    }

    if (aggregate.remainingExposureCount == 0)
    {
        aggregate.status =
            PrivacyStillItemTransactionStatus::CompatibilityRelocked;
    }
    else
    {
        aggregate.status = reconciliationRequired
            ? PrivacyStillItemTransactionStatus::ReconciliationRequired
            : PrivacyStillItemTransactionStatus::RecoveryRequired;
        aggregate.detail = QStringLiteral(
            "%1 of %2 Compatibility exposure(s) remain targeted after shutdown "
            "recovery")
                               .arg(aggregate.remainingExposureCount)
                               .arg(aggregate.requestedCount);
    }

    return aggregate;
}

bool PrivacyThreadImageIOStillItemTransactionOwner::prepareForShutdown() const
{
    return compatibilityRelockAll().succeeded();
}

PrivacyStillItemTransactionResult
PrivacyThreadImageIOStillItemTransactionOwner::resume(
    qlonglong imageId, const QString& transactionUuid,
    const QString& passwordText)
{
    if ((imageId <= 0) || transactionUuid.isEmpty())
    {
        return actionFailure(PrivacyStillItemTransactionStatus::InvalidRequest,
                             QStringLiteral("The recovery request is invalid"));
    }

    QSharedPointer<PrivacyRuntimeCoordinator> runtime;
    QSharedPointer<PrivacyCategorySessionOwner> sessions;

    if (!currentActionComposition(&d->runtime, &runtime, &sessions))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::CategoryUnavailable,
                             QStringLiteral("Privacy startup is still changing"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->persistence.loadSnapshot(&snapshot))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::PersistenceFailure,
                             QStringLiteral("The privacy catalogue could not be read"));
    }

    const PrivacyTransaction* transaction = nullptr;
    const PrivacyItem* item = nullptr;
    const PrivacyTransactionJournal* journal = nullptr;
    int transactionCount = 0;
    int itemCount = 0;
    int journalCount = 0;

    for (const PrivacyTransaction& candidate :
         std::as_const(snapshot.transactions))
    {
        if (candidate.uuid == transactionUuid)
        {
            ++transactionCount;
            transaction = &candidate;
        }
    }

    if (!transaction || (transactionCount != 1) ||
        !transaction->isActive() ||
        (transaction->state != PrivacyTransactionState::Created) ||
        ((transaction->type != PrivacyTransactionType::ProtectItem) &&
         (transaction->type != PrivacyTransactionType::UnprotectItem)))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::RecoveryRequired,
                             QStringLiteral("The exact Created transaction is unavailable"));
    }

    for (const PrivacyItem& candidate : std::as_const(snapshot.items))
    {
        if ((candidate.uuid == transaction->itemUuid) ||
            (candidate.imageId == imageId))
        {
            ++itemCount;

            if ((candidate.uuid == transaction->itemUuid) &&
                (candidate.imageId == imageId))
            {
                item = &candidate;
            }
        }
    }

    for (const PrivacyTransactionJournal& candidate :
         std::as_const(snapshot.transactionJournals))
    {
        if (candidate.transactionUuid == transactionUuid)
        {
            ++journalCount;
            journal = &candidate;
        }
    }

    const PrivacyCategory* const category = categoryForUuid(
        snapshot, transaction->categoryUuid);
    const PrivacyStorageRoot* const root = (journalCount == 1) && journal
                                         ? rootForUuid(snapshot,
                                                       journal->rootUuid)
                                         : nullptr;

    if (!item || (itemCount != 1) || (journalCount != 1) || !category || !root ||
        (item->categoryUuid != category->uuid) ||
        (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (root->kind != PrivacyStorageRootKind::AlbumRoot) ||
        (runtime->rootState(root->uuid) != PrivacyRootRuntimeState::Recovering))
    {
        return actionFailure(PrivacyStillItemTransactionStatus::RecoveryRequired,
                             QStringLiteral(
                                 "The recovery state changed; restart recovery is required"));
    }

    PrivacyStillItemTransactionResult result = actionFailure(
        PrivacyStillItemTransactionStatus::RecoveryRequired,
        QStringLiteral("The transaction was not resumed"));
    const auto resumeWithPassword =
        [this, &runtime, &root, &transaction, &result]
        (const PrivacyPassword& password, bool freshAuthenticationConfirmed)
        {
            QMutexLocker locker(&d->transactionMutex);

            if (!d->authenticatedTransactionUuid.isEmpty() ||
                d->authenticatedPassword || d->authenticatedResult)
            {
                result = actionFailure(
                    PrivacyStillItemTransactionStatus::CategoryUnavailable,
                    QStringLiteral("Another recovery operation is active"));
                return;
            }

            d->authenticatedTransactionUuid = transaction->uuid;
            d->authenticatedPassword = &password;
            d->freshAuthenticationConfirmed = freshAuthenticationConfirmed;
            d->authenticatedResult = &result;
            const ScopeExit clearAuthenticatedContext([this]()
            {
                d->authenticatedTransactionUuid.clear();
                d->authenticatedPassword = nullptr;
                d->freshAuthenticationConfirmed = false;
                d->authenticatedResult = nullptr;
            });

            const PrivacyRootRecoveryResult recovery =
                runtime->recoverRoot(root->uuid);

            if (recovery == PrivacyRootRecoveryResult::PublishedOffline)
            {
                result = actionFailure(
                    PrivacyStillItemTransactionStatus::RootUnavailable,
                    QStringLiteral("The collection storage is offline"));
            }
            else if (recovery ==
                     PrivacyRootRecoveryResult::PublishedIdentityMismatch)
            {
                result = actionFailure(
                    PrivacyStillItemTransactionStatus::RootUnavailable,
                    QStringLiteral(
                        "The collection storage identity no longer matches"));
            }
            else if (result.succeeded() &&
                (recovery != PrivacyRootRecoveryResult::PublishedVerified))
            {
                result = actionFailure(
                    PrivacyStillItemTransactionStatus::RecoveryRequired,
                    QStringLiteral(
                        "The durable operation completed but runtime recovery must be retried"));
            }
        };

    if ((transaction->type == PrivacyTransactionType::ProtectItem) &&
        sessions->ownsSecret(category->uuid))
    {
        const PrivacyCategoryOperationStatus operation =
            sessions->runWithUnlockedSecret(
                category->uuid,
                [&resumeWithPassword](const PrivacyPassword& password)
                {
                    resumeWithPassword(password, false);
                });

        if (operation != PrivacyCategoryOperationStatus::Completed)
        {
            result = actionFailure(
                PrivacyStillItemTransactionStatus::AuthenticationRequired,
                QStringLiteral("The category must be authenticated again"));
        }

        return result;
    }

    const bool freshAuthenticationRequired =
        (transaction->type == PrivacyTransactionType::UnprotectItem);
    const PrivacyCategorySessionResult authentication =
        sessions->runWithFreshlyAuthenticatedSecret(
            category->uuid, passwordText,
            [&resumeWithPassword, freshAuthenticationRequired]
            (const PrivacyPassword& password)
            {
                resumeWithPassword(password, freshAuthenticationRequired);
            },
            transaction->uuid);

    if (!authentication.succeeded())
    {
        result = actionFailure(
            categorySessionTransactionStatus(authentication.status),
            categorySessionFailureDetail(authentication.status));
    }

    return result;
}

PrivacyRecoveryDisposition
PrivacyThreadImageIOStillItemTransactionOwner::recoverRoot(
    const PrivacyStorageRoot& root,
    const PrivacyTransaction& transaction,
    const QList<PrivacyTransactionJournal>& journals) const
{
    if ((root.kind != PrivacyStorageRootKind::AlbumRoot) ||
        ((transaction.type != PrivacyTransactionType::ProtectItem) &&
         (transaction.type != PrivacyTransactionType::UnprotectItem) &&
         (transaction.type != PrivacyTransactionType::CompatibilityUnlock)) ||
        (journals.size() != 1) ||
        (journals.constFirst().transactionUuid != transaction.uuid) ||
        (journals.constFirst().rootUuid != root.uuid))
    {
        return PrivacyRecoveryDisposition::Deferred;
    }

    QMutexLocker locker(&d->transactionMutex);
    PrivacyStillItemTransactionResult result;

    if ((d->authenticatedTransactionUuid == transaction.uuid) &&
        d->authenticatedPassword && d->authenticatedResult)
    {
        result = d->engine.resumeAuthenticated(
            root, transaction.uuid, *d->authenticatedPassword,
            d->freshAuthenticationConfirmed);
        *d->authenticatedResult = result;
    }
    else
    {
        result = d->engine.recover(root, transaction.uuid);
    }

    if (result.succeeded())
    {
        return PrivacyRecoveryDisposition::Recovered;
    }

    switch (result.status)
    {
        case PrivacyStillItemTransactionStatus::AuthenticationRequired:
        case PrivacyStillItemTransactionStatus::CacheTransitionFailure:
        case PrivacyStillItemTransactionStatus::CleanupPending:
        case PrivacyStillItemTransactionStatus::ReconciliationRequired:
        {
            return PrivacyRecoveryDisposition::Deferred;
        }

        default:
        {
            return PrivacyRecoveryDisposition::Failed;
        }
    }
}

bool PrivacyThreadImageIOStillItemTransactionOwner::loadReconciledSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadRuntimeSnapshot(snapshot);
}

} // namespace Digikam

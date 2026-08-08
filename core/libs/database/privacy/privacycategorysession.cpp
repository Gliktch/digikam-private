/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycategorysession.h"

// Qt includes

#include <QCryptographicHash>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QUuid>

// C++ includes

#include <utility>

// Local includes

#include "privacyrepository.h"

namespace Digikam
{

namespace
{

QString normalizedUuid(const QString& value)
{
    const QUuid uuid(value);

    return uuid.isNull() ? QString() : uuid.toString(QUuid::WithoutBraces);
}

QString storeRelativePath(const QString& storeUuid)
{
    return QLatin1String(".digikam-private/stores/") + storeUuid;
}

QString temporaryStoreRelativePath(const QString& storeUuid)
{
    return QLatin1String(".digikam-private/staging/") + storeUuid +
           QLatin1String(".creating");
}

QByteArray sentinelBytes(const QString& categoryUuid, const QString& storeUuid)
{
    QJsonObject object;
    object.insert(QLatin1String("categoryUuid"), categoryUuid);
    object.insert(QLatin1String("formatVersion"), 1);
    object.insert(QLatin1String("kind"), QLatin1String("digikam-private-store-sentinel-v1"));
    object.insert(QLatin1String("storeUuid"), storeUuid);

    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QString sha256(const QByteArray& bytes)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

bool sameCreatingRecords(const PrivacyRepositorySnapshot& snapshot,
                         const PrivacyCategory& category,
                         const PrivacyStorageRoot& root,
                         const PrivacyStore& store,
                         const PrivacyTransaction& transaction)
{
    int categoryMatches = 0;
    int rootMatches = 0;
    int storeMatches = 0;
    int transactionMatches = 0;

    for (const PrivacyCategory& existing : snapshot.categories)
    {
        if (existing.name.compare(category.name, Qt::CaseInsensitive) == 0)
        {
            if ((existing.uuid != category.uuid) ||
                (existing.lifecycleState != PrivacyCategoryLifecycleState::Creating))
            {
                return false;
            }
        }

        if (existing.uuid == category.uuid)
        {
            ++categoryMatches;

            if ((existing.name != category.name) || (existing.backend != category.backend) ||
                (existing.presentationMode != category.presentationMode) ||
                (existing.unlockedThumbnailMode != category.unlockedThumbnailMode) ||
                (existing.tagVisibilityMode != category.tagVisibilityMode) ||
                (existing.lifecycleState != PrivacyCategoryLifecycleState::Creating) ||
                (existing.currentCredentialGeneration != 0))
            {
                return false;
            }
        }
    }

    for (const PrivacyStorageRoot& existing : snapshot.storageRoots)
    {
        if (existing.uuid == root.uuid)
        {
            ++rootMatches;

            if ((existing.kind != root.kind) ||
                (existing.configuredPath != root.configuredPath) ||
                (existing.identityVersion != root.identityVersion) ||
                (existing.identityData != root.identityData) ||
                (existing.markerUuid != root.markerUuid))
            {
                return false;
            }
        }
    }

    for (const PrivacyStore& existing : snapshot.stores)
    {
        if (existing.uuid == store.uuid)
        {
            ++storeMatches;

            if ((existing.categoryUuid != store.categoryUuid) ||
                (existing.rootUuid != store.rootUuid) ||
                (existing.cipherRelativePath != store.cipherRelativePath) ||
                (existing.lifecycleState != PrivacyStoreLifecycleState::Creating) ||
                (existing.configGeneration != -1))
            {
                return false;
            }
        }
    }

    for (const PrivacyTransaction& existing : snapshot.transactions)
    {
        if (existing.uuid == transaction.uuid)
        {
            ++transactionMatches;

            if ((existing.categoryUuid != transaction.categoryUuid) ||
                (existing.type != PrivacyTransactionType::CreateCategory) ||
                (existing.state != PrivacyTransactionState::Created) ||
                (existing.generation != 0))
            {
                return false;
            }
        }
    }

    return ((categoryMatches == 1) && (rootMatches == 1) &&
            (storeMatches == 1) && (transactionMatches == 1));
}

class CategoryBundle
{
public:

    PrivacyCategory category;
    PrivacyCredential credential;
    PrivacyStorageRoot root;
    PrivacyStore store;
};

bool loadActiveBundle(const PrivacyRepositorySnapshot& snapshot,
                      const QString& categoryUuid,
                      CategoryBundle* bundle,
                      PrivacyCategorySessionStatus* failure)
{
    if (!bundle || !failure)
    {
        return false;
    }

    int categoryCount = 0;

    for (const PrivacyCategory& category : snapshot.categories)
    {
        if (category.uuid == categoryUuid)
        {
            ++categoryCount;
            bundle->category = category;
        }
    }

    if ((categoryCount != 1) ||
        (bundle->category.lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        *failure = PrivacyCategorySessionStatus::CategoryNotActive;
        return false;
    }

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if ((transaction.categoryUuid == categoryUuid) && transaction.isActive())
        {
            *failure = PrivacyCategorySessionStatus::TransactionBlocked;
            return false;
        }
    }

    int credentialCount = 0;

    for (const PrivacyCredential& credential : snapshot.credentials)
    {
        if ((credential.categoryUuid == categoryUuid) &&
            (credential.generation == bundle->category.currentCredentialGeneration))
        {
            ++credentialCount;
            bundle->credential = credential;
        }
    }

    QString authorityStoreUuid;
    QString derivativeStoreUuid;
    QString originalStoreUuid;

    for (const PrivacyStoreBinding& binding : snapshot.storeBindings)
    {
        if (binding.categoryUuid != categoryUuid)
        {
            continue;
        }

        if (binding.role == PrivacyStoreRole::CredentialAuthority)
        {
            authorityStoreUuid = binding.storeUuid;
        }
        else if (binding.role == PrivacyStoreRole::Derivatives)
        {
            derivativeStoreUuid = binding.storeUuid;
        }
        else if (binding.role == PrivacyStoreRole::Originals)
        {
            originalStoreUuid = binding.storeUuid;
        }
    }

    const bool rolesValid = !authorityStoreUuid.isEmpty() &&
                            (authorityStoreUuid == derivativeStoreUuid) &&
                            (((bundle->category.backend == PrivacyBackend::Casual) &&
                              originalStoreUuid.isEmpty()) ||
                             ((bundle->category.backend == PrivacyBackend::Strong) &&
                              (authorityStoreUuid == originalStoreUuid)));
    int storeCount = 0;

    for (const PrivacyStore& store : snapshot.stores)
    {
        if (store.uuid == authorityStoreUuid)
        {
            ++storeCount;
            bundle->store = store;
        }
    }

    int rootCount = 0;

    for (const PrivacyStorageRoot& root : snapshot.storageRoots)
    {
        if (root.uuid == bundle->store.rootUuid)
        {
            ++rootCount;
            bundle->root = root;
        }
    }

    if ((credentialCount != 1) || !bundle->credential.isValid() || !rolesValid ||
        (storeCount != 1) || !bundle->store.isValid() ||
        (bundle->store.lifecycleState != PrivacyStoreLifecycleState::Active) ||
        (bundle->store.configGeneration != bundle->credential.generation) ||
        (rootCount != 1) || !bundle->root.isValid())
    {
        *failure = PrivacyCategorySessionStatus::CategoryNotActive;
        return false;
    }

    return true;
}

} // namespace

bool PrivacyCategorySessionResult::succeeded() const
{
    return ((status == PrivacyCategorySessionStatus::Created) ||
            (status == PrivacyCategorySessionStatus::Unlocked) ||
            (status == PrivacyCategorySessionStatus::UnlockedStoreOffline) ||
            (status == PrivacyCategorySessionStatus::Locked) ||
            (status == PrivacyCategorySessionStatus::AlreadyUnlocked) ||
            (status == PrivacyCategorySessionStatus::AlreadyLocked));
}

bool PrivacyCategorySessionResult::recoveryRequired() const
{
    return ((status == PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired) ||
            (status == PrivacyCategorySessionStatus::StrongRecoveryRequired));
}

bool PrivacyCoreDbCategorySessionRepository::loadSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadSnapshot(snapshot);
}

bool PrivacyCoreDbCategorySessionRepository::beginCreation(
    const PrivacyCategory& category, const PrivacyStorageRoot& root,
    const PrivacyStore& store, const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginCategoryCreation(category, root, store,
                                                      transaction, journal);
}

bool PrivacyCoreDbCategorySessionRepository::publishCreation(
    const PrivacyCategory& category, const PrivacyCredential& credential,
    const PrivacyStore& store, const QList<PrivacyStoreBinding>& bindings,
    const PrivacyTransaction& transaction)
{
    return PrivacyRepository().publishCategoryCreation(category, credential, store,
                                                        bindings, transaction);
}

class PrivacyCategorySessionCoordinator::Private
{
public:

    enum class OperationKind
    {
        Create,
        Unlock,
        Lock
    };

    struct Operation
    {
        OperationKind kind = OperationKind::Unlock;
        quint64 token = 0;
        bool cancelRequested = false;
    };

    class Session
    {
    public:

        Session(QString uuid, PrivacyPassword&& normalizedPassword,
                std::unique_ptr<PrivacyCategoryStoreLease>&& storeLease)
            : categoryUuid(std::move(uuid)),
              password(std::make_unique<PrivacyPassword>(
                           std::move(normalizedPassword))),
              lease(std::move(storeLease))
        {
        }

        QString categoryUuid;
        std::unique_ptr<PrivacyPassword> password;
        std::unique_ptr<PrivacyCategoryStoreLease> lease;
    };

    Private(PrivacyCategorySessionRepository& sessionRepository,
            PrivacyCategoryStoreBackend& categoryStoreBackend,
            const PrivacyRootVerifier& categoryRootVerifier,
            PrivacyRuntimeCoordinator& privacyRuntime,
            PrivacySecretLifetimeObserver* lifetimeObserver)
        : repository(sessionRepository),
          storeBackend(categoryStoreBackend),
          rootVerifier(categoryRootVerifier),
          runtime(privacyRuntime),
          observer(lifetimeObserver)
    {
    }

    quint64 beginOperation(const QString& categoryUuid, OperationKind kind)
    {
        if ((lockAllInProgress && (kind != OperationKind::Lock)) ||
            operations.contains(categoryUuid))
        {
            return 0;
        }

        quint64 token = ++nextOperationToken;

        if (token == 0)
        {
            token = ++nextOperationToken;
        }

        Operation operation;
        operation.kind = kind;
        operation.token = token;
        operations.insert(categoryUuid, operation);

        return token;
    }

    bool operationCanceled(const QString& categoryUuid, quint64 token) const
    {
        const auto it = operations.constFind(categoryUuid);

        return ((it == operations.constEnd()) || (it->token != token) ||
                it->cancelRequested || lockAllInProgress);
    }

    void finishOperation(const QString& categoryUuid, quint64 token)
    {
        const auto it = operations.find(categoryUuid);

        if ((it != operations.end()) && (it->token == token))
        {
            operations.erase(it);
            operationChanged.wakeAll();
        }
    }

    PrivacyCategorySessionRepository& repository;
    PrivacyCategoryStoreBackend& storeBackend;
    const PrivacyRootVerifier& rootVerifier;
    PrivacyRuntimeCoordinator& runtime;
    PrivacySecretLifetimeObserver* observer = nullptr;
    mutable QMutex lock;
    QWaitCondition operationChanged;
    QHash<QString, std::shared_ptr<Session> > sessions;
    QHash<QString, Operation> operations;
    quint64 nextOperationToken = 0;
    bool lockAllInProgress = false;
};

PrivacyCategorySessionCoordinator::PrivacyCategorySessionCoordinator(
    PrivacyCategorySessionRepository& repository,
    PrivacyCategoryStoreBackend& storeBackend,
    const PrivacyRootVerifier& rootVerifier,
    PrivacyRuntimeCoordinator& runtime,
    PrivacySecretLifetimeObserver* secretObserver)
    : d(std::make_unique<Private>(repository, storeBackend, rootVerifier,
                                  runtime, secretObserver))
{
}

PrivacyCategorySessionCoordinator::~PrivacyCategorySessionCoordinator()
{
    lockAllCategories();

    // A backend can report that a managed-store unmount failed. Normal lock
    // calls retain that session so the caller can retry without losing track
    // of the live lease or its authentication secret. Destruction has no
    // retrying owner, so force a fail-closed local teardown: publish locked,
    // detach all remaining sessions, destroy their leases and passwords, and
    // only then report that each secret was released. Lease/password
    // destruction and callbacks deliberately happen outside the mutex.
    QList<std::shared_ptr<Private::Session> > remainingSessions;

    {
        QMutexLocker locker(&d->lock);
        remainingSessions = d->sessions.values();
        d->sessions.clear();
    }

    for (const std::shared_ptr<Private::Session>& session :
         std::as_const(remainingSessions))
    {
        d->runtime.setCategoryUnlocked(session->categoryUuid, false);
        session->lease.reset();
        session->password.reset();

        if (d->observer)
        {
            d->observer->secretReleased(session->categoryUuid);
        }
    }
}

PrivacyCategorySessionResult PrivacyCategorySessionCoordinator::createCategory(
    const PrivacyCategoryCreateRequest& request, const QString& passwordText)
{
    PrivacyCategorySessionResult result;

    // Strong categories require the recovery-key export and acknowledgement
    // workflow before any durable category/store mutation. That workflow is
    // deliberately outside this casual-category slice.
    if (request.backend == PrivacyBackend::Strong)
    {
        result.status = PrivacyCategorySessionStatus::StrongRecoveryRequired;
        return result;
    }

    PrivacyPassword password = PrivacyPassword::fromUnicode(passwordText,
                                                             &result.passwordError);

    if (!password.isValid())
    {
        result.status = PrivacyCategorySessionStatus::InvalidPassword;
        return result;
    }

    const QString categoryUuid = normalizedUuid(request.categoryUuid);
    const QString storeUuid = normalizedUuid(request.storeUuid);
    const QString transactionUuid = normalizedUuid(request.transactionUuid);
    const QString name = request.name.trimmed();
    PrivacyStorageRoot root = request.storageRoot;
    root.uuid = normalizedUuid(root.uuid);
    root.markerUuid = normalizedUuid(root.markerUuid);

    if (categoryUuid.isEmpty() || storeUuid.isEmpty() || transactionUuid.isEmpty() ||
        name.isEmpty() || !root.isValid() ||
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot))
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        return result;
    }

    quint64 operationToken = 0;

    {
        QMutexLocker locker(&d->lock);
        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::Create);
    }

    if (operationToken == 0)
    {
        result.status = PrivacyCategorySessionStatus::TransactionBlocked;
        return result;
    }

    const auto finishOperation = [this, &categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    };

    const PrivacyRootRuntimeState rootState = d->rootVerifier.verify(root);

    if (rootState != PrivacyRootRuntimeState::VerifiedAvailable)
    {
        result.status = (rootState == PrivacyRootRuntimeState::Offline)
                      ? PrivacyCategorySessionStatus::StoreOffline
                      : PrivacyCategorySessionStatus::StoreIdentityMismatch;
        finishOperation();
        return result;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    PrivacyCategory category;
    category.uuid = categoryUuid;
    category.name = name;
    category.backend = request.backend;
    category.presentationMode = request.presentationMode;
    category.unlockedThumbnailMode = request.unlockedThumbnailMode;
    category.tagVisibilityMode = request.tagVisibilityMode;
    category.lifecycleState = PrivacyCategoryLifecycleState::Creating;
    category.currentCredentialGeneration = 0;
    category.createdAt = now;

    PrivacyStore store;
    store.uuid = storeUuid;
    store.categoryUuid = categoryUuid;
    store.rootUuid = root.uuid;
    store.format = QLatin1String("gocryptfs");
    store.formatVersion = 2;
    store.cipherRelativePath = storeRelativePath(storeUuid);
    store.configRelativePath = store.cipherRelativePath +
                               QLatin1String("/gocryptfs.conf");
    store.configGeneration = -1;
    store.lifecycleState = PrivacyStoreLifecycleState::Creating;
    store.createdAt = now;

    PrivacyTransaction transaction;
    transaction.uuid = transactionUuid;
    transaction.categoryUuid = categoryUuid;
    transaction.type = PrivacyTransactionType::CreateCategory;
    transaction.state = PrivacyTransactionState::Created;
    transaction.generation = 0;
    transaction.fromCredentialGeneration = -1;
    transaction.toCredentialGeneration = -1;
    transaction.payloadFormatVersion = 1;
    QJsonObject payload;
    payload.insert(QLatin1String("temporaryCipherRelativePath"),
                   temporaryStoreRelativePath(storeUuid));
    transaction.payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    transaction.createdAt = now;
    transaction.updatedAt = now;

    PrivacyTransactionJournal journal;
    journal.transactionUuid = transactionUuid;
    journal.rootUuid = root.uuid;
    journal.journalRelativePath = QLatin1String(".digikam-private/journals/") +
                                  transactionUuid + QLatin1String(".json");
    journal.journalFormatVersion = 1;
    journal.stage = 0;
    journal.updatedAt = now;

    if (!category.isValid() || !store.isValid() || !transaction.isValid() ||
        !journal.isValid())
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        finishOperation();
        return result;
    }

    if (!d->repository.beginCreation(category, root, store, transaction, journal))
    {
        PrivacyRepositorySnapshot snapshot;

        if (!d->repository.loadSnapshot(&snapshot) ||
            !sameCreatingRecords(snapshot, category, root, store, transaction))
        {
            result.status = PrivacyCategorySessionStatus::Conflict;
            finishOperation();
            return result;
        }
    }

    const QByteArray sentinel = sentinelBytes(categoryUuid, storeUuid);
    PrivacyGocryptfsEnvelope envelope;

    if (!d->storeBackend.createOrResume(root, store,
                                        temporaryStoreRelativePath(storeUuid),
                                        password, sentinel, &envelope,
                                        &result.storeError) || !envelope.isValid())
    {
        result.status = PrivacyCategorySessionStatus::StoreFailure;
        finishOperation();
        return result;
    }

    const QByteArray opaqueConfig = envelope.opaqueConfig();
    PrivacyCredential credential;
    credential.categoryUuid = categoryUuid;
    credential.generation = 1;
    credential.encodingVersion = PrivacyPassword::encodingVersion();
    credential.envelopeFormat = envelope.format();
    credential.envelopeBlob = opaqueConfig;
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash = sha256(opaqueConfig);
    credential.createdAt = now;

    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    store.lifecycleState = PrivacyStoreLifecycleState::Active;
    store.configGeneration = 1;
    transaction.state = PrivacyTransactionState::Complete;
    transaction.generation = 1;
    transaction.toCredentialGeneration = 1;
    transaction.updatedAt = QDateTime::currentDateTimeUtc();

    QList<PrivacyStoreBinding> bindings;

    for (const PrivacyStoreRole role : { PrivacyStoreRole::CredentialAuthority,
                                         PrivacyStoreRole::Derivatives })
    {
        PrivacyStoreBinding binding;
        binding.categoryUuid = categoryUuid;
        binding.role = role;
        binding.storeUuid = storeUuid;
        bindings << binding;
    }

    if (!d->repository.publishCreation(category, credential, store,
                                       bindings, transaction))
    {
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        finishOperation();
        return result;
    }

    if (!d->runtime.publishCategory(category))
    {
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        finishOperation();
        return result;
    }

    result.status = PrivacyCategorySessionStatus::Created;
    finishOperation();
    return result;
}

PrivacyCategorySessionResult PrivacyCategorySessionCoordinator::unlockCategory(
    const QString& categoryUuidText, const QString& passwordText)
{
    PrivacyCategorySessionResult result;
    const QString categoryUuid = normalizedUuid(categoryUuidText);
    quint64 operationToken = 0;

    {
        QMutexLocker locker(&d->lock);

        if (d->sessions.contains(categoryUuid))
        {
            result.status = PrivacyCategorySessionStatus::AlreadyUnlocked;
            return result;
        }

        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::Unlock);
    }

    if (operationToken == 0)
    {
        result.status = PrivacyCategorySessionStatus::TransactionBlocked;
        return result;
    }

    const auto finishOperation = [this, &categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    };

    PrivacyPassword password = PrivacyPassword::fromUnicode(passwordText,
                                                             &result.passwordError);

    if (categoryUuid.isEmpty() || !password.isValid())
    {
        result.status = password.isValid()
                      ? PrivacyCategorySessionStatus::InvalidRequest
                      : PrivacyCategorySessionStatus::InvalidPassword;
        finishOperation();
        return result;
    }

    PrivacyRepositorySnapshot snapshot;
    CategoryBundle bundle;

    if (!d->repository.loadSnapshot(&snapshot) ||
        !loadActiveBundle(snapshot, categoryUuid, &bundle, &result.status))
    {
        finishOperation();
        return result;
    }

    if ((bundle.credential.envelopeHashAlgorithm != QLatin1String("sha256")) ||
        (sha256(bundle.credential.envelopeBlob) != bundle.credential.envelopeHash))
    {
        result.status = PrivacyCategorySessionStatus::AuthenticationFailed;
        finishOperation();
        return result;
    }

    const PrivacyGocryptfsEnvelope envelope =
        PrivacyGocryptfsEnvelope::fromOpaqueConfig(bundle.credential.envelopeFormat,
                                                   bundle.credential.envelopeBlob,
                                                   &result.storeError);

    if (!envelope.isValid() ||
        !d->storeBackend.validateEnvelope(envelope, password, &result.storeError))
    {
        result.status = PrivacyCategorySessionStatus::AuthenticationFailed;
        finishOperation();
        return result;
    }

    const PrivacyRootRuntimeState rootState = d->rootVerifier.verify(bundle.root);
    std::unique_ptr<PrivacyCategoryStoreLease> lease;

    if (rootState == PrivacyRootRuntimeState::IdentityMismatch)
    {
        result.status = PrivacyCategorySessionStatus::StoreIdentityMismatch;
        finishOperation();
        return result;
    }

    if (rootState == PrivacyRootRuntimeState::Offline)
    {
        if (bundle.category.backend == PrivacyBackend::Strong)
        {
            result.status = PrivacyCategorySessionStatus::StoreOffline;
            finishOperation();
            return result;
        }
    }
    else if (rootState == PrivacyRootRuntimeState::VerifiedAvailable)
    {
        lease = d->storeBackend.unlock(bundle.root, bundle.store, envelope, password,
                                       sentinelBytes(categoryUuid, bundle.store.uuid),
                                       &result.storeError);

        if (!lease || !lease->isActive())
        {
            result.status = PrivacyCategorySessionStatus::StoreFailure;
            finishOperation();
            return result;
        }
    }
    else
    {
        result.status = PrivacyCategorySessionStatus::StoreIdentityMismatch;
        finishOperation();
        return result;
    }

    const auto retainFailedCleanup = [this, &categoryUuid, operationToken,
                                      &password, &lease]()
    {
        std::shared_ptr<Private::Session> session =
            std::make_shared<Private::Session>(categoryUuid,
                                               std::move(password),
                                               std::move(lease));

        {
            QMutexLocker locker(&d->lock);
            d->sessions.insert(categoryUuid, session);
            d->finishOperation(categoryUuid, operationToken);
        }

        if (d->observer)
        {
            d->observer->secretRetained(categoryUuid);
        }
    };

    const auto canceled = [this, &categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        return d->operationCanceled(categoryUuid, operationToken);
    };

    if (canceled())
    {
        if (!d->storeBackend.lock(lease, &result.storeError))
        {
            d->runtime.setCategoryUnlocked(categoryUuid, true);
            retainFailedCleanup();
            result.status = PrivacyCategorySessionStatus::LockFailed;
            return result;
        }

        finishOperation();
        result.status = PrivacyCategorySessionStatus::Canceled;
        return result;
    }

    if (!d->runtime.setCategoryUnlocked(categoryUuid, true))
    {
        if (!d->storeBackend.lock(lease, &result.storeError))
        {
            d->runtime.setCategoryUnlocked(categoryUuid, true);
            retainFailedCleanup();
            result.status = PrivacyCategorySessionStatus::LockFailed;
            return result;
        }

        finishOperation();
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        return result;
    }

    bool canceledAfterPublication = false;
    std::shared_ptr<Private::Session> session;

    {
        QMutexLocker locker(&d->lock);
        canceledAfterPublication = d->operationCanceled(categoryUuid,
                                                        operationToken);

        if (!canceledAfterPublication)
        {
            session = std::make_shared<Private::Session>(categoryUuid,
                                                        std::move(password),
                                                        std::move(lease));
            d->sessions.insert(categoryUuid, session);
            d->finishOperation(categoryUuid, operationToken);
        }
    }

    if (canceledAfterPublication)
    {
        d->runtime.setCategoryUnlocked(categoryUuid, false);

        if (!d->storeBackend.lock(lease, &result.storeError))
        {
            d->runtime.setCategoryUnlocked(categoryUuid, true);
            retainFailedCleanup();
            result.status = PrivacyCategorySessionStatus::LockFailed;
            return result;
        }

        finishOperation();
        result.status = PrivacyCategorySessionStatus::Canceled;
        return result;
    }

    if (d->observer)
    {
        d->observer->secretRetained(categoryUuid);
    }

    result.status = (rootState == PrivacyRootRuntimeState::Offline)
                  ? PrivacyCategorySessionStatus::UnlockedStoreOffline
                  : PrivacyCategorySessionStatus::Unlocked;
    return result;
}

PrivacyCategorySessionResult PrivacyCategorySessionCoordinator::lockCategory(
    const QString& categoryUuidText)
{
    PrivacyCategorySessionResult result;
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    if (categoryUuid.isEmpty())
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        return result;
    }

    quint64 operationToken = 0;
    std::shared_ptr<Private::Session> session;

    {
        QMutexLocker locker(&d->lock);

        while (d->operations.contains(categoryUuid))
        {
            auto operation = d->operations.find(categoryUuid);

            if (operation->kind == Private::OperationKind::Unlock)
            {
                operation->cancelRequested = true;
            }

            d->operationChanged.wait(&d->lock);
        }

        const auto sessionIt = d->sessions.constFind(categoryUuid);

        if (sessionIt == d->sessions.constEnd())
        {
            result.status = PrivacyCategorySessionStatus::AlreadyLocked;
            return result;
        }

        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::Lock);

        if (operationToken == 0)
        {
            result.status = PrivacyCategorySessionStatus::TransactionBlocked;
            return result;
        }

        session = sessionIt.value();
    }

    const auto finishOperation = [this, &categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    };

    if (!d->runtime.setCategoryUnlocked(categoryUuid, false))
    {
        finishOperation();
        result.status = PrivacyCategorySessionStatus::LockFailed;
        return result;
    }

    if (!d->storeBackend.lock(session->lease, &result.storeError))
    {
        d->runtime.setCategoryUnlocked(categoryUuid, true);
        finishOperation();
        result.status = PrivacyCategorySessionStatus::LockFailed;
        return result;
    }

    {
        QMutexLocker locker(&d->lock);
        const auto sessionIt = d->sessions.find(categoryUuid);

        if ((sessionIt != d->sessions.end()) &&
            (sessionIt.value() == session))
        {
            d->sessions.erase(sessionIt);
        }

        d->finishOperation(categoryUuid, operationToken);
    }

    // Destroy the normalized password before reporting its release. Neither
    // destruction nor the observer callback runs under the coordinator lock.
    session->password.reset();
    session.reset();

    if (d->observer)
    {
        d->observer->secretReleased(categoryUuid);
    }

    result.status = PrivacyCategorySessionStatus::Locked;
    return result;
}

QList<PrivacyCategorySessionResult>
PrivacyCategorySessionCoordinator::lockAllCategories()
{
    QStringList categoryUuids;

    {
        QMutexLocker locker(&d->lock);

        while (d->lockAllInProgress)
        {
            d->operationChanged.wait(&d->lock);
        }

        d->lockAllInProgress = true;

        for (auto it = d->operations.begin() ; it != d->operations.end() ; ++it)
        {
            if (it->kind == Private::OperationKind::Unlock)
            {
                it->cancelRequested = true;
            }
        }

        while (!d->operations.isEmpty())
        {
            d->operationChanged.wait(&d->lock);
        }

        categoryUuids = d->sessions.keys();
    }

    QList<PrivacyCategorySessionResult> results;

    for (const QString& categoryUuid : std::as_const(categoryUuids))
    {
        results << lockCategory(categoryUuid);
    }

    {
        QMutexLocker locker(&d->lock);
        d->lockAllInProgress = false;
        d->operationChanged.wakeAll();
    }

    return results;
}

bool PrivacyCategorySessionCoordinator::ownsSecret(const QString& categoryUuid) const
{
    QMutexLocker locker(&d->lock);

    return d->sessions.contains(normalizedUuid(categoryUuid));
}

} // namespace Digikam

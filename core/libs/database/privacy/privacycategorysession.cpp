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
#include <QUuid>

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

    class Session
    {
    public:

        Session(QString uuid, PrivacyPassword&& normalizedPassword,
                std::unique_ptr<PrivacyCategoryStoreLease>&& storeLease,
                PrivacySecretLifetimeObserver* lifetimeObserver)
            : categoryUuid(std::move(uuid)),
              password(std::move(normalizedPassword)),
              lease(std::move(storeLease)),
              observer(lifetimeObserver)
        {
            if (observer)
            {
                observer->secretRetained(categoryUuid);
            }
        }

        ~Session()
        {
            if (observer)
            {
                observer->secretReleased(categoryUuid);
            }
        }

        QString categoryUuid;
        PrivacyPassword password;
        std::unique_ptr<PrivacyCategoryStoreLease> lease;
        PrivacySecretLifetimeObserver* observer = nullptr;
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

    PrivacyCategorySessionRepository& repository;
    PrivacyCategoryStoreBackend& storeBackend;
    const PrivacyRootVerifier& rootVerifier;
    PrivacyRuntimeCoordinator& runtime;
    PrivacySecretLifetimeObserver* observer = nullptr;
    mutable QMutex lock;
    QHash<QString, std::shared_ptr<Session> > sessions;
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
}

PrivacyCategorySessionResult PrivacyCategorySessionCoordinator::createCategory(
    const PrivacyCategoryCreateRequest& request, const QString& passwordText)
{
    PrivacyCategorySessionResult result;
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

    const PrivacyRootRuntimeState rootState = d->rootVerifier.verify(root);

    if (rootState != PrivacyRootRuntimeState::VerifiedAvailable)
    {
        result.status = (rootState == PrivacyRootRuntimeState::Offline)
                      ? PrivacyCategorySessionStatus::StoreOffline
                      : PrivacyCategorySessionStatus::StoreIdentityMismatch;
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
        return result;
    }

    if (!d->repository.beginCreation(category, root, store, transaction, journal))
    {
        PrivacyRepositorySnapshot snapshot;

        if (!d->repository.loadSnapshot(&snapshot) ||
            !sameCreatingRecords(snapshot, category, root, store, transaction))
        {
            result.status = PrivacyCategorySessionStatus::Conflict;
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
        return result;
    }

    if (request.backend == PrivacyBackend::Strong)
    {
        result.status = PrivacyCategorySessionStatus::StrongRecoveryRequired;
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
        return result;
    }

    if (!d->runtime.publishCategory(category))
    {
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        return result;
    }

    result.status = PrivacyCategorySessionStatus::Created;
    return result;
}

PrivacyCategorySessionResult PrivacyCategorySessionCoordinator::unlockCategory(
    const QString& categoryUuidText, const QString& passwordText)
{
    PrivacyCategorySessionResult result;
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    {
        QMutexLocker locker(&d->lock);

        if (d->sessions.contains(categoryUuid))
        {
            result.status = PrivacyCategorySessionStatus::AlreadyUnlocked;
            return result;
        }
    }

    PrivacyPassword password = PrivacyPassword::fromUnicode(passwordText,
                                                             &result.passwordError);

    if (categoryUuid.isEmpty() || !password.isValid())
    {
        result.status = password.isValid()
                      ? PrivacyCategorySessionStatus::InvalidRequest
                      : PrivacyCategorySessionStatus::InvalidPassword;
        return result;
    }

    PrivacyRepositorySnapshot snapshot;
    CategoryBundle bundle;

    if (!d->repository.loadSnapshot(&snapshot) ||
        !loadActiveBundle(snapshot, categoryUuid, &bundle, &result.status))
    {
        return result;
    }

    if ((bundle.credential.envelopeHashAlgorithm != QLatin1String("sha256")) ||
        (sha256(bundle.credential.envelopeBlob) != bundle.credential.envelopeHash))
    {
        result.status = PrivacyCategorySessionStatus::AuthenticationFailed;
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
        return result;
    }

    const PrivacyRootRuntimeState rootState = d->rootVerifier.verify(bundle.root);
    std::unique_ptr<PrivacyCategoryStoreLease> lease;

    if (rootState == PrivacyRootRuntimeState::IdentityMismatch)
    {
        result.status = PrivacyCategorySessionStatus::StoreIdentityMismatch;
        return result;
    }

    if (rootState == PrivacyRootRuntimeState::Offline)
    {
        if (bundle.category.backend == PrivacyBackend::Strong)
        {
            result.status = PrivacyCategorySessionStatus::StoreOffline;
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
            return result;
        }
    }
    else
    {
        result.status = PrivacyCategorySessionStatus::StoreIdentityMismatch;
        return result;
    }

    QMutexLocker locker(&d->lock);

    if (d->sessions.contains(categoryUuid))
    {
        if (lease)
        {
            d->storeBackend.lock(lease, &result.storeError);
        }

        result.status = PrivacyCategorySessionStatus::AlreadyUnlocked;
        return result;
    }

    std::shared_ptr<Private::Session> session = std::make_shared<Private::Session>(
        categoryUuid, std::move(password), std::move(lease), d->observer);
    d->sessions.insert(categoryUuid, session);

    if (!d->runtime.setCategoryUnlocked(categoryUuid, true))
    {
        std::unique_ptr<PrivacyCategoryStoreLease>& candidateLease = session->lease;
        d->storeBackend.lock(candidateLease, &result.storeError);
        d->sessions.remove(categoryUuid);
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        return result;
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
    QMutexLocker locker(&d->lock);
    const auto it = d->sessions.find(categoryUuid);

    if (it == d->sessions.end())
    {
        result.status = categoryUuid.isEmpty()
                      ? PrivacyCategorySessionStatus::InvalidRequest
                      : PrivacyCategorySessionStatus::AlreadyLocked;
        return result;
    }

    const std::shared_ptr<Private::Session> session = it.value();

    if (!d->runtime.setCategoryUnlocked(categoryUuid, false))
    {
        result.status = PrivacyCategorySessionStatus::LockFailed;
        return result;
    }

    if (!d->storeBackend.lock(session->lease, &result.storeError))
    {
        d->runtime.setCategoryUnlocked(categoryUuid, true);
        result.status = PrivacyCategorySessionStatus::LockFailed;
        return result;
    }

    d->sessions.erase(it);
    result.status = PrivacyCategorySessionStatus::Locked;
    return result;
}

QList<PrivacyCategorySessionResult>
PrivacyCategorySessionCoordinator::lockAllCategories()
{
    QStringList categoryUuids;

    {
        QMutexLocker locker(&d->lock);
        categoryUuids = d->sessions.keys();
    }

    QList<PrivacyCategorySessionResult> results;

    for (const QString& categoryUuid : std::as_const(categoryUuids))
    {
        results << lockCategory(categoryUuid);
    }

    return results;
}

bool PrivacyCategorySessionCoordinator::ownsSecret(const QString& categoryUuid) const
{
    QMutexLocker locker(&d->lock);

    return d->sessions.contains(normalizedUuid(categoryUuid));
}

} // namespace Digikam

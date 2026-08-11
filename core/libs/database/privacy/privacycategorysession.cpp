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
#include <QThread>
#include <QWaitCondition>
#include <QUuid>

// C++ includes

#include <utility>

// Local includes

#include "privacyrepository.h"
#include "privacyexternalcheckouttransaction.h"

namespace Digikam
{

namespace
{

class ScopeExit
{
public:

    explicit ScopeExit(std::function<void()> callback)
        : m_callback(std::move(callback))
    {
    }

    ~ScopeExit()
    {
        m_callback();
    }

private:

    std::function<void()> m_callback;

private:

    Q_DISABLE_COPY(ScopeExit)
};

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
                (existing.format != store.format) ||
                (existing.formatVersion != store.formatVersion) ||
                (existing.cipherRelativePath != store.cipherRelativePath) ||
                (existing.configRelativePath != store.configRelativePath) ||
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
                (existing.generation != 0) ||
                (existing.fromCredentialGeneration != -1) ||
                (existing.toCredentialGeneration != -1) ||
                (existing.payloadFormatVersion != transaction.payloadFormatVersion) ||
                (existing.payloadData != transaction.payloadData))
            {
                return false;
            }
        }
    }

    return ((categoryMatches == 1) && (rootMatches == 1) &&
            (storeMatches == 1) && (transactionMatches == 1));
}

bool loadCompletedCreation(const PrivacyRepositorySnapshot& snapshot,
                           const PrivacyCategory& requestedCategory,
                           const PrivacyStorageRoot& requestedRoot,
                           const PrivacyStore& requestedStore,
                           const PrivacyTransaction& requestedTransaction,
                           const PrivacyTransactionJournal& requestedJournal,
                           PrivacyCategory* category,
                           PrivacyCredential* credential,
                           PrivacyStore* store,
                           QList<PrivacyStoreBinding>* bindings,
                           PrivacyTransactionJournal* journal)
{
    if (!category || !credential || !store || !bindings || !journal)
    {
        return false;
    }

    int categoryCount = 0;
    int credentialCount = 0;
    int rootCount = 0;
    int storeCount = 0;
    int transactionCount = 0;
    int journalCount = 0;

    for (const PrivacyCategory& candidate : snapshot.categories)
    {
        if (candidate.uuid == requestedCategory.uuid)
        {
            ++categoryCount;
            *category = candidate;
        }
    }

    for (const PrivacyCredential& candidate : snapshot.credentials)
    {
        if ((candidate.categoryUuid == requestedCategory.uuid) &&
            (candidate.generation == 1))
        {
            ++credentialCount;
            *credential = candidate;
        }
    }

    for (const PrivacyStorageRoot& candidate : snapshot.storageRoots)
    {
        if (candidate.uuid == requestedRoot.uuid)
        {
            ++rootCount;

            if ((candidate.kind != requestedRoot.kind) ||
                (candidate.configuredPath != requestedRoot.configuredPath) ||
                (candidate.identityVersion != requestedRoot.identityVersion) ||
                (candidate.identityData != requestedRoot.identityData) ||
                (candidate.markerUuid != requestedRoot.markerUuid))
            {
                return false;
            }
        }
    }

    for (const PrivacyStore& candidate : snapshot.stores)
    {
        if (candidate.uuid == requestedStore.uuid)
        {
            ++storeCount;
            *store = candidate;
        }
    }

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.uuid == requestedTransaction.uuid)
        {
            ++transactionCount;

            if ((candidate.categoryUuid != requestedTransaction.categoryUuid) ||
                (candidate.type != PrivacyTransactionType::CreateCategory) ||
                (candidate.state != PrivacyTransactionState::Complete) ||
                (candidate.generation != 1) ||
                (candidate.fromCredentialGeneration != -1) ||
                (candidate.toCredentialGeneration != 1) ||
                (candidate.payloadFormatVersion !=
                 requestedTransaction.payloadFormatVersion) ||
                (candidate.payloadData != requestedTransaction.payloadData))
            {
                return false;
            }
        }
    }

    for (const PrivacyTransactionJournal& candidate : snapshot.transactionJournals)
    {
        if (candidate.transactionUuid == requestedTransaction.uuid)
        {
            ++journalCount;
            *journal = candidate;
        }
    }

    bindings->clear();

    for (const PrivacyStoreBinding& binding : snapshot.storeBindings)
    {
        if (binding.categoryUuid == requestedCategory.uuid)
        {
            bindings->append(binding);
        }
    }

    QSet<PrivacyStoreRole> roles;

    for (const PrivacyStoreBinding& binding : std::as_const(*bindings))
    {
        if ((binding.storeUuid != requestedStore.uuid) || roles.contains(binding.role))
        {
            return false;
        }

        roles.insert(binding.role);
    }

    return ((categoryCount == 1) && (credentialCount == 1) && (rootCount == 1) &&
            (storeCount == 1) && (transactionCount == 1) && (journalCount == 1) &&
            (category->name == requestedCategory.name) &&
            (category->backend == requestedCategory.backend) &&
            (category->presentationMode == requestedCategory.presentationMode) &&
            (category->unlockedThumbnailMode ==
             requestedCategory.unlockedThumbnailMode) &&
            (category->tagVisibilityMode == requestedCategory.tagVisibilityMode) &&
            (category->lifecycleState == PrivacyCategoryLifecycleState::Active) &&
            (category->currentCredentialGeneration == 1) &&
            (store->categoryUuid == requestedStore.categoryUuid) &&
            (store->rootUuid == requestedStore.rootUuid) &&
            (store->format == requestedStore.format) &&
            (store->formatVersion == requestedStore.formatVersion) &&
            (store->cipherRelativePath == requestedStore.cipherRelativePath) &&
            (store->configRelativePath == requestedStore.configRelativePath) &&
            (store->lifecycleState == PrivacyStoreLifecycleState::Active) &&
            (store->configGeneration == 1) &&
            (journal->rootUuid == requestedJournal.rootUuid) &&
            (journal->journalRelativePath == requestedJournal.journalRelativePath) &&
            (journal->journalFormatVersion == requestedJournal.journalFormatVersion) &&
            ((journal->stage == static_cast<int>(PrivacyJournalStage::Created)) ||
             (journal->stage == static_cast<int>(PrivacyJournalStage::Complete))) &&
            (journal->expectedHashAlgorithm == QLatin1String("sha256")) &&
            (QByteArray::fromHex(journal->expectedJournalHash.toLatin1()).size() == 32) &&
            roles.contains(PrivacyStoreRole::CredentialAuthority) &&
            roles.contains(PrivacyStoreRole::Derivatives) &&
            ((category->backend == PrivacyBackend::Casual)
                 ? (roles.size() == 2)
                 : (roles.contains(PrivacyStoreRole::Originals) &&
                    (roles.size() == 3))));
}

bool findExactCreationJournal(const PrivacyRepositorySnapshot& snapshot,
                              const PrivacyTransactionJournal& expected,
                              PrivacyTransactionJournal* existing)
{
    int matches = 0;

    for (const PrivacyTransactionJournal& candidate : snapshot.transactionJournals)
    {
        if (candidate.transactionUuid != expected.transactionUuid)
        {
            continue;
        }

        ++matches;

        if ((candidate.rootUuid != expected.rootUuid) ||
            (candidate.journalRelativePath != expected.journalRelativePath) ||
            (candidate.journalFormatVersion != expected.journalFormatVersion) ||
            (candidate.stage != expected.stage) ||
            ((!candidate.expectedHashAlgorithm.isEmpty() ||
              !candidate.expectedJournalHash.isEmpty()) &&
             ((candidate.expectedHashAlgorithm != QLatin1String("sha256")) ||
              (QByteArray::fromHex(candidate.expectedJournalHash.toLatin1()).size() != 32))))
        {
            return false;
        }

        if (existing)
        {
            *existing = candidate;
        }
    }

    return (matches == 1);
}

PrivacyJournalRootExpectation journalRootExpectation(const PrivacyStorageRoot& root)
{
    PrivacyJournalRootExpectation expectation;
    expectation.rootUuid = root.uuid;
    expectation.markerUuid = root.markerUuid;
    expectation.identitySha256 =
        QCryptographicHash::hash(root.identityData, QCryptographicHash::Sha256);
    return expectation;
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
                      PrivacyCategorySessionStatus* failure,
                      const QString& allowedActiveItemTransactionUuid = QString())
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

    int allowedActiveTransactionCount = 0;

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if ((transaction.categoryUuid == categoryUuid) && transaction.isActive())
        {
            const bool allowed =
                !allowedActiveItemTransactionUuid.isEmpty() &&
                (transaction.uuid == allowedActiveItemTransactionUuid) &&
                ((transaction.type == PrivacyTransactionType::ProtectItem) ||
                 (transaction.type == PrivacyTransactionType::UnprotectItem));

            if (!allowed)
            {
                *failure = PrivacyCategorySessionStatus::TransactionBlocked;
                return false;
            }

            ++allowedActiveTransactionCount;
        }
    }

    if (!allowedActiveItemTransactionUuid.isEmpty() &&
        (allowedActiveTransactionCount != 1))
    {
        *failure = PrivacyCategorySessionStatus::TransactionBlocked;
        return false;
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
            (status == PrivacyCategorySessionStatus::AlreadyLocked) ||
            (status == PrivacyCategorySessionStatus::FreshAuthenticationVerified) ||
            (status == PrivacyCategorySessionStatus::SettingsUpdated));
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

bool PrivacyCoreDbCategorySessionRepository::setCategoryUnlockedThumbnailMode(
    const QString& categoryUuid, PrivacyUnlockedThumbnailMode mode)
{
    return PrivacyRepository().setCategoryUnlockedThumbnailMode(
        categoryUuid, mode, true);
}

bool PrivacyCoreDbCategorySessionRepository::setCategoryTagVisibilityMode(
    const QString& categoryUuid, PrivacyTagVisibilityMode mode)
{
    return PrivacyRepository().setCategoryTagVisibilityMode(categoryUuid, mode, true);
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

bool PrivacyCoreDbCategorySessionRepository::compareAndUpdateCreationJournal(
    const PrivacyTransactionJournal& journal, int expectedStage)
{
    return PrivacyRepository().compareAndUpdateTransactionJournal(journal,
                                                                   expectedStage);
}

bool PrivacyFilesystemCategoryCreationJournalPersistence::createOrLoadExact(
    const PrivacyStorageRoot& root, PrivacyJournalRecord* const record,
    bool allowCreate, QByteArray* const publishedSha256,
    PrivacyJournalError* const error)
{
    if (!record || !publishedSha256 || !root.isValid() ||
        (record->rootUuid != root.uuid) ||
        (record->transactionType != PrivacyTransactionType::CreateCategory))
    {
        if (error)
        {
            *error = PrivacyJournalError::InvalidRecord;
        }

        return false;
    }

    QString detail;
    auto store = PrivacyTransactionJournalStore::open(
        root.configuredPath, journalRootExpectation(root), error, &detail);

    if (!store)
    {
        return false;
    }

    record->rootDevice = store->rootDevice();
    record->rootInode = store->rootInode();
    const QByteArray desired = PrivacyTransactionJournalCodec::encode(
        *record, error, &detail);

    if (desired.isEmpty())
    {
        return false;
    }

    PrivacyJournalLoadResult loaded = store->load(record->transactionUuid);

    if (loaded.disposition == PrivacyJournalLoadDisposition::Missing)
    {
        if (allowCreate)
        {
            if (store->create(*record, publishedSha256, error, &detail))
            {
                return true;
            }

            loaded = store->load(record->transactionUuid);

            if ((loaded.disposition == PrivacyJournalLoadDisposition::Loaded) &&
                loaded.authoritative && (loaded.canonicalBytes == desired))
            {
                *publishedSha256 = loaded.sha256;

                if (error)
                {
                    *error = PrivacyJournalError::None;
                }

                return true;
            }

            return false;
        }

        if (error)
        {
            *error = PrivacyJournalError::PublicationConflict;
        }

        return false;
    }

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || (loaded.canonicalBytes != desired))
    {
        if (error)
        {
            *error = (loaded.error == PrivacyJournalError::None)
                   ? PrivacyJournalError::PublicationConflict : loaded.error;
        }

        return false;
    }

    *publishedSha256 = loaded.sha256;

    if (error)
    {
        *error = PrivacyJournalError::None;
    }

    return true;
}

bool PrivacyFilesystemCategoryCreationJournalPersistence::compareAndUpdateExact(
    const PrivacyStorageRoot& root, const PrivacyJournalRecord& record,
    const QByteArray& expectedCurrentSha256, QByteArray* const publishedSha256,
    PrivacyJournalError* const error)
{
    if (!publishedSha256 || (expectedCurrentSha256.size() != 32))
    {
        if (error)
        {
            *error = PrivacyJournalError::InvalidRecord;
        }

        return false;
    }

    QString detail;
    auto store = PrivacyTransactionJournalStore::open(
        root.configuredPath, journalRootExpectation(root), error, &detail);

    if (!store || (record.rootDevice != store->rootDevice()) ||
        (record.rootInode != store->rootInode()))
    {
        if (store && error)
        {
            *error = PrivacyJournalError::RootIdentityMismatch;
        }

        return false;
    }

    const QByteArray desired = PrivacyTransactionJournalCodec::encode(
        record, error, &detail);
    const PrivacyJournalLoadResult loaded = store->load(record.transactionUuid);

    if (desired.isEmpty() ||
        (loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative)
    {
        if (!desired.isEmpty() && error)
        {
            *error = (loaded.error == PrivacyJournalError::None)
                   ? PrivacyJournalError::PublicationConflict : loaded.error;
        }

        return false;
    }

    if (loaded.canonicalBytes == desired)
    {
        *publishedSha256 = loaded.sha256;
        return true;
    }

    if (loaded.sha256 != expectedCurrentSha256)
    {
        if (error)
        {
            *error = PrivacyJournalError::StaleComparison;
        }

        return false;
    }

    return store->compareAndUpdate(record, expectedCurrentSha256,
                                   publishedSha256, error, &detail);
}

class PrivacyCategorySessionCoordinator::Private
{
public:

    enum class OperationKind
    {
        Create,
        Unlock,
        Lock,
        ItemTransaction
    };

    struct Operation
    {
        OperationKind kind = OperationKind::Unlock;
        quint64 token = 0;
        bool cancelRequested = false;
        Qt::HANDLE ownerThread = nullptr;
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
            PrivacySecretLifetimeObserver* lifetimeObserver,
            PrivacyCategoryCreationJournalPersistence* creationJournal)
        : repository(sessionRepository),
          storeBackend(categoryStoreBackend),
          rootVerifier(categoryRootVerifier),
          runtime(privacyRuntime),
          observer(lifetimeObserver),
          journalPersistence(creationJournal ? creationJournal
                                             : &filesystemJournalPersistence)
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
        operation.ownerThread = QThread::currentThreadId();
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
    PrivacyFilesystemCategoryCreationJournalPersistence filesystemJournalPersistence;
    PrivacyCategoryCreationJournalPersistence* journalPersistence = nullptr;
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
    PrivacySecretLifetimeObserver* secretObserver,
    PrivacyCategoryCreationJournalPersistence* creationJournal)
    : d(std::make_unique<Private>(repository, storeBackend, rootVerifier,
                                  runtime, secretObserver, creationJournal))
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
    category.recoverySetUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
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
    journal.journalRelativePath =
        PrivacyTransactionJournalCodec::relativeJournalPath(transactionUuid);
    journal.journalFormatVersion = PrivacyTransactionJournalCodec::FormatVersion;
    journal.stage = static_cast<int>(PrivacyJournalStage::Created);
    journal.updatedAt = now;

    PrivacyJournalRecord journalRecord;
    journalRecord.transactionUuid = transactionUuid;
    journalRecord.categoryUuid = categoryUuid;
    journalRecord.rootUuid = root.uuid;
    journalRecord.rootIdentitySha256 =
        QCryptographicHash::hash(root.identityData, QCryptographicHash::Sha256);
    journalRecord.transactionType = PrivacyTransactionType::CreateCategory;
    journalRecord.generation = 0;
    journalRecord.credentialGeneration = -1;
    journalRecord.fromCredentialGeneration = -1;
    journalRecord.toCredentialGeneration = -1;
    journalRecord.stage = PrivacyJournalStage::Created;

    if (!category.isValid() || !store.isValid() || !transaction.isValid() ||
        !journal.isValid())
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        finishOperation();
        return result;
    }

    PrivacyTransactionJournal existingJournal = journal;
    PrivacyCredential completedCredential;
    QList<PrivacyStoreBinding> completedBindings;
    bool completedReplay = false;
    const bool beganCreation =
        d->repository.beginCreation(category, root, store, transaction, journal);

    if (!beganCreation)
    {
        PrivacyRepositorySnapshot snapshot;

        if (!d->repository.loadSnapshot(&snapshot))
        {
            result.status = PrivacyCategorySessionStatus::Conflict;
            finishOperation();
            return result;
        }

        if (sameCreatingRecords(snapshot, category, root, store, transaction) &&
            findExactCreationJournal(snapshot, journal, &existingJournal))
        {
            // Resume the exact pending record below.
        }
        else if (loadCompletedCreation(snapshot, category, root, store,
                                       transaction, journal, &category,
                                       &completedCredential, &store,
                                       &completedBindings, &existingJournal))
        {
            completedReplay = true;
            transaction.state = PrivacyTransactionState::Complete;
            transaction.generation = 1;
            transaction.toCredentialGeneration = 1;
        }
        else
        {
            result.status = PrivacyCategorySessionStatus::Conflict;
            finishOperation();
            return result;
        }
    }

    if (existingJournal.stage == static_cast<int>(PrivacyJournalStage::Complete))
    {
        journalRecord.stage = PrivacyJournalStage::Complete;
        journalRecord.generation = 1;
        journalRecord.credentialGeneration = 1;
        journalRecord.toCredentialGeneration = 1;
    }

    QByteArray createdJournalHash;
    PrivacyJournalError journalError = PrivacyJournalError::None;
    bool filesystemAlreadyComplete = false;
    QByteArray expectedCreatedJournalHash;
    const bool mayCreateMissingJournal =
        (beganCreation || (!completedReplay &&
                           existingJournal.expectedJournalHash.isEmpty()));

    if (!d->journalPersistence->createOrLoadExact(root, &journalRecord,
                                                   mayCreateMissingJournal,
                                                   &createdJournalHash,
                                                   &journalError) ||
        (createdJournalHash.size() != 32))
    {
        if (completedReplay &&
            (existingJournal.stage ==
             static_cast<int>(PrivacyJournalStage::Created)))
        {
            const QByteArray createdBytes =
                PrivacyTransactionJournalCodec::encode(journalRecord);

            if (!createdBytes.isEmpty())
            {
                expectedCreatedJournalHash =
                    PrivacyTransactionJournalCodec::sha256(createdBytes);
                journalRecord.stage = PrivacyJournalStage::Complete;
                journalRecord.generation = 1;
                journalRecord.credentialGeneration = 1;
                journalRecord.toCredentialGeneration = 1;
                filesystemAlreadyComplete =
                    d->journalPersistence->createOrLoadExact(
                        root, &journalRecord, false, &createdJournalHash,
                        &journalError) && (createdJournalHash.size() == 32);
            }
        }

        if (!filesystemAlreadyComplete)
        {
            result.status = (journalError == PrivacyJournalError::RootIdentityMismatch)
                          ? PrivacyCategorySessionStatus::StoreIdentityMismatch
                          : PrivacyCategorySessionStatus::Conflict;
            finishOperation();
            return result;
        }
    }

    const QString createdJournalHashHex = QString::fromLatin1(
        createdJournalHash.toHex());

    const QString expectedDatabaseHash = filesystemAlreadyComplete
                                       ? QString::fromLatin1(
                                             expectedCreatedJournalHash.toHex())
                                       : createdJournalHashHex;

    if (!existingJournal.expectedJournalHash.isEmpty() &&
        ((existingJournal.expectedHashAlgorithm != QLatin1String("sha256")) ||
         (existingJournal.expectedJournalHash != expectedDatabaseHash)))
    {
        result.status = PrivacyCategorySessionStatus::Conflict;
        finishOperation();
        return result;
    }

    if (completedReplay)
    {
        if (!d->storeBackend.validateEnvelope(
                PrivacyGocryptfsEnvelope::fromOpaqueConfig(
                    completedCredential.envelopeFormat,
                    completedCredential.envelopeBlob, &result.storeError),
                password, &result.storeError))
        {
            result.status = PrivacyCategorySessionStatus::AuthenticationFailed;
            finishOperation();
            return result;
        }

        if (existingJournal.stage == static_cast<int>(PrivacyJournalStage::Created))
        {
            QByteArray completeHash = createdJournalHash;

            if (!filesystemAlreadyComplete)
            {
                journalRecord.stage = PrivacyJournalStage::Complete;
                journalRecord.generation = 1;
                journalRecord.credentialGeneration = 1;
                journalRecord.toCredentialGeneration = 1;

                if (!d->journalPersistence->compareAndUpdateExact(
                        root, journalRecord, createdJournalHash, &completeHash,
                        &journalError))
                {
                    result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
                    finishOperation();
                    return result;
                }
            }

            existingJournal.stage = static_cast<int>(PrivacyJournalStage::Complete);
            existingJournal.expectedHashAlgorithm = QLatin1String("sha256");
            existingJournal.expectedJournalHash =
                QString::fromLatin1(completeHash.toHex());
            existingJournal.updatedAt = QDateTime::currentDateTimeUtc();

            if (!d->repository.compareAndUpdateCreationJournal(
                    existingJournal,
                    static_cast<int>(PrivacyJournalStage::Created)))
            {
                result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
                finishOperation();
                return result;
            }
        }

        if (!d->runtime.publishCategoryCreation(category, completedCredential,
                                                 root, store,
                                                 completedBindings))
        {
            result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
            finishOperation();
            return result;
        }

        result.status = PrivacyCategorySessionStatus::Created;
        finishOperation();
        return result;
    }

    if (existingJournal.expectedJournalHash.isEmpty())
    {
        journal.expectedHashAlgorithm = QLatin1String("sha256");
        journal.expectedJournalHash = createdJournalHashHex;
        journal.updatedAt = QDateTime::currentDateTimeUtc();

        if (!d->repository.compareAndUpdateCreationJournal(
                journal, static_cast<int>(PrivacyJournalStage::Created)))
        {
            PrivacyRepositorySnapshot snapshot;
            PrivacyTransactionJournal published;

            if (!d->repository.loadSnapshot(&snapshot) ||
                !findExactCreationJournal(snapshot, journal, &published) ||
                (published.expectedHashAlgorithm != journal.expectedHashAlgorithm) ||
                (published.expectedJournalHash != journal.expectedJournalHash))
            {
                result.status = PrivacyCategorySessionStatus::Conflict;
                finishOperation();
                return result;
            }
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

    QList<PrivacyStoreRole> roles;
    roles << PrivacyStoreRole::CredentialAuthority
          << PrivacyStoreRole::Derivatives;

    if (request.backend == PrivacyBackend::Strong)
    {
        roles << PrivacyStoreRole::Originals;
    }

    QList<PrivacyStoreBinding> bindings;

    for (const PrivacyStoreRole role : roles)
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

    journalRecord.stage = PrivacyJournalStage::Complete;
    journalRecord.generation = 1;
    journalRecord.credentialGeneration = 1;
    journalRecord.toCredentialGeneration = 1;
    QByteArray completeJournalHash;

    if (!d->journalPersistence->compareAndUpdateExact(
            root, journalRecord, createdJournalHash, &completeJournalHash,
            &journalError) || (completeJournalHash.size() != 32))
    {
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        finishOperation();
        return result;
    }

    journal.stage = static_cast<int>(PrivacyJournalStage::Complete);
    journal.expectedHashAlgorithm = QLatin1String("sha256");
    journal.expectedJournalHash = QString::fromLatin1(completeJournalHash.toHex());
    journal.updatedAt = QDateTime::currentDateTimeUtc();

    if (!d->repository.compareAndUpdateCreationJournal(
            journal, static_cast<int>(PrivacyJournalStage::Created)))
    {
        result.status = PrivacyCategorySessionStatus::PublicationFailedRecoveryRequired;
        finishOperation();
        return result;
    }

    if (!d->runtime.publishCategoryCreation(category, credential, root, store,
                                             bindings))
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

            if ((operation->kind == Private::OperationKind::ItemTransaction) &&
                (operation->ownerThread == QThread::currentThreadId()))
            {
                result.status = PrivacyCategorySessionStatus::TransactionBlocked;
                return result;
            }

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

    // The Lock operation barrier is authoritative for this category: no new
    // checkout can begin until finishOperation(). Re-read durable state now so
    // manual lock, Lock All, and desktop transitions cannot unmount a store
    // beneath an external application.
    PrivacyRepositorySnapshot snapshot;

    if (!d->repository.loadSnapshot(&snapshot) ||
        std::any_of(snapshot.transactions.cbegin(),
                    snapshot.transactions.cend(),
                    [&categoryUuid](const PrivacyTransaction& transaction)
                    {
                        return (transaction.categoryUuid == categoryUuid) &&
                               (PrivacyExternalCheckoutTransactionEngine::
                                    holdsPlaintextLease(transaction) ||
                                (transaction.isActive() &&
                                 (transaction.type ==
                                  PrivacyTransactionType::CompatibilityUnlock)));
                    }))
    {
        finishOperation();
        result.status = PrivacyCategorySessionStatus::TransactionBlocked;
        return result;
    }

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

        for (auto it = d->operations.constBegin() ; it != d->operations.constEnd() ; ++it)
        {
            if ((it->kind == Private::OperationKind::ItemTransaction) &&
                (it->ownerThread == QThread::currentThreadId()))
            {
                PrivacyCategorySessionResult blocked;
                blocked.status = PrivacyCategorySessionStatus::TransactionBlocked;
                return { blocked };
            }
        }

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

PrivacyCategoryOperationStatus PrivacyCategorySessionCoordinator::runWhileUnlocked(
    const QString& categoryUuidText, const std::function<void()>& operation)
{
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    if (categoryUuid.isEmpty() || !operation)
    {
        return PrivacyCategoryOperationStatus::InvalidRequest;
    }

    quint64 operationToken = 0;
    std::shared_ptr<Private::Session> session;

    {
        QMutexLocker locker(&d->lock);
        const auto sessionIt = d->sessions.constFind(categoryUuid);

        if (sessionIt == d->sessions.constEnd())
        {
            return PrivacyCategoryOperationStatus::CategoryLocked;
        }

        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::ItemTransaction);

        if (operationToken == 0)
        {
            return PrivacyCategoryOperationStatus::TransactionBlocked;
        }

        session = sessionIt.value();
    }

    const ScopeExit finishOperation([this, categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    });

    operation();
    Q_UNUSED(session); // Retain the session and its lease for the callback.

    return PrivacyCategoryOperationStatus::Completed;
}

PrivacyCategoryOperationStatus
PrivacyCategorySessionCoordinator::runWithUnlockedSecret(
    const QString& categoryUuidText,
    const std::function<void(const PrivacyPassword&)>& operation)
{
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    if (categoryUuid.isEmpty() || !operation)
    {
        return PrivacyCategoryOperationStatus::InvalidRequest;
    }

    quint64 operationToken = 0;
    std::shared_ptr<Private::Session> session;

    {
        QMutexLocker locker(&d->lock);
        const auto sessionIt = d->sessions.constFind(categoryUuid);

        if ((sessionIt == d->sessions.constEnd()) ||
            !sessionIt.value()->password ||
            !sessionIt.value()->password->isValid())
        {
            return PrivacyCategoryOperationStatus::CategoryLocked;
        }

        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::ItemTransaction);

        if (operationToken == 0)
        {
            return PrivacyCategoryOperationStatus::TransactionBlocked;
        }

        session = sessionIt.value();
    }

    const ScopeExit finishOperation([this, categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    });

    operation(*session->password);

    return PrivacyCategoryOperationStatus::Completed;
}

PrivacyCategoryOperationStatus
PrivacyCategorySessionCoordinator::runWithUnlockedStore(
    const QString& categoryUuidText,
    const std::function<void(const PrivacyPassword&, const QString&)>& operation)
{
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    if (categoryUuid.isEmpty() || !operation)
    {
        return PrivacyCategoryOperationStatus::InvalidRequest;
    }

    quint64 operationToken = 0;
    std::shared_ptr<Private::Session> session;

    {
        QMutexLocker locker(&d->lock);
        const auto sessionIt = d->sessions.constFind(categoryUuid);

        if ((sessionIt == d->sessions.constEnd()) ||
            !sessionIt.value()->password ||
            !sessionIt.value()->password->isValid())
        {
            return PrivacyCategoryOperationStatus::CategoryLocked;
        }

        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::ItemTransaction);

        if (operationToken == 0)
        {
            return PrivacyCategoryOperationStatus::TransactionBlocked;
        }

        session = sessionIt.value();
    }

    const ScopeExit finishOperation([this, categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    });
    const QString plaintextRoot = session->lease
                                ? session->lease->plaintextRoot()
                                : QString();

    operation(*session->password, plaintextRoot);
    return PrivacyCategoryOperationStatus::Completed;
}

PrivacyCategorySessionResult
PrivacyCategorySessionCoordinator::runWithFreshlyAuthenticatedSecret(
    const QString& categoryUuidText, const QString& passwordText,
    const std::function<void(const PrivacyPassword&)>& operation,
    const QString& allowedActiveItemTransactionUuidText)
{
    PrivacyCategorySessionResult result;
    const QString categoryUuid = normalizedUuid(categoryUuidText);
    const QString allowedActiveItemTransactionUuid =
        normalizedUuid(allowedActiveItemTransactionUuidText);

    if (categoryUuid.isEmpty() || !operation ||
        (!allowedActiveItemTransactionUuidText.isEmpty() &&
         allowedActiveItemTransactionUuid.isEmpty()))
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        return result;
    }

    quint64 operationToken = 0;
    std::shared_ptr<Private::Session> session;

    {
        QMutexLocker locker(&d->lock);
        const auto sessionIt = d->sessions.constFind(categoryUuid);

        if (sessionIt != d->sessions.constEnd())
        {
            session = sessionIt.value();
        }

        operationToken = d->beginOperation(categoryUuid,
                                           Private::OperationKind::ItemTransaction);

        if (operationToken == 0)
        {
            result.status = PrivacyCategorySessionStatus::TransactionBlocked;
            return result;
        }
    }

    const ScopeExit finishOperation([this, categoryUuid, operationToken]()
    {
        QMutexLocker locker(&d->lock);
        d->finishOperation(categoryUuid, operationToken);
    });

    PrivacyPassword password = PrivacyPassword::fromUnicode(passwordText,
                                                             &result.passwordError);

    if (!password.isValid())
    {
        result.status = PrivacyCategorySessionStatus::InvalidPassword;
        return result;
    }

    PrivacyRepositorySnapshot snapshot;
    CategoryBundle bundle;

    if (!d->repository.loadSnapshot(&snapshot) ||
        !loadActiveBundle(snapshot, categoryUuid, &bundle, &result.status,
                          allowedActiveItemTransactionUuid))
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

    operation(password);
    Q_UNUSED(session); // Retain an existing session and lease for the callback.

    result.status = PrivacyCategorySessionStatus::FreshAuthenticationVerified;
    return result;
}

PrivacyCategorySessionResult
PrivacyCategorySessionCoordinator::setCategoryUnlockedThumbnailMode(
    const QString& categoryUuidText, PrivacyUnlockedThumbnailMode mode,
    const QString& passwordText)
{
    PrivacyCategorySessionResult result;
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    if (categoryUuid.isEmpty() ||
        ((mode != PrivacyUnlockedThumbnailMode::AlwaysOpaque) &&
         (mode != PrivacyUnlockedThumbnailMode::FocusedClear) &&
         (mode != PrivacyUnlockedThumbnailMode::AllClearWhileUnlocked)))
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        return result;
    }

    bool updated = false;
    const auto update = [this, &updated, categoryUuid, mode](const PrivacyPassword&)
    {
        PrivacyRepositorySnapshot snapshot;

        if (!d->repository.loadSnapshot(&snapshot))
        {
            return;
        }

        PrivacyCategory category;
        int matches = 0;

        for (const PrivacyCategory& candidate : std::as_const(snapshot.categories))
        {
            if (candidate.uuid == categoryUuid)
            {
                category = candidate;
                ++matches;
            }
        }

        if ((matches != 1) || !category.isValid() ||
            (category.lifecycleState != PrivacyCategoryLifecycleState::Active))
        {
            return;
        }

        const PrivacyUnlockedThumbnailMode previous =
            category.unlockedThumbnailMode;

        if (previous == mode)
        {
            updated = true;
            return;
        }

        const bool revealsMore =
            (static_cast<int>(mode) > static_cast<int>(previous));

        if (revealsMore)
        {
            // Publish a more revealing policy only after it is durable. A
            // runtime failure is compensated back to the prior durable mode.

            if (!d->repository.setCategoryUnlockedThumbnailMode(categoryUuid, mode))
            {
                return;
            }

            if (!d->runtime.setCategoryUnlockedThumbnailMode(
                    categoryUuid, mode, true))
            {
                d->repository.setCategoryUnlockedThumbnailMode(categoryUuid,
                                                                previous);
                return;
            }
        }
        else
        {
            // Hide immediately, then persist. Restore runtime if persistence
            // fails so the current session and restart behavior still agree.

            if (!d->runtime.setCategoryUnlockedThumbnailMode(
                    categoryUuid, mode, true))
            {
                return;
            }

            if (!d->repository.setCategoryUnlockedThumbnailMode(categoryUuid, mode))
            {
                d->runtime.setCategoryUnlockedThumbnailMode(categoryUuid,
                                                             previous, true);
                return;
            }
        }

        updated = true;
    };

    if (ownsSecret(categoryUuid))
    {
        const PrivacyCategoryOperationStatus operation =
            runWithUnlockedSecret(categoryUuid, update);

        if (operation != PrivacyCategoryOperationStatus::Completed)
        {
            result.status = (operation == PrivacyCategoryOperationStatus::TransactionBlocked)
                          ? PrivacyCategorySessionStatus::TransactionBlocked
                          : PrivacyCategorySessionStatus::CategoryLocked;
            return result;
        }
    }
    else
    {
        result = runWithFreshlyAuthenticatedSecret(categoryUuid, passwordText, update);

        if (!result.succeeded())
        {
            return result;
        }
    }

    result.status = updated ? PrivacyCategorySessionStatus::SettingsUpdated
                            : PrivacyCategorySessionStatus::SettingsUpdateFailed;
    return result;
}

PrivacyCategorySessionResult
PrivacyCategorySessionCoordinator::setCategoryTagVisibilityMode(
    const QString& categoryUuidText, PrivacyTagVisibilityMode mode,
    const QString& passwordText)
{
    PrivacyCategorySessionResult result;
    const QString categoryUuid = normalizedUuid(categoryUuidText);

    if (categoryUuid.isEmpty() ||
        ((mode != PrivacyTagVisibilityMode::UnlockedOnly) &&
         (mode != PrivacyTagVisibilityMode::AlwaysVisible)))
    {
        result.status = PrivacyCategorySessionStatus::InvalidRequest;
        return result;
    }

    bool updated = false;
    const auto update = [this, &updated, categoryUuid, mode](const PrivacyPassword&)
    {
        PrivacyRepositorySnapshot snapshot;

        if (!d->repository.loadSnapshot(&snapshot))
        {
            return;
        }

        PrivacyCategory category;
        int matches = 0;

        for (const PrivacyCategory& candidate : std::as_const(snapshot.categories))
        {
            if (candidate.uuid == categoryUuid)
            {
                category = candidate;
                ++matches;
            }
        }

        if ((matches != 1) || !category.isValid() ||
            (category.lifecycleState != PrivacyCategoryLifecycleState::Active))
        {
            return;
        }

        const PrivacyTagVisibilityMode previous = category.tagVisibilityMode;

        if (previous == mode)
        {
            updated = true;
            return;
        }

        if (mode == PrivacyTagVisibilityMode::AlwaysVisible)
        {
            // Publish the less-private policy only after it is durable. A
            // runtime failure leaves the current session more restrictive and
            // the compensating write restores the durable policy.

            if (!d->repository.setCategoryTagVisibilityMode(categoryUuid, mode))
            {
                return;
            }

            if (!d->runtime.setCategoryTagVisibilityMode(categoryUuid, mode, true))
            {
                d->repository.setCategoryTagVisibilityMode(categoryUuid, previous);
                return;
            }
        }
        else
        {
            // Hide immediately, then persist. If persistence fails, restore
            // the prior durable policy so runtime and restart behavior agree.

            if (!d->runtime.setCategoryTagVisibilityMode(categoryUuid, mode, true))
            {
                return;
            }

            if (!d->repository.setCategoryTagVisibilityMode(categoryUuid, mode))
            {
                d->runtime.setCategoryTagVisibilityMode(categoryUuid, previous, true);
                return;
            }
        }

        updated = true;
    };

    if (ownsSecret(categoryUuid))
    {
        const PrivacyCategoryOperationStatus operation =
            runWithUnlockedSecret(categoryUuid, update);

        if (operation != PrivacyCategoryOperationStatus::Completed)
        {
            result.status = (operation == PrivacyCategoryOperationStatus::TransactionBlocked)
                          ? PrivacyCategorySessionStatus::TransactionBlocked
                          : PrivacyCategorySessionStatus::CategoryLocked;
            return result;
        }
    }
    else
    {
        result = runWithFreshlyAuthenticatedSecret(categoryUuid, passwordText, update);

        if (!result.succeeded())
        {
            return result;
        }
    }

    result.status = updated ? PrivacyCategorySessionStatus::SettingsUpdated
                            : PrivacyCategorySessionStatus::SettingsUpdateFailed;
    return result;
}

bool PrivacyCategorySessionCoordinator::ownsSecret(const QString& categoryUuid) const
{
    QMutexLocker locker(&d->lock);

    return d->sessions.contains(normalizedUuid(categoryUuid));
}

} // namespace Digikam

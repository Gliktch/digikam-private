/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycasualpasswordrewrite.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#ifdef Q_OS_UNIX

#   include <fcntl.h>
#   include <sys/statvfs.h>
#   include <unistd.h>

#endif

// Local includes

#include "privacyrepository.h"

namespace Digikam
{

namespace
{

constexpr qsizetype MaximumConfigBytes = 1024 * 1024;
constexpr qlonglong SpaceMarginBytes = 64LL * 1024 * 1024;

PrivacyCasualPasswordRewriteResult failure(
    PrivacyCasualPasswordRewriteStatus status, const QString& detail,
    const QString& transactionUuid = {})
{
    PrivacyCasualPasswordRewriteResult result;
    result.status = status;
    result.detail = detail;
    result.transactionUuid = transactionUuid;
    return result;
}

QByteArray strongSentinelBytes(const QString& categoryUuid,
                               const QString& storeUuid)
{
    QJsonObject object;
    object.insert(QLatin1String("categoryUuid"), categoryUuid);
    object.insert(QLatin1String("formatVersion"), 1);
    object.insert(QLatin1String("kind"),
                  QLatin1String("digikam-private-store-sentinel-v1"));
    object.insert(QLatin1String("storeUuid"), storeUuid);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QString sha256Hex(const QByteArray& bytes)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

QString configPath(const PrivacyStorageRoot& root, const PrivacyStore& store)
{
    return QDir(root.configuredPath).filePath(
        store.cipherRelativePath + QLatin1String("/gocryptfs.conf"));
}

QString backupRelativeDirectory(const QString& storeUuid,
                                const QString& transactionUuid)
{
    return QLatin1String(".digikam-private/staging/") + storeUuid +
           QLatin1String(".rewrap-") + transactionUuid;
}

QByteArray encodePayload(const QString& storeUuid,
                         const QString& backupRelativeDirectory)
{
    QJsonObject object;
    object.insert(QLatin1String("formatVersion"), 1);
    object.insert(QLatin1String("storeUuid"), storeUuid);
    object.insert(QLatin1String("backupRelativeDirectory"),
                  backupRelativeDirectory);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool readConfigBytes(const QString& path, QByteArray* const bytes)
{
    if (!bytes)
    {
        return false;
    }

    const QFileInfo info(path);

    if (!info.isFile() || info.isSymLink() || (info.size() <= 0) ||
        (info.size() > MaximumConfigBytes))
    {
        return false;
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    *bytes = file.readAll();
    return ((bytes->size() == info.size()) && !bytes->isEmpty());
}

bool copyConfigBackup(const QString& source, const QString& backupPath)
{
    const QFileInfo sourceInfo(source);

    if (!sourceInfo.isFile() || sourceInfo.isSymLink() ||
        !QDir().mkpath(QFileInfo(backupPath).absolutePath()))
    {
        return false;
    }

    QFile sourceFile(source);
    QFile backup(backupPath);

    if (!sourceFile.open(QIODevice::ReadOnly) ||
        !backup.open(QIODevice::WriteOnly | QIODevice::NewOnly))
    {
        return false;
    }

    const QByteArray bytes = sourceFile.readAll();

    if ((bytes.size() != sourceInfo.size()) ||
        (backup.write(bytes) != bytes.size()) || !backup.flush())
    {
        return false;
    }

    backup.close();
    return QFile::setPermissions(backupPath,
                                 QFileDevice::ReadOwner |
                                 QFileDevice::WriteOwner);
}

bool removeDirectory(const QString& path)
{
    return (!QFileInfo::exists(path) || QDir(path).removeRecursively());
}

bool fsyncFile(const QString& path)
{
#ifdef Q_OS_UNIX
    const int descriptor = ::open(QFile::encodeName(path).constData(),
                                  O_RDONLY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool synced = (::fsync(descriptor) == 0);
    ::close(descriptor);
    return synced;
#else
    Q_UNUSED(path);
    return true;
#endif
}

bool fsyncDirectory(const QString& path)
{
#ifdef Q_OS_UNIX
    const int descriptor = ::open(QFile::encodeName(path).constData(),
                                  O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool synced = (::fsync(descriptor) == 0);
    ::close(descriptor);
    return synced;
#else
    Q_UNUSED(path);
    return true;
#endif
}

QByteArray hashFile(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    if (!hash.addData(&file))
    {
        return {};
    }

    return hash.result();
}

} // namespace

bool PrivacyCoreDbCasualPasswordRewritePersistence::loadSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadSnapshot(snapshot);
}

bool PrivacyCoreDbCasualPasswordRewritePersistence::beginRewrap(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginPasswordRewrap(transaction, journal);
}

bool PrivacyCoreDbCasualPasswordRewritePersistence::publishRewrap(
    const QString& categoryUuid, qlonglong categoryGeneration,
    const PrivacyCredential& credential, const QString& storeUuid,
    qlonglong storeGeneration, const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState, qlonglong expectedGeneration)
{
    return PrivacyRepository().publishPasswordRewrap(
        categoryUuid, categoryGeneration, credential, storeUuid,
        storeGeneration, transaction, expectedState, expectedGeneration);
}

bool PrivacyCoreDbCasualPasswordRewritePersistence::
    compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration)
{
    return PrivacyRepository().compareAndUpdateTransaction(
        transaction, expectedState, expectedGeneration);
}

bool PrivacyCoreDbCasualPasswordRewritePersistence::
    updateContainerCredentialGeneration(
        const QString& containerUuid, qlonglong expectedGeneration,
        qlonglong generation)
{
    return PrivacyRepository().updateContainerCredentialGeneration(
        containerUuid, generation, expectedGeneration);
}

PrivacyCasualPasswordRewriteEngine::PrivacyCasualPasswordRewriteEngine(
    PrivacyCasualPasswordRewritePersistence& persistence,
    PrivacyCategoryStoreBackend& storeBackend,
    PrivacyCasualArchiveEngine& archiveEngine)
    : m_persistence(persistence),
      m_storeBackend(storeBackend),
      m_archiveEngine(archiveEngine)
{
}

qlonglong PrivacyCasualPasswordRewriteEngine::requiredSpaceForLargestArchive(
    qlonglong largestArchiveBytes)
{
    if (largestArchiveBytes <= 0)
    {
        return 0;
    }

    return (largestArchiveBytes * 2) + SpaceMarginBytes;
}

PrivacyCasualPasswordRewriteSpaceCheck
PrivacyCasualPasswordRewriteEngine::checkSpace(
    const QString& categoryUuid) const
{
    PrivacyCasualPasswordRewriteSpaceCheck check;
    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        check.detail = QStringLiteral(
            "the privacy catalogue could not be read");
        return check;
    }

    Bundle bundle;
    PrivacyCasualPasswordRewriteResult bundleResult;

    if (!loadBundle(snapshot, categoryUuid, &bundle, &bundleResult))
    {
        check.detail = bundleResult.detail;
        return check;
    }

    const QList<PendingContainer> containers =
        pendingContainers(snapshot, bundle);

    if (containers.isEmpty())
    {
        check.valid = true;
        check.requiredBytes = 0;
        check.availableBytes = 0;
        check.insufficient = false;
        return check;
    }

    QString largestDirectory;
    qlonglong largestSize = -1;

    for (const PendingContainer& pending : containers)
    {
        const PrivacyStorageRoot* publicRoot = nullptr;

        for (const PrivacyStorageRoot& candidate :
             std::as_const(snapshot.storageRoots))
        {
            if (candidate.uuid == pending.container.rootUuid)
            {
                publicRoot = &candidate;
                break;
            }
        }

        if (!publicRoot)
        {
            continue;
        }

        const QString archivePath = QDir(
            publicRoot->configuredPath).filePath(
                pending.container.objectRelativePath);
        const QFileInfo info(archivePath);

        if (!info.isFile())
        {
            check.detail = QStringLiteral(
                "a pending Casual archive is missing");
            return check;
        }

        if (info.size() > largestSize)
        {
            largestSize = info.size();
            largestDirectory = info.absolutePath();
        }
    }

    if (largestSize < 0)
    {
        check.detail = QStringLiteral(
            "no pending Casual archives were found");
        return check;
    }

    check.largestArchiveBytes = largestSize;
    check.requiredBytes =
        requiredSpaceForLargestArchive(largestSize);

#ifdef Q_OS_UNIX
    struct statvfs volume = {};

    if (::statvfs(QFile::encodeName(largestDirectory).constData(),
                  &volume) != 0)
    {
        check.detail = QStringLiteral(
            "the archive volume could not be queried");
        return check;
    }

    check.availableBytes =
        static_cast<qlonglong>(volume.f_bavail) *
        static_cast<qlonglong>(volume.f_frsize);
#else
    check.availableBytes = -1;
#endif

    check.valid = true;
    check.insufficient =
        (check.availableBytes >= 0) &&
        (check.availableBytes < check.requiredBytes);
    return check;
}

bool PrivacyCasualPasswordRewriteEngine::loadBundle(
    const PrivacyRepositorySnapshot& snapshot, const QString& categoryUuid,
    Bundle* const bundle,
    PrivacyCasualPasswordRewriteResult* const result) const
{
    if (!bundle || !result)
    {
        return false;
    }

    int categoryCount = 0;
    PrivacyCategory category;

    for (const PrivacyCategory& candidate : snapshot.categories)
    {
        if (candidate.uuid == categoryUuid)
        {
            category = candidate;
            ++categoryCount;
        }
    }

    if ((categoryCount != 1) || !category.isValid() ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (category.backend != PrivacyBackend::Casual) ||
        (category.currentCredentialGeneration < 0))
    {
        *result = failure(
            PrivacyCasualPasswordRewriteStatus::InvalidRequest,
            QStringLiteral("the Casual category is not active"));
        return false;
    }

    int credentialCount = 0;
    PrivacyCredential credential;

    for (const PrivacyCredential& candidate : snapshot.credentials)
    {
        if ((candidate.categoryUuid == categoryUuid) &&
            (candidate.generation == category.currentCredentialGeneration))
        {
            credential = candidate;
            ++credentialCount;
        }
    }

    int storeCount = 0;
    PrivacyStore store;

    for (const PrivacyStore& candidate : snapshot.stores)
    {
        if (candidate.categoryUuid == categoryUuid)
        {
            store = candidate;
            ++storeCount;
        }
    }

    int rootCount = 0;
    PrivacyStorageRoot root;

    for (const PrivacyStorageRoot& candidate : snapshot.storageRoots)
    {
        if (candidate.uuid == store.rootUuid)
        {
            root = candidate;
            ++rootCount;
        }
    }

    if ((credentialCount != 1) || !credential.isValid() ||
        (credential.envelopeHashAlgorithm != QLatin1String("sha256")) ||
        (sha256Hex(credential.envelopeBlob) != credential.envelopeHash) ||
        (storeCount != 1) || !store.isValid() ||
        (store.lifecycleState != PrivacyStoreLifecycleState::Active) ||
        (store.configGeneration != category.currentCredentialGeneration) ||
        (rootCount != 1) || !root.isValid() ||
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot))
    {
        *result = failure(
            PrivacyCasualPasswordRewriteStatus::StoreFailure,
            QStringLiteral("the Casual category store is incomplete"));
        return false;
    }

    bundle->category = category;
    bundle->credential = credential;
    bundle->store = store;
    bundle->root = root;
    return true;
}

QList<PrivacyCasualPasswordRewriteEngine::PendingContainer>
PrivacyCasualPasswordRewriteEngine::pendingContainers(
    const PrivacyRepositorySnapshot& snapshot, const Bundle& bundle) const
{
    QList<PendingContainer> pending;

    for (const PrivacyContainer& container : snapshot.containers)
    {
        if ((container.kind != PrivacyContainerKind::CasualArchive) ||
            (container.credentialGeneration !=
             bundle.category.currentCredentialGeneration))
        {
            continue;
        }

        const PrivacyItem* item = nullptr;

        for (const PrivacyItem& candidate : snapshot.items)
        {
            if (candidate.uuid == container.itemUuid)
            {
                item = &candidate;
                break;
            }
        }

        if (!item || (item->categoryUuid != bundle.category.uuid))
        {
            continue;
        }

        PendingContainer pendingContainer;
        pendingContainer.container = container;
        pendingContainer.item = *item;

        for (const PrivacyAsset& asset : snapshot.assets)
        {
            if (asset.itemUuid == item->uuid)
            {
                pendingContainer.assets << asset;
            }
        }

        pending << pendingContainer;
    }

    return pending;
}

PrivacyCasualPasswordRewriteResult
PrivacyCasualPasswordRewriteEngine::rewrap(
    const QString& categoryUuid, const PrivacyPassword& oldPassword,
    const PrivacyPassword& newPassword)
{
    if (!oldPassword.isValid() || !newPassword.isValid())
    {
        return failure(PrivacyCasualPasswordRewriteStatus::InvalidRequest,
                       QStringLiteral("both passwords are required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the privacy catalogue could not be read"));
    }

    Bundle bundle;
    PrivacyCasualPasswordRewriteResult bundleResult;

    if (!loadBundle(snapshot, categoryUuid, &bundle, &bundleResult))
    {
        return bundleResult;
    }

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.isActive() && (candidate.categoryUuid == categoryUuid))
        {
            return failure(PrivacyCasualPasswordRewriteStatus::AlreadyActive,
                           QStringLiteral("a privacy transaction is already active"),
                           candidate.uuid);
        }
    }

    const QString transactionUuid =
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString backupRelative =
        backupRelativeDirectory(bundle.store.uuid, transactionUuid);
    PrivacyTransaction transaction;
    transaction.uuid = transactionUuid;
    transaction.categoryUuid = categoryUuid;
    transaction.type = PrivacyTransactionType::ChangePassword;
    transaction.state = PrivacyTransactionState::Created;
    transaction.generation = 0;
    transaction.fromCredentialGeneration =
        bundle.category.currentCredentialGeneration;
    transaction.toCredentialGeneration =
        transaction.fromCredentialGeneration + 1;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = encodePayload(bundle.store.uuid, backupRelative);
    transaction.createdAt = now;
    transaction.updatedAt = now;
    PrivacyTransactionJournal journal;
    journal.transactionUuid = transactionUuid;
    journal.rootUuid = bundle.root.uuid;
    journal.journalRelativePath =
        PrivacyTransactionJournalCodec::relativeJournalPath(transactionUuid);
    journal.journalFormatVersion =
        PrivacyTransactionJournalCodec::FormatVersion;
    journal.stage = static_cast<int>(PrivacyJournalStage::Created);
    journal.updatedAt = now;

    if (!m_persistence.beginRewrap(transaction, journal))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the rewrite transaction could not be begun"));
    }

    return runRewrap(bundle, transactionUuid, oldPassword, newPassword);
}

PrivacyCasualPasswordRewriteResult
PrivacyCasualPasswordRewriteEngine::recover(
    const QString& categoryUuid, const PrivacyPassword& oldPassword,
    const PrivacyPassword& newPassword)
{
    if (!oldPassword.isValid() || !newPassword.isValid())
    {
        return failure(PrivacyCasualPasswordRewriteStatus::InvalidRequest,
                       QStringLiteral("both passwords are required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the privacy catalogue could not be read"));
    }

    const PrivacyTransaction* active = nullptr;

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.isActive() &&
            (candidate.type == PrivacyTransactionType::ChangePassword) &&
            (candidate.categoryUuid == categoryUuid))
        {
            if (active)
            {
                return failure(PrivacyCasualPasswordRewriteStatus::AlreadyActive,
                               QStringLiteral("multiple rewrite transactions are active"));
            }

            active = &candidate;
        }
    }

    if (!active)
    {
        return failure(PrivacyCasualPasswordRewriteStatus::RecoveryRequired,
                       QStringLiteral("no pending rewrite transaction exists"));
    }

    Bundle bundle;
    PrivacyCasualPasswordRewriteResult bundleResult;

    if (!loadBundle(snapshot, categoryUuid, &bundle, &bundleResult))
    {
        return bundleResult;
    }

    return runRewrap(bundle, active->uuid, oldPassword, newPassword);
}

PrivacyCasualPasswordRewriteResult
PrivacyCasualPasswordRewriteEngine::runRewrap(
    const Bundle& bundle, const QString& transactionUuid,
    const PrivacyPassword& oldPassword,
    const PrivacyPassword& newPassword)
{
    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the privacy catalogue could not be read"));
    }

    const PrivacyTransaction* pending = nullptr;

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if ((candidate.uuid == transactionUuid) &&
            (candidate.type == PrivacyTransactionType::ChangePassword) &&
            (candidate.categoryUuid == bundle.category.uuid))
        {
            pending = &candidate;
            break;
        }
    }

    if (!pending)
    {
        return failure(PrivacyCasualPasswordRewriteStatus::RecoveryRequired,
                       QStringLiteral("the rewrite transaction is missing"));
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString backupRelative =
        backupRelativeDirectory(bundle.store.uuid, transactionUuid);
    const QString backupDirectory = QDir(bundle.root.configuredPath).filePath(
        backupRelative);
    const QString backupPath = backupDirectory +
                               QLatin1String("/gocryptfs.conf");
    const QString currentConfigPath = configPath(bundle.root, bundle.store);
    const QList<PendingContainer> containersToRewrite =
        pendingContainers(snapshot, bundle);

    if (pending->state == PrivacyTransactionState::Created)
    {
        PrivacyTransaction applying = *pending;
        applying.state = PrivacyTransactionState::Applying;
        applying.generation = 1;
        applying.payloadData = encodePayload(bundle.store.uuid, backupRelative);
        applying.updatedAt = now;

        if (!m_persistence.compareAndUpdateTransaction(
                applying, PrivacyTransactionState::Created, 0))
        {
            return failure(PrivacyCasualPasswordRewriteStatus::JournalFailure,
                           QStringLiteral("the rewrite journal could not advance"),
                           transactionUuid);
        }
    }

    for (const PendingContainer& pendingContainer : containersToRewrite)
    {
        const PrivacyStorageRoot* publicRoot = nullptr;

        for (const PrivacyStorageRoot& candidate : snapshot.storageRoots)
        {
            if (candidate.uuid == pendingContainer.container.rootUuid)
            {
                publicRoot = &candidate;
                break;
            }
        }

        if (!publicRoot)
        {
            return failure(PrivacyCasualPasswordRewriteStatus::ArchiveFailure,
                           QStringLiteral("the public archive root is missing"),
                           transactionUuid);
        }

        const QString archivePath = QDir(publicRoot->configuredPath).filePath(
            pendingContainer.container.objectRelativePath);
        const QByteArray oldHash = hashFile(archivePath);

        if (oldHash.isEmpty())
        {
            return failure(PrivacyCasualPasswordRewriteStatus::ArchiveFailure,
                           QStringLiteral("a Casual archive is missing"),
                           transactionUuid);
        }

        PrivacyCasualArchiveRequest request;
        request.finalArchivePath = archivePath;
        request.categoryUuid = bundle.category.uuid;
        request.containerUuid = pendingContainer.container.uuid;
        request.itemUuid = pendingContainer.item.uuid;
        PrivacyCasualArchiveError archiveError = PrivacyCasualArchiveError::None;
        PrivacyCasualArchiveStage stage = m_archiveEngine.rewriteArchive(
            request, oldPassword, newPassword, {}, &archiveError);

        if (!stage.isValid() ||
            !m_archiveEngine.publishReplacement(&stage, oldHash,
                                                &archiveError) ||
            !m_persistence.updateContainerCredentialGeneration(
                pendingContainer.container.uuid,
                bundle.category.currentCredentialGeneration,
                bundle.category.currentCredentialGeneration + 1))
        {
            return failure(PrivacyCasualPasswordRewriteStatus::ArchiveFailure,
                           QStringLiteral("a Casual archive could not be rewritten"),
                           transactionUuid);
        }
    }

    PrivacyRepositorySnapshot refreshed;

    if (!m_persistence.loadSnapshot(&refreshed))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the privacy catalogue could not be re-read"));
    }

    const PrivacyTransaction* current = nullptr;

    for (const PrivacyTransaction& candidate : refreshed.transactions)
    {
        if (candidate.uuid == transactionUuid)
        {
            current = &candidate;
            break;
        }
    }

    if (!current)
    {
        return failure(PrivacyCasualPasswordRewriteStatus::RecoveryRequired,
                       QStringLiteral("the rewrite transaction disappeared"));
    }

    QByteArray currentConfig;
    QByteArray backupConfig;
    const bool currentReadable = readConfigBytes(currentConfigPath,
                                                 &currentConfig);
    const bool backupReadable = readConfigBytes(backupPath, &backupConfig);

    if (currentReadable && backupReadable && (currentConfig != backupConfig))
    {
        // The category store was already rewrapped; publish the pending
        // credential.
        PrivacyCasualPasswordRewriteResult completed = complete(
            bundle, currentConfig, *current, backupRelative,
            PrivacyTransactionState::Applying, 1);

        if (completed.succeeded())
        {
            removeDirectory(backupDirectory);
        }

        return completed;
    }

    if (!QFileInfo::exists(backupPath))
    {
        if (!copyConfigBackup(currentConfigPath, backupPath) ||
            !fsyncFile(backupPath) || !fsyncDirectory(backupDirectory))
        {
            return failure(
                PrivacyCasualPasswordRewriteStatus::JournalFailure,
                QStringLiteral("the config backup could not be written"),
                transactionUuid);
        }
    }

    const PrivacyGocryptfsEnvelope envelope =
        PrivacyGocryptfsEnvelope::fromOpaqueConfig(
            bundle.credential.envelopeFormat, bundle.credential.envelopeBlob,
            nullptr);

    if (!envelope.isValid())
    {
        return failure(PrivacyCasualPasswordRewriteStatus::StoreFailure,
                       QStringLiteral("the stored gocryptfs envelope is invalid"),
                       transactionUuid);
    }

    QByteArray newConfig;
    PrivacyGocryptfsError storeError = PrivacyGocryptfsError::None;

    if (!m_storeBackend.rewrapPassword(
            bundle.root, bundle.store, envelope, oldPassword, newPassword,
            strongSentinelBytes(bundle.category.uuid, bundle.store.uuid),
            &newConfig, &storeError))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::StoreFailure,
                       QStringLiteral("the category store password rewrap failed"),
                       transactionUuid);
    }

    PrivacyRepositorySnapshot finalSnapshot;

    if (!m_persistence.loadSnapshot(&finalSnapshot))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the privacy catalogue could not be re-read"));
    }

    const PrivacyTransaction* finalTransaction = nullptr;

    for (const PrivacyTransaction& candidate : finalSnapshot.transactions)
    {
        if (candidate.uuid == transactionUuid)
        {
            finalTransaction = &candidate;
            break;
        }
    }

    if (!finalTransaction)
    {
        return failure(PrivacyCasualPasswordRewriteStatus::RecoveryRequired,
                       QStringLiteral("the rewrite transaction disappeared"));
    }

    PrivacyCasualPasswordRewriteResult completed = complete(
        bundle, newConfig, *finalTransaction, backupRelative,
        PrivacyTransactionState::Applying, 1);

    if (completed.succeeded())
    {
        removeDirectory(backupDirectory);
    }

    return completed;
}

PrivacyCasualPasswordRewriteResult
PrivacyCasualPasswordRewriteEngine::complete(
    const Bundle& bundle, const QByteArray& newConfig,
    const PrivacyTransaction& transaction, const QString& backupRelative,
    PrivacyTransactionState expectedState, qlonglong expectedGeneration)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const qlonglong generation = bundle.category.currentCredentialGeneration + 1;
    PrivacyCredential credential;
    credential.categoryUuid = bundle.category.uuid;
    credential.generation = generation;
    credential.encodingVersion = QLatin1String("utf8-nfc-v1");
    credential.envelopeFormat = QLatin1String("gocryptfs-config-v2");
    credential.envelopeBlob = newConfig;
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash = sha256Hex(newConfig);
    credential.createdAt = now;

    if (!credential.isValid())
    {
        return failure(PrivacyCasualPasswordRewriteStatus::StoreFailure,
                       QStringLiteral("the rewritten credential is invalid"),
                       transaction.uuid);
    }

    PrivacyCategory category = bundle.category;
    category.currentCredentialGeneration = generation;
    PrivacyStore store = bundle.store;
    store.configGeneration = generation;
    PrivacyTransaction completed = transaction;
    completed.state = PrivacyTransactionState::Complete;
    completed.generation = transaction.generation + 1;
    completed.payloadData = encodePayload(bundle.store.uuid, backupRelative);
    completed.updatedAt = now;

    if (!m_persistence.publishRewrap(
            category.uuid, category.currentCredentialGeneration,
            credential, store.uuid, store.configGeneration, completed,
            expectedState, expectedGeneration))
    {
        return failure(PrivacyCasualPasswordRewriteStatus::PersistenceFailure,
                       QStringLiteral("the rewritten credential could not be published"),
                       transaction.uuid);
    }

    PrivacyCasualPasswordRewriteResult result;
    result.status = PrivacyCasualPasswordRewriteStatus::Rewritten;
    result.transactionUuid = transaction.uuid;
    return result;
}

} // namespace Digikam

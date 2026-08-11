/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacystrongpasswordrewrap.h"

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
#   include <unistd.h>

#endif

// Local includes

#include "privacyrepository.h"

namespace Digikam
{

namespace
{

constexpr qsizetype MaximumConfigBytes = 1024 * 1024;

PrivacyStrongPasswordRewrapResult failure(
    PrivacyStrongPasswordRewrapStatus status, const QString& detail,
    const QString& transactionUuid = {})
{
    PrivacyStrongPasswordRewrapResult result;
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

} // namespace

bool PrivacyCoreDbStrongPasswordRewrapPersistence::loadSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadSnapshot(snapshot);
}

bool PrivacyCoreDbStrongPasswordRewrapPersistence::beginRewrap(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginPasswordRewrap(transaction, journal);
}

bool PrivacyCoreDbStrongPasswordRewrapPersistence::publishRewrap(
    const QString& categoryUuid, qlonglong categoryGeneration,
    const PrivacyCredential& credential, const QString& storeUuid,
    qlonglong storeGeneration, const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState, qlonglong expectedGeneration)
{
    return PrivacyRepository().publishPasswordRewrap(
        categoryUuid, categoryGeneration, credential, storeUuid,
        storeGeneration, transaction, expectedState, expectedGeneration);
}

bool PrivacyCoreDbStrongPasswordRewrapPersistence::
    compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration)
{
    return PrivacyRepository().compareAndUpdateTransaction(
        transaction, expectedState, expectedGeneration);
}

PrivacyStrongPasswordRewrapEngine::PrivacyStrongPasswordRewrapEngine(
    PrivacyStrongPasswordRewrapPersistence& persistence,
    PrivacyCategoryStoreBackend& storeBackend)
    : m_persistence(persistence),
      m_storeBackend(storeBackend)
{
}

bool PrivacyStrongPasswordRewrapEngine::loadBundle(
    const PrivacyRepositorySnapshot& snapshot, const QString& categoryUuid,
    Bundle* const bundle, PrivacyStrongPasswordRewrapResult* const result) const
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
        (category.backend != PrivacyBackend::Strong) ||
        (category.currentCredentialGeneration < 0))
    {
        *result = failure(
            PrivacyStrongPasswordRewrapStatus::InvalidRequest,
            QStringLiteral("the Strong category is not active"));
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
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot) ||
        (root.uuid != store.rootUuid))
    {
        *result = failure(
            PrivacyStrongPasswordRewrapStatus::StoreFailure,
            QStringLiteral("the Strong category store is incomplete"));
        return false;
    }

    bundle->category = category;
    bundle->credential = credential;
    bundle->store = store;
    bundle->root = root;
    return true;
}

PrivacyStrongPasswordRewrapResult PrivacyStrongPasswordRewrapEngine::rewrap(
    const QString& categoryUuid, const PrivacyPassword& oldPassword,
    const PrivacyPassword& newPassword)
{
    if (!oldPassword.isValid() || !newPassword.isValid())
    {
        return failure(PrivacyStrongPasswordRewrapStatus::InvalidRequest,
                       QStringLiteral("both passwords are required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return failure(PrivacyStrongPasswordRewrapStatus::PersistenceFailure,
                       QStringLiteral("the privacy catalogue could not be read"));
    }

    Bundle bundle;
    PrivacyStrongPasswordRewrapResult bundleResult;

    if (!loadBundle(snapshot, categoryUuid, &bundle, &bundleResult))
    {
        return bundleResult;
    }

    for (const PrivacyTransaction& candidate : snapshot.transactions)
    {
        if (candidate.isActive() && (candidate.categoryUuid == categoryUuid))
        {
            return failure(PrivacyStrongPasswordRewrapStatus::AlreadyActive,
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
        return failure(PrivacyStrongPasswordRewrapStatus::PersistenceFailure,
                       QStringLiteral("the rewrap transaction could not be begun"));
    }

    return runRewrap(bundle, transactionUuid, oldPassword, newPassword);
}

PrivacyStrongPasswordRewrapResult PrivacyStrongPasswordRewrapEngine::recover(
    const QString& categoryUuid, const PrivacyPassword& oldPassword,
    const PrivacyPassword& newPassword)
{
    if (!oldPassword.isValid() || !newPassword.isValid())
    {
        return failure(PrivacyStrongPasswordRewrapStatus::InvalidRequest,
                       QStringLiteral("both passwords are required"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return failure(PrivacyStrongPasswordRewrapStatus::PersistenceFailure,
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
                return failure(PrivacyStrongPasswordRewrapStatus::AlreadyActive,
                               QStringLiteral("multiple rewrap transactions are active"));
            }

            active = &candidate;
        }
    }

    if (!active)
    {
        return failure(PrivacyStrongPasswordRewrapStatus::RecoveryRequired,
                       QStringLiteral("no pending rewrap transaction exists"));
    }

    Bundle bundle;
    PrivacyStrongPasswordRewrapResult bundleResult;

    if (!loadBundle(snapshot, categoryUuid, &bundle, &bundleResult))
    {
        return bundleResult;
    }

    const QString currentConfigPath = configPath(bundle.root, bundle.store);
    const QString backupRelative =
        backupRelativeDirectory(bundle.store.uuid, active->uuid);
    const QString backupPath = QDir(bundle.root.configuredPath).filePath(
        backupRelative + QLatin1String("/gocryptfs.conf"));
    QByteArray currentConfig;
    QByteArray backupConfig;
    const bool currentReadable = readConfigBytes(currentConfigPath,
                                                 &currentConfig);
    const bool backupReadable = readConfigBytes(backupPath, &backupConfig);

    if ((active->state == PrivacyTransactionState::Applying) &&
        currentReadable && backupReadable &&
        (currentConfig != backupConfig))
    {
        // The config has already been rewrapped; verify the new password and
        // publish the pending credential.
        PrivacyStrongPasswordRewrapResult completed =
            complete(bundle, currentConfig, *active, backupRelative,
                     PrivacyTransactionState::Applying, 1);

        if (completed.succeeded())
        {
            removeDirectory(QDir(bundle.root.configuredPath).filePath(
                backupRelative));
        }

        return completed;
    }

    return runRewrap(bundle, active->uuid, oldPassword, newPassword);
}

PrivacyStrongPasswordRewrapResult PrivacyStrongPasswordRewrapEngine::runRewrap(
    const Bundle& bundle, const QString& transactionUuid,
    const PrivacyPassword& oldPassword,
    const PrivacyPassword& newPassword)
{
    PrivacyRepositorySnapshot snapshot;

    if (!m_persistence.loadSnapshot(&snapshot))
    {
        return failure(PrivacyStrongPasswordRewrapStatus::PersistenceFailure,
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
        return failure(PrivacyStrongPasswordRewrapStatus::RecoveryRequired,
                       QStringLiteral("the rewrap transaction is missing"));
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString backupRelative =
        backupRelativeDirectory(bundle.store.uuid, transactionUuid);
    const QString backupDirectory = QDir(bundle.root.configuredPath).filePath(
        backupRelative);
    const QString backupPath = backupDirectory +
                               QLatin1String("/gocryptfs.conf");
    const QString currentConfigPath = configPath(bundle.root, bundle.store);

    if (pending->state == PrivacyTransactionState::Created)
    {
        if (!copyConfigBackup(currentConfigPath, backupPath) ||
            !fsyncFile(backupPath) || !fsyncDirectory(backupDirectory))
        {
            return failure(PrivacyStrongPasswordRewrapStatus::JournalFailure,
                           QStringLiteral("the config backup could not be written"),
                           transactionUuid);
        }

        PrivacyTransaction applying = *pending;
        applying.state = PrivacyTransactionState::Applying;
        applying.generation = 1;
        applying.payloadData = encodePayload(bundle.store.uuid, backupRelative);
        applying.updatedAt = now;

        if (!m_persistence.compareAndUpdateTransaction(
                applying, PrivacyTransactionState::Created, 0))
        {
            return failure(PrivacyStrongPasswordRewrapStatus::JournalFailure,
                           QStringLiteral("the rewrap journal could not advance"),
                           transactionUuid);
        }
    }

    const PrivacyGocryptfsEnvelope envelope =
        PrivacyGocryptfsEnvelope::fromOpaqueConfig(
            bundle.credential.envelopeFormat, bundle.credential.envelopeBlob,
            nullptr);

    if (!envelope.isValid())
    {
        return failure(PrivacyStrongPasswordRewrapStatus::StoreFailure,
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
        return failure(PrivacyStrongPasswordRewrapStatus::StoreFailure,
                       QStringLiteral("the gocryptfs password rewrap failed"),
                       transactionUuid);
    }

    PrivacyRepositorySnapshot refreshed;

    if (!m_persistence.loadSnapshot(&refreshed))
    {
        return failure(PrivacyStrongPasswordRewrapStatus::PersistenceFailure,
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
        return failure(PrivacyStrongPasswordRewrapStatus::RecoveryRequired,
                       QStringLiteral("the rewrap transaction disappeared"));
    }

    PrivacyStrongPasswordRewrapResult completed =
        complete(bundle, newConfig, *current, backupRelative,
                 PrivacyTransactionState::Applying, 1);

    if (completed.succeeded())
    {
        removeDirectory(backupDirectory);
    }

    return completed;
}

PrivacyStrongPasswordRewrapResult PrivacyStrongPasswordRewrapEngine::complete(
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
        return failure(PrivacyStrongPasswordRewrapStatus::StoreFailure,
                       QStringLiteral("the rewrapped credential is invalid"),
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
        return failure(PrivacyStrongPasswordRewrapStatus::PersistenceFailure,
                       QStringLiteral("the rewrapped credential could not be published"),
                       transaction.uuid);
    }

    PrivacyStrongPasswordRewrapResult result;
    result.status = PrivacyStrongPasswordRewrapStatus::Rewrapped;
    result.transactionUuid = transaction.uuid;
    return result;
}

} // namespace Digikam

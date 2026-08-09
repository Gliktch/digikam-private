/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyexternalcheckouttransaction.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QUuid>

// Local includes

#include "privacyrepository.h"

namespace Digikam
{

namespace
{

struct LogicalAsset
{
    int       role = 0;
    int       ordinal = -1;
    QString   publicRelativePath;
    QString   checkoutFileName;
    QDateTime modificationDate;
};

bool canonicalUuid(const QString& value)
{
    const QUuid uuid(value);
    return (!uuid.isNull() &&
            (value == uuid.toString(QUuid::WithoutBraces)));
}

bool safeRelativePath(const QString& path)
{
    return (!path.isEmpty() && !QDir::isAbsolutePath(path) &&
            !path.contains(QChar::Null) &&
            (path == QDir::cleanPath(path)) &&
            !path.startsWith(QLatin1String("../")) &&
            (path != QLatin1String("..")) &&
            (path == path.normalized(QString::NormalizationForm_C)));
}

bool exactKeys(const QJsonObject& object,
               std::initializer_list<const char*> keys)
{
    if (object.size() != static_cast<qsizetype>(keys.size()))
    {
        return false;
    }

    for (const char* const key : keys)
    {
        if (!object.contains(QLatin1String(key)))
        {
            return false;
        }
    }

    return true;
}

QByteArray sha256Bytes(const QString& algorithm, const QString& encoded)
{
    if ((algorithm != QLatin1String("sha256")) || (encoded.size() != 64))
    {
        return {};
    }

    const QByteArray bytes = QByteArray::fromHex(encoded.toLatin1());
    return (bytes.size() == 32) ? bytes : QByteArray();
}

PrivacyJournalObjectFact presentFact(qlonglong size, const QByteArray& sha256)
{
    PrivacyJournalObjectFact fact;
    fact.presence  = PrivacyJournalExpectedPresence::Present;
    fact.size      = size;
    fact.linkCount = 1;
    fact.sha256    = sha256;
    return fact;
}

bool sameFact(const PrivacyJournalObjectFact& left,
              const PrivacyJournalObjectFact& right)
{
    return ((left.presence == right.presence) &&
            (left.size == right.size) &&
            (left.linkCount == right.linkCount) &&
            (left.sha256 == right.sha256));
}

bool sameRecord(const PrivacyJournalRecord& left,
                const PrivacyJournalRecord& right)
{
    const QByteArray leftBytes = PrivacyTransactionJournalCodec::encode(left);
    return (!leftBytes.isEmpty() &&
            (leftBytes == PrivacyTransactionJournalCodec::encode(right)));
}

PrivacyJournalRecord atStage(PrivacyJournalRecord record,
                             PrivacyJournalStage stage)
{
    record.stage = stage;
    return record;
}

QByteArray encodePayload(const PrivacyJournalRecord& record,
                         const QList<LogicalAsset>& logicalAssets)
{
    const QByteArray journal = PrivacyTransactionJournalCodec::encode(record);

    if (journal.isEmpty() ||
        (record.transactionType != PrivacyTransactionType::ExternalCheckout) ||
        (logicalAssets.size() != record.assets.size()))
    {
        return {};
    }

    QJsonArray assets;
    QSet<QString> identities;

    for (const LogicalAsset& asset : logicalAssets)
    {
        if ((asset.role <= 0) || (asset.ordinal < 0) ||
            !safeRelativePath(asset.publicRelativePath) ||
            asset.checkoutFileName.isEmpty() ||
            !asset.modificationDate.isValid())
        {
            return {};
        }

        const QString identity = QStringLiteral("%1:%2")
                                     .arg(asset.role).arg(asset.ordinal);

        if (identities.contains(identity))
        {
            return {};
        }

        identities.insert(identity);
        QJsonObject object;
        object.insert(QStringLiteral("checkoutFileName"),
                      asset.checkoutFileName);
        object.insert(QStringLiteral("modificationDate"),
                      asset.modificationDate.toUTC().toString(Qt::ISODateWithMs));
        object.insert(QStringLiteral("ordinal"), asset.ordinal);
        object.insert(QStringLiteral("publicRelativePath"),
                      asset.publicRelativePath);
        object.insert(QStringLiteral("role"), asset.role);
        assets.append(object);
    }

    QJsonObject object;
    object.insert(QStringLiteral("assets"), assets);
    object.insert(QStringLiteral("formatVersion"), 1);
    object.insert(QStringLiteral("journal"),
                  QString::fromLatin1(journal.toBase64()));
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool decodePayload(const QByteArray& bytes, PrivacyJournalRecord* const record,
                   QList<LogicalAsset>* const logicalAssets)
{
    if (!record || !logicalAssets || bytes.isEmpty())
    {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);

    if ((parseError.error != QJsonParseError::NoError) ||
        !document.isObject())
    {
        return false;
    }

    const QJsonObject object = document.object();

    if (!exactKeys(object, { "assets", "formatVersion", "journal" }) ||
        (object.value(QStringLiteral("formatVersion")).toInt(-1) != 1) ||
        !object.value(QStringLiteral("assets")).isArray() ||
        !object.value(QStringLiteral("journal")).isString())
    {
        return false;
    }

    const QByteArray journal = QByteArray::fromBase64(
        object.value(QStringLiteral("journal")).toString().toLatin1(),
        QByteArray::AbortOnBase64DecodingErrors);
    PrivacyJournalError journalError = PrivacyJournalError::None;
    PrivacyJournalRecord decodedRecord;

    if (!PrivacyTransactionJournalCodec::decode(
            journal, &decodedRecord, &journalError) ||
        (decodedRecord.transactionType !=
         PrivacyTransactionType::ExternalCheckout))
    {
        return false;
    }

    QList<LogicalAsset> decodedAssets;
    QSet<QString> identities;

    for (const QJsonValue& value :
         object.value(QStringLiteral("assets")).toArray())
    {
        if (!value.isObject())
        {
            return false;
        }

        const QJsonObject assetObject = value.toObject();

        if (!exactKeys(assetObject,
                       { "checkoutFileName", "modificationDate", "ordinal",
                         "publicRelativePath", "role" }))
        {
            return false;
        }

        LogicalAsset asset;
        asset.role = assetObject.value(QStringLiteral("role")).toInt(0);
        asset.ordinal = assetObject.value(QStringLiteral("ordinal")).toInt(-1);
        asset.publicRelativePath =
            assetObject.value(QStringLiteral("publicRelativePath")).toString();
        asset.checkoutFileName =
            assetObject.value(QStringLiteral("checkoutFileName")).toString();
        asset.modificationDate = QDateTime::fromString(
            assetObject.value(QStringLiteral("modificationDate")).toString(),
            Qt::ISODateWithMs);
        const QString identity = QStringLiteral("%1:%2")
                                     .arg(asset.role).arg(asset.ordinal);

        if ((asset.role <= 0) || (asset.ordinal < 0) ||
            !safeRelativePath(asset.publicRelativePath) ||
            asset.checkoutFileName.isEmpty() ||
            !asset.modificationDate.isValid() || identities.contains(identity))
        {
            return false;
        }

        identities.insert(identity);
        decodedAssets << asset;
    }

    if (decodedAssets.size() != decodedRecord.assets.size())
    {
        return false;
    }

    *record = decodedRecord;
    *logicalAssets = decodedAssets;
    return true;
}

template <typename Value, typename Predicate>
const Value* uniqueValue(const QList<Value>& values, Predicate predicate)
{
    const Value* result = nullptr;

    for (const Value& value : values)
    {
        if (!predicate(value))
        {
            continue;
        }

        if (result)
        {
            return nullptr;
        }

        result = &value;
    }

    return result;
}

const PrivacyTransaction* transactionFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& uuid)
{
    return uniqueValue(snapshot.transactions,
                       [&uuid](const PrivacyTransaction& transaction)
                       {
                           return (transaction.uuid == uuid);
                       });
}

const PrivacyTransactionJournal* databaseJournalFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& transactionUuid,
    const QString& rootUuid)
{
    return uniqueValue(snapshot.transactionJournals,
                       [&transactionUuid, &rootUuid](
                           const PrivacyTransactionJournal& journal)
                       {
                           return ((journal.transactionUuid == transactionUuid) &&
                                   (journal.rootUuid == rootUuid));
                       });
}

QString fallbackCheckoutName(const PrivacyAsset& asset)
{
    QString extension = QFileInfo(asset.originalName).suffix();

    if (extension.size() > 16)
    {
        extension.clear();
    }

    for (const QChar character : std::as_const(extension))
    {
        if (!character.isLetterOrNumber())
        {
            extension.clear();
            break;
        }
    }

    return QStringLiteral("asset-%1-%2%3")
        .arg(asset.role).arg(asset.ordinal)
        .arg(extension.isEmpty() ? QString()
                                 : (QLatin1Char('.') + extension));
}

PrivacyExternalCheckoutResult failure(PrivacyExternalCheckoutStatus status,
                                      const QString& transactionUuid,
                                      const QString& itemUuid,
                                      const QString& detail)
{
    PrivacyExternalCheckoutResult result;
    result.status = status;
    result.transactionUuid = transactionUuid;
    result.itemUuid = itemUuid;
    result.detail = detail;
    return result;
}

} // namespace

class Q_DECL_HIDDEN PrivacyExternalCheckoutTransactionEngine::Private
{
public:

    explicit Private(PrivacyExternalCheckoutPersistence& value)
        : persistence(value)
    {
    }

    bool load(PrivacyRepositorySnapshot* const snapshot) const
    {
        return persistence.loadSnapshot(snapshot);
    }

    bool advanceFilesystemJournal(
        const PrivacyStorageRoot& root,
        const PrivacyJournalRootExpectation& expectation,
        const PrivacyJournalRecord& current,
        const PrivacyJournalRecord& next,
        QByteArray* const nextHash,
        QString* const detail) const
    {
        PrivacyJournalError error = PrivacyJournalError::None;
        std::unique_ptr<PrivacyTransactionJournalStore> store =
            PrivacyTransactionJournalStore::open(root.configuredPath,
                                                  expectation, &error, detail);

        if (!store)
        {
            return false;
        }

        PrivacyJournalLoadResult loaded = store->load(current.transactionUuid);

        if (loaded.disposition == PrivacyJournalLoadDisposition::Missing)
        {
            if ((current.stage != PrivacyJournalStage::Created) ||
                !sameRecord(current, next))
            {
                if (detail)
                {
                    *detail = QStringLiteral("missing checkout journal is not an exact Created replay");
                }

                return false;
            }

            return store->create(next, nextHash, &error, detail);
        }

        if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !loaded.authoritative || !loaded.hasRecord)
        {
            if (detail && detail->isEmpty())
            {
                *detail = loaded.detail;
            }

            return false;
        }

        if (sameRecord(loaded.record, next))
        {
            if (nextHash)
            {
                *nextHash = loaded.sha256;
            }

            return true;
        }

        if (!sameRecord(loaded.record, current))
        {
            if (detail)
            {
                *detail = QStringLiteral("checkout journal predecessor is not exact");
            }

            return false;
        }

        return store->compareAndUpdate(next, loaded.sha256, nextHash,
                                       &error, detail);
    }

    bool publishDatabaseJournal(
        const PrivacyTransactionJournal& current,
        PrivacyJournalStage nextStage,
        const QByteArray& filesystemHash) const
    {
        PrivacyTransactionJournal next = current;
        next.stage = static_cast<int>(nextStage);
        next.expectedHashAlgorithm = QLatin1String("sha256");
        next.expectedJournalHash = QString::fromLatin1(filesystemHash.toHex());
        next.updatedAt = QDateTime::currentDateTimeUtc();
        return persistence.compareAndUpdateJournal(next, current.stage);
    }

    PrivacyExternalCheckoutResult assetsResult(
        PrivacyExternalCheckoutStatus status,
        const PrivacyStorageRoot& root,
        const PrivacyJournalRecord& record,
        const QList<LogicalAsset>& logicalAssets,
        const QString& detail = QString()) const
    {
        PrivacyExternalCheckoutResult result;
        result.status = status;
        result.transactionUuid = record.transactionUuid;
        result.itemUuid = record.assets.isEmpty()
                        ? QString() : record.assets.constFirst().itemUuid;
        result.detail = detail;

        for (const LogicalAsset& logical : logicalAssets)
        {
            const auto journalIt = std::find_if(
                record.assets.cbegin(), record.assets.cend(),
                [&logical](const PrivacyJournalAsset& asset)
                {
                    return ((asset.role == logical.role) &&
                            (asset.ordinal == logical.ordinal));
                });

            if (journalIt == record.assets.cend())
            {
                return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                               record.transactionUuid, result.itemUuid,
                               QStringLiteral("checkout payload asset mapping is incomplete"));
            }

            PrivacyExternalCheckoutAsset asset;
            asset.role = logical.role;
            asset.ordinal = logical.ordinal;
            asset.logicalUrl = QUrl::fromLocalFile(
                QDir(root.configuredPath).absoluteFilePath(
                    logical.publicRelativePath));
            asset.checkoutUrl = QUrl::fromLocalFile(
                QDir(root.configuredPath).absoluteFilePath(
                    journalIt->publicRelativePath));
            result.assets << asset;
        }

        return result;
    }

public:

    PrivacyExternalCheckoutPersistence& persistence;
};

bool PrivacyExternalCheckoutResult::succeeded() const
{
    return ((status == PrivacyExternalCheckoutStatus::Ready) ||
            (status == PrivacyExternalCheckoutStatus::CompletedUnchanged));
}

bool PrivacyCoreDbExternalCheckoutPersistence::loadSnapshot(
    PrivacyRepositorySnapshot* const snapshot) const
{
    return PrivacyRepository().loadRuntimeSnapshot(snapshot);
}

bool PrivacyCoreDbExternalCheckoutPersistence::beginExternalCheckout(
    const PrivacyTransaction& transaction,
    const PrivacyTransactionJournal& journal)
{
    return PrivacyRepository().beginExternalCheckout(transaction, journal);
}

bool PrivacyCoreDbExternalCheckoutPersistence::compareAndUpdateTransaction(
    const PrivacyTransaction& transaction,
    PrivacyTransactionState expectedState, qlonglong expectedGeneration)
{
    return PrivacyRepository().compareAndUpdateTransaction(
        transaction, expectedState, expectedGeneration);
}

bool PrivacyCoreDbExternalCheckoutPersistence::compareAndUpdateJournal(
    const PrivacyTransactionJournal& journal, int expectedStage)
{
    return PrivacyRepository().compareAndUpdateTransactionJournal(
        journal, expectedStage);
}

PrivacyExternalCheckoutTransactionEngine::
    PrivacyExternalCheckoutTransactionEngine(
        PrivacyExternalCheckoutPersistence& persistence)
    : d(new Private(persistence))
{
}

PrivacyExternalCheckoutTransactionEngine::~PrivacyExternalCheckoutTransactionEngine()
{
}

PrivacyExternalCheckoutResult PrivacyExternalCheckoutTransactionEngine::create(
    const PrivacyExternalCheckoutRequest& request)
{
    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.transactionUuid) || !request.root.isValid() ||
        (request.root.uuid != request.rootExpectation.rootUuid) ||
        (request.rootExpectation.device == 0) ||
        (request.rootExpectation.inode == 0) || request.sources.isEmpty())
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       request.transactionUuid, {},
                       QStringLiteral("External Checkout request is invalid"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, {},
                       QStringLiteral("cannot load privacy state"));
    }

    const PrivacyItem* const item = uniqueValue(
        snapshot.items, [&request](const PrivacyItem& candidate)
        {
            return (candidate.imageId == request.imageId);
        });

    if (!item || (item->categoryUuid != request.categoryUuid))
    {
        return failure(PrivacyExternalCheckoutStatus::ItemUnavailable,
                       request.transactionUuid, {},
                       QStringLiteral("protected item is unavailable"));
    }

    const PrivacyCategory* const category = uniqueValue(
        snapshot.categories, [item](const PrivacyCategory& candidate)
        {
            return (candidate.uuid == item->categoryUuid);
        });
    const PrivacyContainer* const container = uniqueValue(
        snapshot.containers, [item](const PrivacyContainer& candidate)
        {
            return (candidate.itemUuid == item->uuid);
        });

    if (!category || !container ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (container->state != PrivacyContainerState::Verified) ||
        (container->rootUuid != request.root.uuid) ||
        (container->kind != PrivacyContainerKind::CasualArchive))
    {
        return failure(PrivacyExternalCheckoutStatus::ItemUnavailable,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("verified Casual protected object is unavailable"));
    }

    for (const PrivacyTransaction& transaction : snapshot.transactions)
    {
        if (transaction.isActive() &&
            ((transaction.itemUuid == item->uuid) ||
             (transaction.categoryUuid == item->categoryUuid)))
        {
            return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("another privacy transaction is active"));
        }
    }

    QList<PrivacyAsset> assets;

    for (const PrivacyAsset& asset : snapshot.assets)
    {
        if (asset.itemUuid == item->uuid)
        {
            assets << asset;
        }
    }

    std::sort(assets.begin(), assets.end(),
              [](const PrivacyAsset& left, const PrivacyAsset& right)
              {
                  return ((left.role < right.role) ||
                          ((left.role == right.role) &&
                           (left.ordinal < right.ordinal)));
              });

    if (assets.isEmpty() || (assets.size() != request.sources.size()))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("complete associated-asset sources are required"));
    }

    QSet<QString> sourceIdentities;

    for (const PrivacyExternalCheckoutAssetSource& source : request.sources)
    {
        const QString identity = QStringLiteral("%1:%2")
                                     .arg(source.role).arg(source.ordinal);

        if ((source.role <= 0) || (source.ordinal < 0) || !source.producer ||
            sourceIdentities.contains(identity))
        {
            return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("checkout source set is invalid"));
        }

        sourceIdentities.insert(identity);
    }

    PrivacyJournalRecord created;
    created.transactionUuid = request.transactionUuid;
    created.categoryUuid = request.categoryUuid;
    created.rootUuid = request.root.uuid;
    created.rootDevice = request.rootExpectation.device;
    created.rootInode = request.rootExpectation.inode;
    created.rootIdentitySha256 = request.rootExpectation.identitySha256;
    created.transactionType = PrivacyTransactionType::ExternalCheckout;
    created.generation = item->generation;
    created.credentialGeneration = container->credentialGeneration;
    created.fromCredentialGeneration = container->credentialGeneration;
    created.toCredentialGeneration = container->credentialGeneration;
    created.stage = PrivacyJournalStage::Created;
    QList<LogicalAsset> logicalAssets;
    QSet<QString> checkoutNames;
    const PrivacyJournalObjectFact containerFact = presentFact(
        container->protectedSize,
        sha256Bytes(container->protectedHashAlgorithm,
                    container->protectedHash));

    if (containerFact.sha256.isEmpty())
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("protected-object identity is invalid"));
    }

    for (const PrivacyAsset& asset : std::as_const(assets))
    {
        const QString identity = QStringLiteral("%1:%2")
                                     .arg(asset.role).arg(asset.ordinal);

        if (!sourceIdentities.contains(identity) ||
            (asset.containerUuid != container->uuid) ||
            (asset.publicRootUuid != request.root.uuid))
        {
            return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("checkout source does not match protected asset set"));
        }

        QString fileName = asset.originalName.normalized(
            QString::NormalizationForm_C);
        QString relative =
            PrivacyTransactionJournalStore::relativeCheckoutPath(
                request.transactionUuid, fileName);

        if (relative.isEmpty() || checkoutNames.contains(fileName.toCaseFolded()))
        {
            fileName = fallbackCheckoutName(asset);
            relative = PrivacyTransactionJournalStore::relativeCheckoutPath(
                request.transactionUuid, fileName);
        }

        if (relative.isEmpty() || checkoutNames.contains(fileName.toCaseFolded()))
        {
            return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("asset names cannot form a safe checkout"));
        }

        checkoutNames.insert(fileName.toCaseFolded());
        const QByteArray originalHash = sha256Bytes(
            asset.hashAlgorithm, asset.originalHash);

        if (originalHash.isEmpty())
        {
            return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("asset baseline identity is invalid"));
        }

        PrivacyJournalAsset journalAsset;
        journalAsset.itemUuid = item->uuid;
        journalAsset.containerUuid = container->uuid;
        journalAsset.role = asset.role;
        journalAsset.ordinal = asset.ordinal;
        journalAsset.publicRelativePath = relative;
        journalAsset.stagedRelativePath = QStringLiteral(
            ".digikam-private/transactions/%1/preserved/%2")
                .arg(request.transactionUuid, fileName);
        journalAsset.protectedRelativePath = asset.protectedRelativePath;
        journalAsset.containerRelativePath = container->objectRelativePath;
        journalAsset.original = presentFact(asset.originalSize, originalHash);
        journalAsset.proxy.presence = PrivacyJournalExpectedPresence::Absent;
        journalAsset.container = containerFact;
        created.assets << journalAsset;

        LogicalAsset logical;
        logical.role = asset.role;
        logical.ordinal = asset.ordinal;
        logical.publicRelativePath = asset.publicRelativePath;
        logical.checkoutFileName = fileName;
        logical.modificationDate = asset.originalModificationDate.isValid()
                                 ? asset.originalModificationDate
                                 : QDateTime::fromMSecsSinceEpoch(0).toUTC();
        logicalAssets << logical;
    }

    const QByteArray createdPayload = encodePayload(created, logicalAssets);
    const QDateTime now = QDateTime::currentDateTimeUtc();
    PrivacyTransaction transaction;
    transaction.uuid = request.transactionUuid;
    transaction.categoryUuid = request.categoryUuid;
    transaction.itemUuid = item->uuid;
    transaction.type = PrivacyTransactionType::ExternalCheckout;
    transaction.state = PrivacyTransactionState::Created;
    transaction.generation = 0;
    transaction.fromCredentialGeneration = container->credentialGeneration;
    transaction.toCredentialGeneration = container->credentialGeneration;
    transaction.payloadFormatVersion = 1;
    transaction.payloadData = createdPayload;
    transaction.createdAt = now;
    transaction.updatedAt = now;
    PrivacyTransactionJournal databaseJournal;
    databaseJournal.transactionUuid = request.transactionUuid;
    databaseJournal.rootUuid = request.root.uuid;
    databaseJournal.journalRelativePath =
        PrivacyTransactionJournalCodec::relativeJournalPath(
            request.transactionUuid);
    databaseJournal.journalFormatVersion =
        PrivacyTransactionJournalCodec::FormatVersion;
    databaseJournal.stage = static_cast<int>(PrivacyJournalStage::Created);
    databaseJournal.updatedAt = now;

    if (createdPayload.isEmpty() ||
        !d->persistence.beginExternalCheckout(transaction, databaseJournal))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("cannot atomically begin External Checkout"));
    }

    QString detail;
    QByteArray createdJournalHash;

    if (!d->advanceFilesystemJournal(request.root, request.rootExpectation,
                                     created, created, &createdJournalHash,
                                     &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       request.transactionUuid, item->uuid, detail);
    }

    PrivacyJournalError storeError = PrivacyJournalError::None;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(
            request.root.configuredPath, request.rootExpectation,
            &storeError, &detail);

    if (!store)
    {
        return failure(PrivacyExternalCheckoutStatus::RootUnavailable,
                       request.transactionUuid, item->uuid, detail);
    }

    for (const LogicalAsset& logical : std::as_const(logicalAssets))
    {
        const auto journalIt = std::find_if(
            created.assets.cbegin(), created.assets.cend(),
            [&logical](const PrivacyJournalAsset& asset)
            {
                return ((asset.role == logical.role) &&
                        (asset.ordinal == logical.ordinal));
            });
        const auto sourceIt = std::find_if(
            request.sources.cbegin(), request.sources.cend(),
            [&logical](const PrivacyExternalCheckoutAssetSource& source)
            {
                return ((source.role == logical.role) &&
                        (source.ordinal == logical.ordinal));
            });

        if ((journalIt == created.assets.cend()) ||
            (sourceIt == request.sources.cend()) ||
            !store->createCheckoutFile(
                request.transactionUuid, logical.checkoutFileName,
                journalIt->original, sourceIt->producer, nullptr,
                &storeError, &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           request.transactionUuid, item->uuid, detail);
        }
    }

    const PrivacyJournalRecord prepared = atStage(
        created, PrivacyJournalStage::Prepared);
    QByteArray preparedHash;

    if (!d->advanceFilesystemJournal(request.root, request.rootExpectation,
                                     created, prepared, &preparedHash, &detail) ||
        !d->publishDatabaseJournal(databaseJournal,
                                   PrivacyJournalStage::Prepared,
                                   preparedHash))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("checkout is durable but its Prepared journal needs recovery"));
    }

    PrivacyTransaction preparedTransaction = transaction;
    preparedTransaction.state = PrivacyTransactionState::Prepared;
    preparedTransaction.generation = 1;
    preparedTransaction.payloadData = encodePayload(prepared, logicalAssets);
    preparedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

    if (preparedTransaction.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            preparedTransaction, PrivacyTransactionState::Created, 0))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("Prepared checkout requires restart recovery"));
    }

    return d->assetsResult(PrivacyExternalCheckoutStatus::Ready, request.root,
                           prepared, logicalAssets);
}

PrivacyExternalCheckoutResult
PrivacyExternalCheckoutTransactionEngine::resumeAuthenticatedCreate(
    const PrivacyExternalCheckoutRequest& request)
{
    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.transactionUuid) || !request.root.isValid() ||
        (request.root.uuid != request.rootExpectation.rootUuid) ||
        (request.rootExpectation.device == 0) ||
        (request.rootExpectation.inode == 0) || request.sources.isEmpty())
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       request.transactionUuid, {},
                       QStringLiteral("External Checkout resume request is invalid"));
    }

    const PrivacyExternalCheckoutResult passive = recover(
        request.root, request.rootExpectation, request.transactionUuid);

    if (passive.status != PrivacyExternalCheckoutStatus::AuthenticationRequired)
    {
        return passive;
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, {},
                       QStringLiteral("cannot load checkout recovery state"));
    }

    const PrivacyTransaction* const transaction = transactionFor(
        snapshot, request.transactionUuid);
    const PrivacyTransactionJournal* const databaseJournal = databaseJournalFor(
        snapshot, request.transactionUuid, request.root.uuid);
    PrivacyJournalRecord created;
    QList<LogicalAsset> logicalAssets;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        (transaction->state != PrivacyTransactionState::Created) ||
        (transaction->generation != 0) ||
        (transaction->categoryUuid != request.categoryUuid) ||
        !decodePayload(transaction->payloadData, &created, &logicalAssets) ||
        (created.stage != PrivacyJournalStage::Created) ||
        (created.transactionUuid != request.transactionUuid) ||
        (created.categoryUuid != request.categoryUuid) ||
        (created.rootUuid != request.root.uuid) ||
        (created.rootDevice != request.rootExpectation.device) ||
        (created.rootInode != request.rootExpectation.inode) ||
        (created.rootIdentitySha256 !=
         request.rootExpectation.identitySha256) ||
        (databaseJournal->journalRelativePath !=
         PrivacyTransactionJournalCodec::relativeJournalPath(
             request.transactionUuid)) ||
        (databaseJournal->journalFormatVersion !=
         PrivacyTransactionJournalCodec::FormatVersion) ||
        ((databaseJournal->stage !=
          static_cast<int>(PrivacyJournalStage::Created)) &&
         (databaseJournal->stage !=
          static_cast<int>(PrivacyJournalStage::Prepared))) ||
        (logicalAssets.size() != request.sources.size()) ||
        (created.assets.size() != request.sources.size()))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       request.transactionUuid,
                       transaction ? transaction->itemUuid : QString(),
                       QStringLiteral("Created checkout recovery evidence is incomplete"));
    }

    const PrivacyItem* const item = uniqueValue(
        snapshot.items, [&request, transaction](const PrivacyItem& candidate)
        {
            return ((candidate.imageId == request.imageId) &&
                    (candidate.uuid == transaction->itemUuid));
        });
    const PrivacyCategory* const category = item
        ? uniqueValue(snapshot.categories,
                      [item](const PrivacyCategory& candidate)
                      {
                          return (candidate.uuid == item->categoryUuid);
                      })
        : nullptr;
    const PrivacyContainer* const container = item
        ? uniqueValue(snapshot.containers,
                      [item](const PrivacyContainer& candidate)
                      {
                          return (candidate.itemUuid == item->uuid);
                      })
        : nullptr;

    if (!item || !category || !container ||
        (item->categoryUuid != request.categoryUuid) ||
        (item->generation != created.generation) ||
        (category->backend != PrivacyBackend::Casual) ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (container->kind != PrivacyContainerKind::CasualArchive) ||
        (container->state != PrivacyContainerState::Verified) ||
        (container->rootUuid != request.root.uuid) ||
        (container->credentialGeneration != created.credentialGeneration))
    {
        return failure(PrivacyExternalCheckoutStatus::ItemUnavailable,
                       request.transactionUuid,
                       transaction->itemUuid,
                       QStringLiteral("protected checkout source changed before resume"));
    }

    QList<PrivacyAsset> assets;

    for (const PrivacyAsset& asset : std::as_const(snapshot.assets))
    {
        if (asset.itemUuid == item->uuid)
        {
            assets << asset;
        }
    }

    if (assets.size() != logicalAssets.size())
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("associated-asset inventory changed before resume"));
    }

    const PrivacyJournalObjectFact containerFact = presentFact(
        container->protectedSize,
        sha256Bytes(container->protectedHashAlgorithm,
                    container->protectedHash));
    QSet<QString> identities;

    for (const LogicalAsset& logical : std::as_const(logicalAssets))
    {
        const QString identity = QStringLiteral("%1:%2")
                                     .arg(logical.role).arg(logical.ordinal);
        const PrivacyAsset* const asset = uniqueValue(
            assets, [&logical](const PrivacyAsset& candidate)
            {
                return ((candidate.role == logical.role) &&
                        (candidate.ordinal == logical.ordinal));
            });
        const PrivacyJournalAsset* const journalAsset = uniqueValue(
            created.assets, [&logical](const PrivacyJournalAsset& candidate)
            {
                return ((candidate.role == logical.role) &&
                        (candidate.ordinal == logical.ordinal));
            });
        const PrivacyExternalCheckoutAssetSource* const source = uniqueValue(
            request.sources,
            [&logical](const PrivacyExternalCheckoutAssetSource& candidate)
            {
                return ((candidate.role == logical.role) &&
                        (candidate.ordinal == logical.ordinal));
            });
        const PrivacyJournalObjectFact originalFact = asset
            ? presentFact(asset->originalSize,
                          sha256Bytes(asset->hashAlgorithm,
                                      asset->originalHash))
            : PrivacyJournalObjectFact();

        if (identities.contains(identity) || !asset || !journalAsset ||
            !source || !source->producer || containerFact.sha256.isEmpty() ||
            originalFact.sha256.isEmpty() ||
            (asset->containerUuid != container->uuid) ||
            (asset->publicRootUuid != request.root.uuid) ||
            (asset->publicRelativePath != logical.publicRelativePath) ||
            (asset->protectedRelativePath !=
             journalAsset->protectedRelativePath) ||
            (journalAsset->itemUuid != item->uuid) ||
            (journalAsset->containerUuid != container->uuid) ||
            (journalAsset->containerRelativePath !=
             container->objectRelativePath) ||
            (journalAsset->publicRelativePath !=
             PrivacyTransactionJournalStore::relativeCheckoutPath(
                 request.transactionUuid, logical.checkoutFileName)) ||
            !sameFact(journalAsset->original, originalFact) ||
            !sameFact(journalAsset->container, containerFact))
        {
            return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("checkout source mapping changed before resume"));
        }

        identities.insert(identity);
    }

    QString detail;
    PrivacyJournalError storeError = PrivacyJournalError::None;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(
            request.root.configuredPath, request.rootExpectation,
            &storeError, &detail);

    if (!store)
    {
        return failure(PrivacyExternalCheckoutStatus::RootUnavailable,
                       request.transactionUuid, item->uuid, detail);
    }

    const PrivacyJournalRecord prepared = atStage(
        created, PrivacyJournalStage::Prepared);
    const PrivacyJournalLoadResult loaded = store->load(request.transactionUuid);

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord ||
        (!sameRecord(loaded.record, created) &&
         !sameRecord(loaded.record, prepared)))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("checkout journal changed before authenticated resume"));
    }

    for (const LogicalAsset& logical : std::as_const(logicalAssets))
    {
        const PrivacyJournalAsset* const journalAsset = uniqueValue(
            created.assets, [&logical](const PrivacyJournalAsset& candidate)
            {
                return ((candidate.role == logical.role) &&
                        (candidate.ordinal == logical.ordinal));
            });
        const PrivacyExternalCheckoutAssetSource* const source = uniqueValue(
            request.sources,
            [&logical](const PrivacyExternalCheckoutAssetSource& candidate)
            {
                return ((candidate.role == logical.role) &&
                        (candidate.ordinal == logical.ordinal));
            });

        if (!journalAsset || !source ||
            !store->createCheckoutFile(
                request.transactionUuid, logical.checkoutFileName,
                journalAsset->original, source->producer, nullptr,
                &storeError, &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           request.transactionUuid, item->uuid, detail);
        }
    }

    QByteArray preparedHash;

    if (!d->advanceFilesystemJournal(request.root, request.rootExpectation,
                                     loaded.record, prepared, &preparedHash,
                                     &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("resumed checkout needs journal recovery"));
    }

    if (databaseJournal->stage == static_cast<int>(PrivacyJournalStage::Created))
    {
        if (!d->publishDatabaseJournal(*databaseJournal,
                                       PrivacyJournalStage::Prepared,
                                       preparedHash))
        {
            return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("resumed checkout journal needs database recovery"));
        }
    }
    else if ((databaseJournal->expectedHashAlgorithm !=
              QLatin1String("sha256")) ||
             (databaseJournal->expectedJournalHash !=
              QString::fromLatin1(preparedHash.toHex())))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("Prepared database journal does not match checkout"));
    }

    PrivacyTransaction preparedTransaction = *transaction;
    preparedTransaction.state = PrivacyTransactionState::Prepared;
    preparedTransaction.generation = 1;
    preparedTransaction.payloadData = encodePayload(prepared, logicalAssets);
    preparedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

    if (preparedTransaction.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            preparedTransaction, PrivacyTransactionState::Created, 0))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("resumed checkout needs transaction recovery"));
    }

    return d->assetsResult(PrivacyExternalCheckoutStatus::Ready, request.root,
                           prepared, logicalAssets);
}

PrivacyExternalCheckoutResult
PrivacyExternalCheckoutTransactionEngine::authorizeLaunch(
    const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& rootExpectation,
    const QString& transactionUuid)
{
    PrivacyRepositorySnapshot snapshot;

    if (!root.isValid() || (root.uuid != rootExpectation.rootUuid) ||
        !canonicalUuid(transactionUuid) || !d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout launch request is invalid"));
    }

    const PrivacyTransaction* const transaction = transactionFor(
        snapshot, transactionUuid);
    const PrivacyTransactionJournal* const databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, root.uuid);
    PrivacyJournalRecord prepared;
    QList<LogicalAsset> logicalAssets;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        (transaction->state != PrivacyTransactionState::Prepared) ||
        (transaction->generation != 1) ||
        !decodePayload(transaction->payloadData, &prepared, &logicalAssets) ||
        (prepared.stage != PrivacyJournalStage::Prepared) ||
        (databaseJournal->stage != static_cast<int>(prepared.stage)))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid,
                       transaction ? transaction->itemUuid : QString(),
                       QStringLiteral("Prepared checkout evidence is incomplete"));
    }

    const PrivacyJournalRecord exposed = atStage(
        prepared, PrivacyJournalStage::PublicStateVerified);
    QByteArray exposedHash;
    QString detail;

    if (!d->advanceFilesystemJournal(root, rootExpectation, prepared, exposed,
                                     &exposedHash, &detail) ||
        !d->publishDatabaseJournal(*databaseJournal,
                                   PrivacyJournalStage::PublicStateVerified,
                                   exposedHash))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout launch authorization requires recovery"));
    }

    PrivacyTransaction next = *transaction;
    next.state = PrivacyTransactionState::Exposed;
    next.generation = 2;
    next.payloadData = encodePayload(exposed, logicalAssets);
    next.updatedAt = QDateTime::currentDateTimeUtc();

    if (next.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            next, PrivacyTransactionState::Prepared, 1))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("launch authorization requires restart recovery"));
    }

    return d->assetsResult(PrivacyExternalCheckoutStatus::Ready, root,
                           exposed, logicalAssets);
}

PrivacyExternalCheckoutResult PrivacyExternalCheckoutTransactionEngine::reconcile(
    const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& rootExpectation,
    const QString& transactionUuid)
{
    PrivacyRepositorySnapshot snapshot;

    if (!root.isValid() || (root.uuid != rootExpectation.rootUuid) ||
        !canonicalUuid(transactionUuid) || !d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout reconciliation request is invalid"));
    }

    const PrivacyTransaction* transaction = transactionFor(snapshot,
                                                            transactionUuid);
    const PrivacyTransactionJournal* databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, root.uuid);
    PrivacyJournalRecord record;
    QList<LogicalAsset> logicalAssets;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        !decodePayload(transaction->payloadData, &record, &logicalAssets))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid,
                       transaction ? transaction->itemUuid : QString(),
                       QStringLiteral("checkout recovery payload is invalid"));
    }

    if (transaction->state == PrivacyTransactionState::Complete)
    {
        return d->assetsResult(
            PrivacyExternalCheckoutStatus::CompletedUnchanged, root, record,
            logicalAssets);
    }

    if (transaction->state == PrivacyTransactionState::NeedsReconciliation)
    {
        return d->assetsResult(
            PrivacyExternalCheckoutStatus::ChangesPending, root, record,
            logicalAssets,
            QStringLiteral("external changes are preserved for explicit reconciliation"));
    }

    if ((transaction->state != PrivacyTransactionState::Prepared) &&
        (transaction->state != PrivacyTransactionState::Exposed) &&
        (transaction->state != PrivacyTransactionState::Relocking))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout state cannot be reconciled directly"));
    }

    QString detail;
    PrivacyJournalError storeError = PrivacyJournalError::None;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(root.configuredPath,
                                              rootExpectation, &storeError,
                                              &detail);

    if (!store)
    {
        return failure(PrivacyExternalCheckoutStatus::RootUnavailable,
                       transactionUuid, transaction->itemUuid, detail);
    }

    bool unexpected = false;

    if (!store->hasUnexpectedCheckoutEntries(
            transactionUuid, &unexpected, &storeError, &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       transactionUuid, transaction->itemUuid, detail);
    }

    bool changed = unexpected;
    bool missing = false;

    for (const LogicalAsset& logical : std::as_const(logicalAssets))
    {
        const auto journalIt = std::find_if(
            record.assets.cbegin(), record.assets.cend(),
            [&logical](const PrivacyJournalAsset& asset)
            {
                return ((asset.role == logical.role) &&
                        (asset.ordinal == logical.ordinal));
            });

        if (journalIt == record.assets.cend())
        {
            changed = true;
            continue;
        }

        const PrivacyCheckoutInspectionResult inspection =
            store->inspectCheckoutFile(transactionUuid,
                                       logical.checkoutFileName,
                                       journalIt->original);

        if (inspection.disposition ==
            PrivacyCheckoutInspectionDisposition::Missing)
        {
            missing = true;

            if (transaction->state != PrivacyTransactionState::Relocking)
            {
                changed = true;
            }
        }
        else if (inspection.disposition !=
                 PrivacyCheckoutInspectionDisposition::MatchesBaseline)
        {
            changed = true;
        }
    }

    if (changed)
    {
        const PrivacyJournalRecord pending = atStage(
            record, PrivacyJournalStage::ReconciliationRequired);
        QByteArray pendingHash;

        if ((record.stage != PrivacyJournalStage::ReconciliationRequired) &&
            (!d->advanceFilesystemJournal(root, rootExpectation, record,
                                          pending, &pendingHash, &detail) ||
             !d->publishDatabaseJournal(*databaseJournal,
                                        PrivacyJournalStage::ReconciliationRequired,
                                        pendingHash)))
        {
            return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                           transactionUuid, transaction->itemUuid,
                           QStringLiteral("changed checkout is preserved but needs journal recovery"));
        }

        if (transaction->state != PrivacyTransactionState::NeedsReconciliation)
        {
            PrivacyTransaction next = *transaction;
            next.state = PrivacyTransactionState::NeedsReconciliation;
            next.generation = transaction->generation + 1;
            next.payloadData = encodePayload(pending, logicalAssets);
            next.updatedAt = QDateTime::currentDateTimeUtc();

            if (next.payloadData.isEmpty() ||
                !d->persistence.compareAndUpdateTransaction(
                    next, transaction->state, transaction->generation))
            {
                return failure(
                    PrivacyExternalCheckoutStatus::PersistenceFailure,
                    transactionUuid, transaction->itemUuid,
                    QStringLiteral("changed checkout is preserved but its database state needs recovery"));
            }
        }

        return d->assetsResult(
            PrivacyExternalCheckoutStatus::ChangesPending, root, pending,
            logicalAssets,
            QStringLiteral("external changes are preserved for explicit reconciliation"));
    }

    PrivacyJournalRecord relocking = record;

    if (transaction->state != PrivacyTransactionState::Relocking)
    {
        relocking = atStage(record,
                            PrivacyJournalStage::ReconciliationRequired);
        QByteArray relockingHash;

        if (!d->advanceFilesystemJournal(root, rootExpectation, record,
                                         relocking, &relockingHash, &detail) ||
            !d->publishDatabaseJournal(*databaseJournal,
                                       PrivacyJournalStage::ReconciliationRequired,
                                       relockingHash))
        {
            return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                           transactionUuid, transaction->itemUuid,
                           QStringLiteral("unchanged checkout cleanup needs recovery"));
        }

        PrivacyTransaction next = *transaction;
        next.state = PrivacyTransactionState::Relocking;
        next.generation = transaction->generation + 1;
        next.payloadData = encodePayload(relocking, logicalAssets);
        next.updatedAt = QDateTime::currentDateTimeUtc();

        if (next.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                next, transaction->state, transaction->generation))
        {
            return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                           transactionUuid, transaction->itemUuid,
                           QStringLiteral("unchanged checkout cleanup needs database recovery"));
        }

        transaction = nullptr;

        if (!d->load(&snapshot) ||
            !(transaction = transactionFor(snapshot, transactionUuid)) ||
            !(databaseJournal = databaseJournalFor(snapshot, transactionUuid,
                                                    root.uuid)))
        {
            return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                           transactionUuid, {},
                           QStringLiteral("cannot reload relocking checkout"));
        }
    }

    for (const LogicalAsset& logical : std::as_const(logicalAssets))
    {
        const auto journalIt = std::find_if(
            relocking.assets.cbegin(), relocking.assets.cend(),
            [&logical](const PrivacyJournalAsset& asset)
            {
                return ((asset.role == logical.role) &&
                        (asset.ordinal == logical.ordinal));
            });
        bool absent = false;

        if ((journalIt == relocking.assets.cend()) ||
            !store->removeUnchangedCheckoutFile(
                transactionUuid, logical.checkoutFileName,
                journalIt->original, &absent, &storeError, &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           transactionUuid, transaction->itemUuid,
                           detail.isEmpty()
                               ? QStringLiteral("unchanged checkout cleanup failed")
                               : detail);
        }
    }

    Q_UNUSED(missing);
    const PrivacyJournalRecord complete = atStage(
        relocking, PrivacyJournalStage::Complete);
    QByteArray completeHash;

    if (!d->advanceFilesystemJournal(root, rootExpectation, relocking, complete,
                                     &completeHash, &detail) ||
        !d->publishDatabaseJournal(*databaseJournal,
                                   PrivacyJournalStage::Complete,
                                   completeHash))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout bytes are removed but completion needs recovery"));
    }

    PrivacyTransaction completed = *transaction;
    completed.state = PrivacyTransactionState::Complete;
    completed.generation = transaction->generation + 1;
    completed.payloadData = encodePayload(complete, logicalAssets);
    completed.updatedAt = QDateTime::currentDateTimeUtc();

    if (completed.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            completed, transaction->state, transaction->generation))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout completion needs database recovery"));
    }

    return d->assetsResult(
        PrivacyExternalCheckoutStatus::CompletedUnchanged, root, complete,
        logicalAssets);
}

PrivacyExternalCheckoutResult PrivacyExternalCheckoutTransactionEngine::recover(
    const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& rootExpectation,
    const QString& transactionUuid)
{
    PrivacyRepositorySnapshot snapshot;

    if (!root.isValid() || (root.uuid != rootExpectation.rootUuid) ||
        !canonicalUuid(transactionUuid) || !d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout recovery request is invalid"));
    }

    const PrivacyTransaction* transaction = transactionFor(snapshot,
                                                            transactionUuid);

    if (!transaction ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, {},
                       QStringLiteral("External Checkout transaction is unavailable"));
    }

    if (transaction->state != PrivacyTransactionState::Created)
    {
        return reconcile(root, rootExpectation, transactionUuid);
    }

    PrivacyJournalRecord created;
    QList<LogicalAsset> logicalAssets;

    if (!decodePayload(transaction->payloadData, &created, &logicalAssets) ||
        (created.stage != PrivacyJournalStage::Created))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("Created checkout payload is invalid"));
    }

    QString detail;
    PrivacyJournalError storeError = PrivacyJournalError::None;
    std::unique_ptr<PrivacyTransactionJournalStore> store =
        PrivacyTransactionJournalStore::open(root.configuredPath,
                                              rootExpectation, &storeError,
                                              &detail);

    if (!store)
    {
        return failure(PrivacyExternalCheckoutStatus::RootUnavailable,
                       transactionUuid, transaction->itemUuid, detail);
    }

    const PrivacyJournalLoadResult loaded = store->load(transactionUuid);
    const PrivacyJournalRecord prepared = atStage(
        created, PrivacyJournalStage::Prepared);

    if ((loaded.disposition != PrivacyJournalLoadDisposition::Loaded) ||
        !loaded.authoritative || !loaded.hasRecord ||
        (!sameRecord(loaded.record, created) &&
         !sameRecord(loaded.record, prepared)))
    {
        return failure(PrivacyExternalCheckoutStatus::AuthenticationRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout creation must resume while the category is unlocked"));
    }

    bool unexpected = false;

    if (!store->hasUnexpectedCheckoutEntries(transactionUuid, &unexpected,
                                              &storeError, &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       transactionUuid, transaction->itemUuid, detail);
    }

    bool allExact = !unexpected;
    bool anyChanged = unexpected;

    for (const LogicalAsset& logical : std::as_const(logicalAssets))
    {
        const auto journalIt = std::find_if(
            created.assets.cbegin(), created.assets.cend(),
            [&logical](const PrivacyJournalAsset& asset)
            {
                return ((asset.role == logical.role) &&
                        (asset.ordinal == logical.ordinal));
            });

        if (journalIt == created.assets.cend())
        {
            anyChanged = true;
            allExact = false;
            continue;
        }

        const PrivacyCheckoutInspectionResult inspection =
            store->inspectCheckoutFile(transactionUuid,
                                       logical.checkoutFileName,
                                       journalIt->original);
        allExact = allExact &&
            (inspection.disposition ==
             PrivacyCheckoutInspectionDisposition::MatchesBaseline);
        anyChanged = anyChanged ||
            (inspection.disposition ==
             PrivacyCheckoutInspectionDisposition::Changed) ||
            (inspection.disposition ==
             PrivacyCheckoutInspectionDisposition::Unsafe) ||
            (inspection.disposition ==
             PrivacyCheckoutInspectionDisposition::IoFailure);
    }

    if (anyChanged)
    {
        const PrivacyTransactionJournal* const databaseJournal =
            databaseJournalFor(snapshot, transactionUuid, root.uuid);
        const PrivacyJournalRecord pending = atStage(
            created, PrivacyJournalStage::ReconciliationRequired);
        QByteArray pendingHash;

        if (!databaseJournal ||
            !d->advanceFilesystemJournal(root, rootExpectation, loaded.record,
                                          pending, &pendingHash, &detail) ||
            !d->publishDatabaseJournal(*databaseJournal,
                                       PrivacyJournalStage::ReconciliationRequired,
                                       pendingHash))
        {
            return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                           transactionUuid, transaction->itemUuid,
                           QStringLiteral("unexpected checkout content is preserved but needs journal recovery"));
        }

        PrivacyTransaction pendingTransaction = *transaction;
        pendingTransaction.state =
            PrivacyTransactionState::NeedsReconciliation;
        pendingTransaction.generation = 1;
        pendingTransaction.payloadData = encodePayload(pending, logicalAssets);
        pendingTransaction.updatedAt = QDateTime::currentDateTimeUtc();

        if (pendingTransaction.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                pendingTransaction, PrivacyTransactionState::Created, 0))
        {
            return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                           transactionUuid, transaction->itemUuid,
                           QStringLiteral("unexpected checkout content is preserved but needs database recovery"));
        }

        return d->assetsResult(
            PrivacyExternalCheckoutStatus::ChangesPending, root, pending,
            logicalAssets);
    }

    if (!allExact)
    {
        return failure(PrivacyExternalCheckoutStatus::AuthenticationRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("missing checkout bytes require an unlocked category"));
    }

    QByteArray preparedHash;
    const PrivacyTransactionJournal* const databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, root.uuid);

    if (!databaseJournal ||
        !d->advanceFilesystemJournal(root, rootExpectation, loaded.record,
                                     prepared, &preparedHash, &detail) ||
        ((databaseJournal->stage !=
          static_cast<int>(PrivacyJournalStage::Prepared)) &&
         !d->publishDatabaseJournal(*databaseJournal,
                                    PrivacyJournalStage::Prepared,
                                    preparedHash)))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("exact Created checkout needs journal recovery"));
    }

    PrivacyTransaction preparedTransaction = *transaction;
    preparedTransaction.state = PrivacyTransactionState::Prepared;
    preparedTransaction.generation = 1;
    preparedTransaction.payloadData = encodePayload(prepared, logicalAssets);
    preparedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

    if (preparedTransaction.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            preparedTransaction, PrivacyTransactionState::Created, 0))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("exact Created checkout needs database recovery"));
    }

    return reconcile(root, rootExpectation, transactionUuid);
}

} // namespace Digikam

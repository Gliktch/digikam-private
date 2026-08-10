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
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QUuid>

// Local includes

#include "privacycheckoutstore.h"
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
    QString   checkoutRelativePath;
    QDateTime modificationDate;
};

struct CheckoutPayloadContext
{
    QString storeUuid;
    PrivacyJournalRootExpectation publicRootExpectation;
    QByteArray approvedDiscardInventorySha256;
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
            !path.contains(QChar::Null) && !path.contains(QLatin1Char('\\')) &&
            (path == QDir::cleanPath(path)) &&
            !path.startsWith(QLatin1String("../")) &&
            (path != QLatin1String("..")) &&
            (path == path.normalized(QString::NormalizationForm_C)));
}

bool parseUnsigned(const QJsonValue& value, quint64* const result)
{
    if (!result || !value.isString())
    {
        return false;
    }

    const QString encoded = value.toString();

    if (encoded.isEmpty() ||
        ((encoded.size() > 1) && encoded.startsWith(QLatin1Char('0'))))
    {
        return false;
    }

    bool ok = false;
    const quint64 parsed = encoded.toULongLong(&ok, 10);

    if (!ok || (QString::number(parsed) != encoded))
    {
        return false;
    }

    *result = parsed;
    return true;
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
                         const QList<LogicalAsset>& logicalAssets,
                         const CheckoutPayloadContext& context)
{
    const QByteArray journal = PrivacyTransactionJournalCodec::encode(record);

    if (journal.isEmpty() ||
        (record.transactionType != PrivacyTransactionType::ExternalCheckout) ||
        (logicalAssets.size() != record.assets.size()) ||
        !canonicalUuid(context.storeUuid) ||
        !canonicalUuid(context.publicRootExpectation.rootUuid) ||
        (!context.publicRootExpectation.markerUuid.isEmpty() &&
         !canonicalUuid(context.publicRootExpectation.markerUuid)) ||
        (context.publicRootExpectation.device == 0) ||
        (context.publicRootExpectation.inode == 0) ||
        (context.publicRootExpectation.identitySha256.size() != 32) ||
        (!context.approvedDiscardInventorySha256.isEmpty() &&
         (context.approvedDiscardInventorySha256.size() != 32)))
    {
        return {};
    }

    QJsonArray assets;
    QSet<QString> identities;

    for (const LogicalAsset& asset : logicalAssets)
    {
        if ((asset.role <= 0) || (asset.ordinal < 0) ||
            !safeRelativePath(asset.publicRelativePath) ||
            !safeRelativePath(asset.checkoutRelativePath) ||
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
        object.insert(QStringLiteral("checkoutRelativePath"),
                      asset.checkoutRelativePath);
        object.insert(QStringLiteral("modificationDate"),
                      asset.modificationDate.toUTC().toString(Qt::ISODateWithMs));
        object.insert(QStringLiteral("ordinal"), asset.ordinal);
        object.insert(QStringLiteral("publicRelativePath"),
                      asset.publicRelativePath);
        object.insert(QStringLiteral("role"), asset.role);
        assets.append(object);
    }

    QJsonObject publicRoot;
    publicRoot.insert(QStringLiteral("device"),
                      QString::number(context.publicRootExpectation.device));
    publicRoot.insert(QStringLiteral("identitySha256"),
                      QString::fromLatin1(
                          context.publicRootExpectation.identitySha256.toHex()));
    publicRoot.insert(QStringLiteral("inode"),
                      QString::number(context.publicRootExpectation.inode));
    publicRoot.insert(QStringLiteral("markerUuid"),
                      context.publicRootExpectation.markerUuid);
    publicRoot.insert(QStringLiteral("rootUuid"),
                      context.publicRootExpectation.rootUuid);
    QJsonObject object;
    object.insert(QStringLiteral("approvedDiscardInventorySha256"),
                  QString::fromLatin1(
                      context.approvedDiscardInventorySha256.toHex()));
    object.insert(QStringLiteral("assets"), assets);
    object.insert(QStringLiteral("formatVersion"), 2);
    object.insert(QStringLiteral("journal"),
                  QString::fromLatin1(journal.toBase64()));
    object.insert(QStringLiteral("publicRoot"), publicRoot);
    object.insert(QStringLiteral("storeUuid"), context.storeUuid);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool decodePayload(const QByteArray& bytes, PrivacyJournalRecord* const record,
                   QList<LogicalAsset>* const logicalAssets,
                   CheckoutPayloadContext* const context)
{
    if (!record || !logicalAssets || !context || bytes.isEmpty())
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

    if (!exactKeys(object,
                   { "approvedDiscardInventorySha256", "assets",
                     "formatVersion", "journal", "publicRoot", "storeUuid" }) ||
        (object.value(QStringLiteral("formatVersion")).toInt(-1) != 2) ||
        !object.value(
            QStringLiteral("approvedDiscardInventorySha256")).isString() ||
        !object.value(QStringLiteral("assets")).isArray() ||
        !object.value(QStringLiteral("journal")).isString() ||
        !object.value(QStringLiteral("publicRoot")).isObject() ||
        !object.value(QStringLiteral("storeUuid")).isString())
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
                       { "checkoutRelativePath", "modificationDate", "ordinal",
                         "publicRelativePath", "role" }))
        {
            return false;
        }

        LogicalAsset asset;
        asset.role = assetObject.value(QStringLiteral("role")).toInt(0);
        asset.ordinal = assetObject.value(QStringLiteral("ordinal")).toInt(-1);
        asset.publicRelativePath =
            assetObject.value(QStringLiteral("publicRelativePath")).toString();
        asset.checkoutRelativePath =
            assetObject.value(QStringLiteral("checkoutRelativePath")).toString();
        asset.modificationDate = QDateTime::fromString(
            assetObject.value(QStringLiteral("modificationDate")).toString(),
            Qt::ISODateWithMs);
        const QString identity = QStringLiteral("%1:%2")
                                     .arg(asset.role).arg(asset.ordinal);

        if ((asset.role <= 0) || (asset.ordinal < 0) ||
            !safeRelativePath(asset.publicRelativePath) ||
            !safeRelativePath(asset.checkoutRelativePath) ||
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

    const QJsonObject publicRoot =
        object.value(QStringLiteral("publicRoot")).toObject();
    CheckoutPayloadContext decodedContext;
    const QString approvedDiscardHash = object.value(
        QStringLiteral("approvedDiscardInventorySha256")).toString();
    decodedContext.storeUuid =
        object.value(QStringLiteral("storeUuid")).toString();
    decodedContext.approvedDiscardInventorySha256 =
        QByteArray::fromHex(approvedDiscardHash.toLatin1());
    decodedContext.publicRootExpectation.rootUuid =
        publicRoot.value(QStringLiteral("rootUuid")).toString();
    decodedContext.publicRootExpectation.markerUuid =
        publicRoot.value(QStringLiteral("markerUuid")).toString();
    decodedContext.publicRootExpectation.identitySha256 = QByteArray::fromHex(
        publicRoot.value(QStringLiteral("identitySha256")).toString().toLatin1());

    if (!exactKeys(publicRoot,
                   { "device", "identitySha256", "inode", "markerUuid",
                     "rootUuid" }) ||
        !canonicalUuid(decodedContext.storeUuid) ||
        !canonicalUuid(decodedContext.publicRootExpectation.rootUuid) ||
        (!decodedContext.publicRootExpectation.markerUuid.isEmpty() &&
         !canonicalUuid(decodedContext.publicRootExpectation.markerUuid)) ||
        (approvedDiscardHash.isEmpty()
             ? !decodedContext.approvedDiscardInventorySha256.isEmpty()
             : ((approvedDiscardHash.size() != 64) ||
                (decodedContext.approvedDiscardInventorySha256.size() != 32) ||
                (QString::fromLatin1(
                     decodedContext.approvedDiscardInventorySha256.toHex()) !=
                 approvedDiscardHash))) ||
        (decodedContext.publicRootExpectation.identitySha256.size() != 32) ||
        !parseUnsigned(publicRoot.value(QStringLiteral("device")),
                       &decodedContext.publicRootExpectation.device) ||
        !parseUnsigned(publicRoot.value(QStringLiteral("inode")),
                       &decodedContext.publicRootExpectation.inode) ||
        (decodedContext.publicRootExpectation.device == 0) ||
        (decodedContext.publicRootExpectation.inode == 0))
    {
        return false;
    }

    *record = decodedRecord;
    *logicalAssets = decodedAssets;
    *context = decodedContext;
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

const PrivacyStorageRoot* storageRootFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& uuid)
{
    return uniqueValue(snapshot.storageRoots,
                       [&uuid](const PrivacyStorageRoot& root)
                       {
                           return (root.uuid == uuid);
                       });
}

const PrivacyStore* categoryStoreFor(
    const PrivacyRepositorySnapshot& snapshot, const QString& categoryUuid,
    const QString& storeUuid)
{
    const PrivacyStore* const store = uniqueValue(
        snapshot.stores, [&storeUuid](const PrivacyStore& candidate)
        {
            return (candidate.uuid == storeUuid);
        });
    bool authority = false;
    bool derivatives = false;

    for (const PrivacyStoreBinding& binding : snapshot.storeBindings)
    {
        if ((binding.categoryUuid != categoryUuid) ||
            (binding.storeUuid != storeUuid))
        {
            continue;
        }

        authority = authority ||
            (binding.role == PrivacyStoreRole::CredentialAuthority);
        derivatives = derivatives ||
            (binding.role == PrivacyStoreRole::Derivatives);
    }

    return (store && authority && derivatives) ? store : nullptr;
}

bool sameRootExpectation(const PrivacyJournalRootExpectation& left,
                         const PrivacyJournalRootExpectation& right)
{
    return ((left.rootUuid == right.rootUuid) &&
            (left.markerUuid == right.markerUuid) &&
            (left.identitySha256 == right.identitySha256) &&
            (left.device == right.device) && (left.inode == right.inode));
}

bool sameStorageRoot(const PrivacyStorageRoot& left,
                     const PrivacyStorageRoot& right)
{
    return ((left.uuid == right.uuid) && (left.kind == right.kind) &&
            (left.albumRootId == right.albumRootId) &&
            (left.configuredPath == right.configuredPath) &&
            (left.identityVersion == right.identityVersion) &&
            (left.identityData == right.identityData) &&
            (left.markerUuid == right.markerUuid));
}

bool validStoreAccess(const PrivacyExternalCheckoutStoreAccess& access)
{
    return (canonicalUuid(access.storeUuid) && access.root.isValid() &&
            (access.root.kind == PrivacyStorageRootKind::ManagedStoreRoot) &&
            (access.root.uuid == access.rootExpectation.rootUuid) &&
            (access.rootExpectation.device != 0) &&
            (access.rootExpectation.inode != 0) &&
            (access.rootExpectation.identitySha256.size() == 32) &&
            QDir::isAbsolutePath(access.plaintextRoot));
}

bool checkoutLocation(const QList<LogicalAsset>& assets,
                      const QString& transactionUuid,
                      PrivacyCheckoutStoreLocation* const location)
{
    if (!location || assets.isEmpty())
    {
        return false;
    }

    bool checkout = true;
    bool recovery = true;

    for (const LogicalAsset& asset : assets)
    {
        const QString fileName = QFileInfo(asset.checkoutRelativePath).fileName();
        checkout = checkout &&
            (asset.checkoutRelativePath ==
             PrivacyCheckoutStore::workFileRelativePath(
                 transactionUuid, fileName,
                 PrivacyCheckoutStoreLocation::Checkout));
        recovery = recovery &&
            (asset.checkoutRelativePath ==
             PrivacyCheckoutStore::workFileRelativePath(
                 transactionUuid, fileName,
                 PrivacyCheckoutStoreLocation::Recovery));
    }

    if (checkout == recovery)
    {
        return false;
    }

    *location = checkout ? PrivacyCheckoutStoreLocation::Checkout
                         : PrivacyCheckoutStoreLocation::Recovery;
    return true;
}

bool rewriteCheckoutLocation(QList<LogicalAsset>* const assets,
                             const QString& transactionUuid,
                             PrivacyCheckoutStoreLocation location)
{
    if (!assets)
    {
        return false;
    }

    for (LogicalAsset& asset : *assets)
    {
        const QString path = PrivacyCheckoutStore::workFileRelativePath(
            transactionUuid, QFileInfo(asset.checkoutRelativePath).fileName(),
            location);

        if (path.isEmpty())
        {
            return false;
        }

        asset.checkoutRelativePath = path;
    }

    return true;
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
        const PrivacyStorageRoot& publicRoot,
        const PrivacyJournalRecord& record,
        const QList<LogicalAsset>& logicalAssets,
        const PrivacyCheckoutStore* checkoutStore = nullptr,
        const PrivacyCheckoutInventory* inventory = nullptr,
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

            if ((journalIt->publicRelativePath !=
                 logical.publicRelativePath) ||
                !safeRelativePath(logical.checkoutRelativePath))
            {
                return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                               record.transactionUuid, result.itemUuid,
                               QStringLiteral("checkout payload path mapping is invalid"));
            }

            PrivacyExternalCheckoutAsset asset;
            asset.role = logical.role;
            asset.ordinal = logical.ordinal;
            asset.logicalUrl = QUrl::fromLocalFile(
                QDir(publicRoot.configuredPath).absoluteFilePath(
                    logical.publicRelativePath));

            if (checkoutStore && inventory)
            {
                PrivacyCheckoutStoreError storeError =
                    PrivacyCheckoutStoreError::None;
                QString pathDetail;
                const QString runtimePath = checkoutStore->runtimePathForEntry(
                    *inventory, logical.checkoutRelativePath, &storeError,
                    &pathDetail);

                if (runtimePath.isEmpty())
                {
                    return failure(
                        PrivacyExternalCheckoutStatus::RecoveryRequired,
                        record.transactionUuid, result.itemUuid,
                        pathDetail.isEmpty()
                            ? QStringLiteral("checkout launch path is unavailable")
                            : pathDetail);
                }

                asset.checkoutUrl = QUrl::fromLocalFile(
                    runtimePath);
            }

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

bool PrivacyExternalCheckoutTransactionEngine::holdsPlaintextLease(
    const PrivacyTransaction& transaction)
{
    if (!transaction.isActive() ||
        (transaction.type != PrivacyTransactionType::ExternalCheckout))
    {
        return false;
    }

    PrivacyJournalRecord record;
    QList<LogicalAsset> logicalAssets;
    CheckoutPayloadContext payloadContext;
    PrivacyCheckoutStoreLocation location;

    return !decodePayload(transaction.payloadData, &record, &logicalAssets,
                          &payloadContext) ||
           !checkoutLocation(logicalAssets, transaction.uuid, &location) ||
           (location == PrivacyCheckoutStoreLocation::Checkout);
}

PrivacyExternalCheckoutResult PrivacyExternalCheckoutTransactionEngine::create(
    const PrivacyExternalCheckoutRequest& request)
{
    PrivacyExternalCheckoutStoreAccess storeAccess;
    storeAccess.storeUuid = request.storeUuid;
    storeAccess.root = request.storeRoot;
    storeAccess.rootExpectation = request.storeRootExpectation;
    storeAccess.plaintextRoot = request.storePlaintextRoot;

    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.transactionUuid) ||
        !request.publicRoot.isValid() ||
        (request.publicRoot.kind != PrivacyStorageRootKind::AlbumRoot) ||
        (request.publicRoot.uuid != request.publicRootExpectation.rootUuid) ||
        (request.publicRootExpectation.device == 0) ||
        (request.publicRootExpectation.inode == 0) ||
        (request.publicRootExpectation.identitySha256.size() != 32) ||
        !validStoreAccess(storeAccess) || request.sources.isEmpty())
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
    const PrivacyStore* const categoryStore = categoryStoreFor(
        snapshot, request.categoryUuid, request.storeUuid);
    const PrivacyStorageRoot* const publicRoot = storageRootFor(
        snapshot, request.publicRoot.uuid);
    const PrivacyStorageRoot* const storeRoot = storageRootFor(
        snapshot, request.storeRoot.uuid);

    if (!category || !container || !categoryStore || !publicRoot || !storeRoot ||
        (category->lifecycleState != PrivacyCategoryLifecycleState::Active) ||
        (container->state != PrivacyContainerState::Verified) ||
        (container->rootUuid != request.publicRoot.uuid) ||
        (container->kind != PrivacyContainerKind::CasualArchive) ||
        (categoryStore->categoryUuid != request.categoryUuid) ||
        (categoryStore->rootUuid != request.storeRoot.uuid) ||
        (categoryStore->format != QLatin1String("gocryptfs")) ||
        (categoryStore->lifecycleState != PrivacyStoreLifecycleState::Active) ||
        !sameStorageRoot(*publicRoot, request.publicRoot) ||
        !sameStorageRoot(*storeRoot, request.storeRoot))
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
    created.rootUuid = request.storeRoot.uuid;
    created.rootDevice = request.storeRootExpectation.device;
    created.rootInode = request.storeRootExpectation.inode;
    created.rootIdentitySha256 = request.storeRootExpectation.identitySha256;
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
            (asset.publicRootUuid != request.publicRoot.uuid))
        {
            return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                           request.transactionUuid, item->uuid,
                           QStringLiteral("checkout source does not match protected asset set"));
        }

        QString fileName = asset.originalName.normalized(
            QString::NormalizationForm_C);
        QString relative = PrivacyCheckoutStore::workFileRelativePath(
            request.transactionUuid, fileName);

        if (relative.isEmpty() || checkoutNames.contains(fileName.toCaseFolded()))
        {
            fileName = fallbackCheckoutName(asset);
            relative = PrivacyCheckoutStore::workFileRelativePath(
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
        journalAsset.publicRelativePath = asset.publicRelativePath;
        journalAsset.stagedRelativePath =
            PrivacyCheckoutStore::workFileRelativePath(
                request.transactionUuid, fileName,
                PrivacyCheckoutStoreLocation::Recovery);
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
        logical.checkoutRelativePath = relative;
        logical.modificationDate = asset.originalModificationDate.isValid()
                                 ? asset.originalModificationDate
                                 : QDateTime::fromMSecsSinceEpoch(0).toUTC();
        logicalAssets << logical;
    }

    CheckoutPayloadContext payloadContext;
    payloadContext.storeUuid = request.storeUuid;
    payloadContext.publicRootExpectation = request.publicRootExpectation;
    const QByteArray createdPayload = encodePayload(created, logicalAssets,
                                                    payloadContext);
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
    databaseJournal.rootUuid = request.storeRoot.uuid;
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

    if (!d->advanceFilesystemJournal(request.storeRoot,
                                     request.storeRootExpectation,
                                     created, created, &createdJournalHash,
                                     &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::JournalFailure,
                       request.transactionUuid, item->uuid, detail);
    }

    PrivacyCheckoutStoreError storeError = PrivacyCheckoutStoreError::None;
    std::unique_ptr<PrivacyCheckoutStore> store = PrivacyCheckoutStore::open(
        request.storePlaintextRoot, &storeError, &detail);

    if (!store || !store->createOrOpenTransaction(
            request.transactionUuid, nullptr, &storeError, &detail))
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
            !store->createFile(
                request.transactionUuid,
                QFileInfo(logical.checkoutRelativePath).fileName(),
                journalIt->original.size, journalIt->original.sha256,
                sourceIt->producer, nullptr, &storeError, &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           request.transactionUuid, item->uuid, detail);
        }
    }

    const PrivacyJournalRecord prepared = atStage(
        created, PrivacyJournalStage::Prepared);
    QByteArray preparedHash;

    if (!d->advanceFilesystemJournal(request.storeRoot,
                                     request.storeRootExpectation,
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
    preparedTransaction.payloadData = encodePayload(prepared, logicalAssets,
                                                    payloadContext);
    preparedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

    if (preparedTransaction.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            preparedTransaction, PrivacyTransactionState::Created, 0))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("Prepared checkout requires restart recovery"));
    }

    PrivacyCheckoutInventory inventory;

    if (!store->inventory(request.transactionUuid,
                          PrivacyCheckoutStoreLocation::Checkout,
                          &inventory, &storeError, &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       request.transactionUuid, item->uuid, detail);
    }

    return d->assetsResult(PrivacyExternalCheckoutStatus::Ready,
                           request.publicRoot, prepared, logicalAssets,
                           store.get(), &inventory);
}

PrivacyExternalCheckoutResult
PrivacyExternalCheckoutTransactionEngine::resumeAuthenticatedCreate(
    const PrivacyExternalCheckoutRequest& request)
{
    PrivacyExternalCheckoutStoreAccess storeAccess;
    storeAccess.storeUuid = request.storeUuid;
    storeAccess.root = request.storeRoot;
    storeAccess.rootExpectation = request.storeRootExpectation;
    storeAccess.plaintextRoot = request.storePlaintextRoot;

    if ((request.imageId <= 0) || !canonicalUuid(request.categoryUuid) ||
        !canonicalUuid(request.transactionUuid) ||
        !request.publicRoot.isValid() ||
        (request.publicRoot.kind != PrivacyStorageRootKind::AlbumRoot) ||
        (request.publicRoot.uuid != request.publicRootExpectation.rootUuid) ||
        !validStoreAccess(storeAccess) || request.sources.isEmpty())
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       request.transactionUuid, {},
                       QStringLiteral("External Checkout resume request is invalid"));
    }

    const PrivacyExternalCheckoutResult passive = recover(
        request.storeRoot, request.storeRootExpectation,
        request.transactionUuid);

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
        snapshot, request.transactionUuid, request.storeRoot.uuid);
    PrivacyJournalRecord created;
    QList<LogicalAsset> logicalAssets;
    CheckoutPayloadContext payloadContext;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        (transaction->state != PrivacyTransactionState::Created) ||
        (transaction->generation != 0) ||
        (transaction->categoryUuid != request.categoryUuid) ||
        !decodePayload(transaction->payloadData, &created, &logicalAssets,
                       &payloadContext) ||
        (created.stage != PrivacyJournalStage::Created) ||
        (created.transactionUuid != request.transactionUuid) ||
        (created.categoryUuid != request.categoryUuid) ||
        (created.rootUuid != request.storeRoot.uuid) ||
        (created.rootDevice != request.storeRootExpectation.device) ||
        (created.rootInode != request.storeRootExpectation.inode) ||
        (created.rootIdentitySha256 !=
         request.storeRootExpectation.identitySha256) ||
        (payloadContext.storeUuid != request.storeUuid) ||
        !sameRootExpectation(payloadContext.publicRootExpectation,
                             request.publicRootExpectation) ||
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
        (container->rootUuid != request.publicRoot.uuid) ||
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
            (asset->publicRootUuid != request.publicRoot.uuid) ||
            (asset->publicRelativePath != logical.publicRelativePath) ||
            (asset->protectedRelativePath !=
             journalAsset->protectedRelativePath) ||
            (journalAsset->itemUuid != item->uuid) ||
            (journalAsset->containerUuid != container->uuid) ||
            (journalAsset->containerRelativePath !=
             container->objectRelativePath) ||
            (journalAsset->publicRelativePath != logical.publicRelativePath) ||
            (logical.checkoutRelativePath !=
             PrivacyCheckoutStore::workFileRelativePath(
                 request.transactionUuid,
                 QFileInfo(logical.checkoutRelativePath).fileName())) ||
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
    PrivacyCheckoutStoreError storeError = PrivacyCheckoutStoreError::None;
    std::unique_ptr<PrivacyCheckoutStore> store = PrivacyCheckoutStore::open(
        request.storePlaintextRoot, &storeError, &detail);

    if (!store)
    {
        return failure(PrivacyExternalCheckoutStatus::RootUnavailable,
                       request.transactionUuid, item->uuid, detail);
    }

    const PrivacyJournalRecord prepared = atStage(
        created, PrivacyJournalStage::Prepared);
    PrivacyJournalError journalStoreError = PrivacyJournalError::None;
    std::unique_ptr<PrivacyTransactionJournalStore> journalStore =
        PrivacyTransactionJournalStore::open(
            request.storeRoot.configuredPath,
            request.storeRootExpectation, &journalStoreError, &detail);
    const PrivacyJournalLoadResult loaded = journalStore
        ? journalStore->load(request.transactionUuid)
        : PrivacyJournalLoadResult();

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
            !store->createOrOpenTransaction(
                request.transactionUuid, nullptr, &storeError, &detail) ||
            !store->createFile(
                request.transactionUuid,
                QFileInfo(logical.checkoutRelativePath).fileName(),
                journalAsset->original.size, journalAsset->original.sha256,
                source->producer, nullptr, &storeError, &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           request.transactionUuid, item->uuid, detail);
        }
    }

    QByteArray preparedHash;

    if (!d->advanceFilesystemJournal(request.storeRoot,
                                     request.storeRootExpectation,
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
    preparedTransaction.payloadData = encodePayload(prepared, logicalAssets,
                                                    payloadContext);
    preparedTransaction.updatedAt = QDateTime::currentDateTimeUtc();

    if (preparedTransaction.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            preparedTransaction, PrivacyTransactionState::Created, 0))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       request.transactionUuid, item->uuid,
                       QStringLiteral("resumed checkout needs transaction recovery"));
    }

    PrivacyCheckoutInventory inventory;

    if (!store->inventory(request.transactionUuid,
                          PrivacyCheckoutStoreLocation::Checkout,
                          &inventory, &storeError, &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       request.transactionUuid, item->uuid, detail);
    }

    return d->assetsResult(PrivacyExternalCheckoutStatus::Ready,
                           request.publicRoot, prepared, logicalAssets,
                           store.get(), &inventory);
}

PrivacyExternalCheckoutResult
PrivacyExternalCheckoutTransactionEngine::authorizeLaunch(
    const PrivacyExternalCheckoutStoreAccess& storeAccess,
    const QString& transactionUuid)
{
    PrivacyRepositorySnapshot snapshot;

    if (!validStoreAccess(storeAccess) || !canonicalUuid(transactionUuid) ||
        !d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout launch request is invalid"));
    }

    const PrivacyTransaction* const transaction = transactionFor(
        snapshot, transactionUuid);
    const PrivacyTransactionJournal* const databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, storeAccess.root.uuid);
    PrivacyJournalRecord prepared;
    QList<LogicalAsset> logicalAssets;
    CheckoutPayloadContext payloadContext;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        (transaction->state != PrivacyTransactionState::Prepared) ||
        (transaction->generation != 1) ||
        !decodePayload(transaction->payloadData, &prepared, &logicalAssets,
                       &payloadContext) ||
        (prepared.stage != PrivacyJournalStage::Prepared) ||
        (prepared.rootUuid != storeAccess.root.uuid) ||
        (payloadContext.storeUuid != storeAccess.storeUuid) ||
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
    PrivacyCheckoutStoreError checkoutError = PrivacyCheckoutStoreError::None;
    std::unique_ptr<PrivacyCheckoutStore> checkoutStore =
        PrivacyCheckoutStore::open(storeAccess.plaintextRoot, &checkoutError,
                                   &detail);
    PrivacyCheckoutInventory inventory;
    const PrivacyStorageRoot* const publicRoot = storageRootFor(
        snapshot, payloadContext.publicRootExpectation.rootUuid);

    if (!checkoutStore || !publicRoot ||
        !checkoutStore->reopenTransaction(
            transactionUuid, PrivacyCheckoutStoreLocation::Checkout, nullptr,
            &checkoutError, &detail) ||
        !checkoutStore->inventory(
            transactionUuid, PrivacyCheckoutStoreLocation::Checkout,
            &inventory, &checkoutError, &detail) ||
        !d->advanceFilesystemJournal(storeAccess.root,
                                     storeAccess.rootExpectation,
                                     prepared, exposed,
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
    next.payloadData = encodePayload(exposed, logicalAssets, payloadContext);
    next.updatedAt = QDateTime::currentDateTimeUtc();

    if (next.payloadData.isEmpty() ||
        !d->persistence.compareAndUpdateTransaction(
            next, PrivacyTransactionState::Prepared, 1))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("launch authorization requires restart recovery"));
    }

    return d->assetsResult(PrivacyExternalCheckoutStatus::Ready, *publicRoot,
                           exposed, logicalAssets, checkoutStore.get(),
                           &inventory);
}

PrivacyExternalCheckoutResult PrivacyExternalCheckoutTransactionEngine::reconcile(
    const PrivacyExternalCheckoutStoreAccess& storeAccess,
    const QString& transactionUuid)
{
    PrivacyRepositorySnapshot snapshot;

    if (!validStoreAccess(storeAccess) || !canonicalUuid(transactionUuid) ||
        !d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout reconciliation request is invalid"));
    }

    const PrivacyTransaction* transaction = transactionFor(snapshot,
                                                            transactionUuid);
    const PrivacyTransactionJournal* databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, storeAccess.root.uuid);
    PrivacyJournalRecord record;
    QList<LogicalAsset> logicalAssets;
    CheckoutPayloadContext payloadContext;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        !decodePayload(transaction->payloadData, &record, &logicalAssets,
                       &payloadContext) ||
        (payloadContext.storeUuid != storeAccess.storeUuid) ||
        (record.rootUuid != storeAccess.root.uuid))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid,
                       transaction ? transaction->itemUuid : QString(),
                       QStringLiteral("checkout recovery payload is invalid"));
    }

    const PrivacyStorageRoot* const publicRoot = storageRootFor(
        snapshot, payloadContext.publicRootExpectation.rootUuid);

    if (!publicRoot ||
        (publicRoot->kind != PrivacyStorageRootKind::AlbumRoot) ||
        (publicRoot->markerUuid !=
         payloadContext.publicRootExpectation.markerUuid) ||
        (QCryptographicHash::hash(publicRoot->identityData,
                                  QCryptographicHash::Sha256) !=
         payloadContext.publicRootExpectation.identitySha256))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout public-root identity changed"));
    }

    if (transaction->state == PrivacyTransactionState::Complete)
    {
        return d->assetsResult(
            PrivacyExternalCheckoutStatus::CompletedUnchanged, *publicRoot, record,
            logicalAssets);
    }

    if ((transaction->state != PrivacyTransactionState::Prepared) &&
        (transaction->state != PrivacyTransactionState::Exposed) &&
        (transaction->state != PrivacyTransactionState::Relocking) &&
        (transaction->state != PrivacyTransactionState::NeedsReconciliation))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout state cannot be reconciled directly"));
    }

    PrivacyCheckoutStoreLocation checkoutStoreLocation;

    if (!checkoutLocation(logicalAssets, transactionUuid,
                          &checkoutStoreLocation))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout storage location is invalid"));
    }

    QString detail;
    PrivacyCheckoutStoreError storeError = PrivacyCheckoutStoreError::None;
    std::unique_ptr<PrivacyCheckoutStore> store = PrivacyCheckoutStore::open(
        storeAccess.plaintextRoot, &storeError, &detail);
    PrivacyCheckoutInventory inventory;
    bool inventoryAvailable = store && store->reopenTransaction(
        transactionUuid, checkoutStoreLocation, nullptr,
        &storeError, &detail) && store->inventory(
            transactionUuid, checkoutStoreLocation,
            &inventory, &storeError, &detail);

    // Preserve-for-later moves the encrypted directory before publishing its
    // new relative paths. If publication was interrupted, authenticated
    // reconciliation repairs that narrow gap instead of stranding the result.
    if (!inventoryAvailable && store &&
        (transaction->state == PrivacyTransactionState::NeedsReconciliation) &&
        (checkoutStoreLocation == PrivacyCheckoutStoreLocation::Checkout) &&
        (storeError == PrivacyCheckoutStoreError::Missing) &&
        store->reopenTransaction(
            transactionUuid, PrivacyCheckoutStoreLocation::Recovery, nullptr,
            &storeError, &detail) &&
        store->inventory(
            transactionUuid, PrivacyCheckoutStoreLocation::Recovery,
            &inventory, &storeError, &detail))
    {
        if (!rewriteCheckoutLocation(
                &logicalAssets, transactionUuid,
                PrivacyCheckoutStoreLocation::Recovery))
        {
            return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                           transactionUuid, transaction->itemUuid,
                           QStringLiteral("preserved checkout paths could not be repaired"));
        }

        PrivacyTransaction repaired = *transaction;
        repaired.generation = transaction->generation + 1;
        repaired.payloadData = encodePayload(record, logicalAssets,
                                             payloadContext);
        repaired.updatedAt = QDateTime::currentDateTimeUtc();

        if (repaired.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                repaired, transaction->state, transaction->generation))
        {
            return failure(
                PrivacyExternalCheckoutStatus::PersistenceFailure,
                transactionUuid, transaction->itemUuid,
                QStringLiteral("preserved checkout is safe but its catalogue path still needs recovery"));
        }

        checkoutStoreLocation = PrivacyCheckoutStoreLocation::Recovery;
        inventoryAvailable = true;
    }

    if (!inventoryAvailable &&
        !((transaction->state == PrivacyTransactionState::Relocking) &&
          (storeError == PrivacyCheckoutStoreError::Missing)))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       transactionUuid, transaction->itemUuid, detail);
    }

    if (transaction->state == PrivacyTransactionState::NeedsReconciliation)
    {
        return d->assetsResult(
            PrivacyExternalCheckoutStatus::ChangesPending, *publicRoot, record,
            logicalAssets, store.get(), &inventory,
            QStringLiteral("external changes are preserved for explicit reconciliation"));
    }

    QHash<QString, PrivacyCheckoutInventoryEntry> entries;

    for (const PrivacyCheckoutInventoryEntry& entry :
         std::as_const(inventory.entries))
    {
        entries.insert(entry.storeRelativePath, entry);
    }

    QSet<QString> expectedPaths;
    bool changed = false;

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

        expectedPaths.insert(logical.checkoutRelativePath);
        const auto entry = entries.constFind(logical.checkoutRelativePath);

        if (entry == entries.constEnd())
        {
            changed = true;
        }
        else if ((entry->kind != PrivacyCheckoutEntryKind::RegularFile) ||
                 (entry->size != journalIt->original.size) ||
                 (entry->linkCount != 1) ||
                 (entry->sha256 != journalIt->original.sha256))
        {
            changed = true;
        }
    }

    for (auto it = entries.constBegin() ; it != entries.constEnd() ; ++it)
    {
        if (!expectedPaths.contains(it.key()))
        {
            changed = true;
        }
    }

    const bool cleanupAlreadyRemoved =
        (transaction->state == PrivacyTransactionState::Relocking) &&
        !inventoryAvailable &&
        (storeError == PrivacyCheckoutStoreError::Missing);
    const bool approvedDiscardInventory =
        (transaction->state == PrivacyTransactionState::Relocking) &&
        inventoryAvailable &&
        !payloadContext.approvedDiscardInventorySha256.isEmpty() &&
        (inventory.sha256 ==
         payloadContext.approvedDiscardInventorySha256);

    if (cleanupAlreadyRemoved || approvedDiscardInventory)
    {
        changed = false;
    }

    if (changed)
    {
        const PrivacyJournalRecord pending = atStage(
            record, PrivacyJournalStage::ReconciliationRequired);
        QByteArray pendingHash;

        if ((record.stage != PrivacyJournalStage::ReconciliationRequired) &&
            (!d->advanceFilesystemJournal(storeAccess.root,
                                          storeAccess.rootExpectation, record,
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
            payloadContext.approvedDiscardInventorySha256.clear();
            next.payloadData = encodePayload(pending, logicalAssets,
                                             payloadContext);
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
            PrivacyExternalCheckoutStatus::ChangesPending, *publicRoot, pending,
            logicalAssets, store.get(), &inventory,
            QStringLiteral("external changes are preserved for explicit reconciliation"));
    }

    PrivacyJournalRecord relocking = record;

    if (transaction->state != PrivacyTransactionState::Relocking)
    {
        relocking = atStage(record,
                            PrivacyJournalStage::ReconciliationRequired);
        QByteArray relockingHash;

        if (!d->advanceFilesystemJournal(storeAccess.root,
                                         storeAccess.rootExpectation, record,
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
        next.payloadData = encodePayload(relocking, logicalAssets,
                                         payloadContext);
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
                                                    storeAccess.root.uuid)))
        {
            return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                           transactionUuid, {},
                           QStringLiteral("cannot reload relocking checkout"));
        }
    }

    if (inventoryAvailable &&
        !store->removeExact(inventory, &storeError, &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       transactionUuid, transaction->itemUuid,
                       detail.isEmpty()
                           ? QStringLiteral("unchanged checkout cleanup failed")
                           : detail);
    }

    const PrivacyJournalRecord complete = atStage(
        relocking, PrivacyJournalStage::Complete);
    QByteArray completeHash;

    if (!d->advanceFilesystemJournal(storeAccess.root,
                                     storeAccess.rootExpectation,
                                     relocking, complete,
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
    completed.payloadData = encodePayload(complete, logicalAssets,
                                          payloadContext);
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
        PrivacyExternalCheckoutStatus::CompletedUnchanged, *publicRoot, complete,
        logicalAssets);
}

PrivacyExternalCheckoutResult
PrivacyExternalCheckoutTransactionEngine::resolveChanges(
    const PrivacyExternalCheckoutStoreAccess& storeAccess,
    const QString& transactionUuid,
    PrivacyExternalCheckoutDecision decision)
{
    if (!validStoreAccess(storeAccess) || !canonicalUuid(transactionUuid) ||
        ((decision != PrivacyExternalCheckoutDecision::PreserveForLater) &&
         (decision != PrivacyExternalCheckoutDecision::ConfirmedDiscard)))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout decision request is invalid"));
    }

    PrivacyRepositorySnapshot snapshot;

    if (!d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::PersistenceFailure,
                       transactionUuid, {},
                       QStringLiteral("cannot load checkout decision state"));
    }

    const PrivacyTransaction* const transaction = transactionFor(
        snapshot, transactionUuid);
    const PrivacyTransactionJournal* const databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, storeAccess.root.uuid);
    PrivacyJournalRecord record;
    QList<LogicalAsset> logicalAssets;
    CheckoutPayloadContext payloadContext;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        ((transaction->state != PrivacyTransactionState::NeedsReconciliation) &&
         (transaction->state != PrivacyTransactionState::Relocking)) ||
        !decodePayload(transaction->payloadData, &record, &logicalAssets,
                       &payloadContext) ||
        (record.stage != PrivacyJournalStage::ReconciliationRequired) ||
        (databaseJournal->stage != static_cast<int>(record.stage)) ||
        (record.rootUuid != storeAccess.root.uuid) ||
        (record.rootDevice != storeAccess.rootExpectation.device) ||
        (record.rootInode != storeAccess.rootExpectation.inode) ||
        (record.rootIdentitySha256 !=
         storeAccess.rootExpectation.identitySha256) ||
        (payloadContext.storeUuid != storeAccess.storeUuid))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid,
                       transaction ? transaction->itemUuid : QString(),
                       QStringLiteral("checkout decision evidence is incomplete"));
    }

    if (decision == PrivacyExternalCheckoutDecision::ConfirmedDiscard)
    {
        if (transaction->state == PrivacyTransactionState::NeedsReconciliation)
        {
            PrivacyCheckoutStoreLocation location;
            QString detail;
            PrivacyCheckoutStoreError storeError =
                PrivacyCheckoutStoreError::None;
            std::unique_ptr<PrivacyCheckoutStore> store =
                PrivacyCheckoutStore::open(storeAccess.plaintextRoot,
                                           &storeError, &detail);
            PrivacyCheckoutInventory approvedInventory;

            if (!checkoutLocation(logicalAssets, transactionUuid, &location) ||
                !store ||
                !store->reopenTransaction(transactionUuid, location, nullptr,
                                          &storeError, &detail) ||
                !store->inventory(transactionUuid, location,
                                  &approvedInventory, &storeError, &detail))
            {
                return failure(
                    PrivacyExternalCheckoutStatus::CheckoutFailure,
                    transactionUuid, transaction->itemUuid,
                    detail.isEmpty()
                        ? QStringLiteral("checkout discard inventory is unavailable")
                        : detail);
            }

            payloadContext.approvedDiscardInventorySha256 =
                approvedInventory.sha256;
            PrivacyTransaction relocking = *transaction;
            relocking.state = PrivacyTransactionState::Relocking;
            relocking.generation = transaction->generation + 1;
            relocking.payloadData = encodePayload(record, logicalAssets,
                                                  payloadContext);
            relocking.updatedAt = QDateTime::currentDateTimeUtc();

            if (relocking.payloadData.isEmpty() ||
                !d->persistence.compareAndUpdateTransaction(
                    relocking, transaction->state, transaction->generation))
            {
                return failure(
                    PrivacyExternalCheckoutStatus::PersistenceFailure,
                    transactionUuid, transaction->itemUuid,
                    QStringLiteral("confirmed checkout discard was not recorded"));
            }
        }

        return reconcile(storeAccess, transactionUuid);
    }

    if (transaction->state == PrivacyTransactionState::Relocking)
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("confirmed checkout cleanup is already active"));
    }

    const PrivacyStorageRoot* const publicRoot = storageRootFor(
        snapshot, payloadContext.publicRootExpectation.rootUuid);
    PrivacyCheckoutStoreLocation location;

    if (!publicRoot ||
        !checkoutLocation(logicalAssets, transactionUuid, &location))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("preserved checkout mapping is invalid"));
    }

    QString detail;
    PrivacyCheckoutStoreError storeError = PrivacyCheckoutStoreError::None;
    std::unique_ptr<PrivacyCheckoutStore> store = PrivacyCheckoutStore::open(
        storeAccess.plaintextRoot, &storeError, &detail);
    PrivacyCheckoutInventory inventory;

    if (!store)
    {
        return failure(PrivacyExternalCheckoutStatus::RootUnavailable,
                       transactionUuid, transaction->itemUuid, detail);
    }

    bool recoveredMove = false;

    if (!store->reopenTransaction(transactionUuid, location, nullptr,
                                  &storeError, &detail))
    {
        if ((location != PrivacyCheckoutStoreLocation::Checkout) ||
            (storeError != PrivacyCheckoutStoreError::Missing) ||
            !store->reopenTransaction(
                transactionUuid, PrivacyCheckoutStoreLocation::Recovery,
                nullptr, &storeError, &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           transactionUuid, transaction->itemUuid, detail);
        }

        location = PrivacyCheckoutStoreLocation::Recovery;
        recoveredMove = true;
    }

    if (location == PrivacyCheckoutStoreLocation::Checkout)
    {
        if (!store->moveToRecovery(transactionUuid, nullptr, &storeError,
                                   &detail))
        {
            return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                           transactionUuid, transaction->itemUuid, detail);
        }

        location = PrivacyCheckoutStoreLocation::Recovery;
        recoveredMove = true;
    }

    if ((recoveredMove &&
         !rewriteCheckoutLocation(&logicalAssets, transactionUuid, location)) ||
        !store->inventory(transactionUuid, location, &inventory, &storeError,
                          &detail))
    {
        return failure(PrivacyExternalCheckoutStatus::CheckoutFailure,
                       transactionUuid, transaction->itemUuid, detail);
    }

    if (recoveredMove)
    {
        PrivacyTransaction preserved = *transaction;
        preserved.generation = transaction->generation + 1;
        preserved.payloadData = encodePayload(record, logicalAssets,
                                              payloadContext);
        preserved.updatedAt = QDateTime::currentDateTimeUtc();

        if (preserved.payloadData.isEmpty() ||
            !d->persistence.compareAndUpdateTransaction(
                preserved, transaction->state, transaction->generation))
        {
            return failure(
                PrivacyExternalCheckoutStatus::PersistenceFailure,
                transactionUuid, transaction->itemUuid,
                QStringLiteral("checkout is encrypted in recovery storage but its catalogue path needs recovery"));
        }
    }

    return d->assetsResult(
        PrivacyExternalCheckoutStatus::ChangesPending, *publicRoot, record,
        logicalAssets, store.get(), &inventory,
        QStringLiteral("external changes are preserved for later reconciliation"));
}

PrivacyExternalCheckoutResult PrivacyExternalCheckoutTransactionEngine::recover(
    const PrivacyStorageRoot& root,
    const PrivacyJournalRootExpectation& rootExpectation,
    const QString& transactionUuid)
{
    PrivacyRepositorySnapshot snapshot;

    if (!root.isValid() ||
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot) ||
        (root.uuid != rootExpectation.rootUuid) ||
        !canonicalUuid(transactionUuid) || !d->load(&snapshot))
    {
        return failure(PrivacyExternalCheckoutStatus::InvalidRequest,
                       transactionUuid, {},
                       QStringLiteral("checkout recovery request is invalid"));
    }

    const PrivacyTransaction* const transaction = transactionFor(
        snapshot, transactionUuid);
    const PrivacyTransactionJournal* const databaseJournal = databaseJournalFor(
        snapshot, transactionUuid, root.uuid);
    PrivacyJournalRecord record;
    QList<LogicalAsset> logicalAssets;
    CheckoutPayloadContext payloadContext;

    if (!transaction || !databaseJournal ||
        (transaction->type != PrivacyTransactionType::ExternalCheckout) ||
        !decodePayload(transaction->payloadData, &record, &logicalAssets,
                       &payloadContext) ||
        (record.rootUuid != root.uuid))
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, {},
                       QStringLiteral("External Checkout transaction is unavailable"));
    }

    const PrivacyStorageRoot* const publicRoot = storageRootFor(
        snapshot, payloadContext.publicRootExpectation.rootUuid);

    if (!publicRoot)
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout public root is unavailable"));
    }

    if (transaction->state == PrivacyTransactionState::Complete)
    {
        return d->assetsResult(
            PrivacyExternalCheckoutStatus::CompletedUnchanged, *publicRoot,
            record, logicalAssets);
    }

    if (transaction->state == PrivacyTransactionState::NeedsReconciliation)
    {
        return d->assetsResult(
            PrivacyExternalCheckoutStatus::ChangesPending, *publicRoot,
            record, logicalAssets, nullptr, nullptr,
            QStringLiteral("unlock the category to reconcile preserved external changes"));
    }

    if (!transaction->isActive())
    {
        return failure(PrivacyExternalCheckoutStatus::RecoveryRequired,
                       transactionUuid, transaction->itemUuid,
                       QStringLiteral("checkout transaction requires manual recovery"));
    }

    return failure(
        PrivacyExternalCheckoutStatus::AuthenticationRequired,
        transactionUuid, transaction->itemUuid,
        QStringLiteral("unlock the category to inspect its encrypted checkout"));
}

} // namespace Digikam

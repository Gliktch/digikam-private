/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacytransactionjournal.h"
#include "privacyposixstorage_p.h"

// C++ includes

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <limits>
#include <utility>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QStringList>
#include <QUuid>

#ifdef Q_OS_UNIX
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

namespace Digikam
{

namespace
{

constexpr qsizetype MaximumRelativePathBytes = 4096;
constexpr qsizetype MaximumComponentBytes    = 255;
constexpr qsizetype MaximumIntentBytes       = 65;

const QString MetadataDirectory    = QStringLiteral(".digikam-private");
const QString TransactionsDirectory = QStringLiteral("transactions");
const QString JournalFile          = QStringLiteral("journal-v1.json");
const QString NextFile             = QStringLiteral("journal-v1.next");
const QString IntentFile           = QStringLiteral("journal-v1.intent");

void setError(PrivacyJournalError* const error, PrivacyJournalError value,
              QString* const detail, const QString& message)
{
    if (error)
    {
        *error = value;
    }

    if (detail)
    {
        *detail = message;
    }
}

bool canonicalUuid(const QString& value)
{
    const QUuid uuid(value);

    return (!uuid.isNull() &&
            (value == uuid.toString(QUuid::WithoutBraces)));
}

bool validSha256(const QByteArray& value)
{
    return (value.size() == QCryptographicHash::hashLength(
                QCryptographicHash::Sha256));
}

QString hexSha256(const QByteArray& value)
{
    return QString::fromLatin1(value.toHex());
}

bool parseHexSha256(const QJsonValue& value, QByteArray* const result)
{
    if (!value.isString())
    {
        return false;
    }

    const QByteArray encoded = value.toString().toLatin1();

    if (encoded.size() != 64)
    {
        return false;
    }

    for (const char character : encoded)
    {
        if (!(((character >= '0') && (character <= '9')) ||
              ((character >= 'a') && (character <= 'f'))))
        {
            return false;
        }
    }

    const QByteArray decoded = QByteArray::fromHex(encoded);

    if (!validSha256(decoded) || (decoded.toHex() != encoded))
    {
        return false;
    }

    *result = decoded;

    return true;
}

QString decimalSigned(qlonglong value)
{
    return QString::number(value);
}

QString decimalUnsigned(quint64 value)
{
    return QString::number(value);
}

bool parseSigned(const QJsonValue& value, qlonglong minimum,
                 qlonglong* const result)
{
    if (!value.isString())
    {
        return false;
    }

    const QString encoded = value.toString();

    if (encoded.isEmpty() || encoded.startsWith(QLatin1Char('+')) ||
        ((encoded.size() > 1) && encoded.startsWith(QLatin1Char('0'))) ||
        encoded == QLatin1String("-0"))
    {
        return false;
    }

    bool ok = false;
    const qlonglong parsed = encoded.toLongLong(&ok, 10);

    if (!ok || (parsed < minimum) || (decimalSigned(parsed) != encoded))
    {
        return false;
    }

    *result = parsed;

    return true;
}

bool parseUnsigned(const QJsonValue& value, quint64* const result)
{
    if (!value.isString())
    {
        return false;
    }

    const QString encoded = value.toString();

    if (encoded.isEmpty() || encoded.startsWith(QLatin1Char('+')) ||
        ((encoded.size() > 1) && encoded.startsWith(QLatin1Char('0'))))
    {
        return false;
    }

    bool ok = false;
    const quint64 parsed = encoded.toULongLong(&ok, 10);

    if (!ok || (decimalUnsigned(parsed) != encoded))
    {
        return false;
    }

    *result = parsed;

    return true;
}

bool safeRelativePath(const QString& path, bool allowEmpty = false)
{
    if (path.isEmpty())
    {
        return allowEmpty;
    }

    if (QDir::isAbsolutePath(path) || path.contains(QChar::Null) ||
        path.contains(QLatin1Char('\\')) ||
        (path != path.normalized(QString::NormalizationForm_C)))
    {
        return false;
    }

    const QByteArray encoded = path.toUtf8();

    if (encoded.isEmpty() || (encoded.size() > MaximumRelativePathBytes) ||
        (QString::fromUtf8(encoded) != path))
    {
        return false;
    }

    const QStringList components = path.split(QLatin1Char('/'));

    for (const QString& component : components)
    {
        const QByteArray componentBytes = component.toUtf8();

        if (component.isEmpty() || (component == QLatin1String(".")) ||
            (component == QLatin1String("..")) ||
            (componentBytes.size() > MaximumComponentBytes))
        {
            return false;
        }

        for (const QChar character : component)
        {
            if (character.category() == QChar::Other_Control)
            {
                return false;
            }
        }
    }

    return true;
}

bool validTransactionType(PrivacyTransactionType type)
{
    const int value = static_cast<int>(type);

    return ((value >= static_cast<int>(PrivacyTransactionType::ProtectItem)) &&
            (value <= static_cast<int>(PrivacyTransactionType::CreateCategory)));
}

bool validStage(PrivacyJournalStage stage)
{
    const int value = static_cast<int>(stage);

    return ((value >= static_cast<int>(PrivacyJournalStage::Created)) &&
            (value <= static_cast<int>(PrivacyJournalStage::Complete)));
}

bool validPresence(PrivacyJournalExpectedPresence presence)
{
    const int value = static_cast<int>(presence);

    return ((value >= static_cast<int>(PrivacyJournalExpectedPresence::Absent)) &&
            (value <= static_cast<int>(PrivacyJournalExpectedPresence::Unknown)));
}

bool validFact(const PrivacyJournalObjectFact& fact)
{
    if (!validPresence(fact.presence))
    {
        return false;
    }

    if (fact.presence == PrivacyJournalExpectedPresence::Present)
    {
        return ((fact.size >= 0) && (fact.linkCount >= 1) &&
                validSha256(fact.sha256));
    }

    return ((fact.size == -1) && (fact.linkCount == 0) &&
            fact.sha256.isEmpty());
}

bool sameFact(const PrivacyJournalObjectFact& left,
              const PrivacyJournalObjectFact& right)
{
    return ((left.presence == right.presence) && (left.size == right.size) &&
            (left.linkCount == right.linkCount) &&
            (left.sha256 == right.sha256));
}

QJsonObject factObject(const PrivacyJournalObjectFact& fact)
{
    QJsonObject object;
    object.insert(QStringLiteral("linkCount"), decimalUnsigned(fact.linkCount));
    object.insert(QStringLiteral("presence"), static_cast<int>(fact.presence));
    object.insert(QStringLiteral("sha256"), hexSha256(fact.sha256));
    object.insert(QStringLiteral("size"), decimalSigned(fact.size));

    return object;
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

bool decodeFact(const QJsonValue& value, PrivacyJournalObjectFact* const fact)
{
    if (!value.isObject())
    {
        return false;
    }

    const QJsonObject object = value.toObject();

    if (!exactKeys(object, { "linkCount", "presence", "sha256", "size" }) ||
        !object.value(QStringLiteral("presence")).isDouble())
    {
        return false;
    }

    const double presenceNumber = object.value(QStringLiteral("presence")).toDouble();

    if ((presenceNumber != static_cast<int>(presenceNumber)) ||
        (presenceNumber < 1.0) || (presenceNumber > 3.0))
    {
        return false;
    }

    fact->presence = static_cast<PrivacyJournalExpectedPresence>(
        static_cast<int>(presenceNumber));

    if (!parseUnsigned(object.value(QStringLiteral("linkCount")),
                       &fact->linkCount) ||
        !parseSigned(object.value(QStringLiteral("size")), -1, &fact->size) ||
        !object.value(QStringLiteral("sha256")).isString())
    {
        return false;
    }

    const QString hash = object.value(QStringLiteral("sha256")).toString();

    if (hash.isEmpty())
    {
        fact->sha256.clear();
    }
    else if (!parseHexSha256(object.value(QStringLiteral("sha256")), &fact->sha256))
    {
        return false;
    }

    return validFact(*fact);
}

bool assetLess(const PrivacyJournalAsset& left,
               const PrivacyJournalAsset& right)
{
    if (left.itemUuid != right.itemUuid)
    {
        return (left.itemUuid < right.itemUuid);
    }

    if (left.role != right.role)
    {
        return (left.role < right.role);
    }

    return (left.ordinal < right.ordinal);
}

QJsonObject assetObject(const PrivacyJournalAsset& asset)
{
    QJsonObject object;
    object.insert(QStringLiteral("container"), factObject(asset.container));
    object.insert(QStringLiteral("containerRelativePath"), asset.containerRelativePath);
    object.insert(QStringLiteral("containerUuid"), asset.containerUuid);
    object.insert(QStringLiteral("itemUuid"), asset.itemUuid);
    object.insert(QStringLiteral("ordinal"), asset.ordinal);
    object.insert(QStringLiteral("original"), factObject(asset.original));
    object.insert(QStringLiteral("protectedRelativePath"), asset.protectedRelativePath);
    object.insert(QStringLiteral("proxy"), factObject(asset.proxy));
    object.insert(QStringLiteral("publicRelativePath"), asset.publicRelativePath);
    object.insert(QStringLiteral("role"), asset.role);
    object.insert(QStringLiteral("stagedRelativePath"), asset.stagedRelativePath);

    return object;
}

QByteArray encodeUnchecked(const PrivacyJournalRecord& record)
{
    QList<PrivacyJournalAsset> assets = record.assets;
    std::sort(assets.begin(), assets.end(), assetLess);

    QJsonArray assetArray;

    for (const PrivacyJournalAsset& asset : std::as_const(assets))
    {
        assetArray.append(assetObject(asset));
    }

    QJsonObject object;
    object.insert(QStringLiteral("assets"), assetArray);
    object.insert(QStringLiteral("categoryUuid"), record.categoryUuid);
    object.insert(QStringLiteral("credentialGeneration"),
                  decimalSigned(record.credentialGeneration));
    object.insert(QStringLiteral("formatVersion"), record.formatVersion);
    object.insert(QStringLiteral("fromCredentialGeneration"),
                  decimalSigned(record.fromCredentialGeneration));
    object.insert(QStringLiteral("generation"), decimalSigned(record.generation));
    object.insert(QStringLiteral("rootDevice"), decimalUnsigned(record.rootDevice));
    object.insert(QStringLiteral("rootIdentitySha256"),
                  hexSha256(record.rootIdentitySha256));
    object.insert(QStringLiteral("rootInode"), decimalUnsigned(record.rootInode));
    object.insert(QStringLiteral("rootUuid"), record.rootUuid);
    object.insert(QStringLiteral("stage"), static_cast<int>(record.stage));
    object.insert(QStringLiteral("toCredentialGeneration"),
                  decimalSigned(record.toCredentialGeneration));
    object.insert(QStringLiteral("transactionType"),
                  static_cast<int>(record.transactionType));
    object.insert(QStringLiteral("transactionUuid"), record.transactionUuid);

    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool decodeAsset(const QJsonValue& value, PrivacyJournalAsset* const asset)
{
    if (!value.isObject())
    {
        return false;
    }

    const QJsonObject object = value.toObject();

    if (!exactKeys(object,
                   { "container", "containerRelativePath", "containerUuid",
                     "itemUuid", "ordinal", "original", "protectedRelativePath",
                     "proxy", "publicRelativePath", "role", "stagedRelativePath" }))
    {
        return false;
    }

    const QStringList stringKeys =
    {
        QStringLiteral("containerRelativePath"),
        QStringLiteral("containerUuid"),
        QStringLiteral("itemUuid"),
        QStringLiteral("protectedRelativePath"),
        QStringLiteral("publicRelativePath"),
        QStringLiteral("stagedRelativePath")
    };

    for (const QString& key : stringKeys)
    {
        if (!object.value(key).isString())
        {
            return false;
        }
    }

    if (!object.value(QStringLiteral("role")).isDouble() ||
        !object.value(QStringLiteral("ordinal")).isDouble())
    {
        return false;
    }

    const double role = object.value(QStringLiteral("role")).toDouble();
    const double ordinal = object.value(QStringLiteral("ordinal")).toDouble();

    if ((role < 1.0) || (role > std::numeric_limits<int>::max()) ||
        (ordinal < 0.0) || (ordinal > std::numeric_limits<int>::max()) ||
        (role != static_cast<int>(role)) ||
        (ordinal != static_cast<int>(ordinal)))
    {
        return false;
    }

    asset->itemUuid                 = object.value(QStringLiteral("itemUuid")).toString();
    asset->containerUuid            = object.value(QStringLiteral("containerUuid")).toString();
    asset->role                     = static_cast<int>(role);
    asset->ordinal                  = static_cast<int>(ordinal);
    asset->publicRelativePath       = object.value(QStringLiteral("publicRelativePath")).toString();
    asset->stagedRelativePath       = object.value(QStringLiteral("stagedRelativePath")).toString();
    asset->protectedRelativePath    = object.value(QStringLiteral("protectedRelativePath")).toString();
    asset->containerRelativePath    = object.value(QStringLiteral("containerRelativePath")).toString();

    return (decodeFact(object.value(QStringLiteral("original")), &asset->original) &&
            decodeFact(object.value(QStringLiteral("proxy")), &asset->proxy) &&
            decodeFact(object.value(QStringLiteral("container")), &asset->container));
}

bool immutableIdentityMatches(const PrivacyJournalRecord& left,
                              const PrivacyJournalRecord& right)
{
    if ((left.transactionUuid != right.transactionUuid) ||
        (left.categoryUuid != right.categoryUuid) ||
        (left.rootUuid != right.rootUuid) ||
        (left.rootDevice != right.rootDevice) ||
        (left.rootInode != right.rootInode) ||
        (left.rootIdentitySha256 != right.rootIdentitySha256) ||
        (left.transactionType != right.transactionType) ||
        (left.generation != right.generation) ||
        (left.credentialGeneration != right.credentialGeneration) ||
        (left.fromCredentialGeneration != right.fromCredentialGeneration) ||
        (left.toCredentialGeneration != right.toCredentialGeneration) ||
        (left.assets.size() != right.assets.size()))
    {
        return false;
    }

    QList<PrivacyJournalAsset> leftAssets = left.assets;
    QList<PrivacyJournalAsset> rightAssets = right.assets;
    std::sort(leftAssets.begin(), leftAssets.end(), assetLess);
    std::sort(rightAssets.begin(), rightAssets.end(), assetLess);

    for (qsizetype index = 0 ; index < leftAssets.size() ; ++index)
    {
        const PrivacyJournalAsset& a = leftAssets.at(index);
        const PrivacyJournalAsset& b = rightAssets.at(index);

        if ((a.itemUuid != b.itemUuid) || (a.containerUuid != b.containerUuid) ||
            (a.role != b.role) || (a.ordinal != b.ordinal) ||
            (a.publicRelativePath != b.publicRelativePath) ||
            (a.stagedRelativePath != b.stagedRelativePath) ||
            (a.protectedRelativePath != b.protectedRelativePath) ||
            (a.containerRelativePath != b.containerRelativePath))
        {
            return false;
        }
    }

    return true;
}

} // namespace

bool PrivacyTransactionJournalCodec::validate(const PrivacyJournalRecord& record,
                                               QString* const detail)
{
    auto invalid = [detail](const QString& message)
    {
        if (detail)
        {
            *detail = message;
        }

        return false;
    };

    if ((record.formatVersion != FormatVersion) ||
        !canonicalUuid(record.transactionUuid) ||
        !canonicalUuid(record.categoryUuid) || !canonicalUuid(record.rootUuid) ||
        !validSha256(record.rootIdentitySha256) || (record.rootDevice == 0) ||
        (record.rootInode == 0) || !validTransactionType(record.transactionType) ||
        (record.generation < 0) || (record.credentialGeneration < -1) ||
        (record.fromCredentialGeneration < -1) ||
        (record.toCredentialGeneration < -1) || !validStage(record.stage))
    {
        return invalid(QStringLiteral("invalid journal header"));
    }

    const bool createCategory =
        (record.transactionType == PrivacyTransactionType::CreateCategory);
    const bool validCreateHeader =
        (((record.stage == PrivacyJournalStage::Created) &&
          (record.generation == 0) && (record.credentialGeneration == -1) &&
          (record.fromCredentialGeneration == -1) &&
          (record.toCredentialGeneration == -1)) ||
         ((record.stage == PrivacyJournalStage::Complete) &&
          (record.generation == 1) && (record.credentialGeneration == 1) &&
          (record.fromCredentialGeneration == -1) &&
          (record.toCredentialGeneration == 1)));

    if ((createCategory && (!record.assets.isEmpty() || !validCreateHeader)) ||
        (!createCategory && record.assets.isEmpty()) ||
        (record.assets.size() > MaximumAssetCount))
    {
        return invalid(QStringLiteral("invalid asset count"));
    }

    QSet<QString> identities;
    QSet<QString> publicPaths;
    QSet<QString> stagedPaths;
    QSet<QString> protectedPaths;
    QSet<QString> physicalPaths;
    QHash<QString, QString> containerOwners;
    QHash<QString, PrivacyJournalObjectFact> containerFacts;

    for (const PrivacyJournalAsset& asset : record.assets)
    {
        if (!canonicalUuid(asset.itemUuid) || !canonicalUuid(asset.containerUuid) ||
            (asset.role <= 0) || (asset.ordinal < 0) ||
            !safeRelativePath(asset.publicRelativePath) ||
            !safeRelativePath(asset.stagedRelativePath, true) ||
            !safeRelativePath(asset.protectedRelativePath, true) ||
            !safeRelativePath(asset.containerRelativePath, true) ||
            !validFact(asset.original) || !validFact(asset.proxy) ||
            !validFact(asset.container))
        {
            return invalid(QStringLiteral("invalid asset record"));
        }

        const QString identity = QStringLiteral("%1:%2:%3")
                                     .arg(asset.itemUuid)
                                     .arg(asset.role)
                                     .arg(asset.ordinal);

        if (identities.contains(identity))
        {
            return invalid(QStringLiteral("duplicate asset identity"));
        }

        identities.insert(identity);

        const auto insertUniquePath = [&invalid](const QString& path,
                                                  QSet<QString>* const paths,
                                                  const QString& label)
        {
            if (path.isEmpty())
            {
                return true;
            }

            const QString key = path.toCaseFolded();

            if (paths->contains(key))
            {
                return invalid(QStringLiteral("%1 path collision").arg(label));
            }

            paths->insert(key);

            return true;
        };

        if (!insertUniquePath(asset.publicRelativePath, &publicPaths,
                              QStringLiteral("public")) ||
            !insertUniquePath(asset.stagedRelativePath, &stagedPaths,
                              QStringLiteral("staged")) ||
            !insertUniquePath(asset.protectedRelativePath, &protectedPaths,
                              QStringLiteral("protected")))
        {
            return false;
        }

        for (const QString& physicalPath :
             { asset.publicRelativePath, asset.stagedRelativePath })
        {
            const QString key = physicalPath.toCaseFolded();

            if (!physicalPath.isEmpty() && physicalPaths.contains(key))
            {
                return invalid(QStringLiteral("physical path collision"));
            }

            if (!physicalPath.isEmpty())
            {
                physicalPaths.insert(key);
            }
        }

        if (!asset.containerRelativePath.isEmpty())
        {
            const QString key = asset.containerRelativePath.toCaseFolded();

            if (physicalPaths.contains(key) && !containerOwners.contains(key))
            {
                return invalid(QStringLiteral("container/public path collision"));
            }

            if (containerOwners.contains(key) &&
                ((containerOwners.value(key) != asset.containerUuid) ||
                 !sameFact(containerFacts.value(key), asset.container)))
            {
                return invalid(QStringLiteral("container path collision"));
            }

            containerOwners.insert(key, asset.containerUuid);
            containerFacts.insert(key, asset.container);
            physicalPaths.insert(key);
        }
    }

    return true;
}

QByteArray PrivacyTransactionJournalCodec::encode(
    const PrivacyJournalRecord& record, PrivacyJournalError* const error,
    QString* const detail)
{
    QString validationDetail;

    if (!validate(record, &validationDetail))
    {
        setError(error, PrivacyJournalError::InvalidRecord, detail,
                 validationDetail);
        return {};
    }

    const QByteArray bytes = encodeUnchecked(record);

    if (bytes.size() > MaximumEncodedBytes)
    {
        setError(error, PrivacyJournalError::EncodingTooLarge, detail,
                 QStringLiteral("journal exceeds encoded size limit"));
        return {};
    }

    setError(error, PrivacyJournalError::None, detail, {});

    return bytes;
}

bool PrivacyTransactionJournalCodec::decode(
    const QByteArray& bytes, PrivacyJournalRecord* const record,
    PrivacyJournalError* const error, QString* const detail)
{
    if (!record || bytes.isEmpty() || (bytes.size() > MaximumEncodedBytes))
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal size is invalid"));
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);

    if ((parseError.error != QJsonParseError::NoError) || !document.isObject())
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal is not valid JSON"));
        return false;
    }

    const QJsonObject object = document.object();

    if (!exactKeys(object,
                   { "assets", "categoryUuid", "credentialGeneration",
                     "formatVersion", "fromCredentialGeneration", "generation",
                     "rootDevice", "rootIdentitySha256", "rootInode", "rootUuid",
                     "stage", "toCredentialGeneration", "transactionType",
                     "transactionUuid" }) ||
        !object.value(QStringLiteral("formatVersion")).isDouble() ||
        !object.value(QStringLiteral("stage")).isDouble() ||
        !object.value(QStringLiteral("transactionType")).isDouble() ||
        !object.value(QStringLiteral("assets")).isArray())
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal object shape is invalid"));
        return false;
    }

    const QStringList stringKeys =
    {
        QStringLiteral("categoryUuid"), QStringLiteral("credentialGeneration"),
        QStringLiteral("fromCredentialGeneration"), QStringLiteral("generation"),
        QStringLiteral("rootDevice"), QStringLiteral("rootIdentitySha256"),
        QStringLiteral("rootInode"), QStringLiteral("rootUuid"),
        QStringLiteral("toCredentialGeneration"), QStringLiteral("transactionUuid")
    };

    for (const QString& key : stringKeys)
    {
        if (!object.value(key).isString())
        {
            setError(error, PrivacyJournalError::CorruptJournal, detail,
                     QStringLiteral("journal scalar type is invalid"));
            return false;
        }
    }

    PrivacyJournalRecord decoded;
    decoded.formatVersion = object.value(QStringLiteral("formatVersion")).toInt(-1);
    decoded.transactionUuid = object.value(QStringLiteral("transactionUuid")).toString();
    decoded.categoryUuid = object.value(QStringLiteral("categoryUuid")).toString();
    decoded.rootUuid = object.value(QStringLiteral("rootUuid")).toString();

    if (!parseUnsigned(object.value(QStringLiteral("rootDevice")), &decoded.rootDevice) ||
        !parseUnsigned(object.value(QStringLiteral("rootInode")), &decoded.rootInode) ||
        !parseHexSha256(object.value(QStringLiteral("rootIdentitySha256")),
                        &decoded.rootIdentitySha256) ||
        !parseSigned(object.value(QStringLiteral("generation")), 0,
                     &decoded.generation) ||
        !parseSigned(object.value(QStringLiteral("credentialGeneration")), -1,
                     &decoded.credentialGeneration) ||
        !parseSigned(object.value(QStringLiteral("fromCredentialGeneration")), -1,
                     &decoded.fromCredentialGeneration) ||
        !parseSigned(object.value(QStringLiteral("toCredentialGeneration")), -1,
                     &decoded.toCredentialGeneration))
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal numeric or hash value is invalid"));
        return false;
    }

    const double stage = object.value(QStringLiteral("stage")).toDouble();
    const double type = object.value(QStringLiteral("transactionType")).toDouble();

    if ((stage != static_cast<int>(stage)) ||
        (type != static_cast<int>(type)))
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal enum value is invalid"));
        return false;
    }

    decoded.stage = static_cast<PrivacyJournalStage>(static_cast<int>(stage));
    decoded.transactionType = static_cast<PrivacyTransactionType>(static_cast<int>(type));

    const QJsonArray assets = object.value(QStringLiteral("assets")).toArray();

    if ((assets.size() > MaximumAssetCount) ||
        (assets.isEmpty() &&
         (decoded.transactionType != PrivacyTransactionType::CreateCategory)))
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal asset count is invalid"));
        return false;
    }

    for (const QJsonValue& value : assets)
    {
        PrivacyJournalAsset asset;

        if (!decodeAsset(value, &asset))
        {
            setError(error, PrivacyJournalError::CorruptJournal, detail,
                     QStringLiteral("journal asset is invalid"));
            return false;
        }

        decoded.assets.append(asset);
    }

    QString validationDetail;

    if (!validate(decoded, &validationDetail) ||
        (encodeUnchecked(decoded) != bytes))
    {
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 validationDetail.isEmpty()
                     ? QStringLiteral("journal is not canonical")
                     : validationDetail);
        return false;
    }

    *record = decoded;
    setError(error, PrivacyJournalError::None, detail, {});

    return true;
}

QByteArray PrivacyTransactionJournalCodec::sha256(const QByteArray& canonicalBytes)
{
    return QCryptographicHash::hash(canonicalBytes, QCryptographicHash::Sha256);
}

QString PrivacyTransactionJournalCodec::relativeJournalPath(
    const QString& transactionUuid)
{
    if (!canonicalUuid(transactionUuid))
    {
        return {};
    }

    return QStringLiteral("%1/%2/%3/%4")
        .arg(MetadataDirectory, TransactionsDirectory, transactionUuid, JournalFile);
}

namespace
{

#ifdef Q_OS_UNIX

enum class EntryReadStatus
{
    Missing,
    Ok,
    Unsafe,
    IoFailure,
    TooLarge
};

bool syncFd(int fd)
{
    return (::fsync(fd) == 0);
}

bool writeAll(int fd, const QByteArray& bytes)
{
    qsizetype offset = 0;

    while (offset < bytes.size())
    {
        const ssize_t written = ::write(fd, bytes.constData() + offset,
                                        static_cast<size_t>(bytes.size() - offset));

        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return false;
        }

        if (written == 0)
        {
            return false;
        }

        offset += static_cast<qsizetype>(written);
    }

    return true;
}

bool verifyOpenedOwnedRegular(int fd, dev_t expectedDevice,
                              QString* const detail)
{
    struct stat status = {};

    if ((::fstat(fd, &status) != 0) || !S_ISREG(status.st_mode) ||
        (status.st_uid != ::geteuid()) || (status.st_nlink != 1) ||
        (status.st_dev != expectedDevice) ||
        ((status.st_mode & 0777) != 0600))
    {
        if (detail)
        {
            *detail = QStringLiteral("opened entry is not a safe 0600 owned regular file");
        }

        return false;
    }

    return true;
}

EntryReadStatus readOwnedRegularAt(int directoryFd, const QByteArray& name,
                                   dev_t expectedDevice, qsizetype maximumBytes,
                                   QByteArray* const bytes, QString* const detail,
                                   bool exact0600 = true)
{
    const int fd = PrivacyPosixStorage::confinedOpenAt(
        directoryFd, name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);

    if (fd < 0)
    {
        if (errno == ENOENT)
        {
            return EntryReadStatus::Missing;
        }

        struct stat pathStatus = {};

        if ((::fstatat(directoryFd, name.constData(), &pathStatus,
                       AT_SYMLINK_NOFOLLOW) == 0) &&
            (!S_ISREG(pathStatus.st_mode) ||
             (pathStatus.st_uid != ::geteuid()) ||
             (pathStatus.st_nlink != 1) ||
             (pathStatus.st_dev != expectedDevice) ||
             (exact0600 ? ((pathStatus.st_mode & 0777) != 0600)
                        : ((pathStatus.st_mode & 0022) != 0))))
        {
            if (detail)
            {
                *detail = QStringLiteral("unsafe unopened entry type, ownership, link count, mode, or device for %1")
                              .arg(QString::fromUtf8(name));
            }

            return EntryReadStatus::Unsafe;
        }

        if (detail)
        {
            *detail = QStringLiteral("cannot open %1: %2")
                          .arg(QString::fromUtf8(name),
                               QString::fromLocal8Bit(std::strerror(errno)));
        }

        return ((errno == ELOOP) || (errno == EXDEV))
                   ? EntryReadStatus::Unsafe
                   : EntryReadStatus::IoFailure;
    }

    struct stat status = {};
    const bool safe = (::fstat(fd, &status) == 0) && S_ISREG(status.st_mode) &&
                      (status.st_uid == ::geteuid()) && (status.st_nlink == 1) &&
                      (status.st_dev == expectedDevice) &&
                      (exact0600 ? ((status.st_mode & 0777) == 0600)
                                 : ((status.st_mode & 0022) == 0));

    if (!safe)
    {
        ::close(fd);

        if (detail)
        {
            *detail = QStringLiteral("unsafe ownership, type, link count, mode, or device for %1")
                          .arg(QString::fromUtf8(name));
        }

        return EntryReadStatus::Unsafe;
    }

    if ((status.st_size < 0) || (status.st_size > maximumBytes))
    {
        ::close(fd);
        return EntryReadStatus::TooLarge;
    }

    QByteArray result;
    result.resize(static_cast<qsizetype>(status.st_size));
    qsizetype offset = 0;

    while (offset < result.size())
    {
        const ssize_t count = ::read(fd, result.data() + offset,
                                     static_cast<size_t>(result.size() - offset));

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            ::close(fd);
            return EntryReadStatus::IoFailure;
        }

        if (count == 0)
        {
            ::close(fd);
            return EntryReadStatus::IoFailure;
        }

        offset += static_cast<qsizetype>(count);
    }

    struct stat after = {};
    bool unchanged = (::fstat(fd, &after) == 0) &&
                     (after.st_dev == status.st_dev) &&
                     (after.st_ino == status.st_ino) &&
                     (after.st_size == status.st_size);

#ifdef Q_OS_LINUX
    unchanged = unchanged &&
                (after.st_mtim.tv_sec == status.st_mtim.tv_sec) &&
                (after.st_mtim.tv_nsec == status.st_mtim.tv_nsec) &&
                (after.st_ctim.tv_sec == status.st_ctim.tv_sec) &&
                (after.st_ctim.tv_nsec == status.st_ctim.tv_nsec);
#else
    unchanged = unchanged && (after.st_mtime == status.st_mtime) &&
                (after.st_ctime == status.st_ctime);
#endif
    ::close(fd);

    if (!unchanged)
    {
        return EntryReadStatus::IoFailure;
    }

    *bytes = result;

    return EntryReadStatus::Ok;
}

bool verifyOwnedDirectory(int fd, dev_t device, QString* const detail)
{
    struct stat status = {};

    if ((::fstat(fd, &status) != 0) || !S_ISDIR(status.st_mode) ||
        (status.st_uid != ::geteuid()) || (status.st_dev != device) ||
        ((status.st_mode & 0777) != 0700))
    {
        if (detail)
        {
            *detail = QStringLiteral("unsafe private directory");
        }

        return false;
    }

    return true;
}

int openOwnedDirectoryAt(int parentFd, const QByteArray& name, dev_t device,
                         bool create, bool* const created, QString* const detail)
{
    if (created)
    {
        *created = false;
    }

    if (create && (::mkdirat(parentFd, name.constData(), 0700) == 0))
    {
        if (created)
        {
            *created = true;
        }

        if (!syncFd(parentFd))
        {
            if (detail)
            {
                *detail = QStringLiteral("cannot fsync parent after directory creation");
            }

            return -1;
        }
    }
    else if (create && (errno != EEXIST))
    {
        if (detail)
        {
            *detail = QStringLiteral("cannot create private directory: %1")
                          .arg(QString::fromLocal8Bit(std::strerror(errno)));
        }

        return -1;
    }

    const int fd = PrivacyPosixStorage::confinedOpenAt(
        parentFd, name, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);

    if (fd < 0)
    {
        if (detail)
        {
            *detail = QStringLiteral("cannot open private directory: %1")
                          .arg(QString::fromLocal8Bit(std::strerror(errno)));
        }

        return -1;
    }

    if (!verifyOwnedDirectory(fd, device, detail))
    {
        ::close(fd);
        errno = EPERM;
        return -1;
    }

    return fd;
}

#endif // Q_OS_UNIX

} // namespace

class PrivacyTransactionJournalStore::Private
{
public:

    ~Private()
    {
#ifdef Q_OS_UNIX
        if (rootFd >= 0)
        {
            ::close(rootFd);
        }
#endif
    }

    int rootFd = -1;
    quint64 device = 0;
    quint64 inode = 0;
    PrivacyJournalRootExpectation expectation;
    FaultHook faultHook;

    bool fault(PrivacyJournalFaultPoint point, PrivacyJournalError* error,
               QString* detail) const
    {
        if (faultHook && faultHook(point))
        {
            setError(error, PrivacyJournalError::FaultInjected, detail,
                     QStringLiteral("simulated crash at durable journal stage %1")
                         .arg(static_cast<int>(point)));
            return true;
        }

        return false;
    }

#ifdef Q_OS_UNIX
    bool verifyManagedMarker(QString* const detail) const
    {
        if (expectation.markerUuid.isEmpty())
        {
            return true;
        }

        bool unused = false;
        const int metadataFd = openOwnedDirectoryAt(
            rootFd, MetadataDirectory.toUtf8(), static_cast<dev_t>(device),
            false, &unused, detail);

        if (metadataFd < 0)
        {
            if (detail)
            {
                *detail = QStringLiteral("managed root marker directory is unavailable or unsafe");
            }

            return false;
        }

        QByteArray markerBytes;
        const EntryReadStatus markerStatus = readOwnedRegularAt(
            metadataFd, QByteArrayLiteral("root-marker-v1.json"),
            static_cast<dev_t>(device), 4096, &markerBytes, detail, false);
        ::close(metadataFd);

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(markerBytes,
                                                                &parseError);
        QJsonObject expectedMarker;
        expectedMarker.insert(QStringLiteral("kind"),
                              QStringLiteral("digikam-private-root-marker-v1"));
        expectedMarker.insert(QStringLiteral("markerUuid"), expectation.markerUuid);
        expectedMarker.insert(QStringLiteral("rootUuid"), expectation.rootUuid);

        if ((markerStatus != EntryReadStatus::Ok) ||
            (parseError.error != QJsonParseError::NoError) ||
            !document.isObject() ||
            !exactKeys(document.object(), { "kind", "markerUuid", "rootUuid" }) ||
            (document.object() != expectedMarker) ||
            (QJsonDocument(document.object()).toJson(QJsonDocument::Compact) !=
             markerBytes))
        {
            if (detail)
            {
                *detail = QStringLiteral("managed root marker is missing, unsafe, malformed, or mismatched");
            }

            return false;
        }

        return true;
    }

    bool revalidateRoot(QString* const detail) const
    {
        struct stat status = {};

        if ((rootFd < 0) || (::fstat(rootFd, &status) != 0) ||
            !S_ISDIR(status.st_mode) ||
            (static_cast<quint64>(status.st_dev) != device) ||
            (static_cast<quint64>(status.st_ino) != inode))
        {
            if (detail)
            {
                *detail = QStringLiteral("opened root identity changed");
            }

            return false;
        }

        return verifyManagedMarker(detail);
    }

    int openTransactionDirectory(const QString& transactionUuid, bool create,
                                 PrivacyJournalError* const error,
                                 QString* const detail) const
    {
        if (!revalidateRoot(detail))
        {
            setError(error, PrivacyJournalError::RootIdentityMismatch, detail,
                     detail ? *detail : QString());
            return -1;
        }

        bool created = false;
        const int metadataFd = openOwnedDirectoryAt(rootFd,
                                                     MetadataDirectory.toUtf8(),
                                                     static_cast<dev_t>(device),
                                                     create, &created, detail);

        if (metadataFd < 0)
        {
            if (!create && (errno == ENOENT))
            {
                setError(error, PrivacyJournalError::None, detail, {});
            }
            else
            {
                setError(error, PrivacyJournalError::UnsafeStorage, detail,
                         detail ? *detail : QString());
            }

            return -1;
        }

        const int transactionsFd = openOwnedDirectoryAt(
            metadataFd, TransactionsDirectory.toUtf8(),
            static_cast<dev_t>(device), create, &created, detail);
        ::close(metadataFd);

        if (transactionsFd < 0)
        {
            const bool missing = (!create && (errno == ENOENT));
            setError(error, missing ? PrivacyJournalError::None
                                    : PrivacyJournalError::UnsafeStorage,
                     detail, missing ? QString()
                                     : (detail ? *detail : QString()));
            return -1;
        }

        const int transactionFd = openOwnedDirectoryAt(
            transactionsFd, transactionUuid.toUtf8(),
            static_cast<dev_t>(device), create, &created, detail);
        ::close(transactionsFd);

        if (transactionFd < 0)
        {
            const bool missing = (!create && (errno == ENOENT));
            setError(error, missing ? PrivacyJournalError::None
                                    : PrivacyJournalError::UnsafeStorage,
                     detail, missing ? QString()
                                     : (detail ? *detail : QString()));
            return -1;
        }

        setError(error, PrivacyJournalError::None, detail, {});
        return transactionFd;
    }
#endif
};

PrivacyTransactionJournalStore::PrivacyTransactionJournalStore()
    : d(new Private)
{
}

PrivacyTransactionJournalStore::~PrivacyTransactionJournalStore()
{
}

PrivacyTransactionJournalStore::PrivacyTransactionJournalStore(
    PrivacyTransactionJournalStore&& other) noexcept = default;

PrivacyTransactionJournalStore& PrivacyTransactionJournalStore::operator=(
    PrivacyTransactionJournalStore&& other) noexcept = default;

bool PrivacyTransactionJournalStore::inspectRootExpectation(
    const PrivacyStorageRoot& root,
    PrivacyJournalRootExpectation* const expectation,
    PrivacyJournalError* const error, QString* const detail)
{
    if (!expectation)
    {
        setError(error, PrivacyJournalError::InvalidRoot, detail,
                 QStringLiteral("root expectation output is missing"));
        return false;
    }

    *expectation = PrivacyJournalRootExpectation();

    if (!root.isValid())
    {
        setError(error, PrivacyJournalError::InvalidRoot, detail,
                 QStringLiteral("persistent root record is invalid"));
        return false;
    }

    PrivacyJournalRootExpectation candidate;
    candidate.rootUuid = root.uuid;
    candidate.markerUuid = root.markerUuid;
    candidate.identitySha256 = QCryptographicHash::hash(
        root.identityData, QCryptographicHash::Sha256);

    std::unique_ptr<PrivacyTransactionJournalStore> store = open(
        root.configuredPath, candidate, error, detail);

    if (!store)
    {
        return false;
    }

    candidate.device = store->rootDevice();
    candidate.inode = store->rootInode();

    if ((candidate.device == 0) || (candidate.inode == 0))
    {
        setError(error, PrivacyJournalError::RootIdentityMismatch, detail,
                 QStringLiteral("persistent root identity is incomplete"));
        return false;
    }

    *expectation = candidate;
    setError(error, PrivacyJournalError::None, detail, {});
    return true;
}

std::unique_ptr<PrivacyTransactionJournalStore>
PrivacyTransactionJournalStore::open(
    const QString& absoluteRootPath,
    const PrivacyJournalRootExpectation& expectation,
    PrivacyJournalError* const error, QString* const detail)
{
#ifndef Q_OS_UNIX
    Q_UNUSED(absoluteRootPath);
    Q_UNUSED(expectation);
    setError(error, PrivacyJournalError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-relative journal storage requires Unix"));
    return {};
#else
    if (!canonicalUuid(expectation.rootUuid) ||
        (!expectation.markerUuid.isEmpty() && !canonicalUuid(expectation.markerUuid)) ||
        !validSha256(expectation.identitySha256) ||
        absoluteRootPath.isEmpty() || !QDir::isAbsolutePath(absoluteRootPath) ||
        (QDir::cleanPath(absoluteRootPath) != absoluteRootPath))
    {
        setError(error, PrivacyJournalError::InvalidRoot, detail,
                 QStringLiteral("root expectation or path is invalid"));
        return {};
    }

    const QFileInfo rootInfo(absoluteRootPath);

    if (!rootInfo.isDir() || rootInfo.isSymLink() ||
        (rootInfo.canonicalFilePath() != rootInfo.absoluteFilePath()))
    {
        setError(error, PrivacyJournalError::InvalidRoot, detail,
                 QStringLiteral("root path is missing, non-canonical, or symlinked"));
        return {};
    }

    const QByteArray encodedPath = QFile::encodeName(absoluteRootPath);
    const int rootFd = ::open(encodedPath.constData(),
                              O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);

    if (rootFd < 0)
    {
        setError(error, PrivacyJournalError::InvalidRoot, detail,
                 QStringLiteral("cannot open root: %1")
                     .arg(QString::fromLocal8Bit(std::strerror(errno))));
        return {};
    }

    struct stat status = {};

    if ((::fstat(rootFd, &status) != 0) || !S_ISDIR(status.st_mode) ||
        ((expectation.device != 0) &&
         (expectation.device != static_cast<quint64>(status.st_dev))) ||
        ((expectation.inode != 0) &&
         (expectation.inode != static_cast<quint64>(status.st_ino))))
    {
        ::close(rootFd);
        setError(error, PrivacyJournalError::RootIdentityMismatch, detail,
                 QStringLiteral("opened root does not match expected device/inode"));
        return {};
    }

    std::unique_ptr<PrivacyTransactionJournalStore> store(
        new PrivacyTransactionJournalStore);
    store->d->rootFd      = rootFd;
    store->d->device      = static_cast<quint64>(status.st_dev);
    store->d->inode       = static_cast<quint64>(status.st_ino);
    store->d->expectation = expectation;

    QString revalidationDetail;

    if (!store->d->revalidateRoot(&revalidationDetail))
    {
        setError(error, PrivacyJournalError::RootIdentityMismatch, detail,
                 revalidationDetail);
        return {};
    }

    setError(error, PrivacyJournalError::None, detail, {});
    return store;
#endif
}

quint64 PrivacyTransactionJournalStore::rootDevice() const
{
    return d ? d->device : 0;
}

quint64 PrivacyTransactionJournalStore::rootInode() const
{
    return d ? d->inode : 0;
}

void PrivacyTransactionJournalStore::setFaultHook(const FaultHook& hook)
{
    if (d)
    {
        d->faultHook = hook;
    }
}

PrivacyJournalLoadResult PrivacyTransactionJournalStore::load(
    const QString& transactionUuid) const
{
    PrivacyJournalLoadResult result;

#ifndef Q_OS_UNIX
    Q_UNUSED(transactionUuid);
    result.disposition = PrivacyJournalLoadDisposition::UnsafeStorage;
    result.error       = PrivacyJournalError::UnsupportedPlatform;
    result.detail      = QStringLiteral("descriptor-relative journal storage requires Unix");
    return result;
#else
    if (!d || !canonicalUuid(transactionUuid))
    {
        result.disposition = PrivacyJournalLoadDisposition::IdentityMismatch;
        result.error       = PrivacyJournalError::InvalidRecord;
        result.detail      = QStringLiteral("transaction UUID is invalid");
        return result;
    }

    PrivacyJournalError openError = PrivacyJournalError::None;
    QString openDetail;
    const int transactionFd = d->openTransactionDirectory(
        transactionUuid, false, &openError, &openDetail);

    if (transactionFd < 0)
    {
        if (openError == PrivacyJournalError::None)
        {
            return result;
        }

        result.disposition = (openError == PrivacyJournalError::RootIdentityMismatch)
                           ? PrivacyJournalLoadDisposition::IdentityMismatch
                           : PrivacyJournalLoadDisposition::UnsafeStorage;
        result.error       = openError;
        result.detail      = openDetail;
        return result;
    }

    struct Candidate
    {
        EntryReadStatus status = EntryReadStatus::Missing;
        PrivacyJournalRecord record;
        QByteArray bytes;
        QByteArray hash;
        PrivacyJournalError decodeError = PrivacyJournalError::None;
        QString detail;
        bool valid = false;
        bool identityMismatch = false;
    };

    const auto readCandidate = [this, transactionFd, &transactionUuid](
        const QByteArray& name)
    {
        Candidate candidate;
        candidate.status = readOwnedRegularAt(
            transactionFd, name, static_cast<dev_t>(d->device),
            PrivacyTransactionJournalCodec::MaximumEncodedBytes,
            &candidate.bytes, &candidate.detail);

        if (candidate.status == EntryReadStatus::Ok)
        {
            candidate.valid = PrivacyTransactionJournalCodec::decode(
                candidate.bytes, &candidate.record, &candidate.decodeError,
                &candidate.detail);
            candidate.hash = PrivacyTransactionJournalCodec::sha256(candidate.bytes);

            if (candidate.valid &&
                ((candidate.record.transactionUuid != transactionUuid) ||
                 (candidate.record.rootUuid != d->expectation.rootUuid) ||
                 (candidate.record.rootDevice != d->device) ||
                 (candidate.record.rootInode != d->inode) ||
                 (candidate.record.rootIdentitySha256 !=
                  d->expectation.identitySha256)))
            {
                candidate.valid = false;
                candidate.identityMismatch = true;
                candidate.decodeError = PrivacyJournalError::IdentityMismatch;
                candidate.detail = QStringLiteral("journal identity does not match opened root or transaction");
            }
        }

        return candidate;
    };

    const Candidate current = readCandidate(JournalFile.toUtf8());
    const Candidate next    = readCandidate(NextFile.toUtf8());
    QByteArray intentBytes;
    QString intentDetail;
    const EntryReadStatus intentStatus = readOwnedRegularAt(
        transactionFd, IntentFile.toUtf8(), static_cast<dev_t>(d->device),
        MaximumIntentBytes, &intentBytes, &intentDetail);
    ::close(transactionFd);

    if ((current.status == EntryReadStatus::Unsafe) ||
        (next.status == EntryReadStatus::Unsafe) ||
        (intentStatus == EntryReadStatus::Unsafe))
    {
        result.disposition = PrivacyJournalLoadDisposition::UnsafeStorage;
        result.error       = PrivacyJournalError::UnsafeStorage;
        result.detail      = !current.detail.isEmpty() ? current.detail
                           : !next.detail.isEmpty()    ? next.detail
                                                      : intentDetail;
        return result;
    }

    const auto validIntent = [](const QByteArray& bytes)
    {
        if ((bytes.size() != MaximumIntentBytes) || !bytes.endsWith('\n'))
        {
            return false;
        }

        const QByteArray hash = bytes.first(64);

        for (const char character : hash)
        {
            if (!(((character >= '0') && (character <= '9')) ||
                  ((character >= 'a') && (character <= 'f'))))
            {
                return false;
            }
        }

        return true;
    };

    const bool intentExists = (intentStatus != EntryReadStatus::Missing);

    if ((intentStatus == EntryReadStatus::Ok) && !validIntent(intentBytes))
    {
        result.disposition = PrivacyJournalLoadDisposition::Corrupt;
        result.error       = PrivacyJournalError::CorruptJournal;
        result.detail      = QStringLiteral("journal commit intent is malformed");
        return result;
    }

    if ((intentStatus == EntryReadStatus::IoFailure) ||
        (intentStatus == EntryReadStatus::TooLarge))
    {
        result.disposition = PrivacyJournalLoadDisposition::Corrupt;
        result.error       = PrivacyJournalError::CorruptJournal;
        result.detail      = QStringLiteral("journal commit intent cannot be verified");
        return result;
    }

    if (current.identityMismatch || next.identityMismatch)
    {
        result.disposition = PrivacyJournalLoadDisposition::IdentityMismatch;
        result.error       = PrivacyJournalError::IdentityMismatch;
        result.detail      = current.identityMismatch ? current.detail : next.detail;
        return result;
    }

    const bool artifacts = (next.status != EntryReadStatus::Missing) || intentExists;

    if (!artifacts)
    {
        if (current.valid)
        {
            result.disposition    = PrivacyJournalLoadDisposition::Loaded;
            result.error          = PrivacyJournalError::None;
            result.record         = current.record;
            result.canonicalBytes = current.bytes;
            result.sha256         = current.hash;
            result.hasRecord      = true;
            result.authoritative  = true;
            return result;
        }

        if (current.status == EntryReadStatus::Missing)
        {
            return result;
        }

        result.disposition = PrivacyJournalLoadDisposition::Corrupt;
        result.error       = PrivacyJournalError::CorruptJournal;
        result.detail      = QStringLiteral("current journal cannot be verified");
        return result;
    }

    result.disposition   = PrivacyJournalLoadDisposition::DurabilityUncertain;
    result.error         = PrivacyJournalError::DurabilityUncertain;
    result.authoritative = false;

    const auto assignCandidate = [&result](const Candidate& candidate,
                                           bool matchesIntent)
    {
        result.record         = candidate.record;
        result.canonicalBytes = candidate.bytes;
        result.sha256         = candidate.hash;
        result.hasRecord      = true;
        result.matchesCommitIntent = matchesIntent;
    };

    if (intentExists)
    {
        const QByteArray intendedHash = QByteArray::fromHex(intentBytes.first(64));
        const bool currentMatches = current.valid && (current.hash == intendedHash);
        const bool nextMatches    = next.valid && (next.hash == intendedHash);

        if (currentMatches)
        {
            assignCandidate(current, true);
            result.detail = nextMatches
                          ? QStringLiteral("both verified candidates match the interrupted commit intent")
                          : QStringLiteral("verified current journal matches the interrupted commit intent");
        }
        else if (nextMatches)
        {
            assignCandidate(next, true);
            result.detail = QStringLiteral("verified sibling candidate matches the interrupted commit intent");
        }
        else
        {
            result.detail = QStringLiteral("no verified journal candidate matches the commit intent");
        }

        return result;
    }

    if (current.valid)
    {
        assignCandidate(current, false);
        result.detail = QStringLiteral("verified current journal has an uncommitted sibling artifact");
    }
    else if (next.valid)
    {
        assignCandidate(next, false);
        result.detail = QStringLiteral("only an uncommitted sibling candidate is verified");
    }
    else
    {
        result.detail = QStringLiteral("no verified journal survives the interrupted state");
    }

    return result;
#endif
}

bool PrivacyTransactionJournalStore::create(
    const PrivacyJournalRecord& record, QByteArray* const publishedSha256,
    PrivacyJournalError* const error, QString* const detail)
{
    return persist(record, {}, false, publishedSha256, error, detail);
}

bool PrivacyTransactionJournalStore::compareAndUpdate(
    const PrivacyJournalRecord& record,
    const QByteArray& expectedCurrentSha256,
    QByteArray* const publishedSha256,
    PrivacyJournalError* const error, QString* const detail)
{
    return persist(record, expectedCurrentSha256, true, publishedSha256,
                   error, detail);
}

bool PrivacyTransactionJournalStore::persist(
    const PrivacyJournalRecord& record,
    const QByteArray& expectedCurrentSha256, bool update,
    QByteArray* const publishedSha256,
    PrivacyJournalError* const error, QString* const detail)
{
#ifndef Q_OS_UNIX
    Q_UNUSED(record);
    Q_UNUSED(expectedCurrentSha256);
    Q_UNUSED(update);
    Q_UNUSED(publishedSha256);
    setError(error, PrivacyJournalError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-relative journal storage requires Unix"));
    return false;
#else
    if (!d || (record.rootUuid != d->expectation.rootUuid) ||
        (record.rootDevice != d->device) || (record.rootInode != d->inode) ||
        (record.rootIdentitySha256 != d->expectation.identitySha256) ||
        (update && !validSha256(expectedCurrentSha256)))
    {
        setError(error, PrivacyJournalError::IdentityMismatch, detail,
                 QStringLiteral("record does not match opened root identity"));
        return false;
    }

    PrivacyJournalError codecError = PrivacyJournalError::None;
    QString codecDetail;
    const QByteArray bytes = PrivacyTransactionJournalCodec::encode(
        record, &codecError, &codecDetail);

    if (bytes.isEmpty())
    {
        setError(error, codecError, detail, codecDetail);
        return false;
    }

    PrivacyJournalError directoryError = PrivacyJournalError::None;
    QString directoryDetail;
    const int transactionFd = d->openTransactionDirectory(
        record.transactionUuid, true, &directoryError, &directoryDetail);

    if (transactionFd < 0)
    {
        setError(error, directoryError, detail, directoryDetail);
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterDirectoriesFsynced,
                 error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    const PrivacyJournalLoadResult existing = load(record.transactionUuid);

    if (update)
    {
        if ((existing.disposition != PrivacyJournalLoadDisposition::Loaded) ||
            !existing.hasRecord || !existing.authoritative)
        {
            ::close(transactionFd);
            setError(error,
                     (existing.disposition == PrivacyJournalLoadDisposition::DurabilityUncertain)
                         ? PrivacyJournalError::DurabilityUncertain
                         : PrivacyJournalError::StaleComparison,
                     detail, QStringLiteral("current journal is not cleanly verified for update"));
            return false;
        }

        if (existing.sha256 != expectedCurrentSha256)
        {
            ::close(transactionFd);
            setError(error, PrivacyJournalError::StaleComparison, detail,
                     QStringLiteral("journal compare-and-update hash is stale"));
            return false;
        }

        if (!immutableIdentityMatches(existing.record, record))
        {
            ::close(transactionFd);
            setError(error, PrivacyJournalError::IdentityMismatch, detail,
                     QStringLiteral("immutable journal identity or paths changed"));
            return false;
        }

        if (static_cast<int>(record.stage) < static_cast<int>(existing.record.stage))
        {
            ::close(transactionFd);
            setError(error, PrivacyJournalError::StageRegression, detail,
                     QStringLiteral("journal stage cannot move backwards"));
            return false;
        }
    }
    else if (existing.disposition != PrivacyJournalLoadDisposition::Missing)
    {
        ::close(transactionFd);
        setError(error,
                 (existing.disposition == PrivacyJournalLoadDisposition::DurabilityUncertain)
                     ? PrivacyJournalError::DurabilityUncertain
                     : PrivacyJournalError::PublicationConflict,
                 detail, QStringLiteral("journal transaction path already contains state"));
        return false;
    }

    const QByteArray nextName = NextFile.toUtf8();
    const int nextFd = PrivacyPosixStorage::confinedOpenAt(
        transactionFd, nextName,
        O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);

    if (nextFd < 0)
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::PublicationConflict, detail,
                 QStringLiteral("cannot exclusively create journal candidate"));
        return false;
    }

    if (!verifyOpenedOwnedRegular(nextFd, static_cast<dev_t>(d->device), detail))
    {
        ::close(nextFd);
        ::close(transactionFd);
        setError(error, PrivacyJournalError::UnsafeStorage, detail,
                 QStringLiteral("new journal candidate is not a safe owned file"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterNextCreated, error, detail))
    {
        ::close(nextFd);
        ::close(transactionFd);
        return false;
    }

    if (!writeAll(nextFd, bytes))
    {
        ::close(nextFd);
        ::close(transactionFd);
        setError(error, PrivacyJournalError::IoFailure, detail,
                 QStringLiteral("cannot write journal candidate"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterNextWritten, error, detail))
    {
        ::close(nextFd);
        ::close(transactionFd);
        return false;
    }

    if (!syncFd(nextFd))
    {
        ::close(nextFd);
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityFailure, detail,
                 QStringLiteral("cannot fsync journal candidate"));
        return false;
    }

    ::close(nextFd);

    if (d->fault(PrivacyJournalFaultPoint::AfterNextFsynced, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    QByteArray verifiedNext;
    QString readDetail;
    PrivacyJournalRecord verifiedRecord;
    PrivacyJournalError verifiedError = PrivacyJournalError::None;

    if ((readOwnedRegularAt(transactionFd, nextName,
                            static_cast<dev_t>(d->device),
                            PrivacyTransactionJournalCodec::MaximumEncodedBytes,
                            &verifiedNext, &readDetail) != EntryReadStatus::Ok) ||
        (verifiedNext != bytes) ||
        !PrivacyTransactionJournalCodec::decode(
            verifiedNext, &verifiedRecord, &verifiedError, &readDetail))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::CorruptJournal, detail,
                 QStringLiteral("journal candidate readback verification failed"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterNextVerified, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    const QByteArray journalHash = PrivacyTransactionJournalCodec::sha256(bytes);
    const QByteArray intentBytes = journalHash.toHex() + '\n';
    const int intentFd = PrivacyPosixStorage::confinedOpenAt(
        transactionFd, IntentFile.toUtf8(),
        O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);

    if ((intentFd < 0) ||
        !verifyOpenedOwnedRegular(intentFd, static_cast<dev_t>(d->device), detail) ||
        !writeAll(intentFd, intentBytes) || !syncFd(intentFd))
    {
        if (intentFd >= 0)
        {
            ::close(intentFd);
        }

        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityFailure, detail,
                 QStringLiteral("cannot durably write journal commit intent"));
        return false;
    }

    ::close(intentFd);

    QByteArray verifiedIntent;

    if ((readOwnedRegularAt(transactionFd, IntentFile.toUtf8(),
                            static_cast<dev_t>(d->device), MaximumIntentBytes,
                            &verifiedIntent, &readDetail) != EntryReadStatus::Ok) ||
        (verifiedIntent != intentBytes) || !syncFd(transactionFd))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityFailure, detail,
                 QStringLiteral("journal commit intent verification or directory fsync failed"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterIntentFsynced, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    bool atomicUnavailable = false;

    if (!PrivacyPosixStorage::atomicRenameAt(
            transactionFd, nextName, JournalFile.toUtf8(),
            update ? PrivacyPosixStorage::AtomicRenameMode::Exchange
                   : PrivacyPosixStorage::AtomicRenameMode::NoReplace,
            &atomicUnavailable))
    {
        ::close(transactionFd);
        setError(error,
                 atomicUnavailable
                     ? PrivacyJournalError::AtomicPublicationUnavailable
                     : PrivacyJournalError::PublicationConflict,
                 detail, atomicUnavailable
                     ? QStringLiteral("renameat2 atomic publication is unavailable")
                     : QStringLiteral("atomic journal publication failed"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterPublishRename, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    if (!syncFd(transactionFd))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityUncertain, detail,
                 QStringLiteral("journal rename directory fsync failed"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterPublishDirectoryFsync,
                 error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    QByteArray publishedBytes;

    if ((readOwnedRegularAt(transactionFd, JournalFile.toUtf8(),
                            static_cast<dev_t>(d->device),
                            PrivacyTransactionJournalCodec::MaximumEncodedBytes,
                            &publishedBytes, &readDetail) != EntryReadStatus::Ok) ||
        (publishedBytes != bytes) ||
        !PrivacyTransactionJournalCodec::decode(
            publishedBytes, &verifiedRecord, &verifiedError, &readDetail))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityUncertain, detail,
                 QStringLiteral("published journal readback verification failed"));
        return false;
    }

    if (update)
    {
        QByteArray displacedBytes;
        const EntryReadStatus displacedStatus = readOwnedRegularAt(
            transactionFd, nextName, static_cast<dev_t>(d->device),
            PrivacyTransactionJournalCodec::MaximumEncodedBytes,
            &displacedBytes, &readDetail);

        if ((displacedStatus != EntryReadStatus::Ok) ||
            (displacedBytes != existing.canonicalBytes) ||
            (PrivacyTransactionJournalCodec::sha256(displacedBytes) !=
             expectedCurrentSha256))
        {
            const bool rolledBack = PrivacyPosixStorage::atomicRenameAt(
                transactionFd, nextName, JournalFile.toUtf8(),
                PrivacyPosixStorage::AtomicRenameMode::Exchange, nullptr);
            const bool rollbackDurable = rolledBack && syncFd(transactionFd);
            ::close(transactionFd);
            setError(error,
                     rollbackDurable ? PrivacyJournalError::StaleComparison
                                     : PrivacyJournalError::DurabilityUncertain,
                     detail,
                     rollbackDurable
                         ? QStringLiteral("journal changed during atomic compare-and-update; exchange rolled back")
                         : QStringLiteral("journal changed during atomic compare-and-update and rollback is uncertain"));
            return false;
        }
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterPublishedReadback, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    if (update && (::unlinkat(transactionFd, nextName.constData(), 0) != 0))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityUncertain, detail,
                 QStringLiteral("cannot remove displaced previous journal"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterPreviousRemoved, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    if (!syncFd(transactionFd))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityUncertain, detail,
                 QStringLiteral("cannot fsync previous-journal cleanup"));
        return false;
    }

    if (::unlinkat(transactionFd, IntentFile.toUtf8().constData(), 0) != 0)
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityUncertain, detail,
                 QStringLiteral("cannot remove journal commit intent"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterIntentRemoved, error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    if (!syncFd(transactionFd))
    {
        ::close(transactionFd);
        setError(error, PrivacyJournalError::DurabilityUncertain, detail,
                 QStringLiteral("cannot fsync journal cleanup"));
        return false;
    }

    if (d->fault(PrivacyJournalFaultPoint::AfterCleanupDirectoryFsync,
                 error, detail))
    {
        ::close(transactionFd);
        return false;
    }

    ::close(transactionFd);

    if (publishedSha256)
    {
        *publishedSha256 = journalHash;
    }

    setError(error, PrivacyJournalError::None, detail, {});
    return true;
#endif
}

} // namespace Digikam

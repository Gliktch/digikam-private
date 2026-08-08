/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyassetinventory.h"

// C++ includes

#include <algorithm>
#include <utility>

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QUuid>

namespace Digikam
{

namespace
{

bool isSafeRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) || path.contains(QChar::Null) ||
        path.contains(QLatin1Char('\\')) || (QDir::cleanPath(path) != path))
    {
        return false;
    }

    const QStringList parts = path.split(QLatin1Char('/'));

    for (const QString& part : parts)
    {
        if (part.isEmpty() || (part == QLatin1String(".")) || (part == QLatin1String("..")))
        {
            return false;
        }

        for (const QChar character : part)
        {
            if (character.unicode() < 0x20U)
            {
                return false;
            }
        }
    }

    return true;
}

bool isSafeRelativeDirectory(const QString& path)
{
    return path.isEmpty() || isSafeRelativePath(path);
}

bool isSafeEntryName(const QString& name)
{
    return isSafeRelativePath(name) && !name.contains(QLatin1Char('/'));
}

QString locationKey(const PrivacyInventoryLocation& location)
{
    return location.root.uuid + QLatin1Char('\n') + location.root.absolutePath +
           QLatin1Char('\n') + location.relativePath;
}

QString fileIdentityKey(quint64 deviceId, quint64 inode)
{
    return QString::number(deviceId) + QLatin1Char(':') + QString::number(inode);
}

bool sameLocation(const PrivacyInventoryLocation& left,
                  const PrivacyInventoryLocation& right)
{
    return locationKey(left) == locationKey(right);
}

QString joinedRelativePath(const QString& directory, const QString& name)
{
    return directory.isEmpty() ? name : (directory + QLatin1Char('/') + name);
}

PrivacyInventoryLocation childLocation(const PrivacyInventoryRoot& root,
                                       const QString& directory,
                                       const QString& name)
{
    PrivacyInventoryLocation location;
    location.root         = root;
    location.relativePath = joinedRelativePath(directory, name);

    return location;
}

void promoteStatus(PrivacyAssetInventoryResult* const result,
                   PrivacyInventoryStatus status)
{
    if (!result)
    {
        return;
    }

    if ((status == PrivacyInventoryStatus::Rejected) ||
        ((status == PrivacyInventoryStatus::Incomplete) &&
         (result->status == PrivacyInventoryStatus::Ready)))
    {
        result->status = status;
    }
}

void addIssue(PrivacyAssetInventoryResult* const result,
              PrivacyInventoryStatus status,
              PrivacyInventoryIssueCode code,
              const PrivacyInventoryLocation& location,
              const QString& detail = QString())
{
    PrivacyInventoryIssue issue;
    issue.code     = code;
    issue.location = location;
    issue.detail   = detail;
    result->issues << issue;
    promoteStatus(result, status);
}

bool validRole(PrivacyInventoryAssetRole role)
{
    return (role == PrivacyInventoryAssetRole::PrimaryMedia)      ||
           (role == PrivacyInventoryAssetRole::PairedMedia)       ||
           (role == PrivacyInventoryAssetRole::XmpSidecar)        ||
           (role == PrivacyInventoryAssetRole::ConfiguredSidecar);
}

bool validAliasKind(PrivacyInventoryAliasKind kind)
{
    return (kind == PrivacyInventoryAliasKind::HardlinkAlias)        ||
           (kind == PrivacyInventoryAliasKind::DatabaseItemAlias)    ||
           (kind == PrivacyInventoryAliasKind::ContentIdentityAlias) ||
           (kind == PrivacyInventoryAliasKind::DigikamGroupMember);
}

bool validStatus(PrivacyInventoryStatus status)
{
    return (status == PrivacyInventoryStatus::Ready)      ||
           (status == PrivacyInventoryStatus::Incomplete) ||
           (status == PrivacyInventoryStatus::Rejected);
}

bool validIssueCode(PrivacyInventoryIssueCode code)
{
    return (code == PrivacyInventoryIssueCode::InvalidRequest)                     ||
           (code == PrivacyInventoryIssueCode::InvalidConfiguredSidecarExtension)  ||
           (code == PrivacyInventoryIssueCode::PrimaryMissing)                     ||
           (code == PrivacyInventoryIssueCode::UnsafeFileType)                     ||
           (code == PrivacyInventoryIssueCode::UnsafePath)                         ||
           (code == PrivacyInventoryIssueCode::DirectoryEnumerationIncomplete)     ||
           (code == PrivacyInventoryIssueCode::HardlinkEnumerationIncomplete)      ||
           (code == PrivacyInventoryIssueCode::IdentityAliasEnumerationIncomplete) ||
           (code == PrivacyInventoryIssueCode::AmbiguousPairedMedia)                ||
           (code == PrivacyInventoryIssueCode::PathCollision)                      ||
           (code == PrivacyInventoryIssueCode::AliasEvidenceMismatch)               ||
           (code == PrivacyInventoryIssueCode::RequiredAssetIdentityCollision)      ||
           (code == PrivacyInventoryIssueCode::ReservedPrivateArchivePath);
}

bool lessLocation(const PrivacyInventoryLocation& left,
                  const PrivacyInventoryLocation& right)
{
    const int rootUuid = QString::compare(left.root.uuid, right.root.uuid, Qt::CaseSensitive);

    if (rootUuid != 0)
    {
        return (rootUuid < 0);
    }

    const int rootPath = QString::compare(left.root.absolutePath,
                                          right.root.absolutePath,
                                          Qt::CaseSensitive);

    if (rootPath != 0)
    {
        return (rootPath < 0);
    }

    return (QString::compare(left.relativePath, right.relativePath, Qt::CaseSensitive) < 0);
}

bool isAppleImageExtension(const QString& extension)
{
    static const QSet<QString> imageExtensions = {
        QLatin1String("heic"),
        QLatin1String("heif"),
        QLatin1String("jpeg"),
        QLatin1String("jpg")
    };

    return imageExtensions.contains(extension.toLower());
}

bool isAppleVideoExtension(const QString& extension)
{
    return (extension.compare(QLatin1String("mov"), Qt::CaseInsensitive) == 0);
}

bool addRequiredAsset(PrivacyAssetInventoryResult* const result,
                      QHash<QString, int>* const requiredByPath,
                      QHash<QString, QString>* const collisionPaths,
                      const PrivacyInventoryLocation& location,
                      PrivacyInventoryAssetRole role,
                      const PrivacyAssetFilesystemProvider& filesystem)
{
    const QString key = locationKey(location);
    const QString collisionKey = location.root.uuid + QLatin1Char('\n') +
                                 location.root.absolutePath + QLatin1Char('\n') +
                                 location.relativePath.normalized(QString::NormalizationForm_C)
                                                      .toCaseFolded();
    const auto collision = collisionPaths->constFind(collisionKey);

    if ((collision != collisionPaths->constEnd()) && (collision.value() != key))
    {
        addIssue(result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::PathCollision, location);
        return false;
    }

    const auto existing = requiredByPath->constFind(key);

    if (existing != requiredByPath->constEnd())
    {
        if (result->requiredAssets.at(existing.value()).role != role)
        {
            addIssue(result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::PathCollision, location);
            return false;
        }

        return true;
    }

    const PrivacyInventoryFileEvidence evidence = filesystem.inspect(location);

    if (!evidence.isRegular() || !evidence.identityComplete ||
        (evidence.linkCount == 0) || (evidence.byteSize < 0))
    {
        addIssue(result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::UnsafeFileType, location);
        return false;
    }

    PrivacyInventoryAsset asset;
    asset.role     = role;
    asset.location = location;
    asset.evidence = evidence;
    requiredByPath->insert(key, result->requiredAssets.size());
    collisionPaths->insert(collisionKey, key);
    result->requiredAssets << asset;

    return true;
}

void sortAndNumberRequiredAssets(PrivacyAssetInventoryResult* const result)
{
    std::sort(result->requiredAssets.begin(), result->requiredAssets.end(),
              [](const PrivacyInventoryAsset& left, const PrivacyInventoryAsset& right)
              {
                  if (left.role != right.role)
                  {
                      return (static_cast<int>(left.role) < static_cast<int>(right.role));
                  }

                  return lessLocation(left.location, right.location);
              });

    QHash<int, int> nextOrdinal;

    for (PrivacyInventoryAsset& asset : result->requiredAssets)
    {
        const int role = static_cast<int>(asset.role);
        asset.ordinal  = nextOrdinal.value(role, 0);
        nextOrdinal.insert(role, asset.ordinal + 1);
    }
}

void sortAndDeduplicateWarnings(PrivacyAssetInventoryResult* const result)
{
    std::sort(result->exposureWarnings.begin(), result->exposureWarnings.end(),
              [](const PrivacyInventoryExposureWarning& left,
                 const PrivacyInventoryExposureWarning& right)
              {
                  if (left.kind != right.kind)
                  {
                      return (static_cast<int>(left.kind) < static_cast<int>(right.kind));
                  }

                  if (!sameLocation(left.alias, right.alias))
                  {
                      return lessLocation(left.alias, right.alias);
                  }

                  if (!sameLocation(left.source, right.source))
                  {
                      return lessLocation(left.source, right.source);
                  }

                  if (left.imageId != right.imageId)
                  {
                      return (left.imageId < right.imageId);
                  }

                  return (QString::compare(left.contentIdentity,
                                           right.contentIdentity,
                                           Qt::CaseSensitive) < 0);
              });

    const auto duplicate = [](const PrivacyInventoryExposureWarning& left,
                              const PrivacyInventoryExposureWarning& right)
    {
        return (left.kind == right.kind)                 &&
               sameLocation(left.source, right.source)  &&
               sameLocation(left.alias, right.alias)    &&
               (left.imageId == right.imageId)          &&
               (left.contentIdentity == right.contentIdentity);
    };

    result->exposureWarnings.erase(std::unique(result->exposureWarnings.begin(),
                                               result->exposureWarnings.end(),
                                               duplicate),
                                   result->exposureWarnings.end());
}

void sortIssues(PrivacyAssetInventoryResult* const result)
{
    std::sort(result->issues.begin(), result->issues.end(),
              [](const PrivacyInventoryIssue& left, const PrivacyInventoryIssue& right)
              {
                  if (left.code != right.code)
                  {
                      return (static_cast<int>(left.code) < static_cast<int>(right.code));
                  }

                  if (!sameLocation(left.location, right.location))
                  {
                      return lessLocation(left.location, right.location);
                  }

                  return (QString::compare(left.detail, right.detail, Qt::CaseSensitive) < 0);
              });
}

void addWarning(PrivacyAssetInventoryResult* const result,
                PrivacyInventoryAliasKind kind,
                const PrivacyInventoryAsset& source,
                const PrivacyInventoryAliasCandidate& candidate)
{
    PrivacyInventoryExposureWarning warning;
    warning.kind            = kind;
    warning.source          = source.location;
    warning.alias           = candidate.location;
    warning.imageId         = candidate.imageId;
    warning.contentIdentity = candidate.contentIdentity;
    result->exposureWarnings << warning;
}

} // namespace

bool PrivacyInventoryRoot::isValid() const
{
    const QUuid parsedUuid(uuid);

    return !parsedUuid.isNull() &&
           (uuid == parsedUuid.toString(QUuid::WithoutBraces)) &&
           !absolutePath.isEmpty() && (absolutePath != QLatin1String("/")) &&
           QDir::isAbsolutePath(absolutePath) &&
           (QDir::cleanPath(absolutePath) == absolutePath) &&
           !absolutePath.contains(QChar::Null);
}

bool PrivacyInventoryLocation::isValid() const
{
    return root.isValid() && isSafeRelativePath(relativePath);
}

bool PrivacyInventoryFileEvidence::isRegular() const
{
    return (type == PrivacyInventoryFileType::Regular);
}

bool PrivacyInventoryAsset::isValid() const
{
    return validRole(role) && (ordinal >= 0) && location.isValid() &&
           evidence.isRegular() && evidence.identityComplete &&
           (evidence.linkCount > 0) && (evidence.byteSize >= 0);
}

bool PrivacyInventoryAliasCandidate::isValid() const
{
    return validAliasKind(kind) && location.isValid();
}

bool PrivacyInventoryFileIdentity::isValid() const
{
    return (inode > 0);
}

bool PrivacyInventoryHardlinkEvidence::isValid() const
{
    return identity.isValid();
}

bool PrivacyInventoryExposureWarning::isValid() const
{
    return validAliasKind(kind) && source.isValid() && alias.isValid() &&
           !sameLocation(source, alias);
}

bool PrivacyInventoryIssue::isValid() const
{
    const bool locationOptional = (code == PrivacyInventoryIssueCode::InvalidRequest) ||
                                  (code == PrivacyInventoryIssueCode::UnsafePath);

    return validIssueCode(code) && (location.isValid() || locationOptional);
}

bool PrivacyAssetInventoryRequest::isValid() const
{
    return (contractVersion == 1) && primary.isValid();
}

bool PrivacyAssetInventoryResult::isValid() const
{
    if ((contractVersion != 1) || !validStatus(status))
    {
        return false;
    }

    for (const PrivacyInventoryAsset& asset : requiredAssets)
    {
        if (!asset.isValid())
        {
            return false;
        }
    }

    for (const PrivacyInventoryExposureWarning& warning : exposureWarnings)
    {
        if (!warning.isValid())
        {
            return false;
        }
    }

    for (const PrivacyInventoryIssue& issue : issues)
    {
        if (!issue.isValid())
        {
            return false;
        }
    }

    if (status == PrivacyInventoryStatus::Ready)
    {
        return !requiredAssets.isEmpty() && issues.isEmpty();
    }

    return !issues.isEmpty();
}

bool PrivacyAssetInventoryResult::isReady() const
{
    if (!isValid() || (status != PrivacyInventoryStatus::Ready))
    {
        return false;
    }

    return true;
}

QList<PrivacyInventoryHardlinkEvidence>
PrivacyAssetFilesystemProvider::hardlinkAliasesFor(
    const QList<PrivacyInventoryFileIdentity>& identities) const
{
    QList<PrivacyInventoryHardlinkEvidence> results;

    for (const PrivacyInventoryFileIdentity& identity : identities)
    {
        PrivacyInventoryHardlinkEvidence result;
        result.identity = identity;
        result.aliases  = hardlinkAliases(identity.deviceId, identity.inode);
        results << result;
    }

    return results;
}

PrivacyAssetInventoryResult PrivacyAssetInventory::build(
    const PrivacyAssetInventoryRequest& request,
    const PrivacyAssetFilesystemProvider& filesystem,
    const PrivacyAssetIdentityProvider& identities)
{
    PrivacyAssetInventoryResult result;
    result.status = PrivacyInventoryStatus::Ready;

    if (!request.isValid())
    {
        addIssue(&result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::InvalidRequest, request.primary);
        return result;
    }

    if (QFileInfo(request.primary.relativePath).fileName().endsWith(
            QLatin1String(".digikam-private.zip"), Qt::CaseInsensitive))
    {
        addIssue(&result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::ReservedPrivateArchivePath,
                 request.primary);
        return result;
    }

    QHash<QString, QString> rootPathByUuid {
        { request.primary.root.uuid, request.primary.root.absolutePath }
    };
    QHash<QString, QString> rootUuidByPath {
        { request.primary.root.absolutePath, request.primary.root.uuid }
    };
    const auto registerCandidateRoot = [&result, &rootPathByUuid, &rootUuidByPath]
                                       (const PrivacyInventoryLocation& location)
    {
        const auto knownPath = rootPathByUuid.constFind(location.root.uuid);
        const auto knownUuid = rootUuidByPath.constFind(location.root.absolutePath);

        if (((knownPath != rootPathByUuid.constEnd()) &&
             (knownPath.value() != location.root.absolutePath)) ||
            ((knownUuid != rootUuidByPath.constEnd()) &&
             (knownUuid.value() != location.root.uuid)))
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::PathCollision,
                     location,
                     QLatin1String("root UUID/path mapping collision"));
            return false;
        }

        rootPathByUuid.insert(location.root.uuid, location.root.absolutePath);
        rootUuidByPath.insert(location.root.absolutePath, location.root.uuid);

        return true;
    };

    static const QRegularExpression safeExtension(
        QLatin1String("^[A-Za-z0-9][A-Za-z0-9_-]{0,31}$"));

    QStringList sidecarExtensions { QLatin1String("xmp") };

    for (const QString& extension : request.configuredSidecarExtensions)
    {
        if (!safeExtension.match(extension).hasMatch())
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::InvalidConfiguredSidecarExtension,
                     request.primary, extension);
            continue;
        }

        if (!sidecarExtensions.contains(extension))
        {
            sidecarExtensions << extension;
        }
    }

    const QFileInfo primaryRelativeInfo(request.primary.relativePath);
    const QString directory = (primaryRelativeInfo.path() == QLatin1String("."))
                            ? QString()
                            : primaryRelativeInfo.path();

    if (!isSafeRelativeDirectory(directory))
    {
        addIssue(&result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::UnsafePath, request.primary);
        return result;
    }

    const PrivacyInventoryDirectoryEvidence directoryEvidence =
        filesystem.listDirectory(request.primary.root, directory);

    if (!directoryEvidence.complete)
    {
        addIssue(&result, PrivacyInventoryStatus::Incomplete,
                 PrivacyInventoryIssueCode::DirectoryEnumerationIncomplete,
                 request.primary);
    }

    QSet<QString> directoryEntries;
    QStringList sortedEntryNames = directoryEvidence.entryNames;
    sortedEntryNames.sort(Qt::CaseSensitive);

    for (const QString& entry : std::as_const(sortedEntryNames))
    {
        if (!isSafeEntryName(entry))
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::UnsafePath,
                     childLocation(request.primary.root, directory, entry));
            continue;
        }

        if (directoryEntries.contains(entry))
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::PathCollision,
                     childLocation(request.primary.root, directory, entry));
            continue;
        }

        directoryEntries.insert(entry);
    }

    const PrivacyInventoryFileEvidence primaryEvidence = filesystem.inspect(request.primary);

    if (primaryEvidence.type == PrivacyInventoryFileType::Missing)
    {
        addIssue(&result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::PrimaryMissing, request.primary);
        sortIssues(&result);
        return result;
    }

    if (directoryEvidence.complete && !directoryEntries.contains(primaryRelativeInfo.fileName()))
    {
        addIssue(&result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::AliasEvidenceMismatch,
                 request.primary,
                 QLatin1String("primary absent from authoritative directory listing"));
    }

    QHash<QString, int> requiredByPath;
    QHash<QString, QString> collisionPaths;
    addRequiredAsset(&result, &requiredByPath, &collisionPaths, request.primary,
                     PrivacyInventoryAssetRole::PrimaryMedia, filesystem);

    const QString primaryName = primaryRelativeInfo.fileName();
    const QString primaryStem = primaryRelativeInfo.completeBaseName();
    const QString primaryExtension = primaryRelativeInfo.suffix();
    QStringList pairNames;

    if (isAppleImageExtension(primaryExtension) || isAppleVideoExtension(primaryExtension))
    {
        for (const QString& entry : std::as_const(sortedEntryNames))
        {
            if (!isSafeEntryName(entry) || (entry == primaryName))
            {
                continue;
            }

            const QFileInfo candidateInfo(entry);

            if (candidateInfo.completeBaseName() != primaryStem)
            {
                continue;
            }

            const bool compatible = isAppleImageExtension(primaryExtension)
                                  ? isAppleVideoExtension(candidateInfo.suffix())
                                  : isAppleImageExtension(candidateInfo.suffix());

            if (compatible)
            {
                pairNames << entry;
            }
        }
    }

    pairNames.removeDuplicates();
    pairNames.sort(Qt::CaseSensitive);

    if (pairNames.size() > 1)
    {
        addIssue(&result, PrivacyInventoryStatus::Rejected,
                 PrivacyInventoryIssueCode::AmbiguousPairedMedia,
                 request.primary, pairNames.join(QLatin1Char('\n')));
    }
    else if (pairNames.size() == 1)
    {
        addRequiredAsset(&result, &requiredByPath, &collisionPaths,
                         childLocation(request.primary.root, directory, pairNames.constFirst()),
                         PrivacyInventoryAssetRole::PairedMedia, filesystem);
    }

    QList<PrivacyInventoryAsset> sidecarSources = result.requiredAssets;

    for (const PrivacyInventoryAsset& source : std::as_const(sidecarSources))
    {
        if ((source.role != PrivacyInventoryAssetRole::PrimaryMedia) &&
            (source.role != PrivacyInventoryAssetRole::PairedMedia))
        {
            continue;
        }

        const QFileInfo sourceInfo(source.location.relativePath);
        const QString sourceDirectory = (sourceInfo.path() == QLatin1String("."))
                                      ? QString()
                                      : sourceInfo.path();

        for (const QString& extension : std::as_const(sidecarExtensions))
        {
            const QString suffix = QLatin1Char('.') + extension;

            // Match SidecarFinder/DFileOperations: a selected file that is
            // already one of the configured sidecar suffixes is not expanded
            // into a sidecar of its own.

            if (source.location.relativePath.endsWith(suffix))
            {
                continue;
            }

            QStringList candidateNames {
                sourceInfo.fileName() + suffix,
                sourceInfo.completeBaseName() + suffix
            };
            candidateNames.removeDuplicates();

            for (const QString& candidateName : std::as_const(candidateNames))
            {
                if (!directoryEntries.contains(candidateName))
                {
                    continue;
                }

                addRequiredAsset(&result, &requiredByPath, &collisionPaths,
                                 childLocation(request.primary.root,
                                               sourceDirectory,
                                               candidateName),
                                 (extension == QLatin1String("xmp"))
                                     ? PrivacyInventoryAssetRole::XmpSidecar
                                     : PrivacyInventoryAssetRole::ConfiguredSidecar,
                                 filesystem);
            }
        }
    }

    sortAndNumberRequiredAssets(&result);

    QHash<QString, PrivacyInventoryLocation> requiredIdentityLocations;
    QList<PrivacyInventoryFileIdentity> requiredIdentities;

    for (const PrivacyInventoryAsset& asset : std::as_const(result.requiredAssets))
    {
        const QString identity = fileIdentityKey(asset.evidence.deviceId,
                                                 asset.evidence.inode);
        const auto existing = requiredIdentityLocations.constFind(identity);

        if ((existing != requiredIdentityLocations.constEnd()) &&
            !sameLocation(existing.value(), asset.location))
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::RequiredAssetIdentityCollision,
                     asset.location,
                     existing.value().relativePath);
        }
        else
        {
            requiredIdentityLocations.insert(identity, asset.location);
            if (asset.evidence.linkCount > 1)
            {
                PrivacyInventoryFileIdentity fileIdentity;
                fileIdentity.deviceId = asset.evidence.deviceId;
                fileIdentity.inode    = asset.evidence.inode;
                requiredIdentities << fileIdentity;
            }
        }
    }

    std::sort(requiredIdentities.begin(), requiredIdentities.end(),
              [](const PrivacyInventoryFileIdentity& left,
                 const PrivacyInventoryFileIdentity& right)
              {
                  return (left.deviceId == right.deviceId)
                       ? (left.inode < right.inode)
                       : (left.deviceId < right.deviceId);
              });

    const QList<PrivacyInventoryHardlinkEvidence> hardlinkBatch = requiredIdentities.isEmpty()
        ? QList<PrivacyInventoryHardlinkEvidence>()
        : filesystem.hardlinkAliasesFor(requiredIdentities);
    QHash<QString, PrivacyInventoryAliasEvidence> hardlinksByIdentity;
    QSet<QString> requestedIdentityKeys;

    for (const PrivacyInventoryFileIdentity& identity : std::as_const(requiredIdentities))
    {
        requestedIdentityKeys.insert(fileIdentityKey(identity.deviceId, identity.inode));
    }

    for (const PrivacyInventoryHardlinkEvidence& hardlinks : hardlinkBatch)
    {
        const QString key = fileIdentityKey(hardlinks.identity.deviceId,
                                            hardlinks.identity.inode);

        if (!hardlinks.isValid() || !requestedIdentityKeys.contains(key) ||
            hardlinksByIdentity.contains(key))
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::AliasEvidenceMismatch,
                     request.primary,
                     QLatin1String("malformed hardlink batch evidence"));
            continue;
        }

        hardlinksByIdentity.insert(key, hardlinks.aliases);
    }

    QSet<QString> requiredLocations;

    for (const PrivacyInventoryAsset& asset : std::as_const(result.requiredAssets))
    {
        requiredLocations.insert(locationKey(asset.location));
    }

    for (const PrivacyInventoryAsset& asset : std::as_const(result.requiredAssets))
    {
        const QString assetIdentity = fileIdentityKey(asset.evidence.deviceId,
                                                      asset.evidence.inode);
        PrivacyInventoryAliasEvidence hardlinks;

        if (asset.evidence.linkCount == 1)
        {
            // st_nlink proves that the already-inspected path is the only
            // filesystem alias. Avoid an otherwise unnecessary whole-root
            // production scan for this overwhelmingly common case.

            hardlinks.complete = true;
        }
        else
        {
            hardlinks = hardlinksByIdentity.value(assetIdentity);
        }

        if ((asset.evidence.linkCount > 1) &&
            !hardlinksByIdentity.contains(assetIdentity))
        {
            hardlinks.complete = false;
        }

        if (!hardlinks.complete)
        {
            addIssue(&result, PrivacyInventoryStatus::Incomplete,
                     PrivacyInventoryIssueCode::HardlinkEnumerationIncomplete,
                     asset.location);
        }

        QSet<QString> provenHardlinkLocations { locationKey(asset.location) };

        for (const PrivacyInventoryAliasCandidate& candidate : hardlinks.candidates)
        {
            if (!candidate.location.isValid())
            {
                addIssue(&result, PrivacyInventoryStatus::Rejected,
                         PrivacyInventoryIssueCode::UnsafePath,
                         candidate.location);
                continue;
            }

            if (candidate.kind != PrivacyInventoryAliasKind::HardlinkAlias)
            {
                addIssue(&result, PrivacyInventoryStatus::Rejected,
                         PrivacyInventoryIssueCode::AliasEvidenceMismatch,
                         candidate.location,
                         QLatin1String("hardlink provider returned a non-hardlink alias kind"));
                continue;
            }

            if (!registerCandidateRoot(candidate.location))
            {
                continue;
            }

            const PrivacyInventoryFileEvidence evidence = filesystem.inspect(candidate.location);

            if (!evidence.isRegular() || !evidence.identityComplete ||
                (evidence.deviceId != asset.evidence.deviceId) ||
                (evidence.inode != asset.evidence.inode))
            {
                addIssue(&result, PrivacyInventoryStatus::Rejected,
                         PrivacyInventoryIssueCode::AliasEvidenceMismatch,
                         candidate.location);
                continue;
            }

            provenHardlinkLocations.insert(locationKey(candidate.location));

            if (!requiredLocations.contains(locationKey(candidate.location)))
            {
                addWarning(&result, PrivacyInventoryAliasKind::HardlinkAlias,
                           asset, candidate);
            }
        }

        if (provenHardlinkLocations.size() < static_cast<qsizetype>(asset.evidence.linkCount))
        {
            addIssue(&result, PrivacyInventoryStatus::Incomplete,
                     PrivacyInventoryIssueCode::HardlinkEnumerationIncomplete,
                     asset.location,
                     QLatin1String("enumerated path count is below st_nlink"));
        }
        else if (provenHardlinkLocations.size() > static_cast<qsizetype>(asset.evidence.linkCount))
        {
            addIssue(&result, PrivacyInventoryStatus::Rejected,
                     PrivacyInventoryIssueCode::AliasEvidenceMismatch,
                     asset.location,
                     QLatin1String("enumerated path count exceeds st_nlink"));
        }

        const PrivacyInventoryAliasEvidence aliasEvidence = identities.aliasesFor(asset);

        if (!aliasEvidence.complete)
        {
            addIssue(&result, PrivacyInventoryStatus::Incomplete,
                     PrivacyInventoryIssueCode::IdentityAliasEnumerationIncomplete,
                     asset.location);
        }

        for (const PrivacyInventoryAliasCandidate& candidate : aliasEvidence.candidates)
        {
            if (!candidate.location.isValid())
            {
                addIssue(&result, PrivacyInventoryStatus::Rejected,
                         PrivacyInventoryIssueCode::UnsafePath,
                         candidate.location);
                continue;
            }

            if (!validAliasKind(candidate.kind) ||
                (candidate.kind == PrivacyInventoryAliasKind::HardlinkAlias))
            {
                addIssue(&result, PrivacyInventoryStatus::Rejected,
                         PrivacyInventoryIssueCode::AliasEvidenceMismatch,
                         candidate.location,
                         QLatin1String("identity provider returned an invalid alias kind"));
                continue;
            }

            if (!registerCandidateRoot(candidate.location))
            {
                continue;
            }

            const PrivacyInventoryFileEvidence evidence = filesystem.inspect(candidate.location);

            if (!evidence.isRegular())
            {
                addIssue(&result, PrivacyInventoryStatus::Rejected,
                         PrivacyInventoryIssueCode::UnsafeFileType,
                         candidate.location);
                continue;
            }

            if (!requiredLocations.contains(locationKey(candidate.location)))
            {
                addWarning(&result, candidate.kind, asset, candidate);
            }
        }
    }

    sortAndDeduplicateWarnings(&result);
    sortIssues(&result);

    return result;
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyportableimport.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QStringList>
#include <QUuid>

// C++ includes

#include <algorithm>

// Local includes

#include "privacycasualarchive.h"

namespace Digikam
{

namespace
{

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

bool isSafeOriginalName(const QString& name)
{
    if (name.isEmpty() || (name == QLatin1String(".")) ||
        (name == QLatin1String("..")) ||
        name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')) ||
        name.contains(QChar::Null))
    {
        return false;
    }

    return true;
}

bool isSafePublicRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        path.contains(QLatin1Char('\0')) ||
        path.contains(QLatin1Char('\\')))
    {
        return false;
    }

    const QStringList parts = path.split(QLatin1Char('/'));

    for (const QString& part : parts)
    {
        if (part.isEmpty() || (part == QLatin1String(".")) ||
            (part == QLatin1String("..")))
        {
            return false;
        }
    }

    return true;
}

PrivacyPortableImportAuthenticationResult failure(
    PrivacyPortableImportAuthenticationStatus status,
    const QString& detail)
{
    PrivacyPortableImportAuthenticationResult result;
    result.status = status;
    result.detail = detail;
    return result;
}

} // namespace

bool PrivacyPortableImportAssetFact::isValid() const
{
    return ((role > 0) && (ordinal >= 0) &&
            isSafeOriginalName(originalName) &&
            isSafePublicRelativePath(publicRelativePath) &&
            (protectedRelativePath ==
             PrivacyCasualArchiveEngine::expectedMemberPath(
                 role, ordinal, originalName)) &&
            (hashAlgorithm == QLatin1String("sha256")) &&
            (originalSha256.size() == 32) && (originalSize >= 0) &&
            (portableAttributes.size() <= 64 * 1024) &&
            ((unixMode & 0170000) == 0100000));
}

bool PrivacyPortableImportItemFact::isValid() const
{
    return (isCanonicalUuid(itemUuid) &&
            isCanonicalUuid(containerUuid) &&
            QDir::isAbsolutePath(archiveAbsolutePath) &&
            !proxyRelativePath.isEmpty() &&
            (archiveSize >= 0) && (archiveSha256.size() == 32) &&
            !assets.isEmpty());
}

bool PrivacyPortableImportCandidate::isValid() const
{
    return (isCanonicalUuid(recoverySetUuid) &&
            (backend == PrivacyBackend::Casual) &&
            isCanonicalUuid(categoryUuid) && !items.isEmpty());
}

PrivacyPortableImportAuthenticationResult
PrivacyPortableImportAuthenticator::authenticateCasual(
    const PrivacyPortableDiscoveryGroup& group,
    const PrivacyPassword& password,
    const CancellationCheck& isCancelled)
{
    if (!password.isValid())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                       QStringLiteral("the category password is invalid"));
    }

    if (group.backend != PrivacyBackend::Casual)
    {
        return failure(PrivacyPortableImportAuthenticationStatus::UnsupportedBackend,
                       QStringLiteral("Strong portable import authentication is not available yet"));
    }

    if (group.casualArchives.isEmpty())
    {
        return failure(PrivacyPortableImportAuthenticationStatus::InvalidRequest,
                       QStringLiteral("the discovery group has no Casual archives"));
    }

    PrivacyPortableImportCandidate candidate;
    candidate.recoverySetUuid = group.recoverySetUuid;
    candidate.backend = PrivacyBackend::Casual;
    candidate.hasCredential = false;
    PrivacyCasualArchiveEngine engine;
    QSet<QString> itemUuids;

    for (const PrivacyPortableCasualArchiveCandidate& archive :
         group.casualArchives)
    {
        if (isCancelled && isCancelled())
        {
            return failure(PrivacyPortableImportAuthenticationStatus::Cancelled,
                           QStringLiteral("import authentication was cancelled"));
        }

        PrivacyCasualArchiveError archiveError =
            PrivacyCasualArchiveError::None;
        const PrivacyCasualArchiveIdentity identity =
            engine.inspectIdentity(archive.absolutePath, &archiveError);

        if (!identity.valid ||
            (identity.recoverySetUuid != group.recoverySetUuid))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive does not carry the group recovery identity"));
        }

        PrivacyCasualArchiveManifest manifest;

        if (!engine.verifyAndReadManifest(
                archive.absolutePath, password, identity.archiveSize,
                identity.sha256, &manifest, isCancelled, &archiveError))
        {
            if ((archiveError == PrivacyCasualArchiveError::InvalidPassword) ||
                (archiveError == PrivacyCasualArchiveError::DecryptionFailed))
            {
                return failure(
                    PrivacyPortableImportAuthenticationStatus::InvalidPassword,
                    QStringLiteral("the category password does not open the archives"));
            }

            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive could not be fully verified with the password"));
        }

        if (!manifest.isValid())
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive manifest is invalid"));
        }

        if (candidate.categoryUuid.isEmpty())
        {
            candidate.categoryUuid = manifest.categoryUuid;
        }
        else if (candidate.categoryUuid != manifest.categoryUuid)
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("archives in one recovery group belong to different categories"));
        }

        if (itemUuids.contains(manifest.itemUuid))
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("two archives publish the same protected item"));
        }

        itemUuids.insert(manifest.itemUuid);
        PrivacyPortableImportItemFact item;
        item.itemUuid = manifest.itemUuid;
        item.containerUuid = manifest.containerUuid;
        item.archiveAbsolutePath = archive.absolutePath;
        item.proxyRelativePath = archive.proxyRelativePath;
        item.archiveSize = identity.archiveSize;
        item.archiveSha256 = identity.sha256;

        for (const PrivacyCasualArchiveManifestMember& member :
             manifest.members)
        {
            PrivacyPortableImportAssetFact asset;
            asset.role = member.role;
            asset.ordinal = member.ordinal;
            asset.publicRelativePath = member.publicRelativePath;
            asset.originalName = member.originalName;
            asset.protectedRelativePath = member.protectedRelativePath;
            asset.hashAlgorithm = member.hashAlgorithm;
            asset.originalSha256 = member.sha256;
            asset.originalSize = member.size;
            asset.creationTimeUtc = member.creationTimeUtc;
            asset.modificationTimeUtc = member.modificationTimeUtc;
            asset.portableAttributes = member.portableAttributes;
            asset.unixMode = member.unixMode;

            if (!asset.isValid())
            {
                return failure(
                    PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                    QStringLiteral("an archive member fact is invalid"));
            }

            item.assets << asset;
        }

        if (!item.isValid())
        {
            return failure(
                PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
                QStringLiteral("an archive item fact is invalid"));
        }

        candidate.items << item;
    }

    std::sort(candidate.items.begin(), candidate.items.end(),
              [](const PrivacyPortableImportItemFact& left,
                 const PrivacyPortableImportItemFact& right)
              {
                  return (left.itemUuid < right.itemUuid);
              });

    if (!candidate.isValid())
    {
        return failure(
            PrivacyPortableImportAuthenticationStatus::InconsistentManifests,
            QStringLiteral("the authenticated import candidate is invalid"));
    }

    PrivacyPortableImportAuthenticationResult result;
    result.status = PrivacyPortableImportAuthenticationStatus::Authenticated;
    result.candidate = candidate;
    return result;
}

} // namespace Digikam

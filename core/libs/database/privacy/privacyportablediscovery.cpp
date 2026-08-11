/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyportablediscovery.h"

// Qt includes

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QUuid>

// C++ includes

#include <algorithm>
#include <utility>

// Local includes

#include "privacycasualarchive.h"
#include "privacycontracts.h"

namespace Digikam
{

namespace
{

const QString ArchiveSuffix = QStringLiteral(".digikam-private.zip");
const QString MetadataDirectory = QStringLiteral(".digikam-private");
const QString RootMarkerName = QStringLiteral("root-marker-v1.json");
const QString StoresDirectory = QStringLiteral("stores");
const QString GocryptfsConfigName = QStringLiteral("gocryptfs.conf");
constexpr int MaximumScanDepth = 24;

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

QString issueDetail(PrivacyPortableDiscoveryIssueKind kind)
{
    switch (kind)
    {
        case PrivacyPortableDiscoveryIssueKind::InvalidScanRoot:
            return QStringLiteral("the selected folder is not an accessible directory");
        case PrivacyPortableDiscoveryIssueKind::MalformedCasualArchive:
            return QStringLiteral(
                "a candidate archive does not carry a valid private recovery identity");
        case PrivacyPortableDiscoveryIssueKind::InvalidStrongStore:
            return QStringLiteral(
                "a private store root marker or gocryptfs store is missing or unsafe");
        case PrivacyPortableDiscoveryIssueKind::ConflictingIdentity:
            return QStringLiteral(
                "the same recovery identity is used by incompatible private backends or stores");
    }

    return QString();
}

} // namespace

bool PrivacyPortableCasualArchiveCandidate::isValid() const
{
    return (QDir::isAbsolutePath(rootPath) &&
            QDir::isAbsolutePath(absolutePath) &&
            !relativePath.isEmpty() && !proxyRelativePath.isEmpty() &&
            isCanonicalUuid(recoverySetUuid));
}

bool PrivacyPortableStrongStoreCandidate::isValid() const
{
    return (QDir::isAbsolutePath(rootPath) &&
            isCanonicalUuid(storeUuid) &&
            QDir::isAbsolutePath(markerPath) &&
            QDir::isAbsolutePath(configAbsolutePath) &&
            !cipherRelativePath.isEmpty());
}

bool PrivacyPortableDiscoveryGroup::isValid() const
{
    const bool backendOk =
        ((backend == PrivacyBackend::Casual) &&
         !casualArchives.isEmpty()) ||
        ((backend == PrivacyBackend::Strong) &&
         !strongStores.isEmpty());
    const bool identityOk = isCanonicalUuid(recoverySetUuid);

    return (backendOk && identityOk && (rootCount > 0));
}

bool PrivacyPortableDiscoveryResult::isEmpty() const
{
    return (groups.isEmpty() && issues.isEmpty() && !cancelled);
}

void PrivacyPortableDiscovery::walk(
    const QString& root, const QString& directory, int depth,
    PrivacyPortableDiscoveryResult* const result,
    const CancellationCheck& isCancelled)
{
    if (!result || (depth > MaximumScanDepth) || !QDir::isAbsolutePath(root))
    {
        return;
    }

    const QDir dir(directory);

    if (!dir.exists())
    {
        return;
    }

    const QFileInfoList entries =
        dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot |
                          QDir::NoSymLinks);

    for (const QFileInfo& entry : entries)
    {
        if (isCancelled && isCancelled())
        {
            result->cancelled = true;
            return;
        }

        if (entry.isDir())
        {
            if (entry.fileName() != MetadataDirectory)
            {
                walk(root, entry.absoluteFilePath(), depth + 1,
                     result, isCancelled);

                if (result->cancelled)
                {
                    return;
                }
            }

            continue;
        }

        if (!entry.isFile() || !entry.fileName().endsWith(ArchiveSuffix))
        {
            continue;
        }

        const QString absolutePath = entry.absoluteFilePath();
        PrivacyCasualArchiveError error = PrivacyCasualArchiveError::None;
        const PrivacyCasualArchiveIdentity identity =
            PrivacyCasualArchiveEngine().readPublicIdentity(
                absolutePath, &error);

        if (!identity.valid || !isCanonicalUuid(identity.recoverySetUuid))
        {
            PrivacyPortableDiscoveryIssue issue;
            issue.kind = PrivacyPortableDiscoveryIssueKind::MalformedCasualArchive;
            issue.absolutePath = absolutePath;
            issue.detail = issueDetail(issue.kind);
            result->issues << issue;
            continue;
        }

        const QString relativePath = QDir(root).relativeFilePath(absolutePath);
        const QString proxyRelativePath =
            relativePath.left(relativePath.size() - ArchiveSuffix.size());

        if (proxyRelativePath.isEmpty())
        {
            PrivacyPortableDiscoveryIssue issue;
            issue.kind = PrivacyPortableDiscoveryIssueKind::MalformedCasualArchive;
            issue.absolutePath = absolutePath;
            issue.detail = issueDetail(issue.kind);
            result->issues << issue;
            continue;
        }

        PrivacyPortableCasualArchiveCandidate candidate;
        candidate.rootPath = root;
        candidate.absolutePath = absolutePath;
        candidate.relativePath = relativePath;
        candidate.proxyRelativePath = proxyRelativePath;
        candidate.recoverySetUuid = identity.recoverySetUuid;

        if (!candidate.isValid())
        {
            PrivacyPortableDiscoveryIssue issue;
            issue.kind = PrivacyPortableDiscoveryIssueKind::MalformedCasualArchive;
            issue.absolutePath = absolutePath;
            issue.detail = issueDetail(issue.kind);
            result->issues << issue;
            continue;
        }

        PrivacyPortableDiscoveryGroup* group = nullptr;

        for (PrivacyPortableDiscoveryGroup& existing : result->groups)
        {
            if (existing.recoverySetUuid == candidate.recoverySetUuid)
            {
                group = &existing;
                break;
            }
        }

        if (!group)
        {
            PrivacyPortableDiscoveryGroup created;
            created.recoverySetUuid = candidate.recoverySetUuid;
            created.backend = PrivacyBackend::Casual;
            result->groups << created;
            group = &result->groups.last();
        }

        if (group->backend != PrivacyBackend::Casual)
        {
            PrivacyPortableDiscoveryIssue issue;
            issue.kind = PrivacyPortableDiscoveryIssueKind::ConflictingIdentity;
            issue.absolutePath = absolutePath;
            issue.detail = issueDetail(issue.kind);
            result->issues << issue;
            continue;
        }

        group->casualArchives << candidate;
    }
}

bool PrivacyPortableDiscovery::scanRoot(
    const QString& root,
    PrivacyPortableDiscoveryResult* const result,
    const CancellationCheck& isCancelled)
{
    if (!result)
    {
        return false;
    }

    const QFileInfo rootInfo(root);

    if (!rootInfo.isDir() || rootInfo.isSymLink())
    {
        PrivacyPortableDiscoveryIssue issue;
        issue.kind = PrivacyPortableDiscoveryIssueKind::InvalidScanRoot;
        issue.absolutePath = root;
        issue.detail = issueDetail(issue.kind);
        result->issues << issue;
        return false;
    }

    discoverStrongStores(root, result);
    ++result->scannedDirectoryCount;

    walk(root, root, 0, result, isCancelled);

    return true;
}

void PrivacyPortableDiscovery::discoverStrongStores(
    const QString& root,
    PrivacyPortableDiscoveryResult* const result)
{
    const QString markerPath =
        QDir(root).filePath(MetadataDirectory + QLatin1Char('/') +
                            RootMarkerName);
    const QFileInfo markerInfo(markerPath);

    if (!markerInfo.exists())
    {
        return;
    }

    const auto addIssue = [result](const QString& path,
                                         const QString& detail)
    {
        PrivacyPortableDiscoveryIssue issue;
        issue.kind = PrivacyPortableDiscoveryIssueKind::InvalidStrongStore;
        issue.absolutePath = path;
        issue.detail = detail;
        result->issues << issue;
    };

    if (!markerInfo.isFile() || markerInfo.isSymLink())
    {
        addIssue(markerPath, issueDetail(
                     PrivacyPortableDiscoveryIssueKind::InvalidStrongStore));
        return;
    }

    QFile markerFile(markerPath);

    if (!markerFile.open(QIODevice::ReadOnly) ||
        (markerFile.size() > 4096))
    {
        addIssue(markerPath, issueDetail(
                     PrivacyPortableDiscoveryIssueKind::InvalidStrongStore));
        return;
    }

    QString rootUuid;
    QString markerUuid;

    if (!PrivacyRootIdentityCodec::decodeManagedRootMarkerV1(
            markerFile.readAll(), &rootUuid, &markerUuid))
    {
        addIssue(markerPath, issueDetail(
                     PrivacyPortableDiscoveryIssueKind::InvalidStrongStore));
        return;
    }

    const QDir storesDir(QDir(root).filePath(
        MetadataDirectory + QLatin1Char('/') + StoresDirectory));

    if (!storesDir.exists())
    {
        return;
    }

    const QFileInfoList stores =
        storesDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot |
                                QDir::NoSymLinks);

    for (const QFileInfo& storeDir : stores)
    {
        const QString storeUuid = storeDir.fileName();

        if (!isCanonicalUuid(storeUuid))
        {
            addIssue(storeDir.absoluteFilePath(), issueDetail(
                         PrivacyPortableDiscoveryIssueKind::InvalidStrongStore));
            continue;
        }

        const QString configPath = QDir(storeDir.absoluteFilePath())
                                       .filePath(GocryptfsConfigName);
        const QFileInfo configInfo(configPath);

        if (!configInfo.isFile() || configInfo.isSymLink())
        {
            addIssue(configPath, issueDetail(
                         PrivacyPortableDiscoveryIssueKind::InvalidStrongStore));
            continue;
        }

        PrivacyPortableStrongStoreCandidate candidate;
        candidate.rootPath = root;
        candidate.storeUuid = storeUuid;
        candidate.markerPath = markerPath;
        candidate.configAbsolutePath = configPath;
        candidate.cipherRelativePath =
            MetadataDirectory + QLatin1Char('/') + StoresDirectory +
            QLatin1Char('/') + storeUuid;

        if (!candidate.isValid())
        {
            addIssue(configPath, issueDetail(
                         PrivacyPortableDiscoveryIssueKind::InvalidStrongStore));
            continue;
        }

        PrivacyPortableDiscoveryGroup* group = nullptr;

        for (PrivacyPortableDiscoveryGroup& existing : result->groups)
        {
            if (existing.recoverySetUuid == storeUuid)
            {
                group = &existing;
                break;
            }
        }

        if (!group)
        {
            PrivacyPortableDiscoveryGroup created;
            created.recoverySetUuid = storeUuid;
            created.backend = PrivacyBackend::Strong;
            result->groups << created;
            group = &result->groups.last();
        }

        if (group->backend != PrivacyBackend::Strong)
        {
            PrivacyPortableDiscoveryIssue issue;
            issue.kind = PrivacyPortableDiscoveryIssueKind::ConflictingIdentity;
            issue.absolutePath = configPath;
            issue.detail = issueDetail(issue.kind);
            result->issues << issue;
            continue;
        }

        bool duplicate = false;

        for (const PrivacyPortableStrongStoreCandidate& existing :
             std::as_const(group->strongStores))
        {
            if (existing.configAbsolutePath == configPath)
            {
                duplicate = true;
                break;
            }
        }

        if (duplicate)
        {
            continue;
        }

        group->strongStores << candidate;
    }
}

PrivacyPortableDiscoveryResult PrivacyPortableDiscovery::scan(
    const QList<QString>& roots,
    const CancellationCheck& isCancelled)
{
    PrivacyPortableDiscoveryResult result;

    for (const QString& root : roots)
    {
        if (!scanRoot(root, &result, isCancelled))
        {
            if (result.cancelled)
            {
                break;
            }
        }
    }

    // Count distinct contributing roots per group.
    for (PrivacyPortableDiscoveryGroup& group : result.groups)
    {
        QSet<QString> contributingRoots;

        for (const PrivacyPortableCasualArchiveCandidate& candidate :
             std::as_const(group.casualArchives))
        {
            contributingRoots.insert(candidate.rootPath);
        }

        for (const PrivacyPortableStrongStoreCandidate& candidate :
             std::as_const(group.strongStores))
        {
            contributingRoots.insert(candidate.rootPath);
        }

        group.rootCount = contributingRoots.size();
    }

    std::sort(result.groups.begin(), result.groups.end(),
              [](const PrivacyPortableDiscoveryGroup& left,
                 const PrivacyPortableDiscoveryGroup& right)
              {
                  return (left.recoverySetUuid < right.recoverySetUuid);
              });

    return result;
}

} // namespace Digikam

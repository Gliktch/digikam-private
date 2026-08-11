/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <functional>

// Qt includes

#include <QByteArray>
#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyPortableDiscoveryIssueKind
{
    InvalidScanRoot       = 1,
    MalformedCasualArchive = 2,
    InvalidStrongStore    = 3,
    ConflictingIdentity   = 4
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableDiscoveryIssue
{
    PrivacyPortableDiscoveryIssueKind kind = PrivacyPortableDiscoveryIssueKind::InvalidScanRoot;
    QString absolutePath;
    QString detail;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableCasualArchiveCandidate
{
    bool isValid() const;

    QString rootPath;
    QString absolutePath;
    QString relativePath;
    QString proxyRelativePath;
    QString recoverySetUuid;
    qlonglong archiveSize = -1;
    QByteArray sha256;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableStrongStoreCandidate
{
    bool isValid() const;

    QString rootPath;
    QString storeUuid;
    QString markerPath;
    QString configAbsolutePath;
    QString cipherRelativePath;
};

/** One password-authentication row: every candidate sharing the same opaque
 * recovery identity, regardless of how many roots contributed. */
struct DIGIKAM_DATABASE_EXPORT PrivacyPortableDiscoveryGroup
{
    bool isValid() const;

    QString recoverySetUuid;
    PrivacyBackend backend = PrivacyBackend::Casual;
    QList<PrivacyPortableCasualArchiveCandidate> casualArchives;
    QList<PrivacyPortableStrongStoreCandidate> strongStores;
    int rootCount = 0;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableDiscoveryResult
{
    bool isEmpty() const;

    QList<PrivacyPortableDiscoveryGroup> groups;
    QList<PrivacyPortableDiscoveryIssue> issues;
    int scannedDirectoryCount = 0;
    bool cancelled = false;
};

/**
 * Password-free filesystem discovery for portable private-media import.
 * Casual candidates are recognized from their public archive suffix and
 * comment identity; Strong candidates from managed-root markers plus
 * `gocryptfs.conf` under `.digikam-private/stores/<storeUuid>`. The result is
 * a fail-closed pre-scan: malformed or conflicting candidates are issues, not
 * silent groups, and encrypted manifests remain authoritative after
 * authentication.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPortableDiscovery
{
public:

    using CancellationCheck = std::function<bool()>;

    static PrivacyPortableDiscoveryResult scan(
        const QList<QString>& roots,
        const CancellationCheck& isCancelled = {});

private:

    static bool scanRoot(const QString& root,
                         PrivacyPortableDiscoveryResult* result,
                         const CancellationCheck& isCancelled);
    static void walk(const QString& root, const QString& directory, int depth,
                     PrivacyPortableDiscoveryResult* result,
                     const CancellationCheck& isCancelled);
    static void discoverStrongStores(const QString& root,
                                     PrivacyPortableDiscoveryResult* result);
};

} // namespace Digikam

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
#include <QDateTime>
#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacypassword.h"
#include "privacyportablediscovery.h"

namespace Digikam
{

/** One verified original/associated asset fact from a Casual manifest. */
struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportAssetFact
{
    bool isValid() const;

    int       role = 0;
    int       ordinal = -1;
    QString   publicRelativePath;
    QString   originalName;
    QString   protectedRelativePath;
    QString   hashAlgorithm;
    QByteArray originalSha256;
    qlonglong originalSize = -1;
    QDateTime creationTimeUtc;
    QDateTime modificationTimeUtc;
    QByteArray portableAttributes;
    quint32   unixMode = 0;
};

/** One verified protected item (archive + member set) from a Casual group. */
struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportItemFact
{
    bool isValid() const;

    QString itemUuid;
    QString containerUuid;
    QString archiveAbsolutePath;
    QString proxyRelativePath;
    qlonglong archiveSize = -1;
    QByteArray archiveSha256;
    QList<PrivacyPortableImportAssetFact> assets;
};

/** Authenticated, fully verified portable import facts for one category. */
struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportCandidate
{
    bool isValid() const;

    QString recoverySetUuid;
    PrivacyBackend backend = PrivacyBackend::Casual;
    QString categoryUuid;
    /** Empty until the commit stage supplies the localized default for a
     * store-less Casual import whose manifest carries no category name. */
    QString categoryName;
    /** False for store-less Casual import: no credential/store rows exist
     * and authentication must fall back to archive manifest verification. */
    bool hasCredential = false;
    QList<PrivacyPortableImportItemFact> items;
};

enum class PrivacyPortableImportAuthenticationStatus
{
    Authenticated = 1,
    InvalidPassword,
    InconsistentManifests,
    Cancelled,
    UnsupportedBackend,
    InvalidRequest
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportAuthenticationResult
{
    bool succeeded() const
    {
        return (status ==
                PrivacyPortableImportAuthenticationStatus::Authenticated);
    }

    PrivacyPortableImportAuthenticationStatus status =
        PrivacyPortableImportAuthenticationStatus::InvalidRequest;
    QString detail;
    PrivacyPortableImportCandidate candidate;
};

/**
 * Password authentication/preflight for one discovery group. Casual groups
 * are fully verified archive-by-archive with the password; the result either
 * carries complete item/member facts or a fail-closed error. Strong store
 * authentication and category-store linking are added by later slices.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPortableImportAuthenticator
{
public:

    using CancellationCheck = std::function<bool()>;

    static PrivacyPortableImportAuthenticationResult authenticateCasual(
        const PrivacyPortableDiscoveryGroup& group,
        const PrivacyPassword& password,
        const CancellationCheck& isCancelled = {});
};

} // namespace Digikam

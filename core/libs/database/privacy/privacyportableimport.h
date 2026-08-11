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
#include "privacygocryptfsadapter.h"
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

/** One verified protected item (archive or vault object group + members). */
struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportItemFact
{
    bool isValid() const;

    QString itemUuid;
    QString containerUuid;
    PrivacyContainerKind containerKind = PrivacyContainerKind::CasualArchive;
    /** Root-relative archive path (Casual) or vault object directory
     * (Strong), used as PrivacyContainer.objectRelativePath. */
    QString containerRelativePath;
    QString proxyRelativePath;
    /** Absolute archive path for Casual; empty for Strong. */
    QString sourceAbsolutePath;
    qlonglong containerSize = -1;
    QByteArray containerSha256;
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
    /** Present only when hasCredential is true (a copied store was found). */
    QString storeUuid;
    QString managedStoreRootPath;
    QString cipherRelativePath;
    QString credentialEnvelopeFormat;
    QByteArray credentialEnvelopeBlob;
    QList<PrivacyPortableImportItemFact> items;
};

/** Result of inspecting one discovered store with the category password. */
struct DIGIKAM_DATABASE_EXPORT PrivacyPortableStoreInspection
{
    bool valid = false;
    /** Mounted plaintext root while the inspection is held. */
    QString plaintextRoot;
    QString sentinelCategoryUuid;
    QString sentinelStoreUuid;
};

/** Mounts/unmounts a discovered gocryptfs store for import authentication.
 * Production uses the pinned gocryptfs harness; tests inject a fake. */
class DIGIKAM_DATABASE_EXPORT PrivacyPortableStoreInspector
{
public:

    PrivacyPortableStoreInspector()          = default;
    virtual ~PrivacyPortableStoreInspector() = default;

    virtual bool inspect(
        const PrivacyPortableStrongStoreCandidate& store,
        const PrivacyPassword& password,
        PrivacyPortableStoreInspection* inspection,
        QString* error) = 0;
    virtual bool release(
        const PrivacyPortableStoreInspection& inspection,
        QString* error) = 0;

private:

    Q_DISABLE_COPY(PrivacyPortableStoreInspector)
};

/** Production inspector backed by PrivacyGocryptfsStoreHarness. */
class DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsPortableStoreInspector final
    : public PrivacyPortableStoreInspector
{
public:

    PrivacyGocryptfsPortableStoreInspector(
        PrivacyProcessRunner& runner,
        const PrivacyMountStateProbe& mountProbe,
        PrivacyGocryptfsToolPaths toolPaths,
        QString workspaceRoot);
    ~PrivacyGocryptfsPortableStoreInspector() override;

    bool inspect(
        const PrivacyPortableStrongStoreCandidate& store,
        const PrivacyPassword& password,
        PrivacyPortableStoreInspection* inspection,
        QString* error) override;
    bool release(
        const PrivacyPortableStoreInspection& inspection,
        QString* error) override;

private:

    class Private;
    Private* const d = nullptr;
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
 * are fully verified archive-by-archive with the password and then linked to
 * a copied store when one matches by sentinel category UUID (otherwise the
 * candidate stays store-less). Strong groups are verified by mounting the
 * store, validating the sentinel and vault recovery manifest, and checking
 * every vault object byte-for-byte.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPortableImportAuthenticator
{
public:

    using CancellationCheck = std::function<bool()>;

    static PrivacyPortableImportAuthenticationResult authenticateCasual(
        const PrivacyPortableDiscoveryGroup& group,
        const QList<PrivacyPortableStrongStoreCandidate>& storeCandidates,
        const PrivacyPassword& password,
        PrivacyPortableStoreInspector& inspector,
        const CancellationCheck& isCancelled = {});

    static PrivacyPortableImportAuthenticationResult authenticateStrong(
        const PrivacyPortableDiscoveryGroup& group,
        const PrivacyPassword& password,
        PrivacyPortableStoreInspector& inspector,
        const CancellationCheck& isCancelled = {});
};

} // namespace Digikam

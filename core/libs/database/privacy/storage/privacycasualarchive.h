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

class QIODevice;

// Local includes

#include "digikam_export.h"
#include "privacypassword.h"

namespace Digikam
{

enum class PrivacyCasualArchiveError
{
    None,
    Cancelled,
    UnsupportedLibzip,
    InvalidRequest,
    InvalidPassword,
    UnsafeDestination,
    UnsafeSource,
    InvalidMemberName,
    DuplicateMember,
    SourceReadFailed,
    SourceChanged,
    StagingCreateFailed,
    ArchiveWriteFailed,
    ArchiveOpenFailed,
    ArchivePolicyViolation,
    ManifestInvalid,
    DecryptionFailed,
    MemberReadFailed,
    SizeMismatch,
    HashMismatch,
    DurabilityFailed,
    DestinationWriteFailed,
    PublicationConflict,
    ExistingArchiveMismatch,
    PublicationFailed,
    DurabilityUncertain
};

struct DIGIKAM_DATABASE_EXPORT PrivacyCasualArchiveMember
{
    QString    sourcePath;
    QString    protectedRelativePath;
    QString    originalName;
    int        role = 0;
    int        ordinal = -1;
    QDateTime  originalCreationDate;
    QDateTime  originalModificationDate;
    QByteArray portableAttributes;
    quint64    expectedDevice = 0;
    quint64    expectedInode = 0;
    quint64    expectedLinkCount = 0;
    qlonglong  expectedSize = -1;
    QByteArray expectedSha256;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyCasualArchiveRequest
{
    QString                            finalArchivePath;
    /** Optional exact transaction-owned sibling stage. When empty, the
     * archive engine retains its legacy random sibling staging name. */
    QString                            stagingArchivePath;
    QString                            categoryUuid;
    QString                            containerUuid;
    QString                            itemUuid;
    QList<PrivacyCasualArchiveMember>  members;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyCasualArchiveRestoreRequest
{
    QString    archivePath;
    QString    categoryUuid;
    QString    containerUuid;
    QString    itemUuid;
    QString    protectedRelativePath;
    QString    originalName;
    int        role = 0;
    int        ordinal = -1;
    qlonglong  expectedArchiveSize = -1;
    QByteArray expectedArchiveSha256;
    qlonglong  expectedMemberSize = -1;
    QByteArray expectedMemberSha256;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCasualArchiveStage
{
public:

    PrivacyCasualArchiveStage() = default;
    PrivacyCasualArchiveStage(PrivacyCasualArchiveStage&& other) noexcept;
    PrivacyCasualArchiveStage& operator=(PrivacyCasualArchiveStage&& other) = delete;

    PrivacyCasualArchiveStage(const PrivacyCasualArchiveStage&)            = delete;
    PrivacyCasualArchiveStage& operator=(const PrivacyCasualArchiveStage&) = delete;

    bool isValid() const;
    QString stagingPath() const;
    QString finalArchivePath() const;
    qlonglong archiveSize() const;
    QByteArray archiveSha256() const;

private:

    friend class PrivacyCasualArchiveEngine;

    void clear();

private:

    QString    m_stagingPath;
    QString    m_finalArchivePath;
    qlonglong  m_archiveSize = -1;
    QByteArray m_archiveSha256;
    QByteArray m_expectedManifest;
};

/**
 * In-process libzip adapter for the intentionally recoverable casual backend.
 * Staging never modifies the final archive or any plaintext source. A caller
 * journals the returned sibling stage before explicitly publishing it.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyCasualArchiveEngine
{
public:

    using CancellationCheck = std::function<bool()>;

    static QString manifestMemberName();
    static QString expectedMemberPath(int role, int ordinal,
                                      const QString& originalName);

    bool checkCapabilities(PrivacyCasualArchiveError* error = nullptr) const;

    PrivacyCasualArchiveStage stageArchive(
        const PrivacyCasualArchiveRequest& request,
        const PrivacyPassword& password,
        const CancellationCheck& isCancelled = {},
        PrivacyCasualArchiveError* error = nullptr) const;

    /** Rewrites one existing Casual archive from the old category password to
     * the new one without intermediate plaintext files: each decrypted entry
     * streams directly into a same-directory sibling re-encrypted with the
     * new password, the sibling is fully verified with the new password and
     * the old manifest, then published by the caller. */
    PrivacyCasualArchiveStage rewriteArchive(
        const PrivacyCasualArchiveRequest& request,
        const PrivacyPassword& oldPassword,
        const PrivacyPassword& newPassword,
        const CancellationCheck& isCancelled = {},
        PrivacyCasualArchiveError* error = nullptr) const;

    /**
     * Reconstitutes the sole owner of a previously verified stage from its
     * non-secret journal facts. Do not call while another live stage object
     * owns the same path.
     */
    PrivacyCasualArchiveStage resumeStagedArchive(
        const QString& stagingPath,
        const QString& finalArchivePath,
        qlonglong expectedArchiveSize,
        const QByteArray& expectedArchiveSha256,
        const PrivacyPassword& password,
        const CancellationCheck& isCancelled = {},
        PrivacyCasualArchiveError* error = nullptr) const;

    bool verifyStagedArchive(
        const PrivacyCasualArchiveStage& stage,
        const PrivacyPassword& password,
        const CancellationCheck& isCancelled = {},
        PrivacyCasualArchiveError* error = nullptr) const;

    /**
     * Publishes an exact stage whose encrypted contents were fully verified
     * before the caller durably committed its Prepared size/SHA-256 facts.
     * This method rechecks only those opaque byte facts and never decrypts.
     * It must not be used to begin a new archive transaction.
     */
    bool publishExactPreparedStage(
        const QString& stagingPath,
        const QString& finalArchivePath,
        qlonglong expectedArchiveSize,
        const QByteArray& expectedArchiveSha256,
        PrivacyCasualArchiveError* error = nullptr) const;

    /** Fully verifies the archive and its encrypted manifest before streaming
     * one exact member into an already-open caller-owned destination. */
    bool restoreMember(
        const PrivacyCasualArchiveRestoreRequest& request,
        const PrivacyPassword& password,
        QIODevice* destination,
        const CancellationCheck& isCancelled = {},
        PrivacyCasualArchiveError* error = nullptr) const;

    bool publishNew(PrivacyCasualArchiveStage* stage,
                    PrivacyCasualArchiveError* error = nullptr) const;

    bool publishReplacement(PrivacyCasualArchiveStage* stage,
                            const QByteArray& expectedExistingSha256,
                            PrivacyCasualArchiveError* error = nullptr) const;

    bool discardStaged(PrivacyCasualArchiveStage* stage,
                       PrivacyCasualArchiveError* error = nullptr) const;
};

} // namespace Digikam

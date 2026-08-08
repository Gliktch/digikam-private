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
#include <memory>

// Qt includes

#include <QByteArray>
#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyJournalStage
{
    Created                = 1,
    Prepared               = 2,
    Staged                 = 3,
    ProtectedCopyVerified  = 4,
    Applying               = 5,
    PublicStateVerified    = 6,
    ReconciliationRequired = 7,
    Complete               = 8
};

enum class PrivacyJournalExpectedPresence
{
    Absent  = 1,
    Present = 2,
    Unknown = 3
};

struct DIGIKAM_DATABASE_EXPORT PrivacyJournalObjectFact
{
    PrivacyJournalExpectedPresence presence = PrivacyJournalExpectedPresence::Unknown;
    qlonglong                       size     = -1;
    quint64                         linkCount = 0;
    QByteArray                      sha256;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyJournalAsset
{
    QString                  itemUuid;
    QString                  containerUuid;
    int                      role    = 0;
    int                      ordinal = -1;
    QString                  publicRelativePath;
    QString                  stagedRelativePath;
    QString                  protectedRelativePath;
    QString                  containerRelativePath;
    PrivacyJournalObjectFact original;
    PrivacyJournalObjectFact proxy;
    PrivacyJournalObjectFact container;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyJournalRecord
{
    int                    formatVersion = 1;
    QString                transactionUuid;
    QString                categoryUuid;
    QString                rootUuid;
    quint64                rootDevice = 0;
    quint64                rootInode  = 0;
    QByteArray             rootIdentitySha256;
    PrivacyTransactionType transactionType = static_cast<PrivacyTransactionType>(0);
    qlonglong              generation = -1;
    qlonglong              credentialGeneration = -1;
    qlonglong              fromCredentialGeneration = -1;
    qlonglong              toCredentialGeneration   = -1;
    PrivacyJournalStage    stage = static_cast<PrivacyJournalStage>(0);
    QList<PrivacyJournalAsset> assets;
};

enum class PrivacyJournalError
{
    None,
    InvalidRecord,
    EncodingTooLarge,
    InvalidRoot,
    RootIdentityMismatch,
    UnsafeStorage,
    UnsupportedPlatform,
    IoFailure,
    DurabilityFailure,
    AtomicPublicationUnavailable,
    PublicationConflict,
    StaleComparison,
    StageRegression,
    CorruptJournal,
    IdentityMismatch,
    DurabilityUncertain,
    FaultInjected
};

enum class PrivacyJournalLoadDisposition
{
    Missing,
    Loaded,
    Corrupt,
    IdentityMismatch,
    UnsafeStorage,
    DurabilityUncertain
};

struct DIGIKAM_DATABASE_EXPORT PrivacyJournalLoadResult
{
    PrivacyJournalLoadDisposition disposition = PrivacyJournalLoadDisposition::Missing;
    PrivacyJournalError           error       = PrivacyJournalError::None;
    PrivacyJournalRecord          record;
    QByteArray                    canonicalBytes;
    QByteArray                    sha256;
    bool                          hasRecord     = false;
    bool                          authoritative = false;
    bool                          matchesCommitIntent = false;
    QString                       detail;
};

enum class PrivacyJournalFaultPoint
{
    AfterDirectoriesFsynced,
    AfterNextCreated,
    AfterNextWritten,
    AfterNextFsynced,
    AfterNextVerified,
    AfterIntentFsynced,
    AfterPublishRename,
    AfterPublishDirectoryFsync,
    AfterPublishedReadback,
    AfterPreviousRemoved,
    AfterIntentRemoved,
    AfterCleanupDirectoryFsync
};

class DIGIKAM_DATABASE_EXPORT PrivacyTransactionJournalCodec
{
public:

    static constexpr int FormatVersion = 1;
    static constexpr int MaximumAssetCount = 512;
    static constexpr qsizetype MaximumEncodedBytes = 4 * 1024 * 1024;

    static bool validate(const PrivacyJournalRecord& record,
                         QString* detail = nullptr);
    static QByteArray encode(const PrivacyJournalRecord& record,
                             PrivacyJournalError* error = nullptr,
                             QString* detail = nullptr);
    static bool decode(const QByteArray& bytes,
                       PrivacyJournalRecord* record,
                       PrivacyJournalError* error = nullptr,
                       QString* detail = nullptr);
    static QByteArray sha256(const QByteArray& canonicalBytes);
    static QString relativeJournalPath(const QString& transactionUuid);
};

struct DIGIKAM_DATABASE_EXPORT PrivacyJournalRootExpectation
{
    QString    rootUuid;
    QString    markerUuid;
    QByteArray identitySha256;
    quint64    device = 0;
    quint64    inode  = 0;
};

/**
 * Descriptor-relative journal persistence for an already mounted collection
 * root. Linux openat2() confinement is preferred. The fallback accepts only
 * individually validated path components with O_NOFOLLOW and post-open
 * device/owner/type checks; it deliberately fails closed for publication
 * when atomic renameat2() semantics are unavailable.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyTransactionJournalStore
{
public:

    using FaultHook = std::function<bool(PrivacyJournalFaultPoint)>;

    ~PrivacyTransactionJournalStore();

    PrivacyTransactionJournalStore(PrivacyTransactionJournalStore&& other) noexcept;
    PrivacyTransactionJournalStore& operator=(PrivacyTransactionJournalStore&& other) noexcept;

    PrivacyTransactionJournalStore(const PrivacyTransactionJournalStore&)            = delete;
    PrivacyTransactionJournalStore& operator=(const PrivacyTransactionJournalStore&) = delete;

    /**
     * Resolves one validated persistent root to the exact device/inode and
     * identity facts required by a transaction. This is read-only: it opens
     * and verifies the existing root and managed-root marker but never creates
     * journal directories or files.
     */
    static bool inspectRootExpectation(
        const PrivacyStorageRoot& root,
        PrivacyJournalRootExpectation* expectation,
        PrivacyJournalError* error = nullptr,
        QString* detail = nullptr);

    static std::unique_ptr<PrivacyTransactionJournalStore> open(
        const QString& absoluteRootPath,
        const PrivacyJournalRootExpectation& expectation,
        PrivacyJournalError* error = nullptr,
        QString* detail = nullptr);

    PrivacyJournalLoadResult load(const QString& transactionUuid) const;

    bool create(const PrivacyJournalRecord& record,
                QByteArray* publishedSha256 = nullptr,
                PrivacyJournalError* error = nullptr,
                QString* detail = nullptr);

    bool compareAndUpdate(const PrivacyJournalRecord& record,
                          const QByteArray& expectedCurrentSha256,
                          QByteArray* publishedSha256 = nullptr,
                          PrivacyJournalError* error = nullptr,
                          QString* detail = nullptr);

    void setFaultHook(const FaultHook& hook);
    quint64 rootDevice() const;
    quint64 rootInode() const;

private:

    PrivacyTransactionJournalStore();
    bool persist(const PrivacyJournalRecord& record,
                 const QByteArray& expectedCurrentSha256,
                 bool update,
                 QByteArray* publishedSha256,
                 PrivacyJournalError* error,
                 QString* detail);
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Digikam

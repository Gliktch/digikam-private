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

// Qt includes

#include <QDateTime>
#include <QByteArray>
#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyBackend
{
    Casual = 1,
    Strong = 2
};

enum class PrivacyPresentationMode
{
    Blur    = 1,
    Generic = 2
};

enum class PrivacyUnlockedThumbnailMode
{
    AlwaysOpaque = 1,
    FocusedClear = 2,
    AllClearWhileUnlocked = 3
};

enum class PrivacyTagVisibilityMode
{
    UnlockedOnly = 1,
    AlwaysVisible = 2
};

enum class PrivacyCategoryLifecycleState
{
    Creating           = 1,
    Active             = 2,
    TransactionBlocked = 3,
    Error              = 4
};

enum class PrivacyStorageRootKind
{
    AlbumRoot        = 1,
    ManagedStoreRoot = 2
};

enum class PrivacyStoreLifecycleState
{
    Creating = 1,
    Active   = 2,
    Migrating = 3,
    Error    = 4
};

enum class PrivacyStoreRole
{
    CredentialAuthority = 1,
    Originals           = 2,
    Derivatives         = 3
};

enum class PrivacyContainerKind
{
    CasualArchive = 1,
    StrongObject  = 2
};

enum class PrivacyContainerState
{
    Creating = 1,
    Verified = 2,
    Rewriting = 3,
    Error    = 4
};

enum class PrivacyDerivativeKind
{
    ClearThumbnail      = 1,
    BlurredPresentation = 2
};

enum class PrivacyTransactionType
{
    ProtectItem         = 1,
    UnprotectItem       = 2,
    ChangePassword      = 3,
    MigrateBackend      = 4,
    CompatibilityUnlock = 5,
    ExternalCheckout   = 7,
    DeleteProtectedItem = 8,
    ChangePresentation = 9,
    CreateCategory      = 10
};

enum class PrivacyTransactionState
{
    Created             = 1,
    Prepared            = 2,
    Applying            = 3,
    Exposed             = 4,
    Relocking           = 5,
    NeedsReconciliation = 6,
    Complete            = 7,
    Error               = 8
};

class DIGIKAM_DATABASE_EXPORT PrivacyCategory
{
public:

    bool isValid() const;

public:

    QString                 uuid;
    QString                 name;
    PrivacyBackend          backend          = PrivacyBackend::Casual;
    PrivacyPresentationMode presentationMode = PrivacyPresentationMode::Generic;
    PrivacyUnlockedThumbnailMode unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
    PrivacyTagVisibilityMode tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    PrivacyCategoryLifecycleState lifecycleState = PrivacyCategoryLifecycleState::Creating;
    qlonglong               currentCredentialGeneration = 0;
    int                     schemaVersion     = 1;
    QDateTime               createdAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyItem
{
public:

    bool isValid() const;

public:

    qlonglong imageId               = -1;
    QString   uuid;
    QString   categoryUuid;
    QString   originalHash;
    qlonglong originalSize          = -1;
    int       originalWidth         = 0;
    int       originalHeight        = 0;
    QDateTime originalCreationDate;
    QString   expectedProxyHash;
    qlonglong expectedProxySize     = -1;
    int       presentationVersion   = 1;
    qlonglong generation            = 0;
    int       transactionState      = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyCredential
{
public:

    bool isValid() const;

public:

    QString   categoryUuid;
    qlonglong generation = -1;
    QString   encodingVersion;
    QString   envelopeFormat;
    QByteArray envelopeBlob;
    QString   envelopeHashAlgorithm;
    QString   envelopeHash;
    int       recoveryMode = 0;
    int       recoveryState = 0;
    int       recoveryRecordVersion = 1;
    QString   recoveryDocumentUuid;
    QDateTime recoveryAcknowledgedAt;
    QDateTime recoveryVerifiedAt;
    QDateTime createdAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStorageRoot
{
public:

    bool isValid() const;

public:

    QString                uuid;
    PrivacyStorageRootKind kind = static_cast<PrivacyStorageRootKind>(0);
    int                    albumRootId = -1;
    QString                configuredPath;
    int                    identityVersion = 0;
    QByteArray             identityData;
    QString                markerUuid;
    int                    schemaVersion = 1;
    QDateTime              createdAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStore
{
public:

    bool isValid() const;

public:

    QString                    uuid;
    QString                    categoryUuid;
    QString                    rootUuid;
    QString                    format;
    int                        formatVersion = 0;
    QString                    cipherRelativePath;
    QString                    configRelativePath;
    qlonglong                  configGeneration = -1;
    PrivacyStoreLifecycleState lifecycleState = static_cast<PrivacyStoreLifecycleState>(0);
    int                        schemaVersion = 1;
    QDateTime                  createdAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStoreBinding
{
public:

    bool isValid() const;

public:

    QString          categoryUuid;
    PrivacyStoreRole role = static_cast<PrivacyStoreRole>(0);
    QString          storeUuid;
    int              schemaVersion = 1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyContainer
{
public:

    bool isValid() const;

public:

    QString               uuid;
    QString               itemUuid;
    PrivacyContainerKind  kind = static_cast<PrivacyContainerKind>(0);
    QString               rootUuid;
    QString               storeUuid;
    QString               objectRelativePath;
    qlonglong             protectedSize = -1;
    QString               protectedHashAlgorithm;
    QString               protectedHash;
    int                   formatVersion = 0;
    qlonglong             credentialGeneration = -1;
    PrivacyContainerState state = static_cast<PrivacyContainerState>(0);
    QDateTime             createdAt;
    QDateTime             updatedAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAsset
{
public:

    static constexpr int PrimaryMediaRole = 1;

    bool isValid() const;

public:

    QString    itemUuid;
    int        role = 0;
    int        ordinal = -1;
    QString    originalName;
    QString    publicRootUuid;
    QString    publicRelativePath;
    QString    containerUuid;
    QString    protectedRelativePath;
    QString    hashAlgorithm;
    QString    originalHash;
    qlonglong  originalSize = -1;
    QDateTime  originalCreationDate;
    QDateTime  originalModificationDate;
    QByteArray portableAttributes;
    QString    proxyHashAlgorithm;
    QString    proxyHash;
    qlonglong  proxySize = -1;
    int        proxyPresentationVersion = 0;
    qlonglong  proxyGeneration = -1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyDerivative
{
public:

    bool isValid() const;

public:

    QString               itemUuid;
    PrivacyDerivativeKind kind = static_cast<PrivacyDerivativeKind>(0);
    int                   ordinal = -1;
    QString               storeUuid;
    QString               protectedRelativePath;
    QString               sourceHashAlgorithm;
    QString               sourceOriginalHash;
    QString               derivativeFormat;
    QString               derivativeHashAlgorithm;
    QString               derivativeHash;
    qlonglong             derivativeSize = -1;
    int                   presentationVersion = 0;
    qlonglong             generation = -1;
    QDateTime             createdAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyTransaction
{
public:

    bool isValid() const;
    bool isActive() const;

public:

    QString    uuid;
    QString    categoryUuid;
    QString    itemUuid;
    PrivacyTransactionType type = static_cast<PrivacyTransactionType>(0);
    PrivacyTransactionState state = static_cast<PrivacyTransactionState>(0);
    qlonglong  generation = -1;
    qlonglong  fromCredentialGeneration = -1;
    qlonglong  toCredentialGeneration = -1;
    int        payloadFormatVersion = 0;
    QByteArray payloadData;
    QDateTime  createdAt;
    QDateTime  updatedAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyTransactionJournal
{
public:

    bool isValid() const;

public:

    QString   transactionUuid;
    QString   rootUuid;
    QString   journalRelativePath;
    int       journalFormatVersion = 0;
    int       stage = 0;
    QString   expectedHashAlgorithm;
    QString   expectedJournalHash;
    QDateTime updatedAt;
};

class DIGIKAM_DATABASE_EXPORT PrivacyRepositorySnapshot
{
public:

    QList<PrivacyCategory>           categories;
    QList<PrivacyCredential>         credentials;
    QList<PrivacyStorageRoot>        storageRoots;
    QList<PrivacyStore>              stores;
    QList<PrivacyStoreBinding>       storeBindings;
    QList<PrivacyItem>               items;
    QList<PrivacyContainer>          containers;
    QList<PrivacyAsset>              assets;
    QList<PrivacyDerivative>         derivatives;
    QList<PrivacyTransaction>        transactions;
    QList<PrivacyTransactionJournal> transactionJournals;
};

} // namespace Digikam

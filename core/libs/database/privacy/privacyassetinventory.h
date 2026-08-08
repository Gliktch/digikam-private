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

#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyInventoryStatus
{
    Ready      = 1,
    Incomplete = 2,
    Rejected   = 3
};

enum class PrivacyInventoryAssetRole
{
    PrimaryMedia      = 1,
    PairedMedia       = 2,
    XmpSidecar        = 3,
    ConfiguredSidecar = 4
};

enum class PrivacyInventoryFileType
{
    Missing   = 1,
    Regular   = 2,
    Directory = 3,
    Symlink   = 4,
    Special   = 5
};

enum class PrivacyInventoryAliasKind
{
    HardlinkAlias         = 1,
    DatabaseItemAlias     = 2,
    ContentIdentityAlias  = 3,
    DigikamGroupMember    = 4
};

enum class PrivacyInventoryIssueCode
{
    InvalidRequest                    = 1,
    InvalidConfiguredSidecarExtension = 2,
    PrimaryMissing                    = 3,
    UnsafeFileType                    = 4,
    UnsafePath                        = 5,
    DirectoryEnumerationIncomplete    = 6,
    HardlinkEnumerationIncomplete     = 7,
    IdentityAliasEnumerationIncomplete = 8,
    AmbiguousPairedMedia              = 9,
    PathCollision                     = 10,
    AliasEvidenceMismatch             = 11,
    RequiredAssetIdentityCollision    = 12,
    ReservedPrivateArchivePath        = 13
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryRoot
{
public:

    bool isValid() const;

public:

    QString uuid;
    QString absolutePath;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryLocation
{
public:

    bool isValid() const;

public:

    PrivacyInventoryRoot root;
    QString              relativePath;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryFileEvidence
{
public:

    bool isRegular() const;

public:

    PrivacyInventoryFileType type = PrivacyInventoryFileType::Missing;
    bool                      identityComplete = false;
    quint64                   deviceId = 0;
    quint64                   inode = 0;
    quint64                   linkCount = 0;
    qlonglong                 byteSize = -1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryDirectoryEvidence
{
public:

    bool        complete = false;
    QStringList entryNames;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryAsset
{
public:

    bool isValid() const;

public:

    PrivacyInventoryAssetRole role = static_cast<PrivacyInventoryAssetRole>(0);
    int                       ordinal = -1;
    PrivacyInventoryLocation location;
    PrivacyInventoryFileEvidence evidence;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryAliasCandidate
{
public:

    bool isValid() const;

public:

    PrivacyInventoryAliasKind kind = static_cast<PrivacyInventoryAliasKind>(0);
    PrivacyInventoryLocation  location;
    qlonglong                 imageId = -1;
    QString                   contentIdentity;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryAliasEvidence
{
public:

    bool                                  complete = false;
    QList<PrivacyInventoryAliasCandidate> candidates;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryFileIdentity
{
public:

    bool isValid() const;

public:

    quint64 deviceId = 0;
    quint64 inode = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryHardlinkEvidence
{
public:

    bool isValid() const;

public:

    PrivacyInventoryFileIdentity identity;
    PrivacyInventoryAliasEvidence aliases;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryExposureWarning
{
public:

    bool isValid() const;

public:

    PrivacyInventoryAliasKind kind = static_cast<PrivacyInventoryAliasKind>(0);
    PrivacyInventoryLocation  source;
    PrivacyInventoryLocation  alias;
    qlonglong                 imageId = -1;
    QString                   contentIdentity;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryIssue
{
public:

    bool isValid() const;

public:

    PrivacyInventoryIssueCode code = static_cast<PrivacyInventoryIssueCode>(0);
    PrivacyInventoryLocation  location;
    QString                   detail;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryRequest
{
public:

    bool isValid() const;

public:

    int                      contractVersion = 1;
    PrivacyInventoryLocation primary;

    /**
     * Extension tokens configured in digiKam's Custom Sidecar Extensions
     * setting, without a leading dot. XMP is always considered separately.
     */
    QStringList configuredSidecarExtensions;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryResult
{
public:

    bool isValid() const;
    bool isReady() const;

public:

    int                                    contractVersion = 1;
    PrivacyInventoryStatus                 status = PrivacyInventoryStatus::Rejected;
    QList<PrivacyInventoryAsset>            requiredAssets;
    QList<PrivacyInventoryExposureWarning> exposureWarnings;
    QList<PrivacyInventoryIssue>            issues;
};

/**
 * Read-only filesystem evidence boundary. Implementations must use lstat-like
 * facts for inspect(), must not follow symlinks, and may report complete=true
 * only when the requested enumeration was authoritative for the supplied
 * scope at the time it was produced.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyAssetFilesystemProvider
{
public:

    PrivacyAssetFilesystemProvider()          = default;
    virtual ~PrivacyAssetFilesystemProvider() = default;

    virtual PrivacyInventoryFileEvidence inspect(const PrivacyInventoryLocation& location) const = 0;
    virtual PrivacyInventoryDirectoryEvidence listDirectory(const PrivacyInventoryRoot& root,
                                                            const QString& relativeDirectory) const = 0;
    virtual PrivacyInventoryAliasEvidence hardlinkAliases(quint64 deviceId,
                                                          quint64 inode) const = 0;

    /**
     * Batch seam used by one associated-asset inventory. The default preserves
     * compatibility with simple providers; production providers should
     * override it to avoid one tree traversal per required member.
     */
    virtual QList<PrivacyInventoryHardlinkEvidence> hardlinkAliasesFor(
        const QList<PrivacyInventoryFileIdentity>& identities) const;

private:

    Q_DISABLE_COPY(PrivacyAssetFilesystemProvider)
};

/**
 * Read-only database/content-identity evidence boundary. Group candidates are
 * warnings only; the inventory never expands arbitrary digiKam groups.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyAssetIdentityProvider
{
public:

    PrivacyAssetIdentityProvider()          = default;
    virtual ~PrivacyAssetIdentityProvider() = default;

    virtual PrivacyInventoryAliasEvidence aliasesFor(const PrivacyInventoryAsset& asset) const = 0;

private:

    Q_DISABLE_COPY(PrivacyAssetIdentityProvider)
};

/**
 * Builds a deterministic, previewable set of files that one protect
 * transaction must handle. This service performs no filesystem or database
 * mutation. It fails closed unless directory, hardlink and identity-alias
 * enumerations are explicitly complete.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventory
{
public:

    static PrivacyAssetInventoryResult build(const PrivacyAssetInventoryRequest& request,
                                             const PrivacyAssetFilesystemProvider& filesystem,
                                             const PrivacyAssetIdentityProvider& identities);
};

} // namespace Digikam

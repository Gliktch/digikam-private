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

#include <QByteArray>
#include <QList>
#include <QString>
#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyActionKind
{
    Preview             = 1,
    InternalEdit        = 2,
    ExternalOpen        = 3,
    OpenInFileManager   = 4,
    Export              = 5,
    Print               = 6,
    DragOrClipboard     = 7,
    Slideshow           = 8,
    BatchProcess        = 9,
    MetadataWrite       = 10,
    MoveRenameDelete    = 11,
    Analysis            = 12
};

enum class PrivacyRequestedSource
{
    NoPixels         = 1,
    PublicProxy      = 2,
    InternalOriginal = 3,
    WritableCheckout = 4,
    PublicOriginal   = 5
};

enum class PrivacyMutationPolicy
{
    ReadOnly             = 1,
    MayCreateOutputs     = 2,
    CommitProtectedAsset = 3,
    DestructiveMutation  = 4
};

enum class PrivacyPreparedDisposition
{
    Allowed                = 1,
    Excluded               = 2,
    Canceled               = 3,
    UnprotectedPassThrough = 4
};

class DIGIKAM_DATABASE_EXPORT PrivacyActionItem
{
public:

    bool isValid() const;

public:

    qlonglong imageId = -1;
    QString   publicPath;
};

class DIGIKAM_DATABASE_EXPORT PrivacyActionRequest
{
public:

    bool isValid() const;

public:

    int                        contractVersion = 1;
    PrivacyActionKind          actionKind = static_cast<PrivacyActionKind>(0);
    QString                    consumerIdentity;
    QList<PrivacyActionItem>   items;
    PrivacyRequestedSource     requestedSource = static_cast<PrivacyRequestedSource>(0);
    PrivacyMutationPolicy      mutationPolicy = static_cast<PrivacyMutationPolicy>(0);
};

class DIGIKAM_DATABASE_EXPORT PrivacyLeaseToken
{
public:

    bool isValid() const;

public:

    QString   uuid;
    QString   itemUuid;
    qlonglong itemGeneration = -1;
    quint64   categoryEpoch = 0;
    quint64   publicRootEpoch = 0;
    /// Zero when the prepared source does not depend on a category store.
    quint64   storeRootEpoch = 0;
};

enum class PrivacyLeaseValidation
{
    Valid        = 1,
    Revoked      = 2,
    StateChanged = 3,
    RootUnavailable = 4
};

class DIGIKAM_DATABASE_EXPORT PrivacyLeaseAuthority
{
public:

    PrivacyLeaseAuthority()          = default;
    virtual ~PrivacyLeaseAuthority() = default;

    virtual PrivacyLeaseValidation validate(const PrivacyLeaseToken& lease) const = 0;

private:

    Q_DISABLE_COPY(PrivacyLeaseAuthority)
};

class DIGIKAM_DATABASE_EXPORT PreparedPrivacyItem
{
public:

    bool isValid() const;

public:

    PrivacyActionItem           logicalItem;
    QString                     physicalPath;
    PrivacyPreparedDisposition disposition = static_cast<PrivacyPreparedDisposition>(0);
    /// Protected allowed items carry a lease. Ordinary items use the explicit
    /// UnprotectedPassThrough disposition and an empty lease.
    PrivacyLeaseToken           lease;
};

class DIGIKAM_DATABASE_EXPORT PreparedPrivacySelection
{
public:

    bool isValid() const;

public:

    int                         contractVersion = 1;
    /// Allowed may represent a partial selection containing Allowed,
    /// UnprotectedPassThrough, and explicitly Excluded items. Excluded and
    /// Canceled aggregate dispositions require every item to match.
    PrivacyPreparedDisposition disposition = static_cast<PrivacyPreparedDisposition>(0);
    QList<PreparedPrivacyItem>  items;
};

enum class PrivacyRootRuntimeState
{
    Unknown          = 0,
    Recovering       = 1,
    VerifiedAvailable = 2,
    Offline          = 3,
    IdentityMismatch = 4
};

class DIGIKAM_DATABASE_EXPORT PrivacyRootIdentityCodec
{
public:

    static QByteArray encodeAlbumRootV1(int albumRootId, const QString& collectionIdentifier);
    static bool matchesAlbumRootV1(const QByteArray& identityData,
                                   int albumRootId,
                                   const QString& collectionIdentifier);

    /// Neutral marker location below every application-managed storage root.
    static QString managedRootMarkerRelativePathV1();

    /// Persisted identity descriptor. filesystemIdentity may be empty when the
    /// platform cannot provide a stable filesystem UUID.
    static QByteArray encodeManagedRootV1(const QString& markerUuid,
                                          const QString& filesystemIdentity = QString());
    static bool matchesManagedRootV1(const QByteArray& identityData,
                                     const QString& markerUuid,
                                     const QString& filesystemIdentity = QString());

    /// Contents of .digikam-private/root-marker-v1.json.
    static QByteArray encodeManagedRootMarkerV1(const QString& rootUuid,
                                                const QString& markerUuid);
    static bool matchesManagedRootMarkerV1(const QByteArray& markerData,
                                           const QString& rootUuid,
                                           const QString& markerUuid);
};

} // namespace Digikam

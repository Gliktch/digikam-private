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
#include <QHash>
#include <QList>
#include <QScopedPointer>
#include <QSet>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "privacyassetinventory.h"
#include "privacycontracts.h"
#include "privacyposixfilesystemadapter.h"

namespace Digikam
{

class PrivacyRuntimeCoordinator;

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryCatalogueItem
{
public:

    bool isValid() const;

public:

    qlonglong imageId = -1;
    QString   publicRootUuid;
    QString   publicRelativePath;
    qlonglong fileSize = -1;
    QString   databaseIdentity;
    QString   storedContentIdentity;
    bool      contentIdentityAuthoritative = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryGroupRelation
{
public:

    bool isValid() const;

public:

    qlonglong memberImageId = -1;
    qlonglong leaderImageId = -1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryCatalogueSnapshot
{
public:

    QByteArray                               generation;
    bool                                     complete = false;
    QList<PrivacyInventoryCatalogueItem>     items;
    QList<PrivacyInventoryGroupRelation>     groups;
    QSet<qlonglong>                          protectedImageIds;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryCatalogueProvider
{
public:

    PrivacyInventoryCatalogueProvider()          = default;
    virtual ~PrivacyInventoryCatalogueProvider() = default;

    virtual PrivacyInventoryCatalogueSnapshot snapshot(qsizetype maximumItems,
                                                        qsizetype maximumGroups) const = 0;
    virtual bool generationMatches(const QByteArray& generation,
                                   qsizetype maximumItems,
                                   qsizetype maximumGroups) const = 0;

private:

    Q_DISABLE_COPY(PrivacyInventoryCatalogueProvider)
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryRootRecord
{
public:

    QString                 uuid;
    PrivacyRootRuntimeState state = PrivacyRootRuntimeState::Unknown;
    PrivacyPosixRootScope   scope;
    quint64                 epoch = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryRootSnapshot
{
public:

    QByteArray                         generation;
    bool                               complete = false;
    QList<PrivacyInventoryRootRecord>  roots;
};

class DIGIKAM_DATABASE_EXPORT PrivacyInventoryRootProvider
{
public:

    PrivacyInventoryRootProvider()          = default;
    virtual ~PrivacyInventoryRootProvider() = default;

    virtual PrivacyInventoryRootSnapshot snapshot() const = 0;
    virtual bool generationMatches(const QByteArray& generation) const = 0;

private:

    Q_DISABLE_COPY(PrivacyInventoryRootProvider)
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryBridgeLimits
{
public:

    bool isValid() const;

public:

    qsizetype maximumSelectionItems = 1000;
    qsizetype maximumCatalogueItems = 250000;
    qsizetype maximumGroupRelations = 250000;
    qsizetype maximumResultEntries  = 100000;
    PrivacyPosixScanLimits filesystemLimits;
};

enum class PrivacyAssetInventoryBridgeIssueCode
{
    InvalidRequest                 = 1,
    SelectionLimitExceeded        = 2,
    DuplicateSelectedImageId      = 3,
    CatalogueEvidenceIncomplete   = 4,
    SelectedImageMissing          = 5,
    RootEvidenceIncomplete        = 6,
    RootOffline                   = 7,
    RootIdentityMismatch          = 8,
    RootUnavailable              = 9,
    DuplicateSelectedPath         = 10,
    AlreadyProtected             = 11,
    ReservedPrivateArchivePath   = 12,
    Canceled                     = 13,
    GenerationChanged            = 14,
    ResultLimitExceeded          = 15,
    ExactContentIdentityIncomplete = 16
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryBridgeIssue
{
public:

    PrivacyAssetInventoryBridgeIssueCode code =
        PrivacyAssetInventoryBridgeIssueCode::InvalidRequest;
    qlonglong imageId = -1;
    QString   detail;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryBridgeItemResult
{
public:

    qlonglong                       imageId = -1;
    PrivacyAssetInventoryRequest    request;
    PrivacyAssetInventoryResult     inventory;
    QList<PrivacyAssetInventoryBridgeIssue> issues;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryBridgeRequest
{
public:

    QList<qlonglong>                     imageIds;
    QStringList                          configuredSidecarExtensions;
    PrivacyAssetInventoryBridgeLimits    limits;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryBridgeResult
{
public:

    PrivacyInventoryStatus                         status = PrivacyInventoryStatus::Rejected;
    QList<PrivacyAssetInventoryBridgeItemResult>    items;
    QList<PrivacyAssetInventoryBridgeIssue>         issues;
    QByteArray                                      catalogueGeneration;
    QByteArray                                      rootGeneration;
};

/** Immutable adapter over one bounded, transactionally stable catalogue snapshot. */
class DIGIKAM_DATABASE_EXPORT PrivacyCatalogueAssetIdentityProvider final
    : public PrivacyAssetIdentityProvider
{
public:

    PrivacyCatalogueAssetIdentityProvider(
        const PrivacyInventoryCatalogueSnapshot& catalogue,
        const QList<PrivacyInventoryRootRecord>& roots);
    ~PrivacyCatalogueAssetIdentityProvider() override;

    PrivacyInventoryAliasEvidence aliasesFor(
        const PrivacyInventoryAsset& asset) const override;

private:

    class Private;
    const QScopedPointer<Private> d;

    Q_DISABLE_COPY(PrivacyCatalogueAssetIdentityProvider)
};

/** Production CoreDb snapshot provider. Stored digiKam unique hashes are a
 * bounded candidate index only: they are sampled fingerprints, not complete
 * hashes. Exact comparison belongs in protect preflight for those candidates. */
class DIGIKAM_DATABASE_EXPORT PrivacyCoreDbAssetInventoryProvider final
    : public PrivacyInventoryCatalogueProvider
{
public:

    explicit PrivacyCoreDbAssetInventoryProvider(
        const QHash<int, QString>& rootUuidByAlbumRootId);
    ~PrivacyCoreDbAssetInventoryProvider() override;

    PrivacyInventoryCatalogueSnapshot snapshot(qsizetype maximumItems,
                                                qsizetype maximumGroups) const override;
    bool generationMatches(const QByteArray& generation,
                           qsizetype maximumItems,
                           qsizetype maximumGroups) const override;

private:

    const QHash<int, QString> m_rootUuidByAlbumRootId;

    Q_DISABLE_COPY(PrivacyCoreDbAssetInventoryProvider)
};

/** Production collection-root adapter. A path is scoped only after the
 * runtime coordinator has published VerifiedAvailable for its stable root UUID. */
class DIGIKAM_DATABASE_EXPORT PrivacyRuntimeAssetRootProvider final
    : public PrivacyInventoryRootProvider
{
public:

    explicit PrivacyRuntimeAssetRootProvider(
        const QSharedPointer<const PrivacyRuntimeCoordinator>& runtime);
    ~PrivacyRuntimeAssetRootProvider() override;

    PrivacyInventoryRootSnapshot snapshot() const override;
    bool generationMatches(const QByteArray& generation) const override;

private:

    const QSharedPointer<const PrivacyRuntimeCoordinator> m_runtime;

    Q_DISABLE_COPY(PrivacyRuntimeAssetRootProvider)
};

class DIGIKAM_DATABASE_EXPORT PrivacyAssetInventoryBridge
{
public:

    static PrivacyAssetInventoryBridgeResult build(
        const PrivacyAssetInventoryBridgeRequest& request,
        const PrivacyInventoryCatalogueProvider& catalogue,
        const PrivacyInventoryRootProvider& roots,
        const PrivacyPosixInventoryControl* control = nullptr);
};

} // namespace Digikam

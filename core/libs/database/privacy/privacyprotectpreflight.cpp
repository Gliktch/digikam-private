/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprotectpreflight.h"

// Qt includes

#include <QDir>

// C++ includes

#include <utility>

// Local includes

#include "collectionlocation.h"
#include "collectionmanager.h"
#include "iteminfo.h"
#include "privacyruntime.h"

namespace Digikam
{

PrivacyProtectPreflightResult PrivacyProtectPreflight::build(
    const PrivacyAssetInventoryBridgeRequest& request,
    const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
    const PrivacyPosixInventoryControl* const control)
{
    PrivacyProtectPreflightResult result;

    if (!runtime)
    {
        PrivacyAssetInventoryBridgeIssue issue;
        issue.code = PrivacyAssetInventoryBridgeIssueCode::InvalidRequest;
        result.bridge.issues << issue;

        return result;
    }

    QSet<int> selectedAlbumRootIds;
    QSet<qlonglong> selectedImageIds;
    bool requestMayRegister = request.limits.isValid() &&
                              !request.imageIds.isEmpty() &&
                              (request.imageIds.size() <=
                               request.limits.maximumSelectionItems) &&
                              (!control || !control->isCanceled());

    for (qlonglong imageId : request.imageIds)
    {
        if ((imageId <= 0) || selectedImageIds.contains(imageId))
        {
            requestMayRegister = false;
            continue;
        }

        selectedImageIds.insert(imageId);

        if (!requestMayRegister)
        {
            continue;
        }

        const ItemInfo info(imageId);

        if (!info.isNull() && (info.albumRootId() > 0))
        {
            selectedAlbumRootIds.insert(info.albumRootId());
        }
    }

    if (!requestMayRegister)
    {
        selectedAlbumRootIds.clear();
    }

    PrivacyRepository repository;

    for (int albumRootId : std::as_const(selectedAlbumRootIds))
    {
        if (control && control->isCanceled())
        {
            break;
        }

        if (!runtime->rootUuidForAlbumRootId(albumRootId).isEmpty())
        {
            result.registrations.insert(
                albumRootId, PrivacyAlbumRootRegistrationStatus::Existing);
            continue;
        }

        const CollectionLocation location = CollectionManager::instance()
                                                ->locationForAlbumRootId(albumRootId);

        if (location.isNull() || !location.isAvailable())
        {
            result.registrations.insert(
                albumRootId, PrivacyAlbumRootRegistrationStatus::Offline);
            continue;
        }

        const PrivacyAlbumRootRegistrationResult registration =
            repository.ensureAlbumRoot(albumRootId,
                                       QDir::cleanPath(location.albumRootPath()),
                                       location.identifier);
        result.registrations.insert(albumRootId, registration.status);

        if (!registration.succeeded())
        {
            continue;
        }

        if (registration.status == PrivacyAlbumRootRegistrationStatus::Created)
        {
            result.newlyCreatedRootUuids.insert(registration.root.uuid);
        }

        const PrivacyRootRecoveryResult recovery =
            runtime->registerAlbumRoot(registration.root);

        if (recovery == PrivacyRootRecoveryResult::PublishedOffline)
        {
            result.registrations.insert(
                albumRootId, PrivacyAlbumRootRegistrationStatus::Offline);
        }
        else if (recovery == PrivacyRootRecoveryResult::PublishedIdentityMismatch)
        {
            result.registrations.insert(
                albumRootId, PrivacyAlbumRootRegistrationStatus::IdentityMismatch);
        }
        else if (recovery != PrivacyRootRecoveryResult::PublishedVerified)
        {
            result.registrations.insert(
                albumRootId, PrivacyAlbumRootRegistrationStatus::Conflict);
        }
    }

    PrivacyRepositorySnapshot snapshot;
    QHash<int, int> albumRootCounts;

    if (repository.loadSnapshot(&snapshot))
    {
        for (const PrivacyStorageRoot& root : std::as_const(snapshot.storageRoots))
        {
            if (root.kind == PrivacyStorageRootKind::AlbumRoot)
            {
                ++albumRootCounts[root.albumRootId];
            }
        }
    }

    QHash<int, QString> rootUuidsByAlbumRootId;

    for (const PrivacyStorageRoot& root : std::as_const(snapshot.storageRoots))
    {
        if ((root.kind == PrivacyStorageRootKind::AlbumRoot) && root.isValid() &&
            (albumRootCounts.value(root.albumRootId) == 1))
        {
            rootUuidsByAlbumRootId.insert(root.albumRootId, root.uuid);
        }
    }

    PrivacyCoreDbAssetInventoryProvider catalogueProvider(rootUuidsByAlbumRootId);
    PrivacyRuntimeAssetRootProvider rootProvider(runtime);
    result.bridge = PrivacyAssetInventoryBridge::build(
        request, catalogueProvider, rootProvider, control);

    return result;
}

bool PrivacyProtectPreflight::discardNewlyCreatedRoots(
    const PrivacyProtectPreflightResult& preflight,
    const QSharedPointer<PrivacyRuntimeCoordinator>& runtime)
{
    if (!runtime)
    {
        return false;
    }

    PrivacyRepository repository;
    bool discardedEveryRoot = true;

    for (const QString& rootUuid : preflight.newlyCreatedRootUuids)
    {
        bool absent = false;

        if (!repository.removeUnreferencedAlbumRoot(rootUuid, &absent) || !absent)
        {
            discardedEveryRoot = false;
            continue;
        }

        if (!runtime->unregisterUnreferencedAlbumRoot(rootUuid))
        {
            discardedEveryRoot = false;
        }
    }

    return discardedEveryRoot;
}

} // namespace Digikam

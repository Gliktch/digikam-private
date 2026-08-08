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

#include <QHash>
#include <QSet>
#include <QSharedPointer>

// Local includes

#include "digikam_export.h"
#include "privacyassetinventorybridge.h"
#include "privacyrepository.h"

namespace Digikam
{

/** Result of the production boundary that registers only selected collection
 * roots before constructing the otherwise read-only inventory providers. */
class DIGIKAM_DATABASE_EXPORT PrivacyProtectPreflightResult
{
public:

    PrivacyAssetInventoryBridgeResult bridge;
    QHash<int, PrivacyAlbumRootRegistrationStatus> registrations;
    QSet<QString> newlyCreatedRootUuids;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProtectPreflight
{
public:

    static PrivacyProtectPreflightResult build(
        const PrivacyAssetInventoryBridgeRequest& request,
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
        const PrivacyPosixInventoryControl* control = nullptr);
    static bool discardNewlyCreatedRoots(
        const PrivacyProtectPreflightResult& preflight,
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime);
};

} // namespace Digikam

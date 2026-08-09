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

#include <QList>
#include <QSharedPointer>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacycategorysession.h"

namespace Digikam
{

/**
 * Production lifetime owner for one privacy runtime generation and its
 * category-store sessions. Shutdown waits for synchronous operations before
 * locking every category and releasing retained secrets and mount leases.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyCategorySessionOwner
{
public:

    static QSharedPointer<PrivacyCategorySessionOwner> create(
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
        const QSharedPointer<const PrivacyRootVerifier>& rootVerifier);

    ~PrivacyCategorySessionOwner();

    PrivacyCategorySessionResult createCategory(
        const PrivacyCategoryCreateRequest& request,
        const QString& passwordText);
    PrivacyCategorySessionResult unlockCategory(
        const QString& categoryUuid,
        const QString& passwordText);
    PrivacyCategorySessionResult lockCategory(const QString& categoryUuid);
    QList<PrivacyCategorySessionResult> lockAllCategories();

    PrivacyCategoryOperationStatus runWhileUnlocked(
        const QString& categoryUuid,
        const std::function<void()>& operation);
    PrivacyCategoryOperationStatus runWithUnlockedSecret(
        const QString& categoryUuid,
        const std::function<void(const PrivacyPassword&)>& operation);
    PrivacyCategorySessionResult runWithFreshlyAuthenticatedSecret(
        const QString& categoryUuid,
        const QString& passwordText,
        const std::function<void(const PrivacyPassword&)>& operation,
        const QString& allowedActiveItemTransactionUuid = QString());
    PrivacyCategorySessionResult setCategoryTagVisibilityMode(
        const QString& categoryUuid,
        PrivacyTagVisibilityMode mode,
        const QString& passwordText = QString());

    bool ownsSecret(const QString& categoryUuid) const;

    /**
     * Prevents new calls, waits for current calls, then locks all categories.
     * Do not call this from an operation callback on the same owner.
     */
    void shutdown();

private:

    PrivacyCategorySessionOwner(
        const QSharedPointer<PrivacyRuntimeCoordinator>& runtime,
        const QSharedPointer<const PrivacyRootVerifier>& rootVerifier);

private:

    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Digikam

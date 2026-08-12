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

#include <QHash>
#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacyportablediscovery.h"
#include "privacyportableimport.h"
#include "privacytypes.h"

namespace Digikam
{

/** Persistence seam used by the import coordinator. Production delegates to
 * PrivacyRepository/CoreDB; tests inject a fake. */
class DIGIKAM_DATABASE_EXPORT PrivacyPortableImportCommitTarget
{
public:

    PrivacyPortableImportCommitTarget()          = default;
    virtual ~PrivacyPortableImportCommitTarget() = default;

    virtual bool ensureAlbumRoot(
        int albumRootId,
        const QString& configuredPath,
        const QString& collectionIdentifier,
        PrivacyStorageRoot* persisted) = 0;
    virtual bool publish(
        const PrivacyPortableImportPublication& publication) = 0;

private:

    Q_DISABLE_COPY(PrivacyPortableImportCommitTarget)
};

class DIGIKAM_DATABASE_EXPORT PrivacyCoreDbPortableImportCommitTarget final
    : public PrivacyPortableImportCommitTarget
{
public:

    bool ensureAlbumRoot(
        int albumRootId,
        const QString& configuredPath,
        const QString& collectionIdentifier,
        PrivacyStorageRoot* persisted) override;
    bool publish(
        const PrivacyPortableImportPublication& publication) override;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportGroupResult
{
    QString recoverySetUuid;
    PrivacyBackend backend = PrivacyBackend::Casual;
    PrivacyPortableImportAuthenticationStatus status =
        PrivacyPortableImportAuthenticationStatus::InvalidRequest;
    QString detail;
    bool committed = false;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyPortableImportCoordinatorResult
{
    QList<PrivacyPortableImportGroupResult> groups;
    QList<PrivacyPortableDiscoveryIssue> issues;
    bool cancelled = false;
};

/**
 * Runs one portable import pass: filesystem discovery, per-group password
 * authentication (Casual archive/manifest or Strong vault), and atomic
 * per-category publication. Unauthenticated groups are reported as
 * unresolved; each category commits independently.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPortableImportCoordinator
{
public:

    using CancellationCheck = std::function<bool()>;

    explicit PrivacyPortableImportCoordinator(
        PrivacyPortableImportCommitTarget& commitTarget);

    PrivacyPortableImportCoordinatorResult run(
        const QList<QString>& scanRoots,
        const QHash<QString, QString>& passwordsByRecoverySet,
        const QHash<QString, int>& albumRootIdsByPath,
        const QString& defaultCategoryName,
        PrivacyPortableStoreInspector& inspector,
        const CancellationCheck& isCancelled = {});

    /** Publishes one already-authenticated candidate (used by the dialog's
     * Continue Import step). */
    PrivacyPortableImportGroupResult commit(
        const PrivacyPortableImportCandidate& candidate,
        const QHash<QString, int>& albumRootIdsByPath,
        const QString& defaultCategoryName);

private:

    bool buildPublication(
        const PrivacyPortableImportCandidate& candidate,
        const QHash<QString, int>& albumRootIdsByPath,
        const QString& defaultCategoryName,
        PrivacyPortableImportPublication* publication,
        QString* detail);

    PrivacyPortableImportCommitTarget& m_commitTarget;
};

} // namespace Digikam

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
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyManagedRootProvisionStatus
{
    ReadyExisting,
    ReadyCreated,
    InvalidPath,
    PathUnavailable,
    UnsafeRoot,
    InvalidMarker,
    IoFailure,
    UnsupportedPlatform
};

class DIGIKAM_DATABASE_EXPORT PrivacyManagedRootProvisionResult
{
public:

    bool succeeded() const;
    bool createdMarker() const;

public:

    PrivacyManagedRootProvisionStatus status =
        PrivacyManagedRootProvisionStatus::InvalidPath;
    PrivacyStorageRoot root;
    QString detail;

private:

    friend class PrivacyManagedRootProvisioner;

    bool       m_createdMarker = false;
    bool       m_createdMetadataDirectory = false;
    QByteArray m_markerData;
    quint64    m_rootDevice = 0;
    quint64    m_rootInode = 0;
    quint64    m_metadataDevice = 0;
    quint64    m_metadataInode = 0;
};

/**
 * Claims an existing user-owned directory as a neutral application-managed
 * privacy store root. Marker creation and reuse are descriptor-relative and
 * never overwrite an existing marker.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyManagedRootProvisioner
{
public:

    static PrivacyManagedRootProvisionResult provision(const QString& configuredPath);

    /**
     * Removes a marker created by provision() only while the caller still owns
     * the unused result. Call this before supplying the root to any durable
     * category operation. Any changed path, marker or additional private-root
     * content makes rollback fail closed.
     */
    static bool rollbackUnused(const PrivacyManagedRootProvisionResult& result);
};

} // namespace Digikam

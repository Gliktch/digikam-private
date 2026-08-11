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

#include <QString>
#include <QStringList>

// Local includes

#include "digikam_database_export.h"

namespace Digikam
{

struct DIGIKAM_DATABASE_EXPORT PrivacyProfilePreflightResult
{
    bool         success = false;
    int          verifiedRootCount = 0;
    int          verifiedStoreCount = 0;
    int          verifiedContainerCount = 0;
    int          failedRootCount = 0;
    int          failedStoreCount = 0;
    int          failedContainerCount = 0;
    QStringList  warnings;
    QString      error;
};

/**
 * Verifies that a P1 database's referenced privacy storage is present and
 * consistent before that profile is allowed to replace the active profile:
 * managed-root markers, category store directories/configuration and every
 * protected object (archive or vault object) plus its public proxy.
 * This is a read-only existence and size preflight, not an unlock or a
 * cryptographic verification.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyProfilePreflight
{
public:

    static PrivacyProfilePreflightResult verify(const QString& p1DatabasePath);
};

} // namespace Digikam

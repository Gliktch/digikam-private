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

#include <QString>

// Local includes

#include "digikam_database_export.h"

namespace Digikam
{

struct DIGIKAM_DATABASE_EXPORT PrivacySqliteSnapshotResult
{
    bool    success = false;
    bool    canceled = false;
    int     copiedPages = 0;
    int     totalPages = 0;
    QString error;
};

class DIGIKAM_DATABASE_EXPORT PrivacySqliteSnapshot
{
public:

    using Progress = std::function<void(int copiedPages, int totalPages)>;
    using IsCanceled = std::function<bool()>;

    static PrivacySqliteSnapshotResult create(
        const QString& sourcePath,
        const QString& destinationPath,
        const Progress& progress = Progress(),
        const IsCanceled& isCanceled = IsCanceled());
};

} // namespace Digikam

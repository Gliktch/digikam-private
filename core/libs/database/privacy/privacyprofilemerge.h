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
#include <QStringList>

// Local includes

#include "digikam_database_export.h"

namespace Digikam
{

struct DIGIKAM_DATABASE_EXPORT PrivacyProfileMergeResult
{
    bool         success = false;
    bool         canceled = false;
    int          addedRootCount = 0;
    int          addedAlbumCount = 0;
    int          addedItemCount = 0;
    int          skippedExistingItemCount = 0;
    int          addedTagCount = 0;
    int          copiedMetadataRowCount = 0;
    QStringList  warnings;
    QString      error;
};

/**
 * Merges a stock digiKam catalogue (already converted to the P1 schema) into
 * a copy of the active P1 profile. Only items not already represented in the
 * target are inserted; existing target records win. Row IDs are remapped
 * through target-owned tables and unique keys rather than copied verbatim.
 * Saved searches, download history, similarity/face fingerprints and privacy
 * tables are never touched.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyProfileMerge
{
public:

    using Progress = std::function<void(const QString& step, int current, int total)>;
    using IsCanceled = std::function<bool()>;

    static PrivacyProfileMergeResult mergeStockIntoP1(
        const QString& convertedSourceP1Path,
        const QString& candidateP1Path,
        const Progress& progress = Progress(),
        const IsCanceled& isCanceled = IsCanceled());
};

} // namespace Digikam

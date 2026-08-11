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
#include "privacyprofileinspector.h"

namespace Digikam
{

struct DIGIKAM_DATABASE_EXPORT PrivacyProfileImportStageResult
{
    bool                  success = false;
    bool                  canceled = false;
    QString               stageDirectory;
    QString               candidateCoreDatabasePath;
    QString               candidateThumbnailDatabasePath;
    bool                  thumbnailDatabaseIncluded = false;
    PrivacyProfileSummary candidateSummary;
    QStringList           warnings;
    QString               error;
    int                   addedItemCount = 0;
    int                   skippedExistingItemCount = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProfileImportStager
{
public:

    using Progress = std::function<void(const QString& step, int current, int total)>;
    using IsCanceled = std::function<bool()>;

    static PrivacyProfileImportStageResult stage(
        const PrivacyProfileSummary& source,
        const QString& stageDirectory,
        const Progress& progress = Progress(),
        const IsCanceled& isCanceled = IsCanceled(),
        const QString& additiveTargetP1Path = QString());
};

} // namespace Digikam

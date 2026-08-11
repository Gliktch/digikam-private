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

#include <QDateTime>
#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacyprofileimportstager.h"

namespace Digikam
{

struct DIGIKAM_DATABASE_EXPORT PrivacyProfilePaths
{
    QString configHome;
    QString dataHome;
    QString cacheHome;
    QString stateHome;
    QString transactionHome;
    QString configFilePath;
    QString coreDatabasePath;
    QString thumbnailDatabasePath;

    bool isValid() const;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyProfilePublicationResult
{
    bool    success = false;
    QString transactionUuid;
    QString transactionDirectory;
    QString backupDirectory;
    QString error;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyProfileBackup
{
    QString               transactionUuid;
    QString               transactionDirectory;
    QString               backupDirectory;
    QString               configFilePath;
    QString               coreDatabasePath;
    QString               thumbnailDatabasePath;
    QDateTime             createdAt;
    PrivacyProfileSummary summary;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProfilePublication
{
public:

    using Progress = std::function<void(const QString& step, int current, int total)>;

    static PrivacyProfilePublicationResult prepare(
        const PrivacyProfileImportStageResult& staged,
        const PrivacyProfilePaths& target,
        const QString& sourceSettingsPath = QString(),
        const Progress& progress = Progress());

    static PrivacyProfilePublicationResult applyPending(const QString& transactionHome);
    static PrivacyProfilePublicationResult applyPendingFromEnvironment();
    static QList<PrivacyProfileBackup> restorableBackups(const QString& transactionHome);
    static PrivacyProfilePublicationResult prepareRestore(
        const PrivacyProfileBackup& backup,
        const PrivacyProfilePaths& target,
        const Progress& progress = Progress());
};

} // namespace Digikam

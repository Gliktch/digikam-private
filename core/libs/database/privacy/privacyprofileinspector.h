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

#include <QDateTime>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_database_export.h"

namespace Digikam
{

enum class PrivacyProfileSchemaKind
{
    Invalid,
    Stock,
    PrivateP1,
    Unsupported
};

struct DIGIKAM_DATABASE_EXPORT PrivacyProfileSummary
{
    PrivacyProfileSchemaKind schemaKind = PrivacyProfileSchemaKind::Invalid;
    QString                  settingsPath;
    QString                  databasePath;
    QString                  thumbnailDatabasePath;
    QString                  databaseType;
    int                      schemaVersion = -1;
    QString                  versionLabel;
    qlonglong                activeItemCount = 0;
    qlonglong                totalItemCount = 0;
    qlonglong                protectedItemCount = 0;
    int                      privacyCategoryCount = 0;
    int                      incompletePrivacyTransactionCount = 0;
    QStringList              collectionRoots;
    qint64                   databaseBytes = 0;
    QDateTime                latestModification;
    bool                     integrityOk = false;
    QString                  error;

    bool isUsable() const;
    bool isPrivateProfile() const;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProfileInspector
{
public:

    static QString defaultStockSettingsPath();
    static PrivacyProfileSummary inspectSettingsFile(const QString& settingsPath);
    static PrivacyProfileSummary inspectDatabaseFolder(const QString& databaseFolder);
    static PrivacyProfileSummary inspectCoreDatabase(const QString& databasePath,
                                                      const QString& settingsPath = QString(),
                                                      const QString& thumbnailDatabasePath = QString());
    static QString versionLabel(int schemaVersion);
};

} // namespace Digikam

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
#include <QDateTime>
#include <QFileDevice>
#include <QList>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_database_export.h"

namespace Digikam
{

struct DIGIKAM_DATABASE_EXPORT PrivacyStrongObjectMember
{
    QString    sourcePath;
    QString    protectedRelativePath;
    QString    originalName;
    QByteArray expectedSha256;
    qlonglong  expectedSize = -1;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyStrongObjectStageResult
{
    bool        valid = false;
    qlonglong   totalSize = 0;
    QByteArray  totalSha256;
    QStringList stagedRelativePaths;
};

/**
 * Descriptor-confined Strong vault object backend. Objects are staged inside
 * the mounted store's plaintext namespace under a transaction-specific
 * directory and atomically published to their final `originals/...` location.
 * Every protected path is validated against traversal and symlink escape;
 * published objects are hash-verified before and after publication, and
 * removal happens only through an explicit confined delete.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyStrongObjectBackend
{
public:

    static PrivacyStrongObjectStageResult stageObjects(
        const QString& vaultPlaintextRoot,
        const QString& stagedRelativeDirectory,
        const QList<PrivacyStrongObjectMember>& members,
        QString* error = nullptr);

    static bool publishObjects(
        const QString& vaultPlaintextRoot,
        const QString& stagedRelativeDirectory,
        const QString& finalRelativeDirectory,
        const QList<PrivacyStrongObjectMember>& members,
        qlonglong expectedTotalSize,
        const QByteArray& expectedTotalSha256,
        QString* error = nullptr);

    static bool verifyObjects(
        const QString& vaultPlaintextRoot,
        const QString& finalRelativeDirectory,
        const QList<PrivacyStrongObjectMember>& members,
        qlonglong expectedTotalSize,
        const QByteArray& expectedTotalSha256,
        QString* error = nullptr);

    static bool restoreObject(
        const QString& vaultPlaintextRoot,
        const QString& protectedRelativePath,
        const QString& targetAbsolutePath,
        QFileDevice::Permissions permissions,
        const QDateTime& modificationDate,
        QString* error = nullptr);

    static bool removeObjects(
        const QString& vaultPlaintextRoot,
        const QString& protectedRelativeDirectory,
        QString* error = nullptr);
};

} // namespace Digikam

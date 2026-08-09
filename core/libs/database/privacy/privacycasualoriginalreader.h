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

// Local includes

#include "digikam_export.h"
#include "privacycasualarchive.h"
#include "privacytypes.h"

class QIODevice;

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT PrivacyCasualOriginalSource
{
public:

    bool isValid() const;

public:

    qlonglong                           imageId = -1;
    qlonglong                           itemGeneration = -1;
    QString                             logicalFilePath;
    QString                             categoryUuid;
    QString                             itemUuid;
    QString                             originalName;
    QString                             originalHash;
    qlonglong                           originalSize = -1;
    PrivacyCasualArchiveRestoreRequest  restore;
};

/**
 * Converts one internally consistent runtime snapshot into the exact archive
 * member facts needed by ordinary unlocked readers. The caller still owns
 * authorization, category-session lifetime, destination lifetime and relock
 * draining; this class owns no password or plaintext storage.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyCasualOriginalReader
{
public:

    bool prepare(const PrivacyRepositorySnapshot& snapshot,
                 qlonglong imageId,
                 const QString& logicalFilePath,
                 PrivacyCasualOriginalSource* source) const;
    bool prepareAsset(const PrivacyRepositorySnapshot& snapshot,
                      qlonglong imageId,
                      const QString& logicalFilePath,
                      int role, int ordinal,
                      PrivacyCasualOriginalSource* source) const;

    bool restore(const PrivacyCasualOriginalSource& source,
                 const PrivacyPassword& password,
                 QIODevice* destination,
                 PrivacyCasualArchiveError* error = nullptr,
                 const PrivacyCasualArchiveEngine::CancellationCheck& isCancelled = {}) const;
};

} // namespace Digikam

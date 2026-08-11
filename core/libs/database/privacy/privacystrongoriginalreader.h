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

#include "digikam_export.h"
#include "privacytypes.h"

class QIODevice;

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT PrivacyStrongOriginalSource
{
public:

    bool isValid() const;

public:

    qlonglong  imageId = -1;
    qlonglong  itemGeneration = -1;
    QString    logicalFilePath;
    QString    categoryUuid;
    QString    itemUuid;
    QString    originalName;
    QString    originalHash;
    qlonglong  originalSize = -1;
    QString    storeUuid;
    QString    containerObjectRelativePath;
    QString    protectedRelativePath;
    QString    vaultPlaintextRoot;
};

/**
 * Converts one internally consistent runtime snapshot into the exact Strong
 * vault-object facts needed by ordinary unlocked readers. The caller still
 * owns authorization, category-session lifetime, destination lifetime and
 * relock draining; this class owns no password or plaintext storage. The
 * mounted vault plaintext root is supplied by the caller from the
 * authenticated category session.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyStrongOriginalReader
{
public:

    bool prepare(const PrivacyRepositorySnapshot& snapshot,
                 qlonglong imageId,
                 const QString& logicalFilePath,
                 PrivacyStrongOriginalSource* source) const;
    bool prepareAsset(const PrivacyRepositorySnapshot& snapshot,
                      qlonglong imageId,
                      const QString& logicalFilePath,
                      int role, int ordinal,
                      PrivacyStrongOriginalSource* source) const;

    bool restore(const PrivacyStrongOriginalSource& source,
                 QIODevice* destination,
                 QString* error = nullptr,
                 const std::function<bool()>& isCancelled = {}) const;
};

} // namespace Digikam

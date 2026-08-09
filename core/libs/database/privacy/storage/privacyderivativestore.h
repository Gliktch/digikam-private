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

enum class PrivacyDerivativeStoreError
{
    None                = 0,
    InvalidRequest      = 1,
    UnsupportedPlatform = 2,
    UnsafeStore         = 3,
    Conflict            = 4,
    IoFailure           = 5,
    IntegrityFailure    = 6
};

/** Descriptor-confined access to one already mounted category derivative store. */
class DIGIKAM_DATABASE_EXPORT PrivacyDerivativeStore
{
public:

    static QString clearThumbnailRelativePath(
        const QString& itemUuid, const QString& sourceOriginalSha256,
        int presentationVersion);

    bool put(const QString& plaintextRoot,
             const PrivacyDerivative& derivative,
             const QByteArray& encodedBytes,
             PrivacyDerivativeStoreError* error = nullptr,
             QString* detail = nullptr) const;
    QByteArray read(const QString& plaintextRoot,
                    const PrivacyDerivative& derivative,
                    PrivacyDerivativeStoreError* error = nullptr,
                    QString* detail = nullptr) const;
};

} // namespace Digikam

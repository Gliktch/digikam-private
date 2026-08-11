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
#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyPublicRecoveryLocatorError
{
    None = 1,
    Invalid,
    UnsafePath,
    IoFailure
};

/**
 * One non-secret pre-scan quarantine hint below a public collection root.
 * It maps an opaque recovery identity to one relative public proxy path and
 * the expected placeholder facts, so portable import can group candidates
 * and positively identify proxies without attempting passwords. It never
 * contains category names, vault object names or hidden original names; the
 * encrypted manifest remains authoritative after authentication.
 */
struct DIGIKAM_DATABASE_EXPORT PrivacyPublicRecoveryLocatorEntry
{
    bool isValid() const;

    QString       recoverySetUuid;
    PrivacyBackend backend = PrivacyBackend::Casual;
    QString       publicRelativePath;
    QString       placeholderIdentity;
    qlonglong     expectedPlaceholderSize = -1;
    QByteArray    expectedPlaceholderSha256;
};

class DIGIKAM_DATABASE_EXPORT PrivacyPublicRecoveryLocatorCodec
{
public:

    /** `.digikam-private/recovery-locator-v1.json` below a collection root. */
    static QString relativePath();

    static QByteArray encode(
        const QList<PrivacyPublicRecoveryLocatorEntry>& entries,
        PrivacyPublicRecoveryLocatorError* error = nullptr);
    static bool decode(
        const QByteArray& bytes,
        QList<PrivacyPublicRecoveryLocatorEntry>* entries,
        PrivacyPublicRecoveryLocatorError* error = nullptr);
};

} // namespace Digikam

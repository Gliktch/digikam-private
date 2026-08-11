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
#include <QList>
#include <QString>

// Local includes

#include "digikam_database_export.h"

namespace Digikam
{

enum class PrivacyStrongRecoveryManifestError
{
    None = 1,
    Unavailable,
    Invalid,
    UnsafePath,
    IoFailure
};

/**
 * One exact original/associated member inside a Strong vault object.
 * vaultRelativePath is the mounted-store plaintext path (for example
 * `originals/<containerUuid>/0-photo.jpg`).
 */
struct DIGIKAM_DATABASE_EXPORT PrivacyStrongRecoveryMember
{
    QString   vaultRelativePath;
    QString   publicRelativePath;
    QString   originalName;
    int       role = 0;
    int       ordinal = -1;
    QString   hashAlgorithm;
    QString   sha256Hex;
    qlonglong size = -1;
    QDateTime creationTimeUtc;
    QDateTime modificationTimeUtc;
    QByteArray portableAttributes;
    quint32   unixMode = 0;

    bool isValid() const;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyStrongRecoveryItem
{
    QString   itemUuid;
    QString   containerUuid;
    qlonglong generation = -1;
    QList<PrivacyStrongRecoveryMember> members;

    bool isValid() const;
};

/**
 * Versioned encrypted category/item recovery manifest stored inside the
 * mounted Strong store at `digikam-private/recovery-v1.json`. The vault is
 * encrypted, so the manifest is never visible without the category password.
 * It is the authoritative portable mapping for Phase 4 filesystem import.
 */
struct DIGIKAM_DATABASE_EXPORT PrivacyStrongRecoveryManifest
{
    QString   format = QLatin1String("digikam-private-strong");
    int       formatVersion = 1;
    QString   passwordEncoding = QLatin1String("utf8-nfc-v1");
    QString   categoryUuid;
    QString   categoryName;
    int       presentationMode = 0;
    int       unlockedThumbnailMode = 0;
    int       tagVisibilityMode = 0;
    qlonglong currentCredentialGeneration = -1;
    QString   storeUuid;
    QList<PrivacyStrongRecoveryItem> items;

    bool isValid() const;
};

class DIGIKAM_DATABASE_EXPORT PrivacyStrongRecoveryManifestCodec
{
public:

    static QString relativePath();
    static QByteArray encode(
        const PrivacyStrongRecoveryManifest& manifest,
        PrivacyStrongRecoveryManifestError* error = nullptr);
    static bool decode(
        const QByteArray& bytes,
        PrivacyStrongRecoveryManifest* manifest,
        PrivacyStrongRecoveryManifestError* error = nullptr);
};

/**
 * Loads and atomically commits the vault manifest under the mounted Strong
 * store. load() treats a missing manifest as Unavailable so callers can
 * initialize one; commit() writes a same-directory temporary, fsyncs it,
 * renames it into place and fsyncs the containing directory.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyStrongRecoveryManifestStore
{
public:

    static bool load(
        const QString& vaultPlaintextRoot,
        PrivacyStrongRecoveryManifest* manifest,
        PrivacyStrongRecoveryManifestError* error = nullptr);
    static bool commit(
        const QString& vaultPlaintextRoot,
        const PrivacyStrongRecoveryManifest& manifest,
        PrivacyStrongRecoveryManifestError* error = nullptr);
};

} // namespace Digikam

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
#include <memory>

// Qt includes

#include <QByteArray>
#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyCheckoutStoreError
{
    None                = 0,
    InvalidRequest      = 1,
    UnsupportedPlatform = 2,
    UnsafeStore         = 3,
    Missing             = 4,
    Conflict            = 5,
    IoFailure           = 6,
    IntegrityFailure    = 7
};

enum class PrivacyCheckoutStoreLocation
{
    Checkout = 1,
    Recovery = 2
};

enum class PrivacyCheckoutEntryKind
{
    RegularFile = 1,
    Directory   = 2,
    SymbolicLink = 3
};

/** Stable, persistable content evidence for one store-relative checkout node. */
struct DIGIKAM_DATABASE_EXPORT PrivacyCheckoutInventoryEntry
{
    QString storeRelativePath;
    PrivacyCheckoutEntryKind kind = PrivacyCheckoutEntryKind::RegularFile;
    qint64 size = 0;
    quint64 linkCount = 0;
    QByteArray sha256;

    bool operator==(const PrivacyCheckoutInventoryEntry& other) const;
};

/** Exact recursive inventory below one transaction's work directory. */
struct DIGIKAM_DATABASE_EXPORT PrivacyCheckoutInventory
{
    QString transactionUuid;
    PrivacyCheckoutStoreLocation location =
        PrivacyCheckoutStoreLocation::Checkout;
    QString workRelativePath;
    QList<PrivacyCheckoutInventoryEntry> entries;
    QByteArray sha256;

    bool operator==(const PrivacyCheckoutInventory& other) const;
};

/**
 * Descriptor-confined access to one already authenticated category-store mount.
 *
 * The runtime mount path is accepted only while opening this object and is
 * never returned. Public identities are always relative to the mounted store.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyCheckoutStore
{
public:

    using FileProducer = std::function<bool(int, QString*)>;

    ~PrivacyCheckoutStore();

    PrivacyCheckoutStore(PrivacyCheckoutStore&& other) noexcept;
    PrivacyCheckoutStore& operator=(PrivacyCheckoutStore&& other) noexcept;

    PrivacyCheckoutStore(const PrivacyCheckoutStore&)            = delete;
    PrivacyCheckoutStore& operator=(const PrivacyCheckoutStore&) = delete;

    static std::unique_ptr<PrivacyCheckoutStore> open(
        const QString& plaintextRoot,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr);

    static QString transactionRelativePath(
        const QString& transactionUuid,
        PrivacyCheckoutStoreLocation location =
            PrivacyCheckoutStoreLocation::Checkout);
    static QString workRelativePath(
        const QString& transactionUuid,
        PrivacyCheckoutStoreLocation location =
            PrivacyCheckoutStoreLocation::Checkout);
    static QString workFileRelativePath(
        const QString& transactionUuid,
        const QString& fileName,
        PrivacyCheckoutStoreLocation location =
            PrivacyCheckoutStoreLocation::Checkout);

    /** Creates or safely reopens checkouts/<uuid>/work. */
    bool createOrOpenTransaction(
        const QString& transactionUuid,
        QString* relativeWorkPath = nullptr,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr);

    /** Reopens an existing checkout or preserved-recovery transaction. */
    bool reopenTransaction(
        const QString& transactionUuid,
        PrivacyCheckoutStoreLocation location,
        QString* relativeWorkPath = nullptr,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr) const;

    /**
     * Exclusively materializes one 0600 file. Existing exact bytes are reused;
     * an existing mismatch is preserved and reported as a conflict.
     */
    bool createFile(
        const QString& transactionUuid,
        const QString& fileName,
        qint64 expectedSize,
        const QByteArray& expectedSha256,
        const FileProducer& producer,
        QString* storeRelativePath = nullptr,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr);

    /** Recursively inventories regular files, directories and untraversed links. */
    bool inventory(
        const QString& transactionUuid,
        PrivacyCheckoutStoreLocation location,
        PrivacyCheckoutInventory* result,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr) const;

    bool validateInventory(
        const PrivacyCheckoutInventory& expected,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr) const;

    /**
     * Resolves one inventory-owned regular file for immediate process launch.
     * The absolute result is valid only while the authenticated store lease and
     * this object remain live. It is runtime-only data and must never be
     * persisted in a transaction, journal or catalogue row.
     */
    QString runtimePathForEntry(
        const PrivacyCheckoutInventory& inventory,
        const QString& storeRelativePath,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr) const;

    /** Deletes only the exact inventory represented by expected. */
    bool removeExact(
        const PrivacyCheckoutInventory& expected,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr);

    /** Atomically preserves checkouts/<uuid> as recovery/<uuid>. */
    bool moveToRecovery(
        const QString& transactionUuid,
        QString* recoveryRelativePath = nullptr,
        PrivacyCheckoutStoreError* error = nullptr,
        QString* detail = nullptr);

private:

    PrivacyCheckoutStore();

    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Digikam

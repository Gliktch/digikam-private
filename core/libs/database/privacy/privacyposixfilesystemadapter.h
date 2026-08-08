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

#include <QList>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

// Local includes

#include "digikam_export.h"
#include "privacyassetinventory.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT PrivacyPosixRootScope
{
public:

    bool isValid() const;

public:

    PrivacyInventoryRoot root;
    quint64              expectedDeviceId = 0;
    quint64              expectedInode = 0;

    /**
     * False keeps the root configured for direct inspect/list requests but
     * forces hardlink enumeration on its device to report incomplete.
     */
    bool                 includeInHardlinkEnumeration = true;
};

class DIGIKAM_DATABASE_EXPORT PrivacyPosixScanLimits
{
public:

    bool isValid() const;

public:

    qsizetype maximumEntriesPerDirectory = 100000;
    qsizetype maximumEntriesTotal = 1000000;
    qsizetype maximumDirectoriesPerRoot = 100000;
    int       maximumDepth = 128;
};

enum class PrivacyPosixCheckpoint
{
    BeforeRootOpen       = 1,
    AfterRootOpen        = 2,
    BeforeDirectoryRead  = 3,
    AfterDirectoryRead   = 4,
    BeforeRootRevalidate = 5
};

/**
 * Injectable cancellation and deterministic fault-checkpoint seam. The
 * adapter owns neither this object nor work scheduling; it must outlive the
 * adapter and its calls.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPosixInventoryControl
{
public:

    PrivacyPosixInventoryControl()          = default;
    virtual ~PrivacyPosixInventoryControl() = default;

    virtual bool isCanceled() const = 0;
    virtual void checkpoint(PrivacyPosixCheckpoint checkpoint,
                            const PrivacyInventoryRoot& root,
                            const QString& relativePath) const = 0;

private:

    Q_DISABLE_COPY(PrivacyPosixInventoryControl)
};

/**
 * Linux/POSIX read-only evidence adapter for PrivacyAssetInventory.
 *
 * Every configured root is pinned to a previously verified directory
 * device/inode. Paths are walked one component at a time beneath an opened
 * root descriptor, preferring openat2 constraints on Linux and using a safe
 * single-component openat/fstatat fallback. Symlinks are never followed.
 * Unsupported non-Linux builds retain this API as a fail-closed adapter whose
 * configuration is never valid and whose enumeration is always incomplete.
 *
 * A complete hardlink result covers all enabled configured roots whose pinned
 * root device equals the requested device. Nested filesystems are outside a
 * root scope and must be configured as separate roots when they matter.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPosixFilesystemAdapter final
    : public PrivacyAssetFilesystemProvider
{
public:

    explicit PrivacyPosixFilesystemAdapter(
        const QList<PrivacyPosixRootScope>& roots,
        const PrivacyPosixScanLimits& limits = PrivacyPosixScanLimits(),
        const PrivacyPosixInventoryControl* control = nullptr);
    ~PrivacyPosixFilesystemAdapter() override;

    bool isConfigurationValid() const;

    PrivacyInventoryFileEvidence inspect(
        const PrivacyInventoryLocation& location) const override;
    PrivacyInventoryDirectoryEvidence listDirectory(
        const PrivacyInventoryRoot& root,
        const QString& relativeDirectory) const override;
    PrivacyInventoryAliasEvidence hardlinkAliases(
        quint64 deviceId,
        quint64 inode) const override;
    QList<PrivacyInventoryHardlinkEvidence> hardlinkAliasesFor(
        const QList<PrivacyInventoryFileIdentity>& identities) const override;

private:

    class Private;
    const QScopedPointer<Private> d;

    Q_DISABLE_COPY(PrivacyPosixFilesystemAdapter)
};

} // namespace Digikam

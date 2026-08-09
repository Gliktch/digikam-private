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

#include <memory>

// Qt includes

#include <QByteArray>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "privacypassword.h"
#include "privacyprocessrunner.h"

namespace Digikam
{

enum class PrivacyGocryptfsError
{
    None,
    InvalidPassword,
    InvalidEnvelope,
    InvalidSentinel,
    InvalidToolPath,
    UnsupportedToolVersion,
    UnsafeWorkspace,
    WorkspaceNotEmpty,
    FileOperationFailed,
    ProcessFailed,
    InvalidMasterKeyOutput,
    MountNotReady,
    SentinelMismatch,
    UnmountFailed
};

class DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsEnvelope
{
public:

    static PrivacyGocryptfsEnvelope fromOpaqueConfig(const QString& format,
                                                     const QByteArray& opaqueConfig,
                                                     PrivacyGocryptfsError* error = nullptr);

    bool isValid() const;
    QString format() const;
    qsizetype size() const;
    QByteArray opaqueConfig() const;

private:

    friend class PrivacyGocryptfsStoreHarness;

    QString    m_format;
    QByteArray m_opaqueConfig;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsToolPaths
{
    QString gocryptfs;
    QString gocryptfsXray;
    QString fusermount;
};

struct DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsStoreLayout
{
    QString workspaceRoot;
    QString cipherDirectory;
    QString mountDirectory;
    QString runtimeDirectory;
};

class DIGIKAM_DATABASE_EXPORT PrivacyMountStateProbe
{
public:

    enum class State
    {
        Mounted,
        NotMounted,
        Unknown
    };

    virtual ~PrivacyMountStateProbe() = default;
    virtual State state(const QString& mountPoint) const = 0;
};

class DIGIKAM_DATABASE_EXPORT ProcMountInfoPrivacyMountStateProbe : public PrivacyMountStateProbe
{
public:

    State state(const QString& mountPoint) const override;
};

class DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsMountLease
{
public:

    ~PrivacyGocryptfsMountLease();

    PrivacyGocryptfsMountLease(const PrivacyGocryptfsMountLease&)            = delete;
    PrivacyGocryptfsMountLease& operator=(const PrivacyGocryptfsMountLease&) = delete;

    bool isActive();
    QString mountPoint() const;

private:

    friend class PrivacyGocryptfsStoreHarness;

    PrivacyGocryptfsMountLease(QString mountPoint,
                               const PrivacyMountStateProbe* mountProbe,
                               std::unique_ptr<PrivacyProcessHandle>&& process);

private:

    QString                               m_mountPoint;
    const PrivacyMountStateProbe*         m_mountProbe = nullptr;
    std::unique_ptr<PrivacyProcessHandle> m_process;
};

/**
 * A narrow category-store process harness. All paths are derived below one
 * caller-supplied, owned mode-0700 workspace. Tests use QTemporaryDir. The
 * process runner and mount-state probe are injected so unit tests never mount.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyGocryptfsStoreHarness
{
public:

    PrivacyGocryptfsStoreHarness(PrivacyProcessRunner& runner,
                                 const PrivacyMountStateProbe& mountProbe,
                                 PrivacyGocryptfsToolPaths toolPaths,
                                 QString workspaceRoot);
    PrivacyGocryptfsStoreHarness(PrivacyProcessRunner& runner,
                                 const PrivacyMountStateProbe& mountProbe,
                                 PrivacyGocryptfsToolPaths toolPaths,
                                 PrivacyGocryptfsStoreLayout layout);

    bool checkCapabilities(PrivacyGocryptfsError* error = nullptr);
    bool prepareWorkspace(PrivacyGocryptfsError* error = nullptr);

    bool createStore(const PrivacyPassword& password,
                     const QByteArray& expectedSentinel,
                     PrivacyGocryptfsEnvelope* envelope,
                     PrivacyGocryptfsError* error = nullptr);

    bool validateEnvelope(const PrivacyGocryptfsEnvelope& envelope,
                          const PrivacyPassword& password,
                          PrivacyGocryptfsError* error = nullptr);

    std::unique_ptr<PrivacyGocryptfsMountLease> mountStore(
        const PrivacyPassword& password,
        const QByteArray& expectedSentinel,
        PrivacyGocryptfsError* error = nullptr);

    bool unmountStore(PrivacyGocryptfsMountLease& lease,
                      PrivacyGocryptfsError* error = nullptr);

    QString cipherDirectory() const;
    QString mountDirectory() const;
    QString runtimeDirectory() const;
    QString configPath() const;
    QString sentinelPath() const;

private:

    PrivacyProcessSpec processSpec(const QString& program,
                                   const QStringList& arguments,
                                   bool sensitiveOutput = false) const;
    bool toolsAreSafe() const;
    bool workspaceIsSafe() const;
    bool directoryCanBeSecured(const QString& path) const;
    bool directoryIsSafe(const QString& path) const;
    bool configCanBeSecured() const;
    bool configIsSafe() const;
    bool runWithPassword(const PrivacyProcessSpec& spec,
                         const PrivacyPassword& password,
                         PrivacyProcessResult* result) const;
    std::unique_ptr<PrivacyProcessHandle> startMount(
        const PrivacyPassword& password,
        PrivacyGocryptfsError* error) const;
    bool waitForMount(PrivacyProcessHandle& process,
                      PrivacyGocryptfsError* error) const;
    bool createSentinel(const QByteArray& expectedSentinel) const;
    bool syncConfigAndCipherDirectory() const;
    bool validMasterKeyOutput(const QByteArray& output) const;
    bool sentinelMatches(const QByteArray& expectedSentinel) const;
    void cleanUpFailedMount(std::unique_ptr<PrivacyProcessHandle>& process) const;
    void setError(PrivacyGocryptfsError* error, PrivacyGocryptfsError value) const;

private:

    PrivacyProcessRunner&          m_runner;
    const PrivacyMountStateProbe&  m_mountProbe;
    PrivacyGocryptfsToolPaths      m_toolPaths;
    QString                        m_workspaceRoot;
    QString                        m_cipherDirectory;
    QString                        m_mountDirectory;
    QString                        m_runtimeDirectory;
    bool                           m_capabilitiesVerified = false;
};

} // namespace Digikam

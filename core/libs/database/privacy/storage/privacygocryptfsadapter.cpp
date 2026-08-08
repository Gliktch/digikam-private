/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacygocryptfsadapter.h"

// C++ includes

#include <utility>

// Qt includes

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QThread>

#ifdef Q_OS_UNIX
#   include <fcntl.h>
#   include <unistd.h>
#endif

namespace Digikam
{

namespace
{

constexpr qsizetype MaximumEnvelopeBytes = 1024 * 1024;
constexpr qsizetype MaximumSentinelBytes = 4096;
constexpr int MountReadyTimeoutMs         = 3000;
constexpr int MountPollIntervalMs         = 20;

const QString EnvelopeFormat = QStringLiteral("gocryptfs-config-v2");
const QString SentinelName   = QStringLiteral(".digikam-private-store-v1");

QString decodeMountInfoPath(QByteArray path)
{
    path.replace("\\040", " ");
    path.replace("\\011", "\t");
    path.replace("\\012", "\n");
    path.replace("\\134", "\\");

    return QString::fromUtf8(path);
}

bool versionMatches(const QByteArray& output, const QString& toolName)
{
    const QString text = QString::fromUtf8(output);
    const QRegularExpression versionExpression(
        QLatin1String("(^|[^0-9])2\\.6\\.1([^0-9]|$)"));

    return (text.contains(toolName, Qt::CaseInsensitive) &&
            versionExpression.match(text).hasMatch());
}

bool syncFile(QFile& file)
{
    if (!file.flush())
    {
        return false;
    }

#ifdef Q_OS_UNIX
    return (::fsync(file.handle()) == 0);
#else
    return true;
#endif
}

bool syncDirectory(const QString& path)
{
#ifdef Q_OS_UNIX
    const QByteArray encodedPath = QFile::encodeName(path);
    const int descriptor = ::open(encodedPath.constData(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool synced = (::fsync(descriptor) == 0);
    ::close(descriptor);

    return synced;
#else
    Q_UNUSED(path);

    return true;
#endif
}

} // namespace

PrivacyGocryptfsEnvelope PrivacyGocryptfsEnvelope::fromOpaqueConfig(
    const QString& format, const QByteArray& opaqueConfig,
    PrivacyGocryptfsError* const error)
{
    if (error)
    {
        *error = PrivacyGocryptfsError::None;
    }

    PrivacyGocryptfsEnvelope envelope;

    if ((format != EnvelopeFormat) || opaqueConfig.isEmpty() ||
        (opaqueConfig.size() > MaximumEnvelopeBytes))
    {
        if (error)
        {
            *error = PrivacyGocryptfsError::InvalidEnvelope;
        }

        return envelope;
    }

    envelope.m_format       = format;
    envelope.m_opaqueConfig = opaqueConfig;

    return envelope;
}

bool PrivacyGocryptfsEnvelope::isValid() const
{
    return ((m_format == EnvelopeFormat) && !m_opaqueConfig.isEmpty() &&
            (m_opaqueConfig.size() <= MaximumEnvelopeBytes));
}

QString PrivacyGocryptfsEnvelope::format() const
{
    return m_format;
}

qsizetype PrivacyGocryptfsEnvelope::size() const
{
    return m_opaqueConfig.size();
}

QByteArray PrivacyGocryptfsEnvelope::opaqueConfig() const
{
    return m_opaqueConfig;
}

PrivacyMountStateProbe::State ProcMountInfoPrivacyMountStateProbe::state(
    const QString& mountPoint) const
{
    QFile mountInfo(QLatin1String("/proc/self/mountinfo"));

    if (!mountInfo.open(QIODevice::ReadOnly))
    {
        return State::Unknown;
    }

    const QString expected = QDir::cleanPath(mountPoint);

    while (!mountInfo.atEnd())
    {
        const QList<QByteArray> fields = mountInfo.readLine().split(' ');

        if ((fields.size() > 4) &&
            (QDir::cleanPath(decodeMountInfoPath(fields.at(4))) == expected))
        {
            return State::Mounted;
        }
    }

    return (mountInfo.error() == QFileDevice::NoError) ? State::NotMounted
                                                       : State::Unknown;
}

PrivacyGocryptfsMountLease::PrivacyGocryptfsMountLease(
    QString mountPoint, std::unique_ptr<PrivacyProcessHandle>&& process)
    : m_mountPoint(std::move(mountPoint)),
      m_process(std::move(process))
{
}

PrivacyGocryptfsMountLease::~PrivacyGocryptfsMountLease() = default;

bool PrivacyGocryptfsMountLease::isActive()
{
    return (m_process && m_process->isRunning());
}

QString PrivacyGocryptfsMountLease::mountPoint() const
{
    return m_mountPoint;
}

PrivacyGocryptfsStoreHarness::PrivacyGocryptfsStoreHarness(
    PrivacyProcessRunner& runner, const PrivacyMountStateProbe& mountProbe,
    PrivacyGocryptfsToolPaths toolPaths, QString workspaceRoot)
    : PrivacyGocryptfsStoreHarness(
          runner, mountProbe, std::move(toolPaths),
          { QDir::cleanPath(workspaceRoot),
            QDir::cleanPath(workspaceRoot) + QLatin1String("/cipher"),
            QDir::cleanPath(workspaceRoot) + QLatin1String("/mount"),
            QDir::cleanPath(workspaceRoot) + QLatin1String("/runtime") })
{
}

PrivacyGocryptfsStoreHarness::PrivacyGocryptfsStoreHarness(
    PrivacyProcessRunner& runner, const PrivacyMountStateProbe& mountProbe,
    PrivacyGocryptfsToolPaths toolPaths, PrivacyGocryptfsStoreLayout layout)
    : m_runner(runner),
      m_mountProbe(mountProbe),
      m_toolPaths(std::move(toolPaths)),
      m_workspaceRoot(QDir::cleanPath(std::move(layout.workspaceRoot))),
      m_cipherDirectory(QDir::cleanPath(std::move(layout.cipherDirectory))),
      m_mountDirectory(QDir::cleanPath(std::move(layout.mountDirectory))),
      m_runtimeDirectory(QDir::cleanPath(std::move(layout.runtimeDirectory)))
{
}

bool PrivacyGocryptfsStoreHarness::checkCapabilities(PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);
    m_capabilitiesVerified = false;

    if (!toolsAreSafe())
    {
        setError(error, PrivacyGocryptfsError::InvalidToolPath);

        return false;
    }

    PrivacyProcessResult gocryptfsResult =
        m_runner.run(processSpec(m_toolPaths.gocryptfs,
                                 { QLatin1String("-version") }), {});
    PrivacyProcessResult xrayResult =
        m_runner.run(processSpec(m_toolPaths.gocryptfsXray,
                                 { QLatin1String("-version") }), {});
    PrivacyProcessResult fusermountResult =
        m_runner.run(processSpec(m_toolPaths.fusermount,
                                 { QLatin1String("--version") }), {});
    const QByteArray fusermountVersionOutput =
        fusermountResult.standardOutput + fusermountResult.standardError;

    if (!gocryptfsResult.succeeded() || !xrayResult.succeeded() ||
        !fusermountResult.succeeded() ||
        !versionMatches(gocryptfsResult.standardOutput + gocryptfsResult.standardError,
                        QLatin1String("gocryptfs")) ||
        !versionMatches(xrayResult.standardOutput + xrayResult.standardError,
                        QLatin1String("gocryptfs-xray")) ||
        !QString::fromUtf8(fusermountVersionOutput).contains(
            QLatin1String("fusermount3"), Qt::CaseInsensitive))
    {
        setError(error, PrivacyGocryptfsError::UnsupportedToolVersion);

        return false;
    }

    m_capabilitiesVerified = true;

    return true;
}

bool PrivacyGocryptfsStoreHarness::prepareWorkspace(PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);

    if (!workspaceIsSafe())
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);

        return false;
    }

    const QStringList directories = {
        cipherDirectory(), mountDirectory(), runtimeDirectory()
    };

    for (const QString& directory : directories)
    {
        if (!QFileInfo::exists(directory) && !QDir().mkdir(directory))
        {
            setError(error, PrivacyGocryptfsError::FileOperationFailed);

            return false;
        }

        if (!directoryCanBeSecured(directory) ||
            !QFile::setPermissions(directory, QFileDevice::ReadOwner  |
                                              QFileDevice::WriteOwner |
                                              QFileDevice::ExeOwner)  ||
            !directoryIsSafe(directory))
        {
            setError(error, PrivacyGocryptfsError::UnsafeWorkspace);

            return false;
        }
    }

    return true;
}

bool PrivacyGocryptfsStoreHarness::createStore(const PrivacyPassword& password,
                                               const QByteArray& expectedSentinel,
                                               PrivacyGocryptfsEnvelope* const envelope,
                                               PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);

    if (!envelope)
    {
        setError(error, PrivacyGocryptfsError::InvalidEnvelope);

        return false;
    }

    *envelope = PrivacyGocryptfsEnvelope();

    if (!m_capabilitiesVerified || !password.isValid())
    {
        setError(error, password.isValid() ? PrivacyGocryptfsError::ProcessFailed
                                           : PrivacyGocryptfsError::InvalidPassword);

        return false;
    }

    if (expectedSentinel.isEmpty() || (expectedSentinel.size() > MaximumSentinelBytes))
    {
        setError(error, PrivacyGocryptfsError::InvalidSentinel);

        return false;
    }

    if (!prepareWorkspace(error))
    {
        return false;
    }

    if (!QDir(cipherDirectory()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty())
    {
        setError(error, PrivacyGocryptfsError::WorkspaceNotEmpty);

        return false;
    }

    const PrivacyProcessSpec spec = processSpec(
        m_toolPaths.gocryptfs,
        { QLatin1String("-init"), QLatin1String("-q"), QLatin1String("-nosyslog"),
          QLatin1String("-passfile"), QLatin1String("/dev/stdin"),
          cipherDirectory() },
        true);
    PrivacyProcessResult result;

    if (!runWithPassword(spec, password, &result) || !result.succeeded())
    {
        setError(error, PrivacyGocryptfsError::ProcessFailed);

        return false;
    }

    QFile config(configPath());

    if (!configCanBeSecured() ||
        !QFile::setPermissions(configPath(), QFileDevice::ReadOwner | QFileDevice::WriteOwner) ||
        !configIsSafe() || !config.open(QIODevice::ReadWrite) || (config.size() <= 0) ||
        (config.size() > MaximumEnvelopeBytes))
    {
        setError(error, PrivacyGocryptfsError::FileOperationFailed);

        return false;
    }

    PrivacyGocryptfsEnvelope stagedEnvelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
        EnvelopeFormat, config.readAll(), error);

    if (!stagedEnvelope.isValid() || !syncConfigAndCipherDirectory())
    {
        setError(error, stagedEnvelope.isValid() ? PrivacyGocryptfsError::FileOperationFailed
                                                 : PrivacyGocryptfsError::InvalidEnvelope);

        return false;
    }

    std::unique_ptr<PrivacyProcessHandle> process = startMount(password, error);

    if (!process)
    {
        return false;
    }

    if (!waitForMount(*process, error))
    {
        cleanUpFailedMount(process);

        return false;
    }

    if (QFileInfo::exists(sentinelPath()))
    {
        cleanUpFailedMount(process);
        setError(error, PrivacyGocryptfsError::SentinelMismatch);

        return false;
    }

    if (!createSentinel(expectedSentinel) || !sentinelMatches(expectedSentinel))
    {
        cleanUpFailedMount(process);
        setError(error, PrivacyGocryptfsError::FileOperationFailed);

        return false;
    }

    PrivacyGocryptfsMountLease lease(mountDirectory(), std::move(process));

    if (!unmountStore(lease, error))
    {
        return false;
    }

    if (!syncConfigAndCipherDirectory())
    {
        setError(error, PrivacyGocryptfsError::FileOperationFailed);

        return false;
    }

    *envelope = std::move(stagedEnvelope);

    return true;
}

bool PrivacyGocryptfsStoreHarness::validateEnvelope(
    const PrivacyGocryptfsEnvelope& envelope, const PrivacyPassword& password,
    PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);

    if (!m_capabilitiesVerified || !envelope.isValid() || !password.isValid())
    {
        setError(error, !password.isValid() ? PrivacyGocryptfsError::InvalidPassword
                                           : PrivacyGocryptfsError::InvalidEnvelope);

        return false;
    }

    if (!prepareWorkspace(error))
    {
        return false;
    }

    QTemporaryFile config(runtimeDirectory() + QLatin1String("/envelope-XXXXXX"));
    config.setAutoRemove(true);

    if (!config.open() ||
        !config.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner) ||
        (config.write(envelope.m_opaqueConfig) != envelope.m_opaqueConfig.size()) ||
        !config.flush())
    {
        setError(error, PrivacyGocryptfsError::FileOperationFailed);

        return false;
    }

    config.close();

    const PrivacyProcessSpec spec = processSpec(
        m_toolPaths.gocryptfsXray,
        { QLatin1String("-dumpmasterkey"), config.fileName() }, true);
    PrivacyProcessResult result;

    if (!runWithPassword(spec, password, &result) || !result.succeeded())
    {
        setError(error, PrivacyGocryptfsError::ProcessFailed);

        return false;
    }

    if (!validMasterKeyOutput(result.standardOutput))
    {
        setError(error, PrivacyGocryptfsError::InvalidMasterKeyOutput);

        return false;
    }

    result.clearOutput();

    return true;
}

std::unique_ptr<PrivacyGocryptfsMountLease> PrivacyGocryptfsStoreHarness::mountStore(
    const PrivacyPassword& password, const QByteArray& expectedSentinel,
    PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);

    if (!m_capabilitiesVerified || !password.isValid())
    {
        setError(error, PrivacyGocryptfsError::ProcessFailed);

        return {};
    }

    if (expectedSentinel.isEmpty() || (expectedSentinel.size() > MaximumSentinelBytes))
    {
        setError(error, PrivacyGocryptfsError::InvalidSentinel);

        return {};
    }

    if (!prepareWorkspace(error) || !configIsSafe() ||
        (m_mountProbe.state(mountDirectory()) != PrivacyMountStateProbe::State::NotMounted))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);

        return {};
    }

    std::unique_ptr<PrivacyProcessHandle> process = startMount(password, error);

    if (!process)
    {
        return {};
    }

    if (!waitForMount(*process, error))
    {
        cleanUpFailedMount(process);

        return {};
    }

    if (!sentinelMatches(expectedSentinel))
    {
        cleanUpFailedMount(process);
        setError(error, PrivacyGocryptfsError::SentinelMismatch);

        return {};
    }

    return std::unique_ptr<PrivacyGocryptfsMountLease>(
        new PrivacyGocryptfsMountLease(mountDirectory(), std::move(process)));
}

bool PrivacyGocryptfsStoreHarness::unmountStore(PrivacyGocryptfsMountLease& lease,
                                                PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);

    if (!m_capabilitiesVerified || !lease.m_process ||
        (QDir::cleanPath(lease.m_mountPoint) != mountDirectory()))
    {
        setError(error, PrivacyGocryptfsError::UnmountFailed);

        return false;
    }

    PrivacyProcessResult unmountResult =
        m_runner.run(processSpec(m_toolPaths.fusermount,
                                 { QLatin1String("-u"), mountDirectory() }), {});

    if (!unmountResult.succeeded())
    {
        setError(error, PrivacyGocryptfsError::UnmountFailed);

        return false;
    }

    if (m_mountProbe.state(mountDirectory()) != PrivacyMountStateProbe::State::NotMounted)
    {
        setError(error, PrivacyGocryptfsError::UnmountFailed);

        return false;
    }

    // Once disappearance is proven, reap (or terminate on timeout) the
    // foreground child. The mount-table result is authoritative for whether
    // plaintext remains exposed.
    lease.m_process->waitForFinished(5000);
    lease.m_process.reset();

    return true;
}

QString PrivacyGocryptfsStoreHarness::cipherDirectory() const
{
    return m_cipherDirectory;
}

QString PrivacyGocryptfsStoreHarness::mountDirectory() const
{
    return m_mountDirectory;
}

QString PrivacyGocryptfsStoreHarness::runtimeDirectory() const
{
    return m_runtimeDirectory;
}

QString PrivacyGocryptfsStoreHarness::configPath() const
{
    return cipherDirectory() + QLatin1String("/gocryptfs.conf");
}

QString PrivacyGocryptfsStoreHarness::sentinelPath() const
{
    return mountDirectory() + QLatin1Char('/') + SentinelName;
}

PrivacyProcessSpec PrivacyGocryptfsStoreHarness::processSpec(
    const QString& program, const QStringList& arguments, bool sensitiveOutput) const
{
    PrivacyProcessSpec spec;
    spec.program         = program;
    spec.arguments       = arguments;
    spec.sensitiveOutput = sensitiveOutput;

    QProcessEnvironment environment;
    environment.insert(QLatin1String("LANG"),   QLatin1String("C"));
    environment.insert(QLatin1String("LC_ALL"), QLatin1String("C"));
    environment.insert(QLatin1String("PATH"),   QLatin1String("/usr/bin:/bin"));
    spec.environment = environment;

    if (sensitiveOutput)
    {
        spec.maximumStdout = 4096;
        spec.maximumStderr = 4096;
    }

    return spec;
}

bool PrivacyGocryptfsStoreHarness::toolsAreSafe() const
{
    const QStringList paths = {
        m_toolPaths.gocryptfs, m_toolPaths.gocryptfsXray, m_toolPaths.fusermount
    };

    for (const QString& path : paths)
    {
        const QFileInfo info(path);

        if (!info.isAbsolute() || !info.isFile() || !info.isExecutable() || info.isSymLink() ||
            (info.permissions() & (QFileDevice::WriteGroup | QFileDevice::WriteOther)))
        {
            return false;
        }

#ifdef Q_OS_UNIX
        if ((info.ownerId() != static_cast<uint>(geteuid())) && (info.ownerId() != 0U))
        {
            return false;
        }
#endif
    }

    return true;
}

bool PrivacyGocryptfsStoreHarness::workspaceIsSafe() const
{
    return directoryIsSafe(m_workspaceRoot);
}

bool PrivacyGocryptfsStoreHarness::directoryCanBeSecured(const QString& path) const
{
    const QFileInfo info(path);

    if (!info.isAbsolute() || !info.isDir() || info.isSymLink())
    {
        return false;
    }

#ifdef Q_OS_UNIX
    if (info.ownerId() != static_cast<uint>(geteuid()))
    {
        return false;
    }
#endif

    return true;
}

bool PrivacyGocryptfsStoreHarness::directoryIsSafe(const QString& path) const
{
    const QFileInfo info(path);
    const QFileDevice::Permissions forbidden =
        QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
        QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;

    if (!directoryCanBeSecured(path) || (info.permissions() & forbidden))
    {
        return false;
    }

    return true;
}

bool PrivacyGocryptfsStoreHarness::configCanBeSecured() const
{
    const QFileInfo info(configPath());

    if (!info.isAbsolute() || !info.isFile() || info.isSymLink() ||
        (info.size() <= 0) || (info.size() > MaximumEnvelopeBytes))
    {
        return false;
    }

#ifdef Q_OS_UNIX
    if (info.ownerId() != static_cast<uint>(geteuid()))
    {
        return false;
    }
#endif

    return true;
}

bool PrivacyGocryptfsStoreHarness::configIsSafe() const
{
    const QFileInfo info(configPath());

    return (configCanBeSecured() &&
            !(info.permissions() & (QFileDevice::ReadGroup | QFileDevice::WriteGroup |
                                    QFileDevice::ReadOther | QFileDevice::WriteOther)));
}

bool PrivacyGocryptfsStoreHarness::runWithPassword(
    const PrivacyProcessSpec& spec, const PrivacyPassword& password,
    PrivacyProcessResult* const result) const
{
    if (!result)
    {
        return false;
    }

    return password.withStdinLine(
        [this, &spec, result](const QByteArray& stdinLine)
        {
            *result = m_runner.run(spec, stdinLine);

            return true;
        });
}

std::unique_ptr<PrivacyProcessHandle> PrivacyGocryptfsStoreHarness::startMount(
    const PrivacyPassword& password, PrivacyGocryptfsError* const error) const
{
    const PrivacyProcessSpec spec = processSpec(
        m_toolPaths.gocryptfs,
        { QLatin1String("-fg"), QLatin1String("-q"), QLatin1String("-nosyslog"),
          QLatin1String("-nodev"), QLatin1String("-nosuid"), QLatin1String("-noexec"),
          QLatin1String("-passfile"), QLatin1String("/dev/stdin"),
          cipherDirectory(), mountDirectory() },
        true);
    std::unique_ptr<PrivacyProcessHandle> process;

    const bool started = password.withStdinLine(
        [this, &spec, &process](const QByteArray& stdinLine)
        {
            process = m_runner.start(spec, stdinLine);

            return (process && process->started());
        });

    if (!started || !process || !process->isRunning())
    {
        setError(error, PrivacyGocryptfsError::ProcessFailed);

        return {};
    }

    return process;
}

bool PrivacyGocryptfsStoreHarness::waitForMount(
    PrivacyProcessHandle& process, PrivacyGocryptfsError* const error) const
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < MountReadyTimeoutMs)
    {
        if (!process.isRunning())
        {
            setError(error, PrivacyGocryptfsError::ProcessFailed);

            return false;
        }

        const PrivacyMountStateProbe::State mountState =
            m_mountProbe.state(mountDirectory());

        if (mountState == PrivacyMountStateProbe::State::Unknown)
        {
            setError(error, PrivacyGocryptfsError::MountNotReady);

            return false;
        }

        if (mountState == PrivacyMountStateProbe::State::Mounted)
        {
            return true;
        }

        QThread::msleep(MountPollIntervalMs);
    }

    setError(error, PrivacyGocryptfsError::MountNotReady);

    return false;
}

bool PrivacyGocryptfsStoreHarness::createSentinel(
    const QByteArray& expectedSentinel) const
{
    QFile sentinel(sentinelPath());

    return (sentinel.open(QIODevice::WriteOnly | QIODevice::NewOnly) &&
            sentinel.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner) &&
            (sentinel.write(expectedSentinel) == expectedSentinel.size()) &&
            syncFile(sentinel) && syncDirectory(mountDirectory()));
}

bool PrivacyGocryptfsStoreHarness::syncConfigAndCipherDirectory() const
{
    QFile config(configPath());

    return (config.open(QIODevice::ReadWrite) &&
#ifdef Q_OS_UNIX
            (::fsync(config.handle()) == 0) &&
#endif
            syncDirectory(cipherDirectory()));
}

bool PrivacyGocryptfsStoreHarness::validMasterKeyOutput(const QByteArray& output) const
{
    if ((output.size() != 65) || (output.at(64) != '\n'))
    {
        return false;
    }

    for (int i = 0 ; i < 64 ; ++i)
    {
        const char value = output.at(i);

        if (!(((value >= '0') && (value <= '9')) ||
              ((value >= 'a') && (value <= 'f'))))
        {
            return false;
        }
    }

    return true;
}

bool PrivacyGocryptfsStoreHarness::sentinelMatches(
    const QByteArray& expectedSentinel) const
{
    const QFileInfo info(sentinelPath());

    if (!info.isFile() || info.isSymLink())
    {
        return false;
    }

    QFile sentinel(sentinelPath());

    return (sentinel.open(QIODevice::ReadOnly) &&
            (sentinel.size() == expectedSentinel.size()) &&
            (sentinel.readAll() == expectedSentinel));
}

void PrivacyGocryptfsStoreHarness::cleanUpFailedMount(
    std::unique_ptr<PrivacyProcessHandle>& process) const
{
    // Readiness can fail after the kernel mount has appeared. Attempt the same
    // direct, non-shell unmount path before reaping the foreground process.
    m_runner.run(processSpec(m_toolPaths.fusermount,
                             { QLatin1String("-u"), mountDirectory() }), {});

    if (process)
    {
        process->terminate();
        process.reset();
    }
}

void PrivacyGocryptfsStoreHarness::setError(
    PrivacyGocryptfsError* const error, PrivacyGocryptfsError value) const
{
    if (error)
    {
        *error = value;
    }
}

} // namespace Digikam

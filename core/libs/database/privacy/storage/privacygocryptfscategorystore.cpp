/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacygocryptfscategorystore.h"

// C++ includes

#include <utility>

// Qt includes

#include <QDir>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#ifdef Q_OS_UNIX
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>
#endif

// Local includes

#include "privacyposixstorage_p.h"
#include "privacytransactionjournal.h"

namespace Digikam
{

namespace
{

void setError(PrivacyGocryptfsError* const error, PrivacyGocryptfsError value)
{
    if (error)
    {
        *error = value;
    }
}

bool safeRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path) || path.contains(QChar::Null))
    {
        return false;
    }

    for (const QString& part : path.split(QLatin1Char('/')))
    {
        if (part.isEmpty() || (part == QLatin1String(".")) ||
            (part == QLatin1String("..")))
        {
            return false;
        }
    }

    return true;
}

QString pathBelow(const QString& root, const QString& relativePath)
{
    if (!QDir::isAbsolutePath(root) || !safeRelativePath(relativePath))
    {
        return QString();
    }

    const QString cleanRoot = QDir::cleanPath(root);
    const QString path = QDir::cleanPath(cleanRoot + QLatin1Char('/') + relativePath);

    return path.startsWith(cleanRoot + QLatin1Char('/')) ? path : QString();
}

bool secureDirectory(const QString& path)
{
    if (!QFileInfo::exists(path) && !QDir().mkpath(path))
    {
        return false;
    }

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

    return (QFile::setPermissions(path, QFileDevice::ReadOwner |
                                        QFileDevice::WriteOwner |
                                        QFileDevice::ExeOwner));
}

bool readConfig(const QString& cipherDirectory, QByteArray* const config)
{
    if (!config)
    {
        return false;
    }

    const QString path = cipherDirectory + QLatin1String("/gocryptfs.conf");
    const QFileInfo info(path);

    if (!info.isFile() || info.isSymLink() || (info.size() <= 0) ||
        (info.size() > (1024 * 1024)))
    {
        return false;
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    *config = file.readAll();

    return (config->size() == info.size());
}

#ifdef Q_OS_UNIX

class ScopedFd
{
public:

    explicit ScopedFd(int descriptor = -1)
        : fd(descriptor)
    {
    }

    ~ScopedFd()
    {
        if (fd >= 0)
        {
            ::close(fd);
        }
    }

    ScopedFd(const ScopedFd&) = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    int fd = -1;
};

bool safeOwnedDirectory(int descriptor, dev_t device)
{
    struct stat status = {};

    return ((descriptor >= 0) && (::fstat(descriptor, &status) == 0) &&
            S_ISDIR(status.st_mode) && (status.st_uid == geteuid()) &&
            (status.st_dev == device) &&
            ((status.st_mode & (S_IWGRP | S_IWOTH)) == 0));
}

int openOrCreateDirectoryAt(int parentFd, const QByteArray& component,
                            dev_t device)
{
    int descriptor = PrivacyPosixStorage::confinedOpenAt(
        parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if ((descriptor < 0) && (errno == ENOENT))
    {
        if ((::mkdirat(parentFd, component.constData(), S_IRWXU) != 0) &&
            (errno != EEXIST))
        {
            return -1;
        }

        descriptor = PrivacyPosixStorage::confinedOpenAt(
            parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    }

    if (!safeOwnedDirectory(descriptor, device))
    {
        if (descriptor >= 0)
        {
            ::close(descriptor);
        }

        errno = EPERM;
        return -1;
    }

    return descriptor;
}

int openDirectoryAt(int parentFd, const QByteArray& component, dev_t device)
{
    const int descriptor = PrivacyPosixStorage::confinedOpenAt(
        parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return -1;
    }

    if (!safeOwnedDirectory(descriptor, device))
    {
        if (descriptor >= 0)
        {
            ::close(descriptor);
        }

        errno = EPERM;
        return -1;
    }

    return descriptor;
}

QString descriptorChildPath(int directoryFd, const QByteArray& child)
{
    return QStringLiteral("/proc/%1/fd/%2/%3")
        .arg(static_cast<qlonglong>(::getpid()))
        .arg(directoryFd)
        .arg(QString::fromUtf8(child));
}

bool readConfigAt(int cipherDirectoryFd, dev_t device, QByteArray* const config)
{
    if (!config)
    {
        return false;
    }

    ScopedFd configFd(PrivacyPosixStorage::confinedOpenAt(
        cipherDirectoryFd, QByteArrayLiteral("gocryptfs.conf"),
        O_RDONLY | O_CLOEXEC));
    struct stat status = {};

    if ((configFd.fd < 0) || (::fstat(configFd.fd, &status) != 0) ||
        !S_ISREG(status.st_mode) || (status.st_uid != geteuid()) ||
        (status.st_dev != device) || (status.st_nlink != 1) ||
        (status.st_size <= 0) || (status.st_size > (1024 * 1024)))
    {
        return false;
    }

    config->resize(static_cast<int>(status.st_size));
    qint64 offset = 0;

    while (offset < config->size())
    {
        const ssize_t count = ::read(configFd.fd, config->data() + offset,
                                     static_cast<size_t>(config->size() - offset));

        if (count <= 0)
        {
            return false;
        }

        offset += count;
    }

    return true;
}

#endif

class GocryptfsLease final : public PrivacyCategoryStoreLease
{
public:

    GocryptfsLease(std::unique_ptr<PrivacyGocryptfsStoreHarness>&& storeHarness,
                   std::unique_ptr<PrivacyGocryptfsMountLease>&& mountLease)
        : harness(std::move(storeHarness)), lease(std::move(mountLease))
    {
    }

    bool isActive() override
    {
        return lease && lease->isActive();
    }

    QString plaintextRoot() const override
    {
        return lease ? lease->mountPoint() : QString();
    }

    std::unique_ptr<PrivacyGocryptfsStoreHarness> harness;
    std::unique_ptr<PrivacyGocryptfsMountLease> lease;
};

} // namespace

PrivacyGocryptfsCategoryStoreBackend::PrivacyGocryptfsCategoryStoreBackend(
    PrivacyProcessRunner& runner, const PrivacyMountStateProbe& mountProbe,
    PrivacyGocryptfsToolPaths toolPaths, QString runtimeRoot)
    : m_runner(runner),
      m_mountProbe(mountProbe),
      m_toolPaths(std::move(toolPaths)),
      m_runtimeRoot(QDir::cleanPath(std::move(runtimeRoot)))
{
}

bool PrivacyGocryptfsCategoryStoreBackend::createOrResume(
    const PrivacyStorageRoot& root, const PrivacyStore& store,
    const QString& temporaryCipherRelativePath, const PrivacyPassword& password,
    const QByteArray& sentinel, PrivacyGocryptfsEnvelope* const envelope,
    PrivacyGocryptfsError* const error)
{
    setError(error, PrivacyGocryptfsError::None);

    if (!envelope || !root.isValid() || !store.isValid() || !password.isValid() ||
        (root.kind != PrivacyStorageRootKind::ManagedStoreRoot) ||
        (store.rootUuid != root.uuid) ||
        (store.cipherRelativePath !=
         (QLatin1String(".digikam-private/stores/") + store.uuid)) ||
        (temporaryCipherRelativePath !=
         (QLatin1String(".digikam-private/staging/") + store.uuid +
          QLatin1String(".creating"))))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    *envelope = PrivacyGocryptfsEnvelope();
    const QString runtimeWorkspace = m_runtimeRoot + QLatin1Char('/') + store.uuid;

#ifndef Q_OS_UNIX
    Q_UNUSED(runtimeWorkspace);
    setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
    return false;
#else
    const QByteArray rootPath = QFile::encodeName(root.configuredPath);
    ScopedFd rootFd(::open(rootPath.constData(),
                           O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    struct stat rootStatus = {};

    if ((rootFd.fd < 0) || (::fstat(rootFd.fd, &rootStatus) != 0) ||
        !S_ISDIR(rootStatus.st_mode) || (rootStatus.st_uid != geteuid()) ||
        ((rootStatus.st_mode & (S_IWGRP | S_IWOTH)) != 0) ||
        !secureDirectory(m_runtimeRoot) || !secureDirectory(runtimeWorkspace))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    PrivacyJournalRootExpectation rootExpectation;
    rootExpectation.rootUuid = root.uuid;
    rootExpectation.markerUuid = root.markerUuid;
    rootExpectation.identitySha256 =
        QCryptographicHash::hash(root.identityData, QCryptographicHash::Sha256);
    rootExpectation.device = static_cast<quint64>(rootStatus.st_dev);
    rootExpectation.inode = static_cast<quint64>(rootStatus.st_ino);
    PrivacyJournalError rootError = PrivacyJournalError::None;
    QString rootDetail;
    auto verifiedRoot = PrivacyTransactionJournalStore::open(
        root.configuredPath, rootExpectation, &rootError, &rootDetail);

    if (!verifiedRoot)
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    ScopedFd metadataFd(openOrCreateDirectoryAt(
        rootFd.fd, QByteArrayLiteral(".digikam-private"), rootStatus.st_dev));
    ScopedFd stagingFd(openOrCreateDirectoryAt(
        metadataFd.fd, QByteArrayLiteral("staging"), rootStatus.st_dev));
    ScopedFd storesFd(openOrCreateDirectoryAt(
        metadataFd.fd, QByteArrayLiteral("stores"), rootStatus.st_dev));

    if ((metadataFd.fd < 0) || (stagingFd.fd < 0) || (storesFd.fd < 0))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    const QByteArray finalName = store.uuid.toUtf8();
    const QByteArray temporaryName = finalName + QByteArrayLiteral(".creating");
    const QString finalCipher = descriptorChildPath(storesFd.fd, finalName);
    const QString temporaryCipher = descriptorChildPath(stagingFd.fd, temporaryName);
    QByteArray configBytes;
    ScopedFd finalFd(openDirectoryAt(storesFd.fd, finalName, rootStatus.st_dev));

    if (finalFd.fd >= 0)
    {
        if (!readConfigAt(finalFd.fd, rootStatus.st_dev, &configBytes))
        {
            setError(error, PrivacyGocryptfsError::FileOperationFailed);
            return false;
        }

        *envelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
            QLatin1String("gocryptfs-config-v2"), configBytes, error);
    }
    else
    {
        if (errno != ENOENT)
        {
            setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
            return false;
        }

        PrivacyGocryptfsStoreLayout layout;
        layout.workspaceRoot = runtimeWorkspace;
        layout.cipherDirectory = temporaryCipher;
        layout.mountDirectory = runtimeWorkspace + QLatin1String("/mount");
        layout.runtimeDirectory = runtimeWorkspace + QLatin1String("/runtime");
        PrivacyGocryptfsStoreHarness harness(m_runner, m_mountProbe, m_toolPaths, layout);

        if (!harness.checkCapabilities(error))
        {
            return false;
        }

        ScopedFd temporaryFd(openDirectoryAt(stagingFd.fd, temporaryName,
                                              rootStatus.st_dev));

        if ((temporaryFd.fd < 0) && (errno != ENOENT))
        {
            setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
            return false;
        }

        if (temporaryFd.fd >= 0)
        {
            if (!readConfigAt(temporaryFd.fd, rootStatus.st_dev, &configBytes))
            {
                setError(error, PrivacyGocryptfsError::FileOperationFailed);
                return false;
            }

            *envelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
                QLatin1String("gocryptfs-config-v2"), configBytes, error);

            if (!envelope->isValid() || !harness.validateEnvelope(*envelope, password, error))
            {
                return false;
            }

            std::unique_ptr<PrivacyGocryptfsMountLease> staged =
                harness.mountStore(password, sentinel, error);

            if (!staged || !harness.unmountStore(*staged, error))
            {
                return false;
            }
        }
        else if (!harness.createStore(password, sentinel, envelope, error))
        {
            return false;
        }

        if (temporaryFd.fd < 0)
        {
            temporaryFd.fd = openDirectoryAt(stagingFd.fd, temporaryName,
                                             rootStatus.st_dev);
        }
        bool renameUnavailable = false;

        if ((temporaryFd.fd < 0) ||
            !PrivacyPosixStorage::atomicRenameAt(
                stagingFd.fd, temporaryName, storesFd.fd, finalName,
                PrivacyPosixStorage::AtomicRenameMode::NoReplace,
                &renameUnavailable) || renameUnavailable ||
            (::fsync(storesFd.fd) != 0) || (::fsync(stagingFd.fd) != 0))
        {
            setError(error, PrivacyGocryptfsError::FileOperationFailed);
            return false;
        }

        finalFd.fd = openDirectoryAt(storesFd.fd, finalName, rootStatus.st_dev);

        if ((finalFd.fd < 0) ||
            !readConfigAt(finalFd.fd, rootStatus.st_dev, &configBytes) ||
            (configBytes != envelope->opaqueConfig()))
        {
            setError(error, PrivacyGocryptfsError::FileOperationFailed);
            return false;
        }
    }

    PrivacyGocryptfsStoreLayout finalLayout;
    finalLayout.workspaceRoot = runtimeWorkspace;
    finalLayout.cipherDirectory = finalCipher;
    finalLayout.mountDirectory = runtimeWorkspace + QLatin1String("/mount");
    finalLayout.runtimeDirectory = runtimeWorkspace + QLatin1String("/runtime");
    PrivacyGocryptfsStoreHarness finalHarness(m_runner, m_mountProbe, m_toolPaths,
                                              finalLayout);

    if (!envelope->isValid() || !finalHarness.checkCapabilities(error) ||
        !finalHarness.validateEnvelope(*envelope, password, error))
    {
        return false;
    }

    std::unique_ptr<PrivacyGocryptfsMountLease> verified =
        finalHarness.mountStore(password, sentinel, error);

    return (verified && finalHarness.unmountStore(*verified, error));
#endif
}

bool PrivacyGocryptfsCategoryStoreBackend::validateEnvelope(
    const PrivacyGocryptfsEnvelope& envelope, const PrivacyPassword& password,
    PrivacyGocryptfsError* const error)
{
    if (!secureDirectory(m_runtimeRoot))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    QTemporaryDir workspace(m_runtimeRoot + QLatin1String("/validate-XXXXXX"));

    if (!workspace.isValid() ||
        !QFile::setPermissions(workspace.path(), QFileDevice::ReadOwner |
                                                 QFileDevice::WriteOwner |
                                                 QFileDevice::ExeOwner))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    PrivacyGocryptfsStoreHarness harness(m_runner, m_mountProbe, m_toolPaths,
                                         workspace.path());

    return (harness.checkCapabilities(error) &&
            harness.validateEnvelope(envelope, password, error));
}

std::unique_ptr<PrivacyCategoryStoreLease>
PrivacyGocryptfsCategoryStoreBackend::unlock(
    const PrivacyStorageRoot& root, const PrivacyStore& store,
    const PrivacyGocryptfsEnvelope& envelope, const PrivacyPassword& password,
    const QByteArray& sentinel, PrivacyGocryptfsError* const error)
{
    const QString cipher = pathBelow(root.configuredPath, store.cipherRelativePath);
    const QString workspace = m_runtimeRoot + QLatin1Char('/') + store.uuid;
    QByteArray config;

    if (cipher.isEmpty() || !readConfig(cipher, &config) ||
        (config != envelope.opaqueConfig()) || !secureDirectory(m_runtimeRoot) ||
        !secureDirectory(workspace))
    {
        setError(error, PrivacyGocryptfsError::InvalidEnvelope);
        return {};
    }

    PrivacyGocryptfsStoreLayout layout;
    layout.workspaceRoot = workspace;
    layout.cipherDirectory = cipher;
    layout.mountDirectory = workspace + QLatin1String("/mount");
    layout.runtimeDirectory = workspace + QLatin1String("/runtime");
    auto harness = std::make_unique<PrivacyGocryptfsStoreHarness>(
        m_runner, m_mountProbe, m_toolPaths, layout);

    if (!harness->checkCapabilities(error))
    {
        return {};
    }

    std::unique_ptr<PrivacyGocryptfsMountLease> mount =
        harness->mountStore(password, sentinel, error);

    if (!mount)
    {
        return {};
    }

    return std::make_unique<GocryptfsLease>(std::move(harness), std::move(mount));
}

bool PrivacyGocryptfsCategoryStoreBackend::lock(
    std::unique_ptr<PrivacyCategoryStoreLease>& lease,
    PrivacyGocryptfsError* const error)
{
    if (!lease)
    {
        setError(error, PrivacyGocryptfsError::None);
        return true;
    }

    GocryptfsLease* const concrete = dynamic_cast<GocryptfsLease*>(lease.get());

    if (!concrete || !concrete->harness || !concrete->lease ||
        !concrete->harness->unmountStore(*concrete->lease, error))
    {
        setError(error, PrivacyGocryptfsError::UnmountFailed);
        return false;
    }

    lease.reset();
    return true;
}

} // namespace Digikam

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
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#ifdef Q_OS_UNIX
#   include <fcntl.h>
#   include <unistd.h>
#endif

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

bool syncDirectory(const QString& path)
{
#ifdef Q_OS_UNIX
    const QByteArray encoded = QFile::encodeName(path);
    const int descriptor = ::open(encoded.constData(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (descriptor < 0)
    {
        return false;
    }

    const bool success = (::fsync(descriptor) == 0);
    ::close(descriptor);

    return success;
#else
    Q_UNUSED(path);
    return true;
#endif
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
        !safeRelativePath(temporaryCipherRelativePath))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    *envelope = PrivacyGocryptfsEnvelope();
    const QString finalCipher = pathBelow(root.configuredPath, store.cipherRelativePath);
    const QString temporaryCipher = pathBelow(root.configuredPath,
                                               temporaryCipherRelativePath);
    const QString privateRoot = pathBelow(root.configuredPath,
                                          QLatin1String(".digikam-private"));
    const QString runtimeWorkspace = m_runtimeRoot + QLatin1Char('/') + store.uuid;

    if (finalCipher.isEmpty() || temporaryCipher.isEmpty() || privateRoot.isEmpty() ||
        !secureDirectory(privateRoot) ||
        !secureDirectory(QFileInfo(temporaryCipher).absolutePath()) ||
        !secureDirectory(QFileInfo(finalCipher).absolutePath()) ||
        !secureDirectory(m_runtimeRoot) || !secureDirectory(runtimeWorkspace))
    {
        setError(error, PrivacyGocryptfsError::UnsafeWorkspace);
        return false;
    }

    QByteArray configBytes;

    if (QFileInfo::exists(finalCipher))
    {
        if (!readConfig(finalCipher, &configBytes))
        {
            setError(error, PrivacyGocryptfsError::FileOperationFailed);
            return false;
        }

        *envelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
            QLatin1String("gocryptfs-config-v2"), configBytes, error);
    }
    else
    {
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

        if (QFileInfo::exists(temporaryCipher))
        {
            if (!readConfig(temporaryCipher, &configBytes))
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

        if (!QDir().rename(temporaryCipher, finalCipher) ||
            !syncDirectory(QFileInfo(finalCipher).absolutePath()))
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

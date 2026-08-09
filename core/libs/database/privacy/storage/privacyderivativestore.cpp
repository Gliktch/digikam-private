/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyderivativestore.h"

// C++ includes

#include <cerrno>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QUuid>

#if defined(Q_OS_UNIX)

// POSIX includes

#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>

#endif

// Local includes

#include "privacyposixstorage_p.h"

namespace Digikam
{

namespace
{

constexpr qint64 MaximumDerivativeBytes = 32LL * 1024LL * 1024LL;

void setError(PrivacyDerivativeStoreError* const error,
              PrivacyDerivativeStoreError value, QString* const detail,
              const QString& message)
{
    if (error)
    {
        *error = value;
    }

    if (detail)
    {
        *detail = message;
    }
}

bool canonicalUuid(const QString& value)
{
    const QUuid uuid(value);
    return (!uuid.isNull() &&
            (uuid.toString(QUuid::WithoutBraces).toLower() == value));
}

bool canonicalSha256(const QString& value)
{
    if (value.size() != 64)
    {
        return false;
    }

    for (const QChar character : value)
    {
        if (!character.isDigit() &&
            ((character < QLatin1Char('a')) ||
             (character > QLatin1Char('f'))))
        {
            return false;
        }
    }

    return true;
}

bool exactClearDerivative(const PrivacyDerivative& derivative)
{
    return (derivative.isValid() &&
            (derivative.kind == PrivacyDerivativeKind::ClearThumbnail) &&
            (derivative.ordinal == 0) &&
            (derivative.sourceHashAlgorithm == QLatin1String("sha256")) &&
            canonicalSha256(derivative.sourceOriginalHash) &&
            (derivative.derivativeFormat == QLatin1String("jpeg")) &&
            (derivative.derivativeHashAlgorithm == QLatin1String("sha256")) &&
            canonicalSha256(derivative.derivativeHash) &&
            (derivative.derivativeSize > 0) &&
            (derivative.derivativeSize <= MaximumDerivativeBytes) &&
            (derivative.presentationVersion > 0) &&
            (derivative.protectedRelativePath ==
             PrivacyDerivativeStore::clearThumbnailRelativePath(
                 derivative.itemUuid, derivative.sourceOriginalHash,
                 derivative.presentationVersion)));
}

#if defined(Q_OS_UNIX)

class ScopedFd
{
public:

    explicit ScopedFd(int value = -1)
        : fd(value)
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

bool safeDirectory(int fd, dev_t device)
{
    struct stat status = {};
    return ((fd >= 0) && (::fstat(fd, &status) == 0) &&
            S_ISDIR(status.st_mode) && (status.st_uid == geteuid()) &&
            (status.st_dev == device) &&
            ((status.st_mode & (S_IWGRP | S_IWOTH)) == 0));
}

int openDirectoryAt(int parentFd, const QByteArray& component, dev_t device,
                    bool create)
{
    int fd = PrivacyPosixStorage::confinedOpenAt(
        parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if ((fd < 0) && create && (errno == ENOENT))
    {
        if ((::mkdirat(parentFd, component.constData(), S_IRWXU) != 0) &&
            (errno != EEXIST))
        {
            return -1;
        }

        fd = PrivacyPosixStorage::confinedOpenAt(
            parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    }

    if (!safeDirectory(fd, device))
    {
        if (fd >= 0)
        {
            ::close(fd);
        }

        errno = EPERM;
        return -1;
    }

    return fd;
}

bool writeAll(int fd, const QByteArray& bytes)
{
    qint64 offset = 0;

    while (offset < bytes.size())
    {
        const ssize_t written = ::write(
            fd, bytes.constData() + offset,
            static_cast<size_t>(bytes.size() - offset));

        if (written <= 0)
        {
            return false;
        }

        offset += written;
    }

    return true;
}

QByteArray readExactFile(int directoryFd, const QByteArray& name,
                         dev_t device, qint64 expectedSize,
                         const QByteArray& expectedHash)
{
    ScopedFd file(PrivacyPosixStorage::confinedOpenAt(
        directoryFd, name, O_RDONLY | O_CLOEXEC));
    struct stat before = {};

    if ((file.fd < 0) || (::fstat(file.fd, &before) != 0) ||
        !S_ISREG(before.st_mode) || (before.st_uid != geteuid()) ||
        (before.st_dev != device) || (before.st_nlink != 1) ||
        (before.st_size != expectedSize) ||
        (before.st_size <= 0) || (before.st_size > MaximumDerivativeBytes))
    {
        return {};
    }

    QByteArray bytes(static_cast<int>(before.st_size), Qt::Uninitialized);
    qint64 offset = 0;

    while (offset < bytes.size())
    {
        const ssize_t count = ::read(
            file.fd, bytes.data() + offset,
            static_cast<size_t>(bytes.size() - offset));

        if (count <= 0)
        {
            return {};
        }

        offset += count;
    }

    struct stat after = {};

    if ((::fstat(file.fd, &after) != 0) ||
        (before.st_dev != after.st_dev) || (before.st_ino != after.st_ino) ||
        (before.st_size != after.st_size) ||
        (before.st_mtim.tv_sec != after.st_mtim.tv_sec) ||
        (before.st_mtim.tv_nsec != after.st_mtim.tv_nsec) ||
        (QCryptographicHash::hash(bytes, QCryptographicHash::Sha256) !=
         expectedHash))
    {
        return {};
    }

    return bytes;
}

#endif

} // namespace

QString PrivacyDerivativeStore::clearThumbnailRelativePath(
    const QString& itemUuid, const QString& sourceOriginalSha256,
    int presentationVersion)
{
    if (!canonicalUuid(itemUuid) || !canonicalSha256(sourceOriginalSha256) ||
        (presentationVersion <= 0))
    {
        return QString();
    }

    return QStringLiteral("derivatives/%1/clear-0-%2-v%3.jpg")
        .arg(itemUuid, sourceOriginalSha256)
        .arg(presentationVersion);
}

bool PrivacyDerivativeStore::put(
    const QString& plaintextRoot, const PrivacyDerivative& derivative,
    const QByteArray& encodedBytes, PrivacyDerivativeStoreError* const error,
    QString* const detail) const
{
    setError(error, PrivacyDerivativeStoreError::None, detail, {});

    const QByteArray expectedHash = QByteArray::fromHex(
        derivative.derivativeHash.toLatin1());

    if (!QDir::isAbsolutePath(plaintextRoot) ||
        (QDir::cleanPath(plaintextRoot) != plaintextRoot) ||
        !exactClearDerivative(derivative) || encodedBytes.isEmpty() ||
        (encodedBytes.size() != derivative.derivativeSize) ||
        (QCryptographicHash::hash(encodedBytes, QCryptographicHash::Sha256) !=
         expectedHash))
    {
        setError(error, PrivacyDerivativeStoreError::InvalidRequest, detail,
                 QStringLiteral("the clear derivative request is invalid"));
        return false;
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyDerivativeStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined derivative storage requires Unix"));
    return false;
#else
    ScopedFd root(::open(QFile::encodeName(plaintextRoot).constData(),
                         O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    struct stat rootStatus = {};

    if ((root.fd < 0) || (::fstat(root.fd, &rootStatus) != 0) ||
        !safeDirectory(root.fd, rootStatus.st_dev))
    {
        setError(error, PrivacyDerivativeStoreError::UnsafeStore, detail,
                 QStringLiteral("the mounted derivative store is unsafe"));
        return false;
    }

    ScopedFd derivatives(openDirectoryAt(
        root.fd, QByteArrayLiteral("derivatives"), rootStatus.st_dev, true));
    ScopedFd item(openDirectoryAt(
        derivatives.fd, derivative.itemUuid.toUtf8(), rootStatus.st_dev, true));

    if ((derivatives.fd < 0) || (item.fd < 0))
    {
        setError(error, PrivacyDerivativeStoreError::UnsafeStore, detail,
                 QStringLiteral("the derivative namespace is unsafe"));
        return false;
    }

    const QByteArray fileName = QFile::encodeName(
        derivative.protectedRelativePath.section(QLatin1Char('/'), -1));
    const QByteArray existing = readExactFile(
        item.fd, fileName, rootStatus.st_dev, derivative.derivativeSize,
        expectedHash);

    if (!existing.isEmpty())
    {
        return true;
    }

    struct stat existingStatus = {};

    if (::fstatat(item.fd, fileName.constData(), &existingStatus,
                  AT_SYMLINK_NOFOLLOW) == 0)
    {
        setError(error, PrivacyDerivativeStoreError::Conflict, detail,
                 QStringLiteral("a conflicting clear derivative already exists"));
        return false;
    }

    if (errno != ENOENT)
    {
        setError(error, PrivacyDerivativeStoreError::UnsafeStore, detail,
                 QStringLiteral("the derivative destination cannot be verified"));
        return false;
    }

    const QByteArray temporary = QByteArrayLiteral(".tmp-") +
        QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
    ScopedFd output(PrivacyPosixStorage::confinedOpenAt(
        item.fd, temporary, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC,
        S_IRUSR | S_IWUSR));

    if ((output.fd < 0) || !writeAll(output.fd, encodedBytes) ||
        (::fsync(output.fd) != 0))
    {
        ::unlinkat(item.fd, temporary.constData(), 0);
        setError(error, PrivacyDerivativeStoreError::IoFailure, detail,
                 QStringLiteral("the clear derivative could not be written"));
        return false;
    }

    bool unavailable = false;

    if (!PrivacyPosixStorage::atomicRenameAt(
            item.fd, temporary, fileName,
            PrivacyPosixStorage::AtomicRenameMode::NoReplace, &unavailable) ||
        (::fsync(item.fd) != 0))
    {
        ::unlinkat(item.fd, temporary.constData(), 0);
        setError(error,
                 unavailable ? PrivacyDerivativeStoreError::UnsupportedPlatform
                             : PrivacyDerivativeStoreError::Conflict,
                 detail, QStringLiteral("the clear derivative could not be published"));
        return false;
    }

    return true;
#endif
}

QByteArray PrivacyDerivativeStore::read(
    const QString& plaintextRoot, const PrivacyDerivative& derivative,
    PrivacyDerivativeStoreError* const error, QString* const detail) const
{
    setError(error, PrivacyDerivativeStoreError::None, detail, {});

    if (!QDir::isAbsolutePath(plaintextRoot) ||
        (QDir::cleanPath(plaintextRoot) != plaintextRoot) ||
        !exactClearDerivative(derivative))
    {
        setError(error, PrivacyDerivativeStoreError::InvalidRequest, detail,
                 QStringLiteral("the clear derivative request is invalid"));
        return {};
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyDerivativeStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined derivative storage requires Unix"));
    return {};
#else
    ScopedFd root(::open(QFile::encodeName(plaintextRoot).constData(),
                         O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    struct stat rootStatus = {};

    if ((root.fd < 0) || (::fstat(root.fd, &rootStatus) != 0) ||
        !safeDirectory(root.fd, rootStatus.st_dev))
    {
        setError(error, PrivacyDerivativeStoreError::UnsafeStore, detail,
                 QStringLiteral("the mounted derivative store is unsafe"));
        return {};
    }

    ScopedFd derivatives(openDirectoryAt(
        root.fd, QByteArrayLiteral("derivatives"), rootStatus.st_dev, false));
    ScopedFd item(openDirectoryAt(
        derivatives.fd, derivative.itemUuid.toUtf8(), rootStatus.st_dev, false));

    if ((derivatives.fd < 0) || (item.fd < 0))
    {
        setError(error, PrivacyDerivativeStoreError::IoFailure, detail,
                 QStringLiteral("the clear derivative is unavailable"));
        return {};
    }

    const QByteArray bytes = readExactFile(
        item.fd,
        QFile::encodeName(derivative.protectedRelativePath.section(
            QLatin1Char('/'), -1)),
        rootStatus.st_dev, derivative.derivativeSize,
        QByteArray::fromHex(derivative.derivativeHash.toLatin1()));

    if (bytes.isEmpty())
    {
        setError(error, PrivacyDerivativeStoreError::IntegrityFailure, detail,
                 QStringLiteral("the clear derivative failed verification"));
    }

    return bytes;
#endif
}

} // namespace Digikam

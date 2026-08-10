/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycheckoutstore.h"

// C++ includes

#include <algorithm>
#include <cerrno>
#include <utility>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QUuid>

#if defined(Q_OS_UNIX)

// POSIX includes

#   include <dirent.h>
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

constexpr qsizetype MaximumInventoryEntries = 16384;
constexpr int MaximumInventoryDepth = 64;

void setError(PrivacyCheckoutStoreError* const error,
              PrivacyCheckoutStoreError value, QString* const detail,
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

bool safeFileName(const QString& value)
{
    if (value.isEmpty() || (value == QLatin1String(".")) ||
        (value == QLatin1String("..")) || value.contains(QLatin1Char('/')) ||
        value.contains(QChar::Null) ||
        (value != value.normalized(QString::NormalizationForm_C)))
    {
        return false;
    }

    const QByteArray encoded = QFile::encodeName(value);
    return (!encoded.isEmpty() && (QFile::decodeName(encoded) == value));
}

QString namespaceName(PrivacyCheckoutStoreLocation location)
{
    return (location == PrivacyCheckoutStoreLocation::Checkout)
         ? QStringLiteral("checkouts") : QStringLiteral("recovery");
}

bool validLocation(PrivacyCheckoutStoreLocation location)
{
    return ((location == PrivacyCheckoutStoreLocation::Checkout) ||
            (location == PrivacyCheckoutStoreLocation::Recovery));
}

QByteArray inventoryHash(const PrivacyCheckoutInventory& inventory)
{
    if (!canonicalUuid(inventory.transactionUuid) ||
        !validLocation(inventory.location) ||
        (inventory.workRelativePath !=
         PrivacyCheckoutStore::workRelativePath(inventory.transactionUuid,
                                                 inventory.location)))
    {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(inventory.transactionUuid.toUtf8());
    hash.addData("\0", 1);
    hash.addData(QByteArray::number(static_cast<int>(inventory.location)));
    hash.addData("\0", 1);
    hash.addData(inventory.workRelativePath.toUtf8());
    hash.addData("\0", 1);
    QString previous;

    for (const PrivacyCheckoutInventoryEntry& entry : inventory.entries)
    {
        if (entry.storeRelativePath.isEmpty() ||
            (!previous.isEmpty() && (previous >= entry.storeRelativePath)) ||
            (entry.kind < PrivacyCheckoutEntryKind::RegularFile) ||
            (entry.kind > PrivacyCheckoutEntryKind::SymbolicLink) ||
            (entry.size < 0) || (entry.linkCount == 0) ||
            ((entry.kind == PrivacyCheckoutEntryKind::Directory) !=
             entry.sha256.isEmpty()) ||
            ((entry.kind != PrivacyCheckoutEntryKind::Directory) &&
             (entry.sha256.size() != 32)) ||
            !entry.storeRelativePath.startsWith(
                inventory.workRelativePath + QLatin1Char('/')))
        {
            return {};
        }

        previous = entry.storeRelativePath;
        hash.addData(entry.storeRelativePath.toUtf8());
        hash.addData("\0", 1);
        hash.addData(QByteArray::number(static_cast<int>(entry.kind)));
        hash.addData("\0", 1);
        hash.addData(QByteArray::number(entry.size));
        hash.addData("\0", 1);
        hash.addData(QByteArray::number(entry.linkCount));
        hash.addData("\0", 1);
        hash.addData(entry.sha256);
        hash.addData("\0", 1);
    }

    return hash.result();
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
        reset();
    }

    ScopedFd(ScopedFd&& other) noexcept
        : fd(other.release())
    {
    }

    ScopedFd& operator=(ScopedFd&& other) noexcept
    {
        if (this != &other)
        {
            reset(other.release());
        }

        return *this;
    }

    ScopedFd(const ScopedFd&)            = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    int release()
    {
        const int value = fd;
        fd = -1;
        return value;
    }

    void reset(int value = -1)
    {
        if (fd >= 0)
        {
            ::close(fd);
        }

        fd = value;
    }

    int fd = -1;
};

bool stableStat(const struct stat& left, const struct stat& right)
{
    return ((left.st_dev == right.st_dev) &&
            (left.st_ino == right.st_ino) &&
            (left.st_mode == right.st_mode) &&
            (left.st_uid == right.st_uid) &&
            (left.st_nlink == right.st_nlink) &&
            (left.st_size == right.st_size) &&
            (left.st_mtim.tv_sec == right.st_mtim.tv_sec) &&
            (left.st_mtim.tv_nsec == right.st_mtim.tv_nsec) &&
            (left.st_ctim.tv_sec == right.st_ctim.tv_sec) &&
            (left.st_ctim.tv_nsec == right.st_ctim.tv_nsec));
}

bool safeDirectory(int descriptor, dev_t device, bool privateMode)
{
    struct stat status = {};

    return ((descriptor >= 0) && (::fstat(descriptor, &status) == 0) &&
            S_ISDIR(status.st_mode) && (status.st_uid == geteuid()) &&
            (status.st_dev == device) &&
            (!privateMode ||
             ((status.st_mode & (S_IWGRP | S_IWOTH)) == 0)));
}

int openDirectoryAt(int parentFd, const QByteArray& component, dev_t device,
                    bool create, bool privateMode, bool* const created = nullptr)
{
    if (created)
    {
        *created = false;
    }

    int descriptor = PrivacyPosixStorage::confinedOpenAt(
        parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if ((descriptor < 0) && create && (errno == ENOENT))
    {
        if ((::mkdirat(parentFd, component.constData(), S_IRWXU) != 0) &&
            (errno != EEXIST))
        {
            return -1;
        }

        if (created)
        {
            *created = true;
        }

        descriptor = PrivacyPosixStorage::confinedOpenAt(
            parentFd, component, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    }

    if (descriptor < 0)
    {
        return -1;
    }

    if (!safeDirectory(descriptor, device, privateMode))
    {
        if (descriptor >= 0)
        {
            ::close(descriptor);
        }

        errno = EPERM;
        return -1;
    }

    if (created && *created && (::fsync(parentFd) != 0))
    {
        ::close(descriptor);
        return -1;
    }

    return descriptor;
}

bool sameForRemoval(const PrivacyCheckoutInventoryEntry& current,
                    const PrivacyCheckoutInventoryEntry& expected)
{
    if ((current.storeRelativePath != expected.storeRelativePath) ||
        (current.kind != expected.kind))
    {
        return false;
    }

    // Removing an already-authorized child changes its parent's directory
    // link count. The complete tree was compared before cleanup and rmdir()
    // still refuses a directory if an unrecorded child appears.
    return (current.kind == PrivacyCheckoutEntryKind::Directory)
         ? true : (current == expected);
}

bool readFileHash(int directoryFd, const QByteArray& name, dev_t device,
                  struct stat* const stableStatus, QByteArray* const sha256)
{
    if (!stableStatus || !sha256)
    {
        return false;
    }

    ScopedFd file(PrivacyPosixStorage::confinedOpenAt(
        directoryFd, name, O_RDONLY | O_CLOEXEC | O_NONBLOCK));
    struct stat before = {};

    if ((file.fd < 0) || (::fstat(file.fd, &before) != 0) ||
        !S_ISREG(before.st_mode) || (before.st_uid != geteuid()) ||
        (before.st_dev != device) || (before.st_size < 0))
    {
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer(1024 * 1024, Qt::Uninitialized);

    for (;;)
    {
        const ssize_t count = ::read(file.fd, buffer.data(),
                                     static_cast<size_t>(buffer.size()));

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return false;
        }

        if (count == 0)
        {
            break;
        }

        hash.addData(buffer.constData(), count);
    }

    struct stat after = {};

    if ((::fstat(file.fd, &after) != 0) || !stableStat(before, after))
    {
        return false;
    }

    *stableStatus = after;
    *sha256 = hash.result();
    return true;
}

bool readLinkHash(int directoryFd, const QByteArray& name, dev_t device,
                  struct stat* const stableStatus, QByteArray* const sha256)
{
    if (!stableStatus || !sha256)
    {
        return false;
    }

    struct stat before = {};

    if ((::fstatat(directoryFd, name.constData(), &before,
                   AT_SYMLINK_NOFOLLOW) != 0) ||
        !S_ISLNK(before.st_mode) || (before.st_uid != geteuid()) ||
        (before.st_dev != device) || (before.st_size < 0) ||
        (before.st_size > (1024 * 1024)))
    {
        return false;
    }

    QByteArray target(static_cast<qsizetype>(before.st_size) + 1,
                      Qt::Uninitialized);
    const ssize_t count = ::readlinkat(directoryFd, name.constData(),
                                       target.data(),
                                       static_cast<size_t>(target.size()));
    struct stat after = {};

    if ((count < 0) || (count != before.st_size) ||
        (::fstatat(directoryFd, name.constData(), &after,
                   AT_SYMLINK_NOFOLLOW) != 0) ||
        !stableStat(before, after))
    {
        return false;
    }

    target.resize(count);
    *stableStatus = after;
    *sha256 = QCryptographicHash::hash(target, QCryptographicHash::Sha256);
    return true;
}

bool evidenceAt(int directoryFd, const QByteArray& name, dev_t device,
                const QString& storeRelativePath,
                PrivacyCheckoutInventoryEntry* const entry,
                PrivacyCheckoutStoreError* const error,
                QString* const detail)
{
    struct stat status = {};

    if (!entry ||
        (::fstatat(directoryFd, name.constData(), &status,
                   AT_SYMLINK_NOFOLLOW) != 0))
    {
        setError(error, (errno == ENOENT) ? PrivacyCheckoutStoreError::Missing
                                         : PrivacyCheckoutStoreError::IoFailure,
                 detail, QStringLiteral("checkout entry cannot be inspected"));
        return false;
    }

    if ((status.st_uid != geteuid()) || (status.st_dev != device))
    {
        setError(error, PrivacyCheckoutStoreError::UnsafeStore, detail,
                 QStringLiteral("checkout entry escaped the mounted store identity"));
        return false;
    }

    entry->storeRelativePath = storeRelativePath;
    entry->size = 0;
    entry->linkCount = static_cast<quint64>(status.st_nlink);
    entry->sha256.clear();

    if (S_ISREG(status.st_mode))
    {
        QByteArray hash;

        if (!readFileHash(directoryFd, name, device, &status, &hash))
        {
            setError(error, PrivacyCheckoutStoreError::IntegrityFailure, detail,
                     QStringLiteral("checkout file changed while it was hashed"));
            return false;
        }

        entry->kind = PrivacyCheckoutEntryKind::RegularFile;
        entry->size = status.st_size;
        entry->linkCount = static_cast<quint64>(status.st_nlink);
        entry->sha256 = hash;
        return true;
    }

    if (S_ISDIR(status.st_mode))
    {
        entry->kind = PrivacyCheckoutEntryKind::Directory;
        return true;
    }

    if (S_ISLNK(status.st_mode))
    {
        QByteArray hash;

        if (!readLinkHash(directoryFd, name, device, &status, &hash))
        {
            setError(error, PrivacyCheckoutStoreError::IntegrityFailure, detail,
                     QStringLiteral("checkout link changed while it was inspected"));
            return false;
        }

        entry->kind = PrivacyCheckoutEntryKind::SymbolicLink;
        entry->size = status.st_size;
        entry->linkCount = static_cast<quint64>(status.st_nlink);
        entry->sha256 = hash;
        return true;
    }

    setError(error, PrivacyCheckoutStoreError::UnsafeStore, detail,
             QStringLiteral("checkout contains a special filesystem node"));
    return false;
}

bool listNames(int directoryFd, QList<QByteArray>* const names)
{
    if (!names)
    {
        return false;
    }

    names->clear();
    const int duplicate = ::dup(directoryFd);

    if (duplicate < 0)
    {
        return false;
    }

    DIR* const directory = ::fdopendir(duplicate);

    if (!directory)
    {
        ::close(duplicate);
        return false;
    }

    errno = 0;

    while (dirent* const item = ::readdir(directory))
    {
        const QByteArray name(item->d_name);

        if ((name == QByteArrayLiteral(".")) ||
            (name == QByteArrayLiteral("..")))
        {
            continue;
        }

        names->append(name);
    }

    const int readError = errno;
    ::closedir(directory);

    if (readError != 0)
    {
        names->clear();
        return false;
    }

    std::sort(names->begin(), names->end());
    return true;
}

bool inventoryDirectory(int directoryFd, dev_t device,
                        const QString& storeRelativeDirectory, int depth,
                        QList<PrivacyCheckoutInventoryEntry>* const entries,
                        PrivacyCheckoutStoreError* const error,
                        QString* const detail)
{
    struct stat before = {};

    if (!entries || (depth > MaximumInventoryDepth) ||
        (::fstat(directoryFd, &before) != 0) ||
        !S_ISDIR(before.st_mode) || (before.st_uid != geteuid()) ||
        (before.st_dev != device))
    {
        setError(error, PrivacyCheckoutStoreError::UnsafeStore, detail,
                 QStringLiteral("checkout directory is unsafe or too deep"));
        return false;
    }

    QList<QByteArray> names;

    if (!listNames(directoryFd, &names))
    {
        setError(error, PrivacyCheckoutStoreError::IoFailure, detail,
                 QStringLiteral("checkout directory cannot be enumerated"));
        return false;
    }

    for (const QByteArray& encodedName : std::as_const(names))
    {
        const QString name = QFile::decodeName(encodedName);

        if ((QFile::encodeName(name) != encodedName) || !safeFileName(name) ||
            (entries->size() >= MaximumInventoryEntries))
        {
            setError(error, PrivacyCheckoutStoreError::UnsafeStore, detail,
                     QStringLiteral("checkout contains an unrepresentable or excessive entry"));
            return false;
        }

        PrivacyCheckoutInventoryEntry entry;
        const QString relativePath = storeRelativeDirectory +
                                     QLatin1Char('/') + name;

        if (!evidenceAt(directoryFd, encodedName, device, relativePath,
                        &entry, error, detail))
        {
            return false;
        }

        entries->append(entry);

        if (entry.kind == PrivacyCheckoutEntryKind::Directory)
        {
            ScopedFd child(PrivacyPosixStorage::confinedOpenAt(
                directoryFd, encodedName,
                O_RDONLY | O_DIRECTORY | O_CLOEXEC));

            if (!safeDirectory(child.fd, device, false) ||
                !inventoryDirectory(child.fd, device, relativePath, depth + 1,
                                    entries, error, detail))
            {
                if (detail && detail->isEmpty())
                {
                    *detail = QStringLiteral("nested checkout directory is unsafe");
                }

                return false;
            }
        }
    }

    struct stat after = {};

    if ((::fstat(directoryFd, &after) != 0) || !stableStat(before, after))
    {
        setError(error, PrivacyCheckoutStoreError::IntegrityFailure, detail,
                 QStringLiteral("checkout changed while it was inventoried"));
        return false;
    }

    return true;
}

#endif

} // namespace

bool PrivacyCheckoutInventoryEntry::operator==(
    const PrivacyCheckoutInventoryEntry& other) const
{
    return ((storeRelativePath == other.storeRelativePath) &&
            (kind == other.kind) && (size == other.size) &&
            (linkCount == other.linkCount) && (sha256 == other.sha256));
}

bool PrivacyCheckoutInventory::operator==(
    const PrivacyCheckoutInventory& other) const
{
    return ((transactionUuid == other.transactionUuid) &&
            (location == other.location) &&
            (workRelativePath == other.workRelativePath) &&
            (entries == other.entries) && (sha256 == other.sha256));
}

class Q_DECL_HIDDEN PrivacyCheckoutStore::Private
{
public:

#if defined(Q_OS_UNIX)

    ScopedFd root;
    dev_t device = 0;

    int openNamespace(PrivacyCheckoutStoreLocation location, bool create,
                      PrivacyCheckoutStoreError* const error,
                      QString* const detail) const
    {
        bool created = false;
        const int descriptor = openDirectoryAt(
            root.fd, namespaceName(location).toUtf8(), device, create, true,
            &created);

        if (descriptor < 0)
        {
            setError(error, create ? PrivacyCheckoutStoreError::UnsafeStore
                                   : ((errno == ENOENT)
                                      ? PrivacyCheckoutStoreError::Missing
                                      : PrivacyCheckoutStoreError::UnsafeStore),
                     detail, QStringLiteral("checkout namespace is unavailable or unsafe"));
        }

        return descriptor;
    }

    int openTransaction(const QString& transactionUuid,
                        PrivacyCheckoutStoreLocation location, bool create,
                        PrivacyCheckoutStoreError* const error,
                        QString* const detail) const
    {
        ScopedFd namespaceFd(openNamespace(location, create, error, detail));

        if (namespaceFd.fd < 0)
        {
            return -1;
        }

        bool created = false;
        const int descriptor = openDirectoryAt(
            namespaceFd.fd, transactionUuid.toUtf8(), device, create, true,
            &created);

        if (descriptor < 0)
        {
            setError(error, create ? PrivacyCheckoutStoreError::UnsafeStore
                                   : ((errno == ENOENT)
                                      ? PrivacyCheckoutStoreError::Missing
                                      : PrivacyCheckoutStoreError::UnsafeStore),
                     detail, QStringLiteral("checkout transaction directory is unavailable or unsafe"));
        }

        return descriptor;
    }

    int openWork(const QString& transactionUuid,
                 PrivacyCheckoutStoreLocation location, bool create,
                 PrivacyCheckoutStoreError* const error,
                 QString* const detail) const
    {
        ScopedFd transaction(openTransaction(transactionUuid, location, create,
                                             error, detail));

        if (transaction.fd < 0)
        {
            return -1;
        }

        bool created = false;
        const int descriptor = openDirectoryAt(
            transaction.fd, QByteArrayLiteral("work"), device, create, true,
            &created);

        if (descriptor < 0)
        {
            setError(error, create ? PrivacyCheckoutStoreError::UnsafeStore
                                   : ((errno == ENOENT)
                                      ? PrivacyCheckoutStoreError::Missing
                                      : PrivacyCheckoutStoreError::UnsafeStore),
                     detail, QStringLiteral("checkout work directory is unavailable or unsafe"));
        }

        return descriptor;
    }

    int openRelativeParent(const QString& workRelativePath,
                           const QString& entryRelativePath,
                           QByteArray* const leaf,
                           PrivacyCheckoutStoreError* const error,
                           QString* const detail) const
    {
        const QString prefix = workRelativePath + QLatin1Char('/');

        if (!leaf || !entryRelativePath.startsWith(prefix))
        {
            setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                     QStringLiteral("inventory entry is outside its work directory"));
            return -1;
        }

        const QString suffix = entryRelativePath.mid(prefix.size());
        const QStringList components = suffix.split(QLatin1Char('/'));

        if (components.isEmpty())
        {
            setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                     QStringLiteral("inventory entry path is incomplete"));
            return -1;
        }

        const QStringList workComponents = workRelativePath.split(QLatin1Char('/'));
        ScopedFd current(::dup(root.fd));

        for (const QString& component : workComponents)
        {
            if (!safeFileName(component))
            {
                setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                         QStringLiteral("work path is not canonical"));
                return -1;
            }

            ScopedFd next(openDirectoryAt(current.fd, component.toUtf8(),
                                          device, false, true));

            if (next.fd < 0)
            {
                setError(error, PrivacyCheckoutStoreError::Missing, detail,
                         QStringLiteral("work path is unavailable"));
                return -1;
            }

            current = std::move(next);
        }

        for (int index = 0 ; index + 1 < components.size() ; ++index)
        {
            if (!safeFileName(components.at(index)))
            {
                setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                         QStringLiteral("inventory path component is unsafe"));
                return -1;
            }

            ScopedFd next(openDirectoryAt(current.fd,
                                          components.at(index).toUtf8(),
                                          device, false, false));

            if (next.fd < 0)
            {
                setError(error, PrivacyCheckoutStoreError::Missing, detail,
                         QStringLiteral("inventory parent is unavailable"));
                return -1;
            }

            current = std::move(next);
        }

        if (!safeFileName(components.constLast()))
        {
            setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                     QStringLiteral("inventory leaf is unsafe"));
            return -1;
        }

        *leaf = components.constLast().toUtf8();
        return current.release();
    }

#endif

    QString plaintextRoot;
};

PrivacyCheckoutStore::PrivacyCheckoutStore()
    : d(new Private)
{
}

PrivacyCheckoutStore::~PrivacyCheckoutStore() = default;

PrivacyCheckoutStore::PrivacyCheckoutStore(PrivacyCheckoutStore&& other) noexcept = default;

PrivacyCheckoutStore& PrivacyCheckoutStore::operator=(PrivacyCheckoutStore&& other) noexcept = default;

std::unique_ptr<PrivacyCheckoutStore> PrivacyCheckoutStore::open(
    const QString& plaintextRoot, PrivacyCheckoutStoreError* const error,
    QString* const detail)
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!QDir::isAbsolutePath(plaintextRoot) ||
        (QDir::cleanPath(plaintextRoot) != plaintextRoot))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("mounted checkout root path is invalid"));
        return {};
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return {};
#else
    ScopedFd root(::open(QFile::encodeName(plaintextRoot).constData(),
                         O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    struct stat status = {};

    if ((root.fd < 0) || (::fstat(root.fd, &status) != 0) ||
        !safeDirectory(root.fd, status.st_dev, true))
    {
        setError(error, PrivacyCheckoutStoreError::UnsafeStore, detail,
                 QStringLiteral("mounted checkout store is unavailable or unsafe"));
        return {};
    }

    std::unique_ptr<PrivacyCheckoutStore> result(new PrivacyCheckoutStore);
    result->d->root = std::move(root);
    result->d->device = status.st_dev;
    result->d->plaintextRoot = plaintextRoot;
    return result;
#endif
}

QString PrivacyCheckoutStore::transactionRelativePath(
    const QString& transactionUuid, PrivacyCheckoutStoreLocation location)
{
    if (!canonicalUuid(transactionUuid) || !validLocation(location))
    {
        return {};
    }

    return namespaceName(location) + QLatin1Char('/') + transactionUuid;
}

QString PrivacyCheckoutStore::workRelativePath(
    const QString& transactionUuid, PrivacyCheckoutStoreLocation location)
{
    const QString transaction = transactionRelativePath(transactionUuid, location);
    return transaction.isEmpty() ? QString()
                                 : (transaction + QLatin1String("/work"));
}

QString PrivacyCheckoutStore::workFileRelativePath(
    const QString& transactionUuid, const QString& fileName,
    PrivacyCheckoutStoreLocation location)
{
    const QString work = workRelativePath(transactionUuid, location);
    return (work.isEmpty() || !safeFileName(fileName))
         ? QString() : (work + QLatin1Char('/') + fileName);
}

bool PrivacyCheckoutStore::createOrOpenTransaction(
    const QString& transactionUuid, QString* const relativeWorkPath,
    PrivacyCheckoutStoreError* const error, QString* const detail)
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!canonicalUuid(transactionUuid))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("checkout transaction UUID is invalid"));
        return false;
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return false;
#else
    ScopedFd work(d->openWork(transactionUuid,
                              PrivacyCheckoutStoreLocation::Checkout, true,
                              error, detail));

    if (work.fd < 0)
    {
        return false;
    }

    if (relativeWorkPath)
    {
        *relativeWorkPath = workRelativePath(transactionUuid);
    }

    return true;
#endif
}

bool PrivacyCheckoutStore::reopenTransaction(
    const QString& transactionUuid, PrivacyCheckoutStoreLocation location,
    QString* const relativeWorkPath,
    PrivacyCheckoutStoreError* const error, QString* const detail) const
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!canonicalUuid(transactionUuid) || !validLocation(location))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("checkout reopen request is invalid"));
        return false;
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return false;
#else
    ScopedFd work(d->openWork(transactionUuid, location, false, error, detail));

    if (work.fd < 0)
    {
        return false;
    }

    if (relativeWorkPath)
    {
        *relativeWorkPath = workRelativePath(transactionUuid, location);
    }

    return true;
#endif
}

bool PrivacyCheckoutStore::createFile(
    const QString& transactionUuid, const QString& fileName,
    qint64 expectedSize, const QByteArray& expectedSha256,
    const FileProducer& producer, QString* const storeRelativePath,
    PrivacyCheckoutStoreError* const error, QString* const detail)
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});
    const QString relative = workFileRelativePath(transactionUuid, fileName);

    if (relative.isEmpty() || (expectedSize < 0) ||
        (expectedSha256.size() != 32) || !producer)
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("checkout file request is invalid"));
        return false;
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return false;
#else
    if (!createOrOpenTransaction(transactionUuid, nullptr, error, detail))
    {
        return false;
    }

    ScopedFd work(d->openWork(transactionUuid,
                              PrivacyCheckoutStoreLocation::Checkout, false,
                              error, detail));
    const QByteArray encodedName = QFile::encodeName(fileName);
    PrivacyCheckoutInventoryEntry existing;

    if (work.fd < 0)
    {
        return false;
    }

    if (evidenceAt(work.fd, encodedName, d->device, relative, &existing,
                   nullptr, nullptr))
    {
        if ((existing.kind == PrivacyCheckoutEntryKind::RegularFile) &&
            (existing.size == expectedSize) &&
            (existing.sha256 == expectedSha256))
        {
            if (storeRelativePath)
            {
                *storeRelativePath = relative;
            }

            return true;
        }

        setError(error, PrivacyCheckoutStoreError::Conflict, detail,
                 QStringLiteral("existing checkout file differs from its baseline"));
        return false;
    }

    struct stat status = {};

    if ((::fstatat(work.fd, encodedName.constData(), &status,
                   AT_SYMLINK_NOFOLLOW) == 0) || (errno != ENOENT))
    {
        setError(error, PrivacyCheckoutStoreError::Conflict, detail,
                 QStringLiteral("checkout destination cannot be exclusively created"));
        return false;
    }

    ScopedFd output(PrivacyPosixStorage::confinedOpenAt(
        work.fd, encodedName, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC,
        S_IRUSR | S_IWUSR));
    QString producerDetail;

    if ((output.fd < 0) || !producer(output.fd, &producerDetail) ||
        (::fchmod(output.fd, S_IRUSR | S_IWUSR) != 0) ||
        (::fsync(output.fd) != 0))
    {
        output.reset();
        (void)::unlinkat(work.fd, encodedName.constData(), 0);
        (void)::fsync(work.fd);
        setError(error, PrivacyCheckoutStoreError::IoFailure, detail,
                 producerDetail.isEmpty()
                     ? QStringLiteral("checkout file could not be materialized")
                     : producerDetail);
        return false;
    }

    output.reset();
    PrivacyCheckoutInventoryEntry written;

    if (!evidenceAt(work.fd, encodedName, d->device, relative, &written,
                    error, detail) ||
        (written.kind != PrivacyCheckoutEntryKind::RegularFile) ||
        (written.size != expectedSize) ||
        (written.sha256 != expectedSha256) || (::fsync(work.fd) != 0))
    {
        (void)::unlinkat(work.fd, encodedName.constData(), 0);
        (void)::fsync(work.fd);

        if (error && (*error == PrivacyCheckoutStoreError::None))
        {
            setError(error, PrivacyCheckoutStoreError::IntegrityFailure, detail,
                     QStringLiteral("materialized checkout does not match its baseline"));
        }

        return false;
    }

    if (storeRelativePath)
    {
        *storeRelativePath = relative;
    }

    return true;
#endif
}

bool PrivacyCheckoutStore::inventory(
    const QString& transactionUuid, PrivacyCheckoutStoreLocation location,
    PrivacyCheckoutInventory* const result,
    PrivacyCheckoutStoreError* const error, QString* const detail) const
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!result || !canonicalUuid(transactionUuid) || !validLocation(location))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("checkout inventory request is invalid"));
        return false;
    }

    *result = PrivacyCheckoutInventory();

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return false;
#else
    ScopedFd work(d->openWork(transactionUuid, location, false, error, detail));

    if (work.fd < 0)
    {
        return false;
    }

    PrivacyCheckoutInventory value;
    value.transactionUuid = transactionUuid;
    value.location = location;
    value.workRelativePath = workRelativePath(transactionUuid, location);

    if (!inventoryDirectory(work.fd, d->device, value.workRelativePath, 0,
                            &value.entries, error, detail))
    {
        return false;
    }

    std::sort(value.entries.begin(), value.entries.end(),
              [](const PrivacyCheckoutInventoryEntry& left,
                 const PrivacyCheckoutInventoryEntry& right)
              {
                  return left.storeRelativePath < right.storeRelativePath;
              });
    value.sha256 = inventoryHash(value);

    if (value.sha256.isEmpty())
    {
        setError(error, PrivacyCheckoutStoreError::IntegrityFailure, detail,
                 QStringLiteral("checkout inventory could not be canonicalized"));
        return false;
    }

    *result = value;
    return true;
#endif
}

bool PrivacyCheckoutStore::validateInventory(
    const PrivacyCheckoutInventory& expected,
    PrivacyCheckoutStoreError* const error, QString* const detail) const
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});
    const QByteArray canonicalHash = inventoryHash(expected);

    if (canonicalHash.isEmpty() || (canonicalHash != expected.sha256))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("expected checkout inventory is invalid"));
        return false;
    }

    PrivacyCheckoutInventory current;

    if (!inventory(expected.transactionUuid, expected.location, &current,
                   error, detail))
    {
        return false;
    }

    if (!(current == expected))
    {
        setError(error, PrivacyCheckoutStoreError::Conflict, detail,
                 QStringLiteral("checkout inventory changed"));
        return false;
    }

    return true;
}

QString PrivacyCheckoutStore::runtimePathForEntry(
    const PrivacyCheckoutInventory& inventoryValue,
    const QString& storeRelativePath,
    PrivacyCheckoutStoreError* const error, QString* const detail) const
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!validateInventory(inventoryValue, error, detail))
    {
        return {};
    }

    const auto found = std::find_if(
        inventoryValue.entries.cbegin(), inventoryValue.entries.cend(),
        [&storeRelativePath](const PrivacyCheckoutInventoryEntry& entry)
        {
            return (entry.storeRelativePath == storeRelativePath);
        });

    if ((found == inventoryValue.entries.cend()) ||
        (found->kind != PrivacyCheckoutEntryKind::RegularFile))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("runtime checkout path is not an inventory-owned regular file"));
        return {};
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return {};
#else
    QByteArray leaf;
    ScopedFd parent(d->openRelativeParent(
        inventoryValue.workRelativePath, storeRelativePath, &leaf,
        error, detail));
    PrivacyCheckoutInventoryEntry current;

    if ((parent.fd < 0) ||
        !evidenceAt(parent.fd, leaf, d->device, storeRelativePath,
                    &current, error, detail) || !(current == *found))
    {
        if (error && (*error == PrivacyCheckoutStoreError::None))
        {
            setError(error, PrivacyCheckoutStoreError::Conflict, detail,
                     QStringLiteral("runtime checkout entry changed during resolution"));
        }

        return {};
    }

    // The store root remains private; this absolute spelling is an ephemeral
    // launch capability tied to the live authenticated mount and store object.
    return QDir(d->plaintextRoot).absoluteFilePath(storeRelativePath);
#endif
}

bool PrivacyCheckoutStore::removeExact(
    const PrivacyCheckoutInventory& expected,
    PrivacyCheckoutStoreError* const error, QString* const detail)
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!validateInventory(expected, error, detail))
    {
        return false;
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("descriptor-confined checkout storage requires Unix"));
    return false;
#else
    QList<PrivacyCheckoutInventoryEntry> removal = expected.entries;
    std::sort(removal.begin(), removal.end(),
              [](const PrivacyCheckoutInventoryEntry& left,
                 const PrivacyCheckoutInventoryEntry& right)
              {
                  const int leftDepth = left.storeRelativePath.count(QLatin1Char('/'));
                  const int rightDepth = right.storeRelativePath.count(QLatin1Char('/'));
                  return (leftDepth != rightDepth)
                       ? (leftDepth > rightDepth)
                       : (left.storeRelativePath > right.storeRelativePath);
              });

    for (const PrivacyCheckoutInventoryEntry& entry : std::as_const(removal))
    {
        QByteArray leaf;
        ScopedFd parent(d->openRelativeParent(
            expected.workRelativePath, entry.storeRelativePath, &leaf,
            error, detail));
        PrivacyCheckoutInventoryEntry current;

        if ((parent.fd < 0) ||
            !evidenceAt(parent.fd, leaf, d->device, entry.storeRelativePath,
                        &current, error, detail) ||
            !sameForRemoval(current, entry))
        {
            if (error && (*error == PrivacyCheckoutStoreError::None))
            {
                setError(error, PrivacyCheckoutStoreError::Conflict, detail,
                         QStringLiteral("checkout changed during exact removal"));
            }

            return false;
        }

        const int flags = (entry.kind == PrivacyCheckoutEntryKind::Directory)
                        ? AT_REMOVEDIR : 0;

        if ((::unlinkat(parent.fd, leaf.constData(), flags) != 0) ||
            (::fsync(parent.fd) != 0))
        {
            setError(error, PrivacyCheckoutStoreError::IoFailure, detail,
                     QStringLiteral("exact checkout removal was interrupted"));
            return false;
        }
    }

    ScopedFd namespaceFd(d->openNamespace(expected.location, false,
                                           error, detail));
    ScopedFd transaction(d->openTransaction(expected.transactionUuid,
                                             expected.location, false,
                                             error, detail));

    if ((namespaceFd.fd < 0) || (transaction.fd < 0) ||
        (::unlinkat(transaction.fd, "work", AT_REMOVEDIR) != 0) ||
        (::fsync(transaction.fd) != 0) ||
        (::unlinkat(namespaceFd.fd, expected.transactionUuid.toUtf8().constData(),
                    AT_REMOVEDIR) != 0) || (::fsync(namespaceFd.fd) != 0))
    {
        setError(error, PrivacyCheckoutStoreError::IoFailure, detail,
                 QStringLiteral("checkout directory cleanup was interrupted"));
        return false;
    }

    return true;
#endif
}

bool PrivacyCheckoutStore::moveToRecovery(
    const QString& transactionUuid, QString* const recoveryRelativePath,
    PrivacyCheckoutStoreError* const error, QString* const detail)
{
    setError(error, PrivacyCheckoutStoreError::None, detail, {});

    if (!canonicalUuid(transactionUuid))
    {
        setError(error, PrivacyCheckoutStoreError::InvalidRequest, detail,
                 QStringLiteral("checkout preservation UUID is invalid"));
        return false;
    }

#if !defined(Q_OS_UNIX)
    setError(error, PrivacyCheckoutStoreError::UnsupportedPlatform, detail,
             QStringLiteral("atomic checkout preservation requires Unix"));
    return false;
#else
    if (!reopenTransaction(transactionUuid,
                           PrivacyCheckoutStoreLocation::Checkout,
                           nullptr, error, detail))
    {
        return false;
    }

    ScopedFd checkout(d->openNamespace(PrivacyCheckoutStoreLocation::Checkout,
                                        false, error, detail));
    ScopedFd recovery(d->openNamespace(PrivacyCheckoutStoreLocation::Recovery,
                                        true, error, detail));

    if ((checkout.fd < 0) || (recovery.fd < 0))
    {
        return false;
    }

    const QByteArray name = transactionUuid.toUtf8();
    struct stat existing = {};

    if ((::fstatat(recovery.fd, name.constData(), &existing,
                   AT_SYMLINK_NOFOLLOW) == 0) || (errno != ENOENT))
    {
        setError(error, PrivacyCheckoutStoreError::Conflict, detail,
                 QStringLiteral("a preserved checkout already exists"));
        return false;
    }

    bool unavailable = false;

    if (!PrivacyPosixStorage::atomicRenameAt(
            checkout.fd, name, recovery.fd, name,
            PrivacyPosixStorage::AtomicRenameMode::NoReplace, &unavailable))
    {
        setError(error, unavailable ? PrivacyCheckoutStoreError::UnsupportedPlatform
                                    : PrivacyCheckoutStoreError::IoFailure,
                 detail, unavailable
                     ? QStringLiteral("atomic no-replace preservation is unavailable")
                     : QStringLiteral("checkout could not be atomically preserved"));
        return false;
    }

    if ((::fsync(checkout.fd) != 0) || (::fsync(recovery.fd) != 0) ||
        !reopenTransaction(transactionUuid,
                           PrivacyCheckoutStoreLocation::Recovery,
                           nullptr, error, detail))
    {
        setError(error, PrivacyCheckoutStoreError::IoFailure, detail,
                 QStringLiteral("preserved checkout move is durability-uncertain"));
        return false;
    }

    if (recoveryRelativePath)
    {
        *recoveryRelativePath = transactionRelativePath(
            transactionUuid, PrivacyCheckoutStoreLocation::Recovery);
    }

    return true;
#endif
}

} // namespace Digikam

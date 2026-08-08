/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyposixfilesystemadapter.h"
#include "storage/privacyposixstorage_p.h"

// C++ includes

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <utility>

#if defined(Q_OS_LINUX) && !defined(DIGIKAM_POSIX_ADAPTER_FORCE_STUB)

// POSIX includes

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/openat2.h>
#include <sys/syscall.h>

// Qt includes

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QSet>

#endif // Linux implementation

namespace Digikam
{

#if defined(Q_OS_LINUX) && !defined(DIGIKAM_POSIX_ADAPTER_FORCE_STUB)

namespace
{

class ScopedFd
{
public:

    explicit ScopedFd(int descriptor = -1)
        : m_descriptor(descriptor)
    {
    }

    ~ScopedFd()
    {
        if (m_descriptor >= 0)
        {
            ::close(m_descriptor);
        }
    }

    int get() const
    {
        return m_descriptor;
    }

    int release()
    {
        const int descriptor = m_descriptor;
        m_descriptor         = -1;

        return descriptor;
    }

    void reset(int descriptor = -1)
    {
        if (m_descriptor >= 0)
        {
            ::close(m_descriptor);
        }

        m_descriptor = descriptor;
    }

private:

    int m_descriptor = -1;

    Q_DISABLE_COPY(ScopedFd)
};

struct DirectorySnapshot
{
    bool        complete = false;
    QStringList names;
    struct stat stableStat = {};
};

struct HardlinkScanState
{
    bool                                  complete = true;
    qsizetype                             totalEntries = 0;
    qsizetype                             directories = 0;
    QSet<QString>                         visitedDirectories;
    QHash<quint64, QList<PrivacyInventoryAliasCandidate> > candidatesByInode;
};

bool isSafeRelativePath(const QString& path, bool allowEmpty)
{
    if (path.isEmpty())
    {
        return allowEmpty;
    }

    if (QDir::isAbsolutePath(path) || path.contains(QChar::Null) ||
        path.contains(QLatin1Char('\\')) || (QDir::cleanPath(path) != path))
    {
        return false;
    }

    const QStringList components = path.split(QLatin1Char('/'));

    for (const QString& component : components)
    {
        if (component.isEmpty() || (component == QLatin1String(".")) ||
            (component == QLatin1String("..")))
        {
            return false;
        }

        for (const QChar character : component)
        {
            if (character.unicode() < 0x20U)
            {
                return false;
            }
        }
    }

    return true;
}

bool exactUtf8(const QString& text, QByteArray* const bytes)
{
    if (!bytes)
    {
        return false;
    }

    *bytes = text.toUtf8();

    return (QString::fromUtf8(*bytes) == text) && !bytes->contains('\0');
}

bool sameRoot(const PrivacyInventoryRoot& left, const PrivacyInventoryRoot& right)
{
    return (left.uuid == right.uuid) && (left.absolutePath == right.absolutePath);
}

bool pathContains(const QString& parent, const QString& child)
{
    if (parent == child)
    {
        return true;
    }

    const QString prefix = parent.endsWith(QLatin1Char('/'))
                         ? parent
                         : (parent + QLatin1Char('/'));

    return child.startsWith(prefix);
}

QString statIdentity(const struct stat& facts)
{
    return QString::number(static_cast<qulonglong>(facts.st_dev)) +
           QLatin1Char(':') +
           QString::number(static_cast<qulonglong>(facts.st_ino));
}

bool sameDirectorySnapshot(const struct stat& left, const struct stat& right)
{
    return (left.st_dev == right.st_dev)       &&
           (left.st_ino == right.st_ino)       &&
           (left.st_mode == right.st_mode)     &&
           (left.st_nlink == right.st_nlink)   &&
           (left.st_size == right.st_size)     &&
           (left.st_mtim.tv_sec == right.st_mtim.tv_sec)   &&
           (left.st_mtim.tv_nsec == right.st_mtim.tv_nsec) &&
           (left.st_ctim.tv_sec == right.st_ctim.tv_sec)   &&
           (left.st_ctim.tv_nsec == right.st_ctim.tv_nsec);
}

PrivacyInventoryFileType fileType(mode_t mode)
{
    if (S_ISREG(mode))
    {
        return PrivacyInventoryFileType::Regular;
    }

    if (S_ISDIR(mode))
    {
        return PrivacyInventoryFileType::Directory;
    }

    if (S_ISLNK(mode))
    {
        return PrivacyInventoryFileType::Symlink;
    }

    return PrivacyInventoryFileType::Special;
}

PrivacyInventoryFileEvidence evidenceFromStat(const struct stat& facts)
{
    PrivacyInventoryFileEvidence evidence;
    evidence.type             = fileType(facts.st_mode);
    evidence.identityComplete = !S_ISLNK(facts.st_mode);
    evidence.deviceId         = static_cast<quint64>(facts.st_dev);
    evidence.inode            = static_cast<quint64>(facts.st_ino);
    evidence.linkCount        = static_cast<quint64>(facts.st_nlink);
    evidence.byteSize         = static_cast<qlonglong>(facts.st_size);

    return evidence;
}

int openDirectoryComponent(int parentFd, const QByteArray& component,
                           dev_t expectedDevice)
{
    const int descriptor = PrivacyPosixStorage::confinedOpenAt(
        parentFd, component,
        O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);

    if (descriptor < 0)
    {
        return -1;
    }

    struct stat facts = {};

    if ((::fstat(descriptor, &facts) != 0) || !S_ISDIR(facts.st_mode) ||
        (facts.st_dev != expectedDevice))
    {
        ::close(descriptor);
        return -1;
    }

    return descriptor;
}

int openPathComponent(int parentFd, const QByteArray& component)
{
    int descriptor = -1;

#if defined(SYS_openat2)

    struct open_how how = {};
    how.flags   = O_PATH | O_CLOEXEC | O_NOFOLLOW;
    how.resolve = RESOLVE_BENEATH | RESOLVE_NO_MAGICLINKS | RESOLVE_NO_SYMLINKS;
    descriptor = static_cast<int>(::syscall(SYS_openat2, parentFd,
                                            component.constData(),
                                            &how, sizeof(how)));

    if ((descriptor < 0) && (errno != ENOSYS) && (errno != EINVAL) &&
        (errno != E2BIG))
    {
        return -1;
    }

#endif

    if (descriptor < 0)
    {
        descriptor = ::openat(parentFd, component.constData(),
                              O_PATH | O_CLOEXEC | O_NOFOLLOW);
    }

    return descriptor;
}

bool compareLocations(const PrivacyInventoryAliasCandidate& left,
                      const PrivacyInventoryAliasCandidate& right)
{
    const int uuid = QString::compare(left.location.root.uuid,
                                      right.location.root.uuid,
                                      Qt::CaseSensitive);

    if (uuid != 0)
    {
        return (uuid < 0);
    }

    return (QString::compare(left.location.relativePath,
                             right.location.relativePath,
                             Qt::CaseSensitive) < 0);
}

} // namespace

class PrivacyPosixFilesystemAdapter::Private
{
public:

    Private(const QList<PrivacyPosixRootScope>& rootScopes,
            const PrivacyPosixScanLimits& scanLimits,
            const PrivacyPosixInventoryControl* inventoryControl)
        : roots(rootScopes),
          limits(scanLimits),
          control(inventoryControl)
    {
        configurationValid = limits.isValid() && !roots.isEmpty();

        QSet<QString> uuids;
        QSet<QString> paths;

        for (const PrivacyPosixRootScope& scope : std::as_const(roots))
        {
            if (!scope.isValid() || uuids.contains(scope.root.uuid) ||
                paths.contains(scope.root.absolutePath))
            {
                configurationValid = false;
            }

            uuids.insert(scope.root.uuid);
            paths.insert(scope.root.absolutePath);
        }

        for (qsizetype left = 0 ; left < roots.size() ; ++left)
        {
            for (qsizetype right = left + 1 ; right < roots.size() ; ++right)
            {
                if ((roots.at(left).expectedDeviceId == roots.at(right).expectedDeviceId) &&
                    ((roots.at(left).expectedInode == roots.at(right).expectedInode) ||
                     pathContains(roots.at(left).root.absolutePath,
                                  roots.at(right).root.absolutePath) ||
                     pathContains(roots.at(right).root.absolutePath,
                                  roots.at(left).root.absolutePath)))
                {
                    configurationValid = false;
                }
            }
        }
    }

    bool canceled() const
    {
        return control && control->isCanceled();
    }

    void checkpoint(PrivacyPosixCheckpoint point,
                    const PrivacyInventoryRoot& root,
                    const QString& relativePath = QString()) const
    {
        if (control)
        {
            control->checkpoint(point, root, relativePath);
        }
    }

    const PrivacyPosixRootScope* scopeFor(const PrivacyInventoryRoot& root) const
    {
        for (const PrivacyPosixRootScope& scope : roots)
        {
            if (sameRoot(scope.root, root))
            {
                return &scope;
            }
        }

        return nullptr;
    }

    int openVerifiedRoot(const PrivacyPosixRootScope& scope) const
    {
        if (canceled())
        {
            return -1;
        }

        checkpoint(PrivacyPosixCheckpoint::BeforeRootOpen, scope.root);

        QByteArray path;

        if (!exactUtf8(scope.root.absolutePath, &path))
        {
            return -1;
        }

        const int descriptor = ::open(path.constData(),
                                      O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);

        if (descriptor < 0)
        {
            return -1;
        }

        struct stat facts = {};

        if ((::fstat(descriptor, &facts) != 0) || !S_ISDIR(facts.st_mode) ||
            (static_cast<quint64>(facts.st_dev) != scope.expectedDeviceId) ||
            (static_cast<quint64>(facts.st_ino) != scope.expectedInode))
        {
            ::close(descriptor);
            return -1;
        }

        checkpoint(PrivacyPosixCheckpoint::AfterRootOpen, scope.root);

        if (canceled())
        {
            ::close(descriptor);
            return -1;
        }

        return descriptor;
    }

    bool revalidateRoot(const PrivacyPosixRootScope& scope) const
    {
        checkpoint(PrivacyPosixCheckpoint::BeforeRootRevalidate, scope.root);

        if (canceled())
        {
            return false;
        }

        QByteArray path;

        if (!exactUtf8(scope.root.absolutePath, &path))
        {
            return false;
        }

        ScopedFd descriptor(::open(path.constData(),
                                   O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
        struct stat facts = {};

        return (descriptor.get() >= 0) && (::fstat(descriptor.get(), &facts) == 0) &&
               S_ISDIR(facts.st_mode) &&
               (static_cast<quint64>(facts.st_dev) == scope.expectedDeviceId) &&
               (static_cast<quint64>(facts.st_ino) == scope.expectedInode);
    }

    int openDirectoryPath(int rootFd,
                          const PrivacyPosixRootScope& scope,
                          const QString& relativeDirectory) const
    {
        int current = ::dup(rootFd);

        if (current < 0)
        {
            return -1;
        }

        if (relativeDirectory.isEmpty())
        {
            return current;
        }

        const QStringList components = relativeDirectory.split(QLatin1Char('/'));

        for (const QString& component : components)
        {
            if (canceled())
            {
                ::close(current);
                return -1;
            }

            QByteArray name;

            if (!exactUtf8(component, &name))
            {
                ::close(current);
                return -1;
            }

            const int next = openDirectoryComponent(current, name,
                                                    static_cast<dev_t>(scope.expectedDeviceId));
            ::close(current);

            if (next < 0)
            {
                return -1;
            }

            current = next;
        }

        return current;
    }

    DirectorySnapshot readDirectoryStable(int directoryFd,
                                          const PrivacyPosixRootScope& scope,
                                          const QString& relativeDirectory) const
    {
        DirectorySnapshot snapshot;
        struct stat before = {};

        if (canceled() || (::fstat(directoryFd, &before) != 0) ||
            !S_ISDIR(before.st_mode) ||
            (static_cast<quint64>(before.st_dev) != scope.expectedDeviceId))
        {
            return snapshot;
        }

        checkpoint(PrivacyPosixCheckpoint::BeforeDirectoryRead,
                   scope.root, relativeDirectory);

        const int duplicate = ::dup(directoryFd);

        if (duplicate < 0)
        {
            return snapshot;
        }

        DIR* const directory = ::fdopendir(duplicate);

        if (!directory)
        {
            ::close(duplicate);
            return snapshot;
        }

        bool readComplete = true;

        while (true)
        {
            errno = 0;
            dirent* const entry = ::readdir(directory);

            if (!entry)
            {
                if (errno != 0)
                {
                    readComplete = false;
                }

                break;
            }

            if (canceled())
            {
                readComplete = false;
                break;
            }

            const QByteArray rawName(entry->d_name);

            if ((rawName == ".") || (rawName == ".."))
            {
                continue;
            }

            if (snapshot.names.size() >= limits.maximumEntriesPerDirectory)
            {
                readComplete = false;
                break;
            }

            const QString name = QString::fromUtf8(rawName);

            if (name.toUtf8() != rawName)
            {
                readComplete = false;
                break;
            }

            snapshot.names << name;
        }

        ::closedir(directory);
        snapshot.names.sort(Qt::CaseSensitive);

        checkpoint(PrivacyPosixCheckpoint::AfterDirectoryRead,
                   scope.root, relativeDirectory);

        struct stat after = {};
        const bool stable = (::fstat(directoryFd, &after) == 0) &&
                            sameDirectorySnapshot(before, after);
        snapshot.complete   = readComplete && stable && !canceled();
        snapshot.stableStat = after;

        return snapshot;
    }

    bool scanDirectory(const PrivacyPosixRootScope& scope,
                       int directoryFd,
                       const QString& relativeDirectory,
                       int depth,
                       quint64 targetDevice,
                       const QSet<quint64>& targetInodes,
                       HardlinkScanState* const state) const
    {
        if (!state || canceled() || (depth > limits.maximumDepth))
        {
            return false;
        }

        struct stat directoryFacts = {};

        if ((::fstat(directoryFd, &directoryFacts) != 0) ||
            !S_ISDIR(directoryFacts.st_mode) ||
            (static_cast<quint64>(directoryFacts.st_dev) != scope.expectedDeviceId))
        {
            return false;
        }

        const QString directoryIdentity = statIdentity(directoryFacts);

        if (state->visitedDirectories.contains(directoryIdentity))
        {
            return false;
        }

        state->visitedDirectories.insert(directoryIdentity);
        ++state->directories;

        if (state->directories > limits.maximumDirectoriesPerRoot)
        {
            return false;
        }

        const DirectorySnapshot snapshot = readDirectoryStable(directoryFd,
                                                               scope,
                                                               relativeDirectory);

        if (!snapshot.complete)
        {
            return false;
        }

        for (const QString& name : snapshot.names)
        {
            if (canceled() || (++state->totalEntries > limits.maximumEntriesTotal))
            {
                return false;
            }

            QByteArray encodedName;

            if (!exactUtf8(name, &encodedName))
            {
                return false;
            }

            struct stat facts = {};

            if (::fstatat(directoryFd, encodedName.constData(),
                          &facts, AT_SYMLINK_NOFOLLOW) != 0)
            {
                return false;
            }

            const QString relativePath = relativeDirectory.isEmpty()
                                       ? name
                                       : (relativeDirectory + QLatin1Char('/') + name);

            if (S_ISREG(facts.st_mode))
            {
                const quint64 inode = static_cast<quint64>(facts.st_ino);

                if ((static_cast<quint64>(facts.st_dev) == targetDevice) &&
                    targetInodes.contains(inode))
                {
                    PrivacyInventoryAliasCandidate candidate;
                    candidate.kind                  = PrivacyInventoryAliasKind::HardlinkAlias;
                    candidate.location.root         = scope.root;
                    candidate.location.relativePath = relativePath;
                    state->candidatesByInode[inode] << candidate;
                }

                continue;
            }

            if (!S_ISDIR(facts.st_mode) ||
                (static_cast<quint64>(facts.st_dev) != scope.expectedDeviceId))
            {
                continue;
            }

            ScopedFd child(openDirectoryComponent(directoryFd, encodedName,
                                                   static_cast<dev_t>(scope.expectedDeviceId)));
            struct stat openedFacts = {};

            if ((child.get() < 0) || (::fstat(child.get(), &openedFacts) != 0) ||
                (openedFacts.st_dev != facts.st_dev) ||
                (openedFacts.st_ino != facts.st_ino) ||
                !S_ISDIR(openedFacts.st_mode) ||
                !scanDirectory(scope, child.get(), relativePath, depth + 1,
                               targetDevice, targetInodes, state))
            {
                return false;
            }
        }

        struct stat finalFacts = {};

        return (::fstat(directoryFd, &finalFacts) == 0) &&
               sameDirectorySnapshot(snapshot.stableStat, finalFacts);
    }

public:

    QList<PrivacyPosixRootScope>       roots;
    PrivacyPosixScanLimits             limits;
    const PrivacyPosixInventoryControl* control = nullptr;
    bool                               configurationValid = false;
};

bool PrivacyPosixRootScope::isValid() const
{
    return root.isValid() && (expectedInode > 0) &&
           (static_cast<quint64>(static_cast<dev_t>(expectedDeviceId)) == expectedDeviceId) &&
           (static_cast<quint64>(static_cast<ino_t>(expectedInode)) == expectedInode);
}

bool PrivacyPosixScanLimits::isValid() const
{
    return (maximumEntriesPerDirectory > 0) &&
           (maximumEntriesTotal > 0) &&
           (maximumDirectoriesPerRoot > 0) &&
           (maximumDepth >= 0);
}

PrivacyPosixFilesystemAdapter::PrivacyPosixFilesystemAdapter(
    const QList<PrivacyPosixRootScope>& roots,
    const PrivacyPosixScanLimits& limits,
    const PrivacyPosixInventoryControl* control)
    : d(new Private(roots, limits, control))
{
}

PrivacyPosixFilesystemAdapter::~PrivacyPosixFilesystemAdapter() = default;

bool PrivacyPosixFilesystemAdapter::isConfigurationValid() const
{
    return d->configurationValid;
}

PrivacyInventoryFileEvidence PrivacyPosixFilesystemAdapter::inspect(
    const PrivacyInventoryLocation& location) const
{
    PrivacyInventoryFileEvidence evidence;

    if (!d->configurationValid || !location.isValid() || d->canceled())
    {
        return evidence;
    }

    const PrivacyPosixRootScope* const scope = d->scopeFor(location.root);

    if (!scope)
    {
        return evidence;
    }

    ScopedFd rootFd(d->openVerifiedRoot(*scope));

    if (rootFd.get() < 0)
    {
        return evidence;
    }

    const QFileInfo pathInfo(location.relativePath);
    const QString directory = (pathInfo.path() == QLatin1String("."))
                            ? QString()
                            : pathInfo.path();
    ScopedFd parentFd(d->openDirectoryPath(rootFd.get(), *scope, directory));

    if (parentFd.get() < 0)
    {
        return evidence;
    }

    QByteArray name;

    if (!exactUtf8(pathInfo.fileName(), &name))
    {
        return evidence;
    }

    struct stat before = {};

    if (::fstatat(parentFd.get(), name.constData(), &before,
                  AT_SYMLINK_NOFOLLOW) != 0)
    {
        return evidence;
    }

    evidence = evidenceFromStat(before);

    if (evidence.type == PrivacyInventoryFileType::Symlink)
    {
        return evidence;
    }

    ScopedFd opened(openPathComponent(parentFd.get(), name));
    struct stat after = {};

    if ((opened.get() < 0) || (::fstat(opened.get(), &after) != 0) ||
        (before.st_dev != after.st_dev) || (before.st_ino != after.st_ino) ||
        (before.st_mode != after.st_mode) || !d->revalidateRoot(*scope))
    {
        evidence.identityComplete = false;
        return evidence;
    }

    return evidenceFromStat(after);
}

PrivacyInventoryDirectoryEvidence PrivacyPosixFilesystemAdapter::listDirectory(
    const PrivacyInventoryRoot& root,
    const QString& relativeDirectory) const
{
    PrivacyInventoryDirectoryEvidence evidence;

    if (!d->configurationValid || !root.isValid() ||
        !isSafeRelativePath(relativeDirectory, true) || d->canceled())
    {
        return evidence;
    }

    const PrivacyPosixRootScope* const scope = d->scopeFor(root);

    if (!scope)
    {
        return evidence;
    }

    ScopedFd rootFd(d->openVerifiedRoot(*scope));
    ScopedFd directoryFd((rootFd.get() < 0)
                       ? -1
                       : d->openDirectoryPath(rootFd.get(), *scope, relativeDirectory));

    if (directoryFd.get() < 0)
    {
        return evidence;
    }

    const DirectorySnapshot snapshot = d->readDirectoryStable(directoryFd.get(),
                                                              *scope,
                                                              relativeDirectory);
    evidence.entryNames = snapshot.names;
    evidence.complete   = snapshot.complete && d->revalidateRoot(*scope);

    return evidence;
}

PrivacyInventoryAliasEvidence PrivacyPosixFilesystemAdapter::hardlinkAliases(
    quint64 deviceId,
    quint64 inode) const
{
    PrivacyInventoryFileIdentity identity;
    identity.deviceId = deviceId;
    identity.inode    = inode;
    const QList<PrivacyInventoryHardlinkEvidence> results =
        hardlinkAliasesFor({ identity });

    return results.isEmpty() ? PrivacyInventoryAliasEvidence()
                             : results.constFirst().aliases;
}

QList<PrivacyInventoryHardlinkEvidence>
PrivacyPosixFilesystemAdapter::hardlinkAliasesFor(
    const QList<PrivacyInventoryFileIdentity>& identities) const
{
    QList<PrivacyInventoryHardlinkEvidence> results;
    QHash<quint64, QSet<quint64> > targetsByDevice;

    for (const PrivacyInventoryFileIdentity& identity : identities)
    {
        PrivacyInventoryHardlinkEvidence result;
        result.identity = identity;
        results << result;

        if (identity.isValid())
        {
            targetsByDevice[identity.deviceId].insert(identity.inode);
        }
    }

    if (!d->configurationValid || d->canceled())
    {
        return results;
    }

    QHash<quint64, bool> completenessByDevice;
    QHash<quint64, QHash<quint64, QList<PrivacyInventoryAliasCandidate> > >
        candidatesByDeviceAndInode;
    qsizetype globalEntries = 0;

    QList<quint64> devices = targetsByDevice.keys();
    std::sort(devices.begin(), devices.end());

    for (quint64 deviceId : std::as_const(devices))
    {
        HardlinkScanState state;
        state.totalEntries = globalEntries;
        bool foundMatchingScope = false;

        for (const PrivacyPosixRootScope& scope : std::as_const(d->roots))
        {
            if (scope.expectedDeviceId != deviceId)
            {
                continue;
            }

            foundMatchingScope       = true;
            state.directories        = 0;
            state.visitedDirectories.clear();

            if (!scope.includeInHardlinkEnumeration || d->canceled())
            {
                state.complete = false;
                continue;
            }

            ScopedFd rootFd(d->openVerifiedRoot(scope));

            if ((rootFd.get() < 0) ||
                !d->scanDirectory(scope, rootFd.get(), QString(), 0,
                                  deviceId, targetsByDevice.value(deviceId), &state) ||
                !d->revalidateRoot(scope))
            {
                state.complete = false;
            }
        }

        if (!foundMatchingScope)
        {
            state.complete = false;
        }

        globalEntries = state.totalEntries;
        completenessByDevice.insert(deviceId, state.complete && !d->canceled());
        candidatesByDeviceAndInode.insert(deviceId, state.candidatesByInode);
    }

    for (PrivacyInventoryHardlinkEvidence& result : results)
    {
        if (!result.identity.isValid())
        {
            continue;
        }

        result.aliases.complete = completenessByDevice.value(result.identity.deviceId, false) &&
                                  !d->canceled();
        result.aliases.candidates = candidatesByDeviceAndInode
            .value(result.identity.deviceId)
            .value(result.identity.inode);
        auto& candidates = result.aliases.candidates;
        std::sort(candidates.begin(), candidates.end(), compareLocations);
        candidates.erase(std::unique(candidates.begin(), candidates.end(),
                                     [](const PrivacyInventoryAliasCandidate& left,
                                        const PrivacyInventoryAliasCandidate& right)
                                     {
                                         return sameRoot(left.location.root,
                                                         right.location.root) &&
                                                (left.location.relativePath ==
                                                 right.location.relativePath);
                                     }),
                         candidates.end());
    }

    return results;
}

#else

class PrivacyPosixFilesystemAdapter::Private
{
};

bool PrivacyPosixRootScope::isValid() const
{
    return root.isValid() && (expectedInode > 0);
}

bool PrivacyPosixScanLimits::isValid() const
{
    return (maximumEntriesPerDirectory > 0) &&
           (maximumEntriesTotal > 0) &&
           (maximumDirectoriesPerRoot > 0) &&
           (maximumDepth >= 0);
}

PrivacyPosixFilesystemAdapter::PrivacyPosixFilesystemAdapter(
    const QList<PrivacyPosixRootScope>&,
    const PrivacyPosixScanLimits&,
    const PrivacyPosixInventoryControl*)
    : d(new Private)
{
}

PrivacyPosixFilesystemAdapter::~PrivacyPosixFilesystemAdapter() = default;

bool PrivacyPosixFilesystemAdapter::isConfigurationValid() const
{
    return false;
}

PrivacyInventoryFileEvidence PrivacyPosixFilesystemAdapter::inspect(
    const PrivacyInventoryLocation&) const
{
    return PrivacyInventoryFileEvidence();
}

PrivacyInventoryDirectoryEvidence PrivacyPosixFilesystemAdapter::listDirectory(
    const PrivacyInventoryRoot&,
    const QString&) const
{
    return PrivacyInventoryDirectoryEvidence();
}

PrivacyInventoryAliasEvidence PrivacyPosixFilesystemAdapter::hardlinkAliases(
    quint64,
    quint64) const
{
    return PrivacyInventoryAliasEvidence();
}

QList<PrivacyInventoryHardlinkEvidence>
PrivacyPosixFilesystemAdapter::hardlinkAliasesFor(
    const QList<PrivacyInventoryFileIdentity>& identities) const
{
    QList<PrivacyInventoryHardlinkEvidence> results;

    for (const PrivacyInventoryFileIdentity& identity : identities)
    {
        PrivacyInventoryHardlinkEvidence result;
        result.identity = identity;
        results << result;
    }

    return results;
}

#endif // Linux implementation

} // namespace Digikam

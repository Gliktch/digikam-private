/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacymanagedrootprovisioner.h"

// Qt includes

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QUuid>

// C++ includes

#include <cerrno>

#ifdef Q_OS_UNIX

// POSIX includes

#   include <dirent.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <unistd.h>

#endif

// Local includes

#include "privacycontracts.h"
#include "privacyrootidentity_p.h"
#include "storage/privacyposixstorage_p.h"

namespace Digikam
{

bool PrivacyManagedRootProvisionResult::succeeded() const
{
    return ((status == PrivacyManagedRootProvisionStatus::ReadyExisting) ||
            (status == PrivacyManagedRootProvisionStatus::ReadyCreated));
}

bool PrivacyManagedRootProvisionResult::createdMarker() const
{
    return m_createdMarker;
}

namespace
{

constexpr qint64 markerMaximumBytes = 4096;
constexpr auto metadataDirectoryName = ".digikam-private";
constexpr auto markerFileName = "root-marker-v1.json";
constexpr auto markerCandidatePrefix = "root-marker-v1.json.tmp-";

#ifdef Q_OS_UNIX

class ScopedFileDescriptor
{
public:

    explicit ScopedFileDescriptor(int descriptor = -1)
        : m_descriptor(descriptor)
    {
    }

    ~ScopedFileDescriptor()
    {
        reset();
    }

    int get() const
    {
        return m_descriptor;
    }

    int release()
    {
        const int descriptor = m_descriptor;
        m_descriptor = -1;

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

    Q_DISABLE_COPY(ScopedFileDescriptor)

private:

    int m_descriptor = -1;
};

enum class MarkerReadState
{
    Missing,
    Read,
    Invalid
};

QString uuidText()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool writeAll(int descriptor, const QByteArray& bytes)
{
    qint64 written = 0;

    while (written < bytes.size())
    {
        const ssize_t count = ::write(descriptor, bytes.constData() + written,
                                      static_cast<size_t>(bytes.size() - written));

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
            return false;
        }

        written += count;
    }

    return true;
}

bool isOwnedPrivateDirectory(const struct stat& value,
                             dev_t expectedDevice = 0,
                             bool requireDevice = false)
{
    return (S_ISDIR(value.st_mode) && (value.st_uid == geteuid()) &&
            ((value.st_mode & S_IRWXU) == S_IRWXU) &&
            ((value.st_mode & (S_IWGRP | S_IWOTH)) == 0) &&
            (!requireDevice || (value.st_dev == expectedDevice)));
}

bool openAbsoluteDirectory(const QString& cleanPath,
                           ScopedFileDescriptor* const descriptor,
                           int* const openError)
{
    if (!descriptor || !QDir::isAbsolutePath(cleanPath))
    {
        return false;
    }

    ScopedFileDescriptor current(::open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC));

    if (current.get() < 0)
    {
        if (openError)
        {
            *openError = errno;
        }

        return false;
    }

    for (const QString& part : cleanPath.split(QLatin1Char('/'), Qt::SkipEmptyParts))
    {
        const QByteArray encoded = QFile::encodeName(part);

        if (encoded.isEmpty() || encoded.contains('/'))
        {
            if (openError)
            {
                *openError = EINVAL;
            }

            return false;
        }

        const int next = ::openat(current.get(), encoded.constData(),
                                  O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);

        if (next < 0)
        {
            if (openError)
            {
                *openError = errno;
            }

            return false;
        }

        current.reset(next);
    }

    descriptor->reset(current.release());

    return true;
}

MarkerReadState readMarker(int metadataDescriptor, dev_t rootDevice,
                           QByteArray* const markerData,
                           struct stat* const markerStat)
{
    ScopedFileDescriptor marker(PrivacyPosixStorage::confinedOpenAt(
        metadataDescriptor, QByteArray(markerFileName),
        O_RDONLY | O_CLOEXEC));

    if (marker.get() < 0)
    {
        return (errno == ENOENT) ? MarkerReadState::Missing
                                 : MarkerReadState::Invalid;
    }

    struct stat value = {};

    if ((::fstat(marker.get(), &value) != 0) || !S_ISREG(value.st_mode) ||
        (value.st_uid != geteuid()) || (value.st_dev != rootDevice) ||
        (value.st_nlink != 1) || ((value.st_mode & (S_IWGRP | S_IWOTH)) != 0) ||
        (value.st_size <= 0) || (value.st_size > markerMaximumBytes))
    {
        return MarkerReadState::Invalid;
    }

    QByteArray bytes(static_cast<int>(value.st_size), Qt::Uninitialized);
    qint64 readTotal = 0;

    while (readTotal < bytes.size())
    {
        const ssize_t count = ::read(marker.get(), bytes.data() + readTotal,
                                     static_cast<size_t>(bytes.size() - readTotal));

        if (count < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            return MarkerReadState::Invalid;
        }

        if (count == 0)
        {
            return MarkerReadState::Invalid;
        }

        readTotal += count;
    }

    char extra = 0;
    ssize_t extraCount = -1;

    do
    {
        extraCount = ::read(marker.get(), &extra, 1);
    }
    while ((extraCount < 0) && (errno == EINTR));

    if (extraCount != 0)
    {
        return MarkerReadState::Invalid;
    }

    if (markerData)
    {
        *markerData = bytes;
    }

    if (markerStat)
    {
        *markerStat = value;
    }

    return MarkerReadState::Read;
}

bool metadataContainsOnlyExpectedEntries(int metadataDescriptor,
                                         dev_t rootDevice,
                                         bool allowFinalMarker,
                                         bool allowCandidates)
{
    const int duplicate = ::dup(metadataDescriptor);

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

    bool valid = true;
    errno = 0;

    while (const dirent* const entry = ::readdir(directory))
    {
        const QByteArray name(entry->d_name);

        if ((name == QByteArrayLiteral(".")) || (name == QByteArrayLiteral("..")) ||
            (allowFinalMarker && (name == QByteArray(markerFileName))))
        {
            continue;
        }

        if (allowCandidates &&
            name.startsWith(QByteArray(markerCandidatePrefix)))
        {
            ScopedFileDescriptor candidate(PrivacyPosixStorage::confinedOpenAt(
                metadataDescriptor, name, O_RDONLY | O_CLOEXEC));
            struct stat candidateStat = {};

            if ((candidate.get() >= 0) &&
                (::fstat(candidate.get(), &candidateStat) == 0) &&
                S_ISREG(candidateStat.st_mode) &&
                (candidateStat.st_uid == geteuid()) &&
                (candidateStat.st_dev == rootDevice) &&
                (candidateStat.st_nlink == 1) &&
                ((candidateStat.st_mode & S_IRWXU) == (S_IRUSR | S_IWUSR)) &&
                ((candidateStat.st_mode & (S_IRWXG | S_IRWXO)) == 0) &&
                (candidateStat.st_size >= 0) &&
                (candidateStat.st_size <= markerMaximumBytes))
            {
                continue;
            }
        }

        valid = false;
        break;
    }

    if (errno != 0)
    {
        valid = false;
    }

    ::closedir(directory);

    return valid;
}

bool publishMarker(int metadataDescriptor, const QByteArray& markerData,
                   bool* const created, QString* const detail)
{
    const QByteArray candidateName = QByteArray(markerCandidatePrefix) +
                                     uuidText().toLatin1();
    ScopedFileDescriptor candidate(PrivacyPosixStorage::confinedOpenAt(
        metadataDescriptor, candidateName,
        O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR));

    if (candidate.get() < 0)
    {
        *detail = QLatin1String("Cannot create the managed-root marker candidate");
        return false;
    }

    struct stat candidateStat = {};

    if ((::fchmod(candidate.get(), S_IRUSR | S_IWUSR) != 0) ||
        (::fstat(candidate.get(), &candidateStat) != 0) ||
        !S_ISREG(candidateStat.st_mode) || (candidateStat.st_uid != geteuid()) ||
        (candidateStat.st_nlink != 1) ||
        ((candidateStat.st_mode & S_IRWXU) != (S_IRUSR | S_IWUSR)) ||
        ((candidateStat.st_mode & (S_IRWXG | S_IRWXO)) != 0) ||
        !writeAll(candidate.get(), markerData) || (::fsync(candidate.get()) != 0))
    {
        ::unlinkat(metadataDescriptor, candidateName.constData(), 0);
        *detail = QLatin1String("Cannot durably write the managed-root marker candidate");
        return false;
    }

    candidate.reset();
    bool unavailable = false;

    if (!PrivacyPosixStorage::atomicRenameAt(
            metadataDescriptor, candidateName,
            QByteArray(markerFileName),
            PrivacyPosixStorage::AtomicRenameMode::NoReplace, &unavailable))
    {
        const int renameError = errno;

        if (unavailable)
        {
            if (::linkat(metadataDescriptor, candidateName.constData(),
                         metadataDescriptor, markerFileName, 0) == 0)
            {
                if (::unlinkat(metadataDescriptor, candidateName.constData(), 0) != 0)
                {
                    ::unlinkat(metadataDescriptor, markerFileName, 0);
                    *detail = QLatin1String("Cannot finalize the managed-root marker link");
                    return false;
                }

                *created = true;
            }
            else if (errno != EEXIST)
            {
                ::unlinkat(metadataDescriptor, candidateName.constData(), 0);
                *detail = QLatin1String("Cannot publish the managed-root marker");
                return false;
            }
        }
        else if (renameError != EEXIST)
        {
            ::unlinkat(metadataDescriptor, candidateName.constData(), 0);
            *detail = QLatin1String("Cannot publish the managed-root marker");
            return false;
        }

        if (!*created)
        {
            ::unlinkat(metadataDescriptor, candidateName.constData(), 0);
        }
    }
    else
    {
        *created = true;
    }

    if (::fsync(metadataDescriptor) != 0)
    {
        *detail = QLatin1String("Cannot fsync the managed-root marker directory");
        return false;
    }

    return true;
}

bool sameDescriptorObject(int firstDescriptor, int secondDescriptor)
{
    struct stat first = {};
    struct stat second = {};

    return ((::fstat(firstDescriptor, &first) == 0) &&
            (::fstat(secondDescriptor, &second) == 0) &&
            (first.st_dev == second.st_dev) && (first.st_ino == second.st_ino));
}

#endif

} // namespace

PrivacyManagedRootProvisionResult PrivacyManagedRootProvisioner::provision(
    const QString& configuredPath)
{
    PrivacyManagedRootProvisionResult result;
    const QString cleanPath = QDir::cleanPath(configuredPath);

    if (configuredPath.contains(QChar::Null) || !QDir::isAbsolutePath(cleanPath) ||
        (cleanPath == QDir::rootPath()))
    {
        result.detail = QLatin1String("The store root must be an existing absolute directory");
        return result;
    }

#ifndef Q_OS_UNIX

    result.status = PrivacyManagedRootProvisionStatus::UnsupportedPlatform;
    result.detail = QLatin1String("Managed-root provisioning requires Unix descriptor operations");
    return result;

#else

    ScopedFileDescriptor rootDescriptor;
    int openError = 0;

    if (!openAbsoluteDirectory(cleanPath, &rootDescriptor, &openError))
    {
        result.status = (openError == ENOENT)
                      ? PrivacyManagedRootProvisionStatus::PathUnavailable
                      : PrivacyManagedRootProvisionStatus::UnsafeRoot;
        result.detail = QLatin1String("The selected store root is unavailable or unsafe");
        return result;
    }

    struct stat rootStat = {};

    if ((::fstat(rootDescriptor.get(), &rootStat) != 0) ||
        !isOwnedPrivateDirectory(rootStat))
    {
        result.status = PrivacyManagedRootProvisionStatus::UnsafeRoot;
        result.detail = QLatin1String("The selected store root must be owned by this user and not group- or world-writable");
        return result;
    }

    bool createdMetadataDirectory = false;
    ScopedFileDescriptor metadataDescriptor(PrivacyPosixStorage::confinedOpenAt(
        rootDescriptor.get(), QByteArray(metadataDirectoryName),
        O_RDONLY | O_DIRECTORY | O_CLOEXEC));

    if ((metadataDescriptor.get() < 0) && (errno == ENOENT))
    {
        if (::mkdirat(rootDescriptor.get(), metadataDirectoryName,
                      S_IRWXU) == 0)
        {
            createdMetadataDirectory = true;

            if ((::fchmodat(rootDescriptor.get(), metadataDirectoryName,
                            S_IRWXU, 0) != 0) ||
                (::fsync(rootDescriptor.get()) != 0))
            {
                result.status = PrivacyManagedRootProvisionStatus::IoFailure;
                result.detail = QLatin1String("Cannot fsync the selected store root");
                return result;
            }
        }
        else if (errno != EEXIST)
        {
            result.status = PrivacyManagedRootProvisionStatus::IoFailure;
            result.detail = QLatin1String("Cannot create the private metadata directory");
            return result;
        }

        metadataDescriptor.reset(PrivacyPosixStorage::confinedOpenAt(
            rootDescriptor.get(), QByteArray(metadataDirectoryName),
            O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    }

    struct stat metadataStat = {};

    if ((metadataDescriptor.get() < 0) ||
        (::fstat(metadataDescriptor.get(), &metadataStat) != 0) ||
        !isOwnedPrivateDirectory(metadataStat, rootStat.st_dev, true))
    {
        result.status = PrivacyManagedRootProvisionStatus::UnsafeRoot;
        result.detail = QLatin1String("The private metadata directory is unsafe");
        return result;
    }

    QByteArray markerData;
    MarkerReadState markerState = readMarker(metadataDescriptor.get(), rootStat.st_dev,
                                             &markerData, nullptr);
    bool createdMarker = false;

    if (markerState == MarkerReadState::Missing)
    {
        if (!metadataContainsOnlyExpectedEntries(metadataDescriptor.get(),
                                                 rootStat.st_dev, false, true))
        {
            result.status = PrivacyManagedRootProvisionStatus::InvalidMarker;
            result.detail = QLatin1String("The private metadata directory is not an unclaimed root");
            return result;
        }

        const QString rootUuid = uuidText();
        const QString markerUuid = uuidText();
        const QByteArray candidateData =
            PrivacyRootIdentityCodec::encodeManagedRootMarkerV1(rootUuid, markerUuid);

        if (candidateData.isEmpty() ||
            !publishMarker(metadataDescriptor.get(), candidateData,
                           &createdMarker, &result.detail))
        {
            result.status = PrivacyManagedRootProvisionStatus::IoFailure;
            return result;
        }

        markerState = readMarker(metadataDescriptor.get(), rootStat.st_dev,
                                 &markerData, nullptr);

        if ((markerState != MarkerReadState::Read) ||
            (createdMarker && (markerData != candidateData)))
        {
            result.status = PrivacyManagedRootProvisionStatus::InvalidMarker;
            result.detail = QLatin1String("The published managed-root marker changed unexpectedly");
            return result;
        }
    }
    else if (markerState != MarkerReadState::Read)
    {
        result.status = PrivacyManagedRootProvisionStatus::InvalidMarker;
        result.detail = QLatin1String("The existing managed-root marker is unsafe");
        return result;
    }

    QString rootUuid;
    QString markerUuid;

    if (!PrivacyRootIdentityCodec::decodeManagedRootMarkerV1(
            markerData, &rootUuid, &markerUuid))
    {
        result.status = PrivacyManagedRootProvisionStatus::InvalidMarker;
        result.detail = QLatin1String("The existing managed-root marker is malformed");
        return result;
    }

    ScopedFileDescriptor reopenedRoot;

    if (!openAbsoluteDirectory(cleanPath, &reopenedRoot, nullptr) ||
        !sameDescriptorObject(rootDescriptor.get(), reopenedRoot.get()))
    {
        if (createdMarker)
        {
            ::unlinkat(metadataDescriptor.get(), markerFileName, 0);
            ::fsync(metadataDescriptor.get());
        }

        result.status = PrivacyManagedRootProvisionStatus::UnsafeRoot;
        result.detail = QLatin1String("The selected store root changed during provisioning");
        return result;
    }

    result.root.uuid = rootUuid;
    result.root.kind = PrivacyStorageRootKind::ManagedStoreRoot;
    result.root.configuredPath = cleanPath;
    result.root.identityVersion = 1;
    result.root.identityData = PrivacyRootIdentityCodec::encodeManagedRootV1(
        markerUuid,
        PrivacyRootIdentityInternal::filesystemUuidForDevice(rootStat.st_dev));
    result.root.markerUuid = markerUuid;
    result.root.createdAt = QDateTime::currentDateTimeUtc();

    if (!result.root.isValid())
    {
        result.status = PrivacyManagedRootProvisionStatus::InvalidMarker;
        result.detail = QLatin1String("The managed-root identity could not be constructed");
        return result;
    }

    result.status = createdMarker
                  ? PrivacyManagedRootProvisionStatus::ReadyCreated
                  : PrivacyManagedRootProvisionStatus::ReadyExisting;
    result.m_createdMarker = createdMarker;
    result.m_createdMetadataDirectory = createdMetadataDirectory;
    result.m_markerData = markerData;
    result.m_rootDevice = static_cast<quint64>(rootStat.st_dev);
    result.m_rootInode = static_cast<quint64>(rootStat.st_ino);
    result.m_metadataDevice = static_cast<quint64>(metadataStat.st_dev);
    result.m_metadataInode = static_cast<quint64>(metadataStat.st_ino);

    return result;

#endif
}

bool PrivacyManagedRootProvisioner::rollbackUnused(
    const PrivacyManagedRootProvisionResult& result)
{
#ifndef Q_OS_UNIX

    Q_UNUSED(result);
    return false;

#else

    if (!result.succeeded() || !result.m_createdMarker ||
        result.root.configuredPath.isEmpty() || result.m_markerData.isEmpty())
    {
        return false;
    }

    ScopedFileDescriptor rootDescriptor;

    if (!openAbsoluteDirectory(result.root.configuredPath, &rootDescriptor, nullptr))
    {
        return false;
    }

    struct stat rootStat = {};

    if ((::fstat(rootDescriptor.get(), &rootStat) != 0) ||
        (static_cast<quint64>(rootStat.st_dev) != result.m_rootDevice) ||
        (static_cast<quint64>(rootStat.st_ino) != result.m_rootInode))
    {
        return false;
    }

    ScopedFileDescriptor metadataDescriptor(PrivacyPosixStorage::confinedOpenAt(
        rootDescriptor.get(), QByteArray(metadataDirectoryName),
        O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    struct stat metadataStat = {};

    if ((metadataDescriptor.get() < 0) ||
        (::fstat(metadataDescriptor.get(), &metadataStat) != 0) ||
        (static_cast<quint64>(metadataStat.st_dev) != result.m_metadataDevice) ||
        (static_cast<quint64>(metadataStat.st_ino) != result.m_metadataInode) ||
        !metadataContainsOnlyExpectedEntries(metadataDescriptor.get(),
                                             rootStat.st_dev, true, false))
    {
        return false;
    }

    QByteArray markerData;

    if ((readMarker(metadataDescriptor.get(), rootStat.st_dev,
                    &markerData, nullptr) != MarkerReadState::Read) ||
        (markerData != result.m_markerData) ||
        (::unlinkat(metadataDescriptor.get(), markerFileName, 0) != 0) ||
        (::fsync(metadataDescriptor.get()) != 0))
    {
        return false;
    }

    if (result.m_createdMetadataDirectory)
    {
        if ((::unlinkat(rootDescriptor.get(), metadataDirectoryName,
                        AT_REMOVEDIR) == 0) &&
            (::fsync(rootDescriptor.get()) != 0))
        {
            return false;
        }
    }

    return true;

#endif
}

} // namespace Digikam

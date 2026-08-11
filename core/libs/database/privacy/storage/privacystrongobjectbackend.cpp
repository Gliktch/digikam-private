/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacystrongobjectbackend.h"

// C++ includes

#include <functional>

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#ifdef Q_OS_UNIX

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

bool confinedRelativePath(const QString& relativePath)
{
    if (relativePath.isEmpty() || QDir::isAbsolutePath(relativePath) ||
        relativePath.contains(QLatin1Char('\0')))
    {
        return false;
    }

    const QStringList parts = relativePath.split(QLatin1Char('/'));

    for (const QString& part : parts)
    {
        if (part.isEmpty() || (part == QLatin1String(".")) ||
            (part == QLatin1String("..")))
        {
            return false;
        }
    }

    return true;
}

bool ancestorChainIsSafe(const QString& root, const QString& relativePath)
{
    QString current = QDir(root).filePath(relativePath);
    const QString rootPath = QDir(root).absolutePath();

    while (current != rootPath)
    {
        const QFileInfo info(current);

        if (info.isSymLink())
        {
            return false;
        }

        const QString parent = info.absolutePath();

        if ((parent == current) || parent.isEmpty())
        {
            return false;
        }

        current = parent;
    }

    return true;
}

bool pathIsUnder(const QString& root, const QString& path)
{
    const QString rootCanonical = QDir(root).canonicalPath();
    const QString pathCanonical = QFileInfo(path).canonicalFilePath();

    if (rootCanonical.isEmpty() || pathCanonical.isEmpty())
    {
        return false;
    }

    return (pathCanonical == rootCanonical) ||
           pathCanonical.startsWith(rootCanonical + QLatin1Char('/'));
}

bool fsyncPath(const QString& path)
{
#ifdef Q_OS_UNIX
    const int fd = ::open(QFile::encodeName(path).constData(), O_RDONLY);

    if (fd < 0)
    {
        return false;
    }

    const bool synced = (::fsync(fd) == 0);
    ::close(fd);
    return synced;
#else
    Q_UNUSED(path);
    return true;
#endif
}

bool copyStream(const QString& sourcePath, const QString& targetPath,
                QString* const error)
{
    QFile source(sourcePath);

    if (!source.open(QIODevice::ReadOnly))
    {
        if (error)
        {
            *error = QString::fromLatin1("cannot open source object: %1").arg(sourcePath);
        }

        return false;
    }

    QFile target(targetPath);

    if (!target.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (error)
        {
            *error = QString::fromLatin1("cannot create target object: %1").arg(targetPath);
        }

        return false;
    }

    QByteArray buffer;
    buffer.resize(1024 * 1024);

    while (!source.atEnd())
    {
        const qint64 read = source.read(buffer.data(), buffer.size());

        if ((read <= 0) || (target.write(buffer.constData(), read) != read))
        {
            if (error)
            {
                *error = QString::fromLatin1("cannot copy object bytes: %1").arg(targetPath);
            }

            return false;
        }
    }

    if (!target.flush() || !fsyncPath(targetPath))
    {
        if (error)
        {
            *error = QString::fromLatin1("cannot flush object: %1").arg(targetPath);
        }

        return false;
    }

    return true;
}

bool fileFacts(const QString& path, QByteArray* const sha256, qlonglong* const size)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    QByteArray buffer;
    buffer.resize(1024 * 1024);
    qlonglong total = 0;

    while (!file.atEnd())
    {
        const qint64 read = file.read(buffer.data(), buffer.size());

        if (read <= 0)
        {
            return false;
        }

        hasher.addData(buffer.constData(), read);
        total += read;
    }

    *sha256 = hasher.result();
    *size = total;
    return true;
}

// Verifies every member at the resolved path (raw bytes in member order) and
// the total of those bytes, mirroring how stageObjects computed the facts.
bool verifyMemberSet(
    const QString& vaultPlaintextRoot,
    const QList<PrivacyStrongObjectMember>& members,
    const std::function<QString(const PrivacyStrongObjectMember&)>& resolve,
    qlonglong expectedTotalSize,
    const QByteArray& expectedTotalSha256,
    QString* const error)
{
    QCryptographicHash totalHash(QCryptographicHash::Sha256);
    qlonglong totalSize = 0;

    for (const PrivacyStrongObjectMember& member : members)
    {
        const QString path = resolve(member);
        const QFileInfo info(path);

        if (!confinedRelativePath(member.protectedRelativePath) ||
            !info.isFile() || info.isSymLink() ||
            !pathIsUnder(vaultPlaintextRoot, path))
        {
            *error = QString::fromLatin1(
                "Strong object is missing or unsafe: %1").arg(path);
            return false;
        }

        QByteArray objectHash;
        qlonglong objectSize = 0;

        if (!fileFacts(path, &objectHash, &objectSize) ||
            (objectSize != member.expectedSize) ||
            (objectHash != member.expectedSha256))
        {
            *error = QString::fromLatin1(
                "Strong object fails exact verification: %1").arg(path);
            return false;
        }

        QFile object(path);

        if (!object.open(QIODevice::ReadOnly))
        {
            *error = QString::fromLatin1(
                "Strong object cannot be reopened: %1").arg(path);
            return false;
        }

        QByteArray buffer;
        buffer.resize(1024 * 1024);

        while (!object.atEnd())
        {
            const qint64 read = object.read(buffer.data(), buffer.size());

            if (read <= 0)
            {
                *error = QLatin1String("Strong object read failed during total verification");
                return false;
            }

            totalHash.addData(buffer.constData(), read);
            totalSize += read;
        }
    }

    if ((totalSize != expectedTotalSize) ||
        (totalHash.result() != expectedTotalSha256))
    {
        *error = QLatin1String(
            "Strong object set fails total verification");
        return false;
    }

    return true;
}

QString stagedPathFor(const QString& vaultPlaintextRoot,
                      const QString& stagedRelativeDirectory,
                      const PrivacyStrongObjectMember& member)
{
    return QDir(QDir(vaultPlaintextRoot).filePath(stagedRelativeDirectory)).filePath(
        QFileInfo(member.protectedRelativePath).fileName());
}

} // namespace

PrivacyStrongObjectStageResult PrivacyStrongObjectBackend::stageObjects(
    const QString& vaultPlaintextRoot,
    const QString& stagedRelativeDirectory,
    const QList<PrivacyStrongObjectMember>& members,
    QString* const error)
{
    PrivacyStrongObjectStageResult result;
    const QFileInfo rootInfo(vaultPlaintextRoot);

    if (members.isEmpty() || !rootInfo.isAbsolute() || !rootInfo.isDir())
    {
        if (error)
        {
            *error = QLatin1String("invalid vault root or empty object set");
        }

        return result;
    }

    if (!confinedRelativePath(stagedRelativeDirectory) ||
        !ancestorChainIsSafe(vaultPlaintextRoot, stagedRelativeDirectory))
    {
        if (error)
        {
            *error = QLatin1String("unsafe staged object directory");
        }

        return result;
    }

    const QString stagedDirectory = QDir(vaultPlaintextRoot).filePath(
        stagedRelativeDirectory);

    if (QFileInfo::exists(stagedDirectory) ||
        !QDir().mkpath(stagedDirectory) ||
        !QFile::setPermissions(stagedDirectory,
                               QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                               QFileDevice::ExeOwner))
    {
        if (error)
        {
            *error = QLatin1String("cannot create staged object directory");
        }

        return result;
    }

    QCryptographicHash totalHash(QCryptographicHash::Sha256);

    for (const PrivacyStrongObjectMember& member : members)
    {
        if (!confinedRelativePath(member.protectedRelativePath) ||
            !member.protectedRelativePath.startsWith(QLatin1String("originals/")) ||
            (member.expectedSize < 0) || member.expectedSha256.isEmpty() ||
            QFileInfo(member.sourcePath).isSymLink())
        {
            if (error)
            {
                *error = QString::fromLatin1(
                    "unsafe or incomplete Strong object member: %1")
                             .arg(member.protectedRelativePath);
            }

            return result;
        }

        const QString stagedPath = stagedPathFor(vaultPlaintextRoot,
                                                 stagedRelativeDirectory,
                                                 member);
        qlonglong copiedBytes = 0;

        {
            QFile source(member.sourcePath);

            if (!source.open(QIODevice::ReadOnly))
            {
                if (error)
                {
                    *error = QString::fromLatin1(
                        "cannot open source object: %1").arg(member.sourcePath);
                }

                return result;
            }

            QFile staged(stagedPath);

            if (!staged.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                if (error)
                {
                    *error = QString::fromLatin1(
                        "cannot create staged object: %1").arg(stagedPath);
                }

                return result;
            }

            QByteArray buffer;
            buffer.resize(1024 * 1024);

            while (!source.atEnd())
            {
                const qint64 read = source.read(buffer.data(), buffer.size());

                if ((read <= 0) ||
                    (staged.write(buffer.constData(), read) != read))
                {
                    if (error)
                    {
                        *error = QString::fromLatin1(
                            "cannot copy staged object: %1").arg(stagedPath);
                    }

                    return result;
                }

                totalHash.addData(buffer.constData(), read);
                copiedBytes += read;
            }

            if (!staged.flush() || !fsyncPath(stagedPath))
            {
                if (error)
                {
                    *error = QString::fromLatin1(
                        "cannot flush staged object: %1").arg(stagedPath);
                }

                return result;
            }
        }

        if (copiedBytes != member.expectedSize)
        {
            if (error)
            {
                *error = QString::fromLatin1(
                    "staged object size mismatch: %1").arg(stagedPath);
            }

            return result;
        }

        QByteArray stagedHash;
        qlonglong stagedSize = 0;

        if (!fileFacts(stagedPath, &stagedHash, &stagedSize) ||
            (stagedHash != member.expectedSha256))
        {
            if (error)
            {
                *error = QString::fromLatin1(
                    "staged object fails exact hash verification: %1")
                             .arg(stagedPath);
            }

            return result;
        }

        result.stagedRelativePaths << QDir(stagedRelativeDirectory).filePath(
            QFileInfo(member.protectedRelativePath).fileName());
        result.totalSize += copiedBytes;
    }

    if (!fsyncPath(stagedDirectory))
    {
        if (error)
        {
            *error = QLatin1String("cannot fsync staged object directory");
        }

        return result;
    }

    result.totalSha256 = totalHash.result();
    result.valid = true;
    return result;
}

bool PrivacyStrongObjectBackend::publishObjects(
    const QString& vaultPlaintextRoot,
    const QString& stagedRelativeDirectory,
    const QString& finalRelativeDirectory,
    const QList<PrivacyStrongObjectMember>& members,
    qlonglong expectedTotalSize,
    const QByteArray& expectedTotalSha256,
    QString* const error)
{
    if (!confinedRelativePath(stagedRelativeDirectory) ||
        !confinedRelativePath(finalRelativeDirectory) ||
        !finalRelativeDirectory.startsWith(QLatin1String("originals/")) ||
        (stagedRelativeDirectory == finalRelativeDirectory) ||
        !ancestorChainIsSafe(vaultPlaintextRoot, stagedRelativeDirectory) ||
        !ancestorChainIsSafe(vaultPlaintextRoot, finalRelativeDirectory))
    {
        if (error)
        {
            *error = QLatin1String("unsafe Strong object publication paths");
        }

        return false;
    }

    const QString stagedDirectory = QDir(vaultPlaintextRoot).filePath(
        stagedRelativeDirectory);
    const QString finalDirectory = QDir(vaultPlaintextRoot).filePath(
        finalRelativeDirectory);

    if (!QFileInfo(stagedDirectory).isDir() || QFileInfo::exists(finalDirectory))
    {
        if (error)
        {
            *error = QLatin1String(
                "staged object directory missing or final directory exists");
        }

        return false;
    }

    if (!verifyMemberSet(
            vaultPlaintextRoot, members,
            [&](const PrivacyStrongObjectMember& member)
            {
                return stagedPathFor(vaultPlaintextRoot,
                                     stagedRelativeDirectory, member);
            },
            expectedTotalSize, expectedTotalSha256, error))
    {
        return false;
    }

#ifdef Q_OS_UNIX
    const int rootFd = ::open(QFile::encodeName(vaultPlaintextRoot).constData(),
                              O_RDONLY | O_DIRECTORY);

    if (rootFd < 0)
    {
        if (error)
        {
            *error = QLatin1String("cannot open vault root for publication");
        }

        return false;
    }

    bool renameUnavailable = false;
    const bool renamed = PrivacyPosixStorage::atomicRenameAt(
        rootFd, QFile::encodeName(stagedRelativeDirectory),
        QFile::encodeName(finalRelativeDirectory),
        PrivacyPosixStorage::AtomicRenameMode::NoReplace,
        &renameUnavailable);
    ::close(rootFd);

    if (!renamed)
    {
        if (error)
        {
            *error = QString::fromLatin1(
                "cannot atomically publish Strong objects (%1)")
                         .arg(renameUnavailable
                                  ? QLatin1String("rename unavailable")
                                  : QLatin1String("target exists or failed"));
        }

        return false;
    }
#else
    if (!QDir().rename(stagedDirectory, finalDirectory))
    {
        if (error)
        {
            *error = QLatin1String("cannot publish Strong objects");
        }

        return false;
    }
#endif

    if (!verifyMemberSet(
            vaultPlaintextRoot, members,
            [&](const PrivacyStrongObjectMember& member)
            {
                return QDir(vaultPlaintextRoot).filePath(
                    member.protectedRelativePath);
            },
            expectedTotalSize, expectedTotalSha256, error))
    {
        return false;
    }

    if (!fsyncPath(vaultPlaintextRoot))
    {
        if (error)
        {
            *error = QLatin1String("cannot fsync vault root after publication");
        }

        return false;
    }

    return true;
}

bool PrivacyStrongObjectBackend::verifyObjects(
    const QString& vaultPlaintextRoot,
    const QString& finalRelativeDirectory,
    const QList<PrivacyStrongObjectMember>& members,
    qlonglong expectedTotalSize,
    const QByteArray& expectedTotalSha256,
    QString* const error)
{
    if (!confinedRelativePath(finalRelativeDirectory) ||
        !finalRelativeDirectory.startsWith(QLatin1String("originals/")))
    {
        if (error)
        {
            *error = QLatin1String("unsafe verification directory");
        }

        return false;
    }

    for (const PrivacyStrongObjectMember& member : members)
    {
        if (QFileInfo(member.protectedRelativePath).path() !=
            QDir::cleanPath(finalRelativeDirectory))
        {
            if (error)
            {
                *error = QString::fromLatin1(
                    "member escapes the final object directory: %1")
                             .arg(member.protectedRelativePath);
            }

            return false;
        }
    }

    return verifyMemberSet(
        vaultPlaintextRoot, members,
        [&](const PrivacyStrongObjectMember& member)
        {
            return QDir(vaultPlaintextRoot).filePath(
                member.protectedRelativePath);
        },
        expectedTotalSize, expectedTotalSha256, error);
}

bool PrivacyStrongObjectBackend::restoreObject(
    const QString& vaultPlaintextRoot,
    const QString& protectedRelativePath,
    const QString& targetAbsolutePath,
    QFileDevice::Permissions permissions,
    const QDateTime& modificationDate,
    QString* const error)
{
    const QFileInfo targetInfo(targetAbsolutePath);

    if (!targetInfo.isAbsolute() ||
        !confinedRelativePath(protectedRelativePath) ||
        !protectedRelativePath.startsWith(QLatin1String("originals/")))
    {
        if (error)
        {
            *error = QLatin1String("unsafe Strong object restore paths");
        }

        return false;
    }

    const QString objectPath = QDir(vaultPlaintextRoot).filePath(
        protectedRelativePath);
    const QFileInfo objectInfo(objectPath);

    if (!objectInfo.isFile() || objectInfo.isSymLink() ||
        !pathIsUnder(vaultPlaintextRoot, objectPath) ||
        !QDir().mkpath(targetInfo.absolutePath()) ||
        !copyStream(objectPath, targetAbsolutePath, error))
    {
        if (error && !error->startsWith(QLatin1String("cannot copy")))
        {
            *error = QString::fromLatin1(
                "restore source is missing or unsafe: %1").arg(objectPath);
        }

        return false;
    }

    if (!QFile::setPermissions(targetAbsolutePath, permissions) ||
        !fsyncPath(targetAbsolutePath))
    {
        if (error)
        {
            *error = QLatin1String("cannot secure restore target");
        }

        return false;
    }

    if (modificationDate.isValid())
    {
#ifdef Q_OS_UNIX
        const qint64 seconds = modificationDate.toSecsSinceEpoch();
        struct timespec times[2];
        times[0].tv_sec = seconds;
        times[0].tv_nsec = 0;
        times[1].tv_sec = seconds;
        times[1].tv_nsec = 0;

        if (::utimensat(AT_FDCWD,
                        QFile::encodeName(targetAbsolutePath).constData(),
                        times, 0) != 0)
        {
            if (error)
            {
                *error = QLatin1String("cannot restore modification time");
            }

            return false;
        }
#else
        QFile restored(targetAbsolutePath);

        if (!restored.open(QIODevice::ReadWrite) ||
            !restored.setFileTime(modificationDate,
                                  QFileDevice::FileModificationTime))
        {
            if (error)
            {
                *error = QLatin1String("cannot restore modification time");
            }

            return false;
        }

        restored.close();
#endif
    }

    return true;
}

bool PrivacyStrongObjectBackend::removeObjects(
    const QString& vaultPlaintextRoot,
    const QString& protectedRelativeDirectory,
    QString* const error)
{
    if (!confinedRelativePath(protectedRelativeDirectory) ||
        !protectedRelativeDirectory.startsWith(QLatin1String("originals/")))
    {
        if (error)
        {
            *error = QLatin1String("unsafe Strong object removal directory");
        }

        return false;
    }

    const QString directory = QDir(vaultPlaintextRoot).filePath(
        protectedRelativeDirectory);
    const QFileInfo info(directory);

    if (!info.exists())
    {
        return true;
    }

    if (!info.isDir() || info.isSymLink() ||
        !pathIsUnder(vaultPlaintextRoot, directory) ||
        !QDir(directory).removeRecursively())
    {
        if (error)
        {
            *error = QLatin1String("refusing to remove unsafe Strong object directory");
        }

        return false;
    }

    return fsyncPath(vaultPlaintextRoot);
}

} // namespace Digikam

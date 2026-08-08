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

#include <cerrno>

// Qt includes

#include <QByteArray>
#include <QtGlobal>

#if defined(Q_OS_UNIX)

// POSIX includes

#   include <fcntl.h>
#   include <sys/types.h>
#   include <unistd.h>

#endif

#if defined(Q_OS_LINUX)

#   include <linux/openat2.h>
#   include <sys/syscall.h>

#endif

namespace Digikam
{

namespace PrivacyPosixStorage
{

enum class AtomicRenameMode
{
    NoReplace,
    Exchange
};

inline int confinedOpenAt(int directoryFd, const QByteArray& name, int flags,
                          unsigned int mode = 0)
{
#if defined(Q_OS_UNIX)

#   if defined(Q_OS_LINUX) && defined(SYS_openat2)

    struct open_how how = {};
    how.flags   = static_cast<quint64>(flags);
    how.mode    = static_cast<quint64>(mode);
    how.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS |
                  RESOLVE_NO_MAGICLINKS | RESOLVE_NO_XDEV;

    const int opened = static_cast<int>(::syscall(SYS_openat2, directoryFd,
                                                   name.constData(), &how,
                                                   sizeof(how)));

    if (opened >= 0)
    {
        return opened;
    }

    if ((errno != ENOSYS) && (errno != EINVAL) && (errno != E2BIG))
    {
        return -1;
    }

#   endif

    // Callers pass one validated path component. The fallback still refuses
    // symlinks, and each caller verifies the opened descriptor with fstat().

    return ::openat(directoryFd, name.constData(), flags | O_NOFOLLOW,
                    static_cast<mode_t>(mode));

#else

    Q_UNUSED(directoryFd);
    Q_UNUSED(name);
    Q_UNUSED(flags);
    Q_UNUSED(mode);

    errno = ENOSYS;
    return -1;

#endif
}

inline bool atomicRenameAt(int fromDirectoryFd, const QByteArray& from,
                           int toDirectoryFd, const QByteArray& to,
                           AtomicRenameMode mode, bool* const unavailable)
{
    if (unavailable)
    {
        *unavailable = false;
    }

#if defined(Q_OS_LINUX) && defined(SYS_renameat2)

    const unsigned int flags = (mode == AtomicRenameMode::Exchange)
                             ? RENAME_EXCHANGE
                             : RENAME_NOREPLACE;

    if (::syscall(SYS_renameat2, fromDirectoryFd, from.constData(), toDirectoryFd,
                  to.constData(), flags) == 0)
    {
        return true;
    }

    if ((errno == ENOSYS) || (errno == EINVAL))
    {
        if (unavailable)
        {
            *unavailable = true;
        }
    }

    return false;

#else

    Q_UNUSED(fromDirectoryFd);
    Q_UNUSED(toDirectoryFd);
    Q_UNUSED(from);
    Q_UNUSED(to);
    Q_UNUSED(mode);

    if (unavailable)
    {
        *unavailable = true;
    }

    errno = ENOSYS;
    return false;

#endif
}

inline bool atomicRenameAt(int directoryFd, const QByteArray& from,
                           const QByteArray& to, AtomicRenameMode mode,
                           bool* const unavailable)
{
    return atomicRenameAt(directoryFd, from, directoryFd, to, mode,
                          unavailable);
}

} // namespace PrivacyPosixStorage

} // namespace Digikam

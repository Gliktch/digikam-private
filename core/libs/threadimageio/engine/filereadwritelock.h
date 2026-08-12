/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-29
 * Description : Intra-process file i/o lock
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QTemporaryFile>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class FileReadWriteLockPriv;

class DIGIKAM_EXPORT FileReadWriteLockKey
{
public:

    explicit FileReadWriteLockKey(const QString& filePath);
    ~FileReadWriteLockKey();

    void lockForRead();
    void lockForWrite();
    bool tryLockForRead();
    bool tryLockForRead(int timeout);
    bool tryLockForWrite();
    bool tryLockForWrite(int timeout);
    void unlock();

private:

    /// @note disabled
    FileReadWriteLockKey(const FileReadWriteLockKey&)            = delete;
    FileReadWriteLockKey& operator=(const FileReadWriteLockKey&) = delete;

    FileReadWriteLockPriv* d = nullptr;
};

// ----------------------------------------------------------------------

class DIGIKAM_EXPORT FileReadLocker
{
public:

    explicit FileReadLocker(const QString& filePath);
    ~FileReadLocker();

private:

    /// @note disabled
    FileReadLocker(const FileReadLocker&)            = delete;
    FileReadLocker& operator=(const FileReadLocker&) = delete;

    FileReadWriteLockPriv* d = nullptr;
};

// ----------------------------------------------------------------------

class DIGIKAM_EXPORT FileWriteLocker
{
public:

    explicit FileWriteLocker(const QString& filePath);
    ~FileWriteLocker();

private:

    /// @note disabled
    FileWriteLocker(const FileWriteLocker&)            = delete;
    FileWriteLocker& operator=(const FileWriteLocker&) = delete;

    FileReadWriteLockPriv* d = nullptr;
};

// ----------------------------------------------------------------------

class DIGIKAM_EXPORT SafeTemporaryFile : public QTemporaryFile
{
    Q_OBJECT

public:

    explicit SafeTemporaryFile(const QString& templ);
    SafeTemporaryFile();

    bool open();

    QString safeFilePath() const;

protected:

    bool open(QIODevice::OpenMode) override;

private:

    /// @note disabled
    SafeTemporaryFile(QObject*) = delete;

private:

    QString m_templ;
};

} // namespace Digikam

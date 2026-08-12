/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2015 The Qt Company Ltd.
 *               This file is based on qzipwriter_p.h from Qt with the original license
 *               below, taken from
 *               https://code.qt.io/cgit/qt/qt.git/plain/src/gui/text/qzipwriter_p.h
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR LGPL-3.0-only WITH Qt-LGPL-exception-1.1 OR GPL-3.0-only OR LicenseRef-Qt-Commercial
 *
 * ============================================================ */

#pragma once

#ifndef QT_NO_TEXTODFWRITER

// Qt includes

#   include <QDateTime>
#   include <QFile>
#   include <QString>

// Local includes

#   include "digikam_export.h"

namespace Marble
{

class MarbleZipReaderPrivate;

class DIGIKAM_EXPORT MarbleZipReader
{
public:

    enum Status
    {
        NoError,
        FileReadError,
        FileOpenError,
        FilePermissionsError,
        FileError
    };

public:

    explicit MarbleZipReader(const QString& fileName, QIODevice::OpenMode mode = QIODevice::ReadOnly);

    explicit MarbleZipReader(QIODevice* device);
    ~MarbleZipReader();

public:

    QIODevice* device()                             const;

    bool isReadable()                               const;
    bool exists()                                   const;

    struct DIGIKAM_EXPORT FileInfo
    {
        FileInfo();
        FileInfo(const FileInfo& other);
        ~FileInfo();
        FileInfo& operator=(const FileInfo& other);
        bool isValid()                              const;

        QString filePath;
        uint isDir                      = false;
        uint isFile                     = false;
        uint isSymLink                  = false;
        QFile::Permissions permissions;
        uint               crc32        = 0;
        qint64             size         = 0;
        QDateTime          lastModified;
        void*              d            = nullptr;
    };

    QList<FileInfo> fileInfoList()                  const;
    int count()                                     const;

    FileInfo entryInfoAt(int index)                 const;
    QByteArray fileData(const QString& fileName)    const;
    bool extractAll(const QString& destinationDir)  const;

    Status status()                                 const;

    void close();

private:

    MarbleZipReaderPrivate* d = nullptr;

    Q_DISABLE_COPY(MarbleZipReader)
};

} // namespace Marble

#endif // QT_NO_TEXTODFWRITER

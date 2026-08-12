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

#   include <QString>
#   include <QFile>

// Local includes

#   include "digikam_export.h"

namespace Marble
{

class MarbleZipWriterPrivate;

class DIGIKAM_EXPORT MarbleZipWriter
{
public:

    enum Status
    {
        NoError,
        FileWriteError,
        FileOpenError,
        FilePermissionsError,
        FileError
    };

    enum CompressionPolicy
    {
        AlwaysCompress,
        NeverCompress,
        AutoCompress
    };

public:

    explicit MarbleZipWriter(const QString& fileName,
                             QIODevice::OpenMode mode = (QIODevice::WriteOnly | QIODevice::Truncate));

    explicit MarbleZipWriter(QIODevice* device);
    ~MarbleZipWriter();

public:

    QIODevice* device()                             const;

    bool isWritable()                               const;
    bool exists()                                   const;

    Status status()                                 const;

    void setCompressionPolicy(CompressionPolicy policy);
    CompressionPolicy compressionPolicy()           const;

    void setCreationPermissions(QFile::Permissions permissions);
    QFile::Permissions creationPermissions()        const;

    void addFile(const QString& fileName, const QByteArray& data);

    void addFile(const QString& fileName, QIODevice* device);

    void addDirectory(const QString& dirName);

    void addSymLink(const QString& fileName, const QString& destination);

    void close();

private:

    MarbleZipWriterPrivate* d = nullptr;

    Q_DISABLE_COPY(MarbleZipWriter)
};

} // namespace Marble

#endif // QT_NO_TEXTODFWRITER

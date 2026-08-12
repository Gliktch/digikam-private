/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imageshackmpform.h"

// Qt includes

#include <QFile>
#include <QUrl>
#include <QMimeDatabase>
#include <QMimeType>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericImageShackPlugin
{

ImageShackMPForm::ImageShackMPForm()
{
    reset();
}

void ImageShackMPForm::reset()
{
    m_buffer.resize(0);
    QByteArray str(contentType().toLatin1());
    str += "\r\nMIME-version: 1.0\r\n\r\n";
    m_buffer.append(str);
}

void ImageShackMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

void ImageShackMPForm::addPair(const QString& name, const QString& value)
{
    QByteArray str;

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toLatin1();
        str += "\"\r\n";
    }

    str += "\r\n";
    str += value.toUtf8();
    str += "\r\n";

    m_buffer.append(str);
}

bool ImageShackMPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime = ptr.name();

    if (mime.isEmpty())
    {
        return false;
    }

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray imageData = imageFile.readAll();

    QString file_size = QString::number(imageFile.size());
    imageFile.close();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"fileupload\"; filename=\"";
    str += QFile::encodeName(name);
    str += "\"\r\n";
    str += "Content-Length: ";
    str += file_size.toLatin1();
    str += "\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString ImageShackMPForm::contentType() const
{
    return QLatin1String("multipart/form-data; boundary=") + QLatin1String(m_boundary);
}

QString ImageShackMPForm::boundary() const
{
    return QLatin1String(m_boundary);
}

QByteArray ImageShackMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericImageShackPlugin

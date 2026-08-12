/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpmpform.h"

// Qt includes

#include <QUrl>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>

namespace DigikamGenericGoogleServicesPlugin
{

void GPMPForm::reset()
{
    m_buffer.resize(0);
}

void GPMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";

    m_buffer.append(str);
}

bool GPMPForm::addPair(const QString& name,
                       const QString& value,
                       const QString& contentType)
{
    QByteArray str;
    QString content_length = QString::number(value.length());
    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toLatin1();
        str += "\"\r\n";
    }

    if (!contentType.isEmpty())
    {
        str += "Content-Type: "+ QByteArray(contentType.toLatin1());
        str += "\r\n";
        str += "Mime-version: 1.0 ";
        str += "\r\n";
    }

    str += "Content-Length: " ;
    str += content_length.toLatin1();
    str += "\r\n\r\n";
    str += value.toUtf8();
    str += "\r\n";

    m_buffer.append(str);

    return true;
}

bool GPMPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();

    if (mime.isEmpty())
    {
        // if we ourselves cannot determine the mime of the local file,
        // very unlikely the remote site will be able to identify it

        return false;
    }

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray imageData = imageFile.readAll();
    imageFile.close();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += name.toLatin1();
    str += "\"; ";
    str += "filename=\"";
    str += QUrl::fromLocalFile(path).fileName().toLatin1();
    str += "\"\r\n";
    str += "Content-Length: ";
    str += QString::number(imageFile.size()).toLatin1();
    str += "\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString GPMPForm::contentType() const
{
    return QLatin1String("multipart/related; boundary=") +
           QLatin1String(m_boundary);
}

QString GPMPForm::boundary() const
{
    return QLatin1String(m_boundary);
}

QByteArray GPMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericGoogleServicesPlugin

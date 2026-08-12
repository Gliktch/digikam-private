/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 * SPDX-FileCopyrightText: 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "twittermpform.h"

// Qt includes

#include <QFile>
#include <QUrl>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericTwitterPlugin
{

TwMPForm::TwMPForm()
{
    reset();
}

void TwMPForm::reset()
{
    m_buffer.resize(0);
    m_chunks.clear();
}

QByteArray TwMPForm::createPair(const QByteArray& key, const QByteArray& value)
{
    QByteArray data("--");
    data += m_boundary;
    data += "\r\n";
    data += "Content-Disposition: form-data; name=\"";
    data += key;
    data += "\"\r\n";
    data += "\r\n";
    data += value;
    data += "\r\n";

    return data;
}

bool TwMPForm::addPair(const QByteArray& data)
{
    m_buffer.append(data);

    return true;
}

QByteArray TwMPForm::fileHeader(const QString& imgPath)
{
    QByteArray data("--");
    data += m_boundary;
    data += "\r\n";
    data += "Content-Disposition: form-data; name=\"media\"; filename=\"";
    data += QFile::encodeName(QUrl(imgPath).fileName());
    data += "\"\r\n";
    data += "Content-Type: application/octet-stream\r\n\r\n";

    return data;
}

bool TwMPForm::addFile(const QString& imgPath, bool fragmented)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    if (!fragmented)
    {
        m_buffer.append(fileHeader(imgPath));
        m_buffer.append(file.readAll());
        m_buffer.append("\r\n");
    }
    else
    {
        formChunks(file.readAll());
    }

    file.close();

    return true;
}

QByteArray TwMPForm::getChunk(int index) const
{
    return m_chunks.at(index);
}

QString TwMPForm::contentType() const
{
    QString type = QString::fromLatin1("multipart/form-data, boundary=\"%1\"").arg(QLatin1String(m_boundary));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << type;

    return type;
}

QByteArray TwMPForm::border()
{
    QByteArray data;
    data += "--";
    data += m_boundary;
    data += "--\r\n";

    return data;
}

void TwMPForm::finish()
{
    m_buffer.append(border());
}

QByteArray TwMPForm::formData() const
{
    return m_buffer;
}

void TwMPForm::formChunks(const QByteArray& data)
{
    int mediaSize  = data.size();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "mediaSize: " << mediaSize;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MAX_MEDIA_SIZE " << MAX_MEDIA_SIZE;
    int breakPoint = 0;

    for ( ; breakPoint < mediaSize ; breakPoint += MAX_MEDIA_SIZE)
    {
        m_chunks << data.mid(breakPoint, MAX_MEDIA_SIZE);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "breakpoint " << breakPoint;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "number of chunks: " << m_chunks.size();

    for (const QByteArray& chunk : std::as_const(m_chunks))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "size of chunks: " << chunk.size();
    }
}

int TwMPForm::numberOfChunks() const
{
    return m_chunks.size();
}

} // namespace DigikamGenericTwitterPlugin

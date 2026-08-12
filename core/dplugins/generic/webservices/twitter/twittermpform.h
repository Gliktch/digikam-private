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

#pragma once

// Qt includes

#include <QByteArray>
#include <QString>
#include <QList>

// Local includes

#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericTwitterPlugin
{

static const int MAX_MEDIA_SIZE = 1048576;

class TwMPForm
{

public:

    TwMPForm();
    ~TwMPForm() = default;

    void reset();
    QByteArray fileHeader(const QString& imgPath);
    bool addFile(const QString& imgPath, bool fragmented = false);
    QByteArray createPair(const QByteArray& name, const QByteArray& value);
    bool addPair(const QByteArray& pair);
    void finish();
    QByteArray border();

    int numberOfChunks()           const;
    QString contentType()          const;
    QByteArray formData()          const;
    QByteArray getChunk(int index) const;

private:

    void formChunks(const QByteArray& data);

private:

    QByteArray        m_buffer;
    QByteArray        m_boundary = QByteArray("00TwDK")                          +
                                   WSToolUtils::randomString(42 + 13).toLatin1() +
                                   QByteArray("KDwT99");
    QList<QByteArray> m_chunks;
};

} // namespace DigikamGenericTwitterPlugin

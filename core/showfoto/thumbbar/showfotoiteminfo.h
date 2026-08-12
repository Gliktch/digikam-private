/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-1
 * Description : Showfoto item info container
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QByteArray>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>

// Local includes

#include "photoinfocontainer.h"
#include "videoinfocontainer.h"

using namespace Digikam;

class QDataStream;

namespace ShowFoto
{

class ShowfotoItemInfo
{
public:

    ShowfotoItemInfo()  = default;
    ~ShowfotoItemInfo() = default;

    /**
     * Return true if all member in this container are null.
     */
    bool isNull() const;

    /**
     * Compare for information equality and un-equality, not including variable values.
     */
    bool operator==(const ShowfotoItemInfo& info) const;
    bool operator!=(const ShowfotoItemInfo& info) const;

    static ShowfotoItemInfo itemInfoFromFile(const QFileInfo& inf);

public:

    /// Static values.
    qint64             size     = -1;        ///< file size in bytes.
    QUrl               url;                  ///< file Url

    QString            name;                 ///< File name in file-system
    QString            folder;               ///< Folder path to access to file
    QString            mime;                 ///< Type mime of file

    /// Unique image id
    qlonglong          id       = -1;

    PhotoInfoContainer photoInfo;

    QDateTime          dtime;                ///< creation time on disk
    QDateTime          ctime;                ///< camera date stamp
    int                width    = 0;         ///< Image width in pixels
    int                height   = 0;         ///< Image height in pixels
};

QDataStream& operator<<(QDataStream&, const ShowfotoItemInfo&);
QDataStream& operator>>(QDataStream&, ShowfotoItemInfo&);

typedef QList<ShowfotoItemInfo> ShowfotoItemInfoList;

//! qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const ShowfotoItemInfo& info);

} // namespace Showfoto

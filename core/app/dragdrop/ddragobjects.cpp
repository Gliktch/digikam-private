/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-02-29
 * Description : Drag object info containers.
 *
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ddragobjects.h"

// Qt includes

#include <QDir>
#include <QIODevice>
#include <QByteArray>
#include <QDataStream>
#include <QApplication>

// Local includes

#include "digikam_config.h"

namespace Digikam
{

DItemDrag::DItemDrag(const QList<QUrl>& urls,
                     const QList<int>& albumIDs,
                     const QList<qlonglong>& imageIDs)
    : QMimeData()
{
    // digiKam specific mime data

    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << urls;
    setData(QLatin1String("digikam/item-urls"), ba);

    // inconsistent numbering is legacy

    QByteArray  ba3;
    QDataStream ds3(&ba3, QIODevice::WriteOnly);
    ds3 << albumIDs;
    setData(QLatin1String("digikam/album-ids"), ba3);

    QByteArray  ba4;
    QDataStream ds4(&ba4, QIODevice::WriteOnly);
    ds4 << imageIDs;
    setData(QLatin1String("digikam/item-ids"), ba4);

    QString txt;

    for (const QUrl& url : std::as_const(urls))
    {

#ifdef Q_OS_WIN

        txt.append(QString::fromUtf8("%1\r\n").arg(QDir::toNativeSeparators(url.toLocalFile())));

#else

        txt.append(QString::fromUtf8("%1\n").arg(url.toLocalFile()));

#endif

    }

    if (urls.size() == 1)
    {

#ifdef Q_OS_WIN

        txt.chop(2);

#else

        txt.chop(1);

#endif

    }

    setText(txt);

    // commonly accessible mime data, for dragging to outside digikam

    setUrls(urls);
}

QStringList DItemDrag::mimeTypes()
{
    // we do not want to decode text/uri-list with this object,
    // we only export this data above for dragging to outside digikam.

    return QStringList() << QLatin1String("digikam/item-urls")
                         << QLatin1String("digikam/album-ids")
                         << QLatin1String("digikam/item-ids");
}

bool DItemDrag::canDecode(const QMimeData* e)
{
    const auto mimes = mimeTypes();

    for (const QString& mimeType : mimes)
    {
        if (!e->hasFormat(mimeType))
        {
            return false;
        }
    }

    return true;
}

bool DItemDrag::decode(const QMimeData* e,
                       QList<QUrl>& urls,
                       QList<int>& albumIDs,
                       QList<qlonglong>& imageIDs)
{
    urls.clear();
    albumIDs.clear();
    imageIDs.clear();

    QByteArray ba = e->data(QLatin1String("digikam/item-urls"));

    if (ba.size())
    {
        QDataStream ds(ba);

        if (!ds.atEnd())
        {
            ds >> urls;
        }
    }

    if (!urls.isEmpty())
    {
        QByteArray albumarray = e->data(QLatin1String("digikam/album-ids"));
        QByteArray imagearray = e->data(QLatin1String("digikam/item-ids"));

        if (albumarray.size() && imagearray.size())
        {
            QDataStream dsAlbums(albumarray);

            if (!dsAlbums.atEnd())
            {
                dsAlbums >> albumIDs;
            }

            QDataStream dsImages(imagearray);

            if (!dsImages.atEnd())
            {
                dsImages >> imageIDs;
            }

            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------------------

DAlbumDrag::DAlbumDrag(const QUrl& databaseUrl, int albumid, const QUrl& fileUrl)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << (QList<QUrl>() << databaseUrl);
    setData(QLatin1String("digikam/digikamalbums"), ba);

    QByteArray  ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << (QList<int>() << albumid);
    setData(QLatin1String("digikam/album-ids"), ba2);

    // commonly accessible mime data, for dragging to outside digikam

    if (!fileUrl.isEmpty())
    {
        setUrls(QList<QUrl>() << fileUrl);
    }
}

QStringList DAlbumDrag::mimeTypes()
{
    return QStringList() << QLatin1String("digikam/album-ids")
                         << QLatin1String("digikam/digikamalbums");
}

bool DAlbumDrag::canDecode(const QMimeData* e)
{
    if (e->hasFormat(QLatin1String("digikam/item-urls")) || e->hasFormat(QLatin1String("digikam/item-ids")))
    {
        return false;
    }

    const auto mimes = mimeTypes();

    for (const QString& mimeType : mimes)
    {
        if (!e->hasFormat(mimeType))
        {
            return false;
        }
    }

    return true;
}

bool DAlbumDrag::decode(const QMimeData* e, QList<QUrl>& kioUrls, int& albumID)
{
    albumID               = -1;
    QByteArray albumarray = e->data(QLatin1String("digikam/album-ids"));
    QByteArray kioarray   = e->data(QLatin1String("digikam/digikamalbums"));

    if (albumarray.size() && kioarray.size())
    {
        QDataStream dsAlbums(albumarray);

        if (!dsAlbums.atEnd())
        {
            QList<int> ids;
            dsAlbums >> ids;
            albumID = ids.first();
        }

        QDataStream dsKio(kioarray);

        if (!dsKio.atEnd())
        {
            dsKio >> kioUrls;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DTagListDrag::DTagListDrag(const QList<int>& tagIDs)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << tagIDs;
    setData(QLatin1String("digikam/taglist"), ba);
}

QStringList DTagListDrag::mimeTypes()
{
    return QStringList() << QLatin1String("digikam/taglist");
}

bool DTagListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat(QLatin1String("digikam/taglist"));
}

bool DTagListDrag::decode(const QMimeData* e, QList<int>& tagIDs)
{
    tagIDs.clear();

    QByteArray ba = e->data(QLatin1String("digikam/taglist"));

    if (ba.size())
    {
        QDataStream ds(ba);

        if (!ds.atEnd())
        {
            ds >> tagIDs;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DCameraItemListDrag::DCameraItemListDrag(const QStringList& cameraItemPaths)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << cameraItemPaths;
    setData(QLatin1String("digikam/cameraItemlist"), ba);
}

QStringList DCameraItemListDrag::mimeTypes()
{
    return QStringList() << QLatin1String("digikam/cameraItemlist");
}

bool DCameraItemListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat(QLatin1String("digikam/cameraItemlist"));
}

bool DCameraItemListDrag::decode(const QMimeData* e, QStringList& cameraItemPaths)
{
    cameraItemPaths.clear();

    QByteArray ba = e->data(QLatin1String("digikam/cameraItemlist"));

    if (ba.size())
    {
        QDataStream ds(ba);

        if (!ds.atEnd())
        {
            ds >> cameraItemPaths;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DCameraDragObject::DCameraDragObject(const CameraType& ctype)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << ctype.title();
    ds << ctype.model();
    ds << ctype.port();
    ds << ctype.path();
    ds << ctype.startingNumber();
    setData(QLatin1String("camera/unknown"), ba);
}

QStringList DCameraDragObject::mimeTypes()
{
    return QStringList() << QLatin1String("camera/unknown");
}

bool DCameraDragObject::canDecode(const QMimeData* e)
{
    return e->hasFormat(QLatin1String("camera/unknown"));
}

bool DCameraDragObject::decode(const QMimeData* e, CameraType& ctype)
{
    QByteArray ba = e->data(QLatin1String("camera/unknown"));

    if (ba.size())
    {
        int     startingNumber;
        QString title, model, port, path;

        QDataStream ds(ba);
        ds >> title;
        ds >> model;
        ds >> port;
        ds >> path;
        ds >> startingNumber;

        ctype = CameraType(title, model, port, path, startingNumber);

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace Digikam

#include "moc_ddragobjects.cpp"

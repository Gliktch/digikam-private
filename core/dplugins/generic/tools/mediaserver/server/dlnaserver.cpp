/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
 *               Implementation inspired on Platinum File Media Server.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dlnaserver.h"

namespace DigikamGenericMediaServerPlugin
{

DLNAMediaServer::DLNAMediaServer(const char*  friendly_name,
                                 bool         show_ip,
                                 const char*  uuid,
                                 NPT_UInt16   port,
                                 bool         port_rebind)
    : PLT_MediaServer(friendly_name,
                      show_ip,
                      uuid,
                      port,
                      port_rebind),
      DLNAMediaServerDelegate("/")
{
    SetDelegate(this);
}

void DLNAMediaServer::addAlbumsOnServer(const MediaServerMap& map)
{
    static_cast<DLNAMediaServerDelegate*>(GetDelegate())->DLNAMediaServerDelegate::addAlbumsOnServer(map);
}

NPT_Result DLNAMediaServer::SetupIcons()
{
    QString path;

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-digikam.webp"));
    }
    else
    {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("showfoto/data/logo-showfoto.webp"));
    }

    QByteArray icon;
    QImage     img(path);
    QString    uri;
    int        depth;
    int        size;

    if (!img.isNull())
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Setup Media Server icons";

        size = 256;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 120;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 48;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 32;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);
        size = 16;
        icon = iconData(img, size, uri, depth);
        AddIcon(PLT_DeviceIcon("image/png", size, size, depth, uri.toLatin1().data()), icon.data(), icon.size(), true);

        return NPT_SUCCESS;
    }

    return NPT_FAILURE;
}

QByteArray DLNAMediaServer::iconData(const QImage& img, int size, QString& uri, int& depth) const
{
    QByteArray ba;
    QBuffer    buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    QImage icon = img.scaled(size, size);
    icon.save(&buffer, "PNG");
    buffer.close();

    uri         = QString::fromLatin1("/icon%1x%2.png").arg(size).arg(size);
    depth       = icon.depth();

    return ba;
}

} // namespace DigikamGenericMediaServerPlugin

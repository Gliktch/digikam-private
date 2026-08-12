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

#pragma once

// Qt includes

#include <QImage>
#include <QString>
#include <QByteArray>

// Qt includes

#include <QApplication>
#include <QStandardPaths>
#include <QBuffer>
#include <QIODevice>

// Local includes

#include "digikam_debug.h"
#include "dmediaserver.h"
#include "dlnaserverdelegate.h"

// Platinum includes

#include "Neptune.h"
#include "PltMediaServer.h"

using namespace Digikam;

namespace DigikamGenericMediaServerPlugin
{

/**
 * File Media Server for digiKam.
 * The DLNAMediaServer class is based on PLT_MediaServer implementation
 * for a file system backed Media Server with album contents.
 */
class DLNAMediaServer : public PLT_MediaServer,
                        public DLNAMediaServerDelegate
{

public:

    explicit DLNAMediaServer(const char*  friendly_name,
                             bool         show_ip = false,
                             const char*  uuid = nullptr,
                             NPT_UInt16   port = 0,
                             bool         port_rebind = false);

    void addAlbumsOnServer(const MediaServerMap& map) override;

    NPT_Result SetupIcons() override;

protected:

    ~DLNAMediaServer()      override = default;

private:

    QByteArray iconData(const QImage& img, int size, QString& uri, int& depth) const;
};

} // namespace DigikamGenericMediaServerPlugin

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media server manager
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

// Local includes

#include "dmediaserver.h"

namespace DigikamGenericMediaServerPlugin
{

class DMediaServerMngr : public QObject
{
    Q_OBJECT

public:

    DMediaServerMngr();
    ~DMediaServerMngr() override;

    /// This manager is a singleton. Use this method to control the DLNA server instance.
    static DMediaServerMngr* instance();

    /// Setup the list of items to share with the DLNA server into a single album.
    void setItemsList(const QString& aname, const QList<QUrl>& urls);

    /// Return a flat list of items shared
    QList<QUrl> itemsList()                   const;

    /// Setup the list of albums to share with DLNA server.
    void setCollectionMap(const MediaServerMap&);

    /// Return the current album map shared.
    MediaServerMap collectionMap()            const;

    /// Start the DLNA server and share the contents. Return true is all is on-line.
    bool startMediaServer();

    /// Stop the DLNA server and clean-up.
    void cleanUp();

    /// Low level methods to save and load from xml data file.
    bool save();
    bool load();

    /// Wrapper to check if server configuration must be saved and restored between application sessions.
    bool loadAtStartup();
    void saveAtShutdown();

    /// Return true if server is running in background.
    bool isRunning()                          const;

    /// Return some stats about total albums and total items shared on the network.
    int  albumsShared()                       const;
    int  itemsShared()                        const;

    /// Config properties methods.
    QString configGroupName()                 const;
    QString configStartServerOnStartupEntry() const;

    /// Send a notification message if MediaServer have been started or not.
    void mediaServerNotification(bool started);

private:

    /// @note disabled
    explicit DMediaServerMngr(QObject*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMediaServerPlugin

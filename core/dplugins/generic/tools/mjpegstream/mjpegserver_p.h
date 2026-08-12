/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : a MJPEG Stream server to export items on the network.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QStringList>
#include <QFuture>
#include <QMutex>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>

// Local includes

#include "mjpegserver.h"
#include "digikam_debug.h"

namespace DigikamGenericMjpegStreamPlugin
{

/**
 * MJPEG Server private container.
 * Internal use only.
 */
class Q_DECL_HIDDEN MjpegServer::Private : public QObject
{
    Q_OBJECT

public:

    explicit Private(QObject* const parent);
    ~Private() = default;

    /**
     * Handle maximum clients connected to the server.
     */
    void setMaxClients(int);
    int  maxClients() const;

    /**
     * Write data in native socket file descriptor.
     * We need to use native low level socket to write data inside
     * from separated threads, as QTCPSocket only work with a single thread.
     */
    int writeInSocket(int sock, const QByteArray& data) const;

    /**
     * Return an human readable description of client connected through a socket.
     */
    QString clientDescription(QTcpSocket* const client) const;

    /**
     * Return true if the server is running.
     */
    bool isOpened() const;

    /**
     * Initialize the server.
     */
    bool open(const QString& address, int port);

    /**
     * Start the server (aka listening on network).
     */
    void start();

    /**
     * Stop the server (aka not listening on network).
     */
    void stop();

    /**
     * Shutdown the server.
     */
    void close();

public:

    QTcpServer*        server   = nullptr;  ///< main tcp/ip server.
    int                rate     = 15;       ///< stream frames rate per secs [1...30].
    int                delay    = 40000;    ///< delay between frames in us (1E6/rate).
    QList<QTcpSocket*> clients;             ///< list of client connected sockets.
    QByteArray         lastFrame;           ///< the current JPEG frame to dispatch to all connected clients.
    QFuture<void>      srvTask;             ///< server threaded task used to stream on clients.
    QMutex             mutexClients;        ///< to protect current clients list.
    QMutex             mutexFrame;          ///< to protect current frame data.
    QStringList        blackList;           ///< Clients Ip address list to ban.

private Q_SLOTS:

    /**
     * Called by server when new clients arrive.
     */
    void slotNewConnection();

    /**
     * Called by server when client left the server.
     */
    void slotClientDisconnected();

private:

    /**
     * Single thread method called to write data to all clients.
     */
    void writerThread();

    /**
     * This method dispatch to all clients connected to a
     * separated thread running on one available core automatically.
     * This method is called through writerThread().
     */
    void clientWriteMultithreaded(int client, const QByteArray& data);
};

} // namespace DigikamGenericMjpegStreamPlugin

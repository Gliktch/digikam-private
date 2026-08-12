/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * SPDX-FileCopyrightText: 2018      by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * SPDX-FileCopyrightText: 2018-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QTimerEvent>

namespace DigikamGenericIpfsPlugin
{

enum class IpfsTalkerActionType
{
    IMG_UPLOAD, // Action: upload Result: image
};

struct IpfsTalkerAction
{
    IpfsTalkerActionType type;

    struct
    {
        QString imgpath;
        QString title;
        QString description;
    } upload;
};

struct IpfsTalkerResult
{
    const IpfsTalkerAction* action;

    struct IPFSImage
    {
        QString    name;
        QString    url;
        uint       size;
    } image;
};

/* Main class, handles the client side of the globalupload.io API
 */
class IpfsTalker : public QObject
{
Q_OBJECT

public:

    explicit IpfsTalker(QObject* const parent = nullptr);
    ~IpfsTalker()                           override;

    unsigned int workQueueLength();
    void queueWork(const IpfsTalkerAction& action);
    void cancelAllWork();

Q_SIGNALS:

    /* Emitted on progress changes.
     */
    void progress(unsigned int percent, const IpfsTalkerAction& action);
    void success(const IpfsTalkerResult& result);
    void error(const QString& msg, const IpfsTalkerAction& action);

    /* Emitted when the status changes.
     */
    void busy(bool b);

public Q_SLOTS:

    /* Connected to the current QNetworkReply.
     */
    void uploadProgress(qint64 sent, qint64 total);
    void replyFinished();

protected:

    void timerEvent(QTimerEvent* event)     override;

private:

    /* Starts m_work_timer if m_work_queue not empty.
     */
    void startWorkTimer();

    /* Stops m_work_timer if running.
     */
    void stopWorkTimer();

    /* Start working on the first item of m_work_queue
     * by sending a request.
     */
    void doWork();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericIpfsPlugin

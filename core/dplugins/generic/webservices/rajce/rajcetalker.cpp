/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rajcetalker.h"

// Qt includes

#include <QWidget>
#include <QQueue>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QXmlResultItems>
#include <QXmlQuery>
#include <QRecursiveMutex>
#include <QFileInfo>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "rajcempform.h"
#include "rajcecommand.h"
#include "digikam_version.h"
#include "dmetadata.h"
#include "wstoolutils.h"
#include "networkmanager.h"
#include "previewloadthread.h"

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

const QUrl RAJCE_URL(QLatin1String("https://www.rajce.idnes.cz/liveAPI/index.php"));

class Q_DECL_HIDDEN RajceTalker::Private
{
public:

    explicit Private()
      : netMngr    (nullptr),
        reply      (nullptr)
    {
    }

    QQueue<QSharedPointer<RajceCommand> > commandQueue;

    QRecursiveMutex                       queueAccess;

    QString                               tmpDir;

    QNetworkAccessManager*                netMngr;
    QNetworkReply*                        reply;

    RajceSession                          session;
};

RajceTalker::RajceTalker(QWidget* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->tmpDir  = WSToolUtils::makeTemporaryDir("rajce").absolutePath() + QLatin1Char('/');
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

RajceTalker::~RajceTalker()
{
    WSToolUtils::removeTemporaryDir("rajce");

    delete d;
}

const RajceSession& RajceTalker::session() const
{
    return d->session;
}

void RajceTalker::startCommand(const QSharedPointer<RajceCommand>& command)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Sending command:\n" << command->getXml();

    QByteArray data = command.data()->encode();

    QNetworkRequest netRequest(RAJCE_URL);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, command.data()->contentType());

    d->reply = d->netMngr->post(netRequest, data);

    connect(d->reply, SIGNAL(uploadProgress(qint64,qint64)),
            SLOT(slotUploadProgress(qint64,qint64)));

    Q_EMIT signalBusyStarted(command.data()->commandType());
}

void RajceTalker::login(const QString& username, const QString& password)
{
    QSharedPointer<RajceCommand> command = QSharedPointer<LoginCommand>(new LoginCommand(username, password));
    enqueueCommand(command);
}

void RajceTalker::loadAlbums()
{
    QSharedPointer<RajceCommand> command = QSharedPointer<AlbumListCommand>(new AlbumListCommand(d->session));
    enqueueCommand(command);
}

void RajceTalker::createAlbum(const QString& name, const QString& description, bool visible)
{
    QSharedPointer<RajceCommand> command = QSharedPointer<CreateAlbumCommand>(new CreateAlbumCommand(name, description, visible, d->session));
    enqueueCommand(command);
}

void RajceTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    QString response      = QString::fromUtf8(reply->readAll());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << response;

    d->queueAccess.lock();

    QSharedPointer<RajceCommand> command = d->commandQueue.head();
    d->reply                             = nullptr;

    command->processResponse(response, d->session);

    RajceCommandType type = command.data()->commandType();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "State after command: " << d->session;

    // Let the users react on the command before we
    // let the next queued command in.
    // This enables the connected slots to read in
    // reliable values from the state and/or
    // clear the error state once it's handled.
    Q_EMIT signalBusyFinished(type);

    reply->deleteLater();

    // Only dequeue the command after the above signal has been
    // emitted so that the users can queue other commands
    // without them being started straight away in the enqueue
    // method which would happen if the command was dequeued
    // before the signal and the signal was emitted in the same
    // thread (which is the case (always?)).
    d->commandQueue.dequeue();

    // see if there's something to continue with
    if (d->commandQueue.size() > 0)
    {
        startCommand(d->commandQueue.head());
    }

    d->queueAccess.unlock();
}

void RajceTalker::logout()
{
    //TODO
}

void RajceTalker::openAlbum(const RajceAlbum& album)
{
    QSharedPointer<RajceCommand> command = QSharedPointer<OpenAlbumCommand>(new OpenAlbumCommand(album.id, d->session));
    enqueueCommand(command);
}

void RajceTalker::closeAlbum()
{
    if (!d->session.openAlbumToken().isEmpty())
    {
        QSharedPointer<RajceCommand> command = QSharedPointer<CloseAlbumCommand>(new CloseAlbumCommand(d->session));
        enqueueCommand(command);
    }
    else
    {
        Q_EMIT signalBusyFinished(CloseAlbum);
    }
}

void RajceTalker::uploadPhoto(const QString& path, unsigned dimension, int jpgQuality)
{
    QSharedPointer<RajceCommand> command = QSharedPointer<AddPhotoCommand>(new AddPhotoCommand(d->tmpDir, path, dimension, jpgQuality, d->session));
    enqueueCommand(command);
}

void RajceTalker::clearLastError()
{
    d->session.lastErrorCode()    = 0;
    d->session.lastErrorMessage() = QLatin1String("");
}

void RajceTalker::slotUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if (bytesTotal <= 0)
    {
        return;
    }

    unsigned percent = (unsigned)((float)bytesSent / bytesTotal * 100);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Percent signalled: " << percent;

    QSharedPointer<RajceCommand> command = d->commandQueue.head();
    Q_EMIT signalBusyProgress(command.data()->commandType(), percent);
}

void RajceTalker::enqueueCommand(const QSharedPointer<RajceCommand>& command)
{
    if (d->session.lastErrorCode() != 0)
    {
        return;
    }

    d->queueAccess.lock();
    d->commandQueue.enqueue(command);

    if (d->commandQueue.size() == 1)
    {
        startCommand(command);
    }

    d->queueAccess.unlock();
}

void RajceTalker::cancelCurrentCommand()
{
    if (d->reply != nullptr)
    {
        slotFinished(d->reply);
        d->reply->abort();
        d->reply = nullptr;
    }
}

void RajceTalker::init(const RajceSession& initialState)
{
    d->session = initialState;
}

} // namespace DigikamGenericRajcePlugin

#include "moc_rajcetalker.cpp"

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

#pragma once

// Qt includes

#include <QObject>
#include <QString>
#include <QSharedPointer>

// Local includes

#include "rajcesession.h"

class QWidget;
class QNetworkReply;

namespace DigikamGenericRajcePlugin
{

class RajceCommand;

class RajceTalker : public QObject
{
    Q_OBJECT

public:

    explicit RajceTalker(QWidget* const parent);
    ~RajceTalker() override;

public:

    void  init(const RajceSession& initialState);
    const RajceSession& session() const;
    void  login(const QString& username, const QString& password);
    void  logout();

    void  loadAlbums();
    void  createAlbum(const QString& name, const QString& description, bool visible);
    void  openAlbum(const RajceAlbum& album);
    void  closeAlbum();

    void  uploadPhoto(const QString& path, unsigned dimension, int jpgQuality);

    void  clearLastError();
    void  cancelCurrentCommand();

Q_SIGNALS:

    void signalBusyStarted(unsigned);
    void signalBusyFinished(unsigned);
    void signalBusyProgress(unsigned, unsigned percent);

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);
    void slotUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:

    void startCommand(const QSharedPointer<RajceCommand>&);
    void enqueueCommand(const QSharedPointer<RajceCommand>&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericRajcePlugin

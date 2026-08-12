/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QNetworkReply>

// Local includes

#include "oditem.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericOneDrivePlugin
{

class ODTalker : public QObject
{
    Q_OBJECT

public:

    explicit ODTalker(QWidget* const parent);
    ~ODTalker() override;

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listFolders(const QString& folder = QString());
    void createFolder(QString& path);
    void setAccessToken(const QString& token);
    void readSettings();
    void writeSettings();

Q_SIGNALS:

    void signalBusy(bool val);
    void signalTransferCancel();
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalSetUserName(const QString& msg);
    void signalListAlbumsFailed(const QString& msg);
    void signalListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateFolderFailed(const QString& msg);
    void signalCreateFolderSucceeded();
    void signalAddPhotoFailed(const QString& msg);
    void signalAddPhotoSucceeded();
    void oneDriveLinkingSucceeded();
    void oneDriveLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotCatchUrl(const QUrl& url);
    void slotFinished(QNetworkReply* reply);

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListFolders(const QByteArray& data);
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericOneDrivePlugin

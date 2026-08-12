/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
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

#include "boxitem.h"
#include "o2.h"
#include "o0globals.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericBoxPlugin
{

class BOXTalker : public QObject
{
    Q_OBJECT

public:

    explicit BOXTalker(QWidget* const parent);
    ~BOXTalker() override;

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listFolders(const QString& path = QString());
    void createFolder(const QString& path);
    void setAccessToken(const QString& token);
    QMap<QString, QString> ParseUrlParameters(const QString& url);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalSetUserName(const QString& msg);
    void signalListAlbumsFailed(const QString& msg);
    void signalListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateFolderFailed(const QString& msg);
    void signalCreateFolderSucceeded();
    void signalAddPhotoFailed(const QString& msg);
    void signalAddPhotoSucceeded();
    void boxLinkingSucceeded();
    void boxLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotFinished(QNetworkReply* reply);
    void slotOpenBrowser(const QUrl& url);

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListFolders(const QByteArray& data);
/*
    QList<QPair<QString, QString> > parseListFoldersRequest(const QByteArray& data);
*/
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericBoxPlugin

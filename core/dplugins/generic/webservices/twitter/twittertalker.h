/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

// OAuth2 library includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wextra-semi"
#endif

#include "o2.h"
#include "o0globals.h"
#include "o1twitter.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "twitteritem.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericTwitterPlugin
{

class TwTalker : public QObject
{
    Q_OBJECT

public:

    explicit TwTalker(QWidget* const parent);
    ~TwTalker() override;

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listFolders(const QString& path = QString());
    void createFolder(QString& path);
    void setAccessToken(const QString& token);
    QMap<QString, QString> ParseUrlParameters(const QString& url);
    void createTweet(const QString& mediaId);

    bool addPhotoSingleUpload(const QString& imgPath);

    bool addPhotoInit(const QString& imgPath);
    bool addPhotoAppend(const QString& mediaId, int segmentIndex=0);
    bool addPhotoFinalize(const QString& mediaId);

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
    void twitterLinkingSucceeded();
    void twitterLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotOpenBrowser(const QUrl& url);
    void slotFinished(QNetworkReply* reply);

    void slotCheckUploadStatus();

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListFolders(const QByteArray& data);
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponseAddPhotoInit(const QByteArray& data);
    void parseResponseAddPhotoAppend(const QByteArray& data, int segmentIndex);
    void parseResponseAddPhotoFinalize(const QByteArray& data);
    void parseResponseCreateTweet(const QByteArray& data);
    void parseCheckUploadStatus(const QByteArray& data);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericTwitterPlugin

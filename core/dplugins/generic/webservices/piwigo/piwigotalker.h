/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2019 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QUrl>
#include <QNetworkReply>

// Local includes

#include "piwigoitem.h"
#include "dinfointerface.h"

using namespace Digikam;

template <class T> class QList;

namespace DigikamGenericPiwigoPlugin
{

class PiwigoTalker : public QObject
{
    Q_OBJECT

public:

    enum State
    {
        PG_LOGOUT = -1,
        PG_LOGIN  = 0,
        PG_GETVERSION,
        PG_LISTALBUMS,
        PG_CHECKPHOTOEXIST,
        PG_GETINFO,
        PG_SETINFO,
        PG_ADDPHOTOCHUNK,
        PG_ADDPHOTOSUMMARY
    };

    enum
    {
        CHUNK_MAX_SIZE = 512*1024,
        PIWIGO_VER_2_4 = 204
    };

public:

    explicit PiwigoTalker(DInfoInterface* const iface,
                          QWidget* const parent);
    ~PiwigoTalker() override;

public:

    bool loggedIn() const;

    void login(const QUrl& url, const QString& name, const QString& passwd);
    void listAlbums();
    void listPhotos(const QString& albumName);

/* TODO Implement this function
    void createAlbum(const QString& parentAlbumName,
                     const QString& albumName,
                     const QString& albumTitle,
                     const QString& albumCaption);
*/

    bool addPhoto(int   albumId,
                  const QString& photoPath,
                  bool  rescale = false,
                  int   maxWidth = 1600,
                  int   maxHeight = 1600,
                  int   quality = 95);

    void cancel();

    static QString getAuthToken();

Q_SIGNALS:

    void signalProgressInfo(const QString& msg);
    void signalError(const QString& msg);
    void signalLoginFailed(const QString& msg);
    void signalBusy(bool val);
    void signalAlbums(const QList<PiwigoAlbum>& albumList);
    void signalAddPhotoSucceeded();
    void signalAddPhotoFailed(const QString& msg);

private:

    void parseResponseLogin(const QByteArray& data);
    void parseResponseGetVersion(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseDoesPhotoExist(const QByteArray& data);
    void parseResponseGetInfo(const QByteArray& data);
    void parseResponseSetInfo(const QByteArray& data);

    void addNextChunk();
    void parseResponseAddPhotoChunk(const QByteArray& data);
    void addPhotoSummary();
    void parseResponseAddPhotoSummary(const QByteArray& data);

    QByteArray computeMD5Sum(const QString& filepath);
    void deleteTemporaryFile();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d = nullptr;

    static QString s_authToken;
};

} // namespace DigikamGenericPiwigoPlugin

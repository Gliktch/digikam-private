/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QString>
#include <QTime>
#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QMap>

// Local includes

#include "fbitem.h"

class QDomElement;

namespace DigikamGenericFaceBookPlugin
{

class FbTalker : public QObject
{
    Q_OBJECT

public:

    explicit FbTalker(QWidget* const parent);
    ~FbTalker();

    void    link();
    void    unlink();
    bool    linked();
    void    cancel();

    FbUser  getUser() const;

    void    logout();

    void    listAlbums(long long userID = 0);

    void    createAlbum(const FbAlbum& album);

    void    addPhoto(const QString& imgPath, const QString& albumID,
                     const QString& caption);

    void    readSettings();
    void    writeSettings();

Q_SIGNALS:

    void    signalBusy(bool val);
    void    signalListAlbumsDone(int errCode, const QString& errMsg, const QList <FbAlbum>& albumsList);
    void    signalCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumId);
    void    signalAddPhotoDone(int errCode, const QString& errMsg);
    void    signalLoginProgress(int step, int maxStep = 0, const QString& label = QString());
    void    signalLoginDone(int errCode, const QString& errMsg);
    void    linkingSucceeded();

private:

    void    getLoggedInUser();

    QString errorToText(int errCode, const QString& errMsg);
    int     parseErrorResponse(const QDomElement& e, QString& errMsg);
    void    parseResponseGetLoggedInUser(const QByteArray& data);
    void    parseResponseAddPhoto(const QByteArray& data);
    void    parseResponseCreateAlbum(const QByteArray& data);
    void    parseResponseListAlbums(const QByteArray& data);
    void    parseResponseLogoutUser();

private Q_SLOTS:

    void    slotLinkingSucceeded();
    void    slotCatchUrl(const QUrl& url);
    void    slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFaceBookPlugin

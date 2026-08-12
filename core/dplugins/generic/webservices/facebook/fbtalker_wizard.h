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
#include <QSettings>

// Local includes

#include "fbitem.h"
#include "wsnewalbumdialog.h"
#include "wstalker.h"
#include "wsitem.h"

// O2 include

#include "o2.h"
#include "o0globals.h"

class QDomElement;

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

class FbTalker : public WSTalker
{
    Q_OBJECT

public:

    explicit FbTalker(QWidget* const parent, WSNewAlbumDialog* albumDlg=0);
    ~FbTalker();

    void    link();
    void    unlink();
    bool    linked() const;

    void    resetTalker(const QString& expire, const QString& accessToken, const QString& refreshToken);

    FbUser  getUser() const;

    void    authenticate();
    void    logout();

    void    listAlbums(long long userID = 0);

    void    createNewAlbum();
    void    createAlbum(const FbAlbum& album);

    void    addPhoto(const QString& imgPath, const QString& albumID,
                     const QString& caption);

Q_SIGNALS:

    void    signalLoginProgress(int step, int maxStep = 0, const QString& label = QString());
    void    signalLoginDone(int errCode, const QString& errMsg);

private:

    //QString getApiSig(const QMap<QString, QString>& args);
    void    authenticationDone(int errCode, const QString& errMsg);
    void    getLoggedInUser();

    QString errorToText(int errCode, const QString& errMsg);
    int     parseErrorResponse(const QDomElement& e, QString& errMsg);
    void    parseResponseGetLoggedInUser(const QByteArray& data);
    void    parseResponseAddPhoto(const QByteArray& data);
    void    parseResponseCreateAlbum(const QByteArray& data);
    void    parseResponseListAlbums(const QByteArray& data);

private Q_SLOTS:

    void    slotResponseTokenReceived(const QMap<QString, QString>& rep);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFaceBookPlugin

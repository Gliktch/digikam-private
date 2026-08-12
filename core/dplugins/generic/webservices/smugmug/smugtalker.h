/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
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
#include <QObject>

// Local includes

#include "smugitem.h"
#include "dinfointerface.h"

// OAuth2 library includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wextra-semi"
#endif

#include "o0globals.h"
#include "o1smugmug.h"
#include "o1requestor.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

class QNetworkReply;

using namespace Digikam;

namespace DigikamGenericSmugPlugin
{

class SmugTalker : public QObject
{
    Q_OBJECT

public:

    explicit SmugTalker(DInfoInterface* const iface, QWidget* const parent);
    ~SmugTalker() override;

public:

    SmugUser getUser() const;

    bool    loggedIn() const;
    void    cancel();

    void    link();
    void    unlink();
    void    removeUserAccount(const QString& userName);
    void    login();
    void    getLoginedUser();
    void    logout();

    void    listAlbums(const QString& nickName = QString());
    void    listPhotos(qint64 albumID,
                       const QString& albumKey,
                       const QString& albumPassword = QString(),
                       const QString& sitePassword = QString());
    void    listAlbumTmpl();

/*  Categories are deprecated in API v2
    void    listCategories();
    void    listSubCategories(qint64 categoryID);
*/

    void    createAlbum(const SmugAlbum& album);

    bool    addPhoto(const QString& imgPath,
                     qint64 albumID,
                     const QString& albumKey,
                     const QString& caption);

    void    getPhoto(const QString& imgPath);

    QString createAlbumName(const QString& word);

    QString createAlbumUrl(const QString& name);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLoginProgress(int step,
                            int maxStep = 0,
                            const QString& label = QString());
    void signalLoginDone(int errCode, const QString& errMsg);

    void signalAddPhotoDone(int errCode, const QString& errMsg);

    void signalGetPhotoDone(int errCode, const QString& errMsg,
                            const QByteArray& photoData);
    void signalCreateAlbumDone(int errCode, const QString& errMsg, qint64 newAlbumID,
                               const QString& newAlbumKey);
    void signalListAlbumsDone(int errCode, const QString& errMsg,
                              const QList <SmugAlbum>& albumsList);
    void signalListPhotosDone(int errCode, const QString& errMsg,
                              const QList <SmugPhoto>& photosList);
    void signalListAlbumTmplDone(int errCode, const QString& errMsg,
                                 const QList <SmugAlbumTmpl>& albumTList);

/* Categories deprecated in API v2

    void signalListCategoriesDone(int errCode, const QString& errMsg,
                                  const QList <SmugCategory>& categoriesList);
    void signalListSubCategoriesDone(int errCode, const QString& errMsg,
                                     const QList <SmugCategory>& categoriesList);
*/

private:

    QString htmlToText(const QString& htmlText) const;
    QString errorToText(int errCode, const QString& errMsg) const;
    void parseResponseLogin(const QByteArray& data);

//     void parseResponseLogout(const QByteArray& data);

    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponseCreateAlbum(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseListPhotos(const QByteArray& data);
    void parseResponseListAlbumTmpl(const QByteArray& data);

/*  Categories deprecated in API v2

    void parseResponseListCategories(const QByteArray& data);
    void parseResponseListSubCategories(const QByteArray& data);
*/

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);
    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotOpenBrowser(const QUrl& url);
    void slotCloseBrowser();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSmugPlugin

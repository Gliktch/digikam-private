/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2009 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2019 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QList>
#include <QPair>
#include <QString>
#include <QObject>
#include <QNetworkReply>

// Local includes

#include "dinfointerface.h"
#include "flickritem.h"

class QProgressDialog;

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

class FPhotoInfo;
class FPhotoSet;

class FlickrTalker : public QObject
{
    Q_OBJECT

public:

    enum State
    {
        FE_LOGOUT = -1,
        FE_LOGIN  = 0,
        FE_LISTPHOTOSETS,
        FE_LISTPHOTOS,
        FE_GETPHOTOPROPERTY,
        FE_ADDPHOTO,
        FE_CREATEPHOTOSET,
        FE_ADDPHOTOTOPHOTOSET,
        FE_GETMAXSIZE,
        FE_SETGEO
    };

public:

    explicit FlickrTalker(QWidget* const parent, const QString& serviceName,
                          DInfoInterface* const iface);
    ~FlickrTalker() override;

    void    link(const QString& userName);
    void    unLink();
    void    removeUserName(const QString& userName);
    QString getUserName() const;
    QString getUserId() const;
    void    maxAllowedFileSize();
    QString getMaxAllowedFileSize();
    void    getPhotoProperty(const QString& method, const QStringList& argList);
    void    cancel();

    void    listPhotoSets();
    void    listPhotos(const QString& albumName);
    void    createPhotoSet(const QString& name,
                           const QString& title,
                           const QString& desc,
                           const QString& primaryPhotoId);

    void    addPhotoToPhotoSet(const QString& photoId, const QString& photoSetId);
    bool    addPhoto(const QString& photoPath, const FPhotoInfo& info,
                     bool original = false, bool rescale = false,
                     int maxDim = 600, int imageQuality = 85);
    void    setGeoLocation(const QString& photoId, const QString& lat, const QString& lon);

public:

    QProgressDialog*  m_authProgressDlg  = nullptr;
    QList<FPhotoSet>* m_photoSetsList;
    FPhotoSet         m_selectedPhotoSet;

Q_SIGNALS:

    void signalError(const QString& msg);
    void signalBusy(bool val);
    void signalAddPhotoSucceeded(const QString&);
    void signalAddPhotoSetSucceeded();
    void signalListPhotoSetsSucceeded();
    void signalAddPhotoFailed(const QString& msg);
    void signalListPhotoSetsFailed(const QString& msg);
    void signalLinkingSucceeded();

private:

/*
    void parseResponseLogin(const QByteArray& data);
*/
    void parseResponseMaxSize(const QByteArray& data);
    void parseResponseListPhotoSets(const QByteArray& data);
    void parseResponseListPhotos(const QByteArray& data);
    void parseResponseCreateAlbum(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponsePhotoProperty(const QByteArray& data);
    void parseResponseCreatePhotoSet(const QByteArray& data);
    void parseResponseAddPhotoToPhotoSet(const QByteArray& data);
    void parseResponseSetGeoLocation(const QByteArray& data);

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotCatchUrl(const QUrl& url);
    void slotOpenBrowser(const QUrl& url);
    void slotError(const QString& msg);
    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericFlickrPlugin

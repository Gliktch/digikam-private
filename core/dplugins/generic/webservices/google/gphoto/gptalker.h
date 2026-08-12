/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-16-07
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QPointer>

// Local includes

#include "gsitem.h"
#include "gstalkerbase.h"

namespace DigikamGenericGoogleServicesPlugin
{

class GPTalker : public GSTalkerBase
{
    Q_OBJECT

public:

    explicit GPTalker(QWidget* const parent);
    ~GPTalker() override;

public:

    void    getLoggedInUser();

    void    listAlbums(const QString& nextPageToken = QString());
    void    listPhotos(const QString& albumId,
                       const QString& nextPageToken = QString());

    void    createAlbum(const GSFolder& newAlbum);

    bool    addPhoto(const QString& photoPath,
                     const GSPhoto& info,
                     const QString& albumId,
                     bool original,
                     bool rescale,
                     int maxDim,
                     int imageQuality);
    bool    updatePhoto(const QString& photoPath,
                        GSPhoto& info,
                        bool rescale,
                        int maxDim,
                        int imageQuality);

    void    getPhoto(const QString& imgPath);

    QStringList getUploadTokenList();

    void cancel();

Q_SIGNALS:

    void signalSetUserName(const QString& msg);
    void signalError(const QString& msg);
    void signalListAlbumsDone(int, const QString&, const QList <GSFolder>&);
    void signalListPhotosDone(int, const QString&, const QList <GSPhoto>&);
    void signalCreateAlbumDone(int, const QString&, const QString&);
    void signalAddPhotoDone(int, const QString&);
    void signalUploadPhotoDone(int, const QString&, const QStringList&);
    void signalGetPhotoDone(int errCode, const QString& errMsg,
                            const QByteArray& photoData, const QString& fileName);
    void signalReadyToUpload();

private:

    void parseResponseGetLoggedInUser(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseListPhotos(const QByteArray& data);
    void parseResponseCreateAlbum(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponseUploadPhoto(const QByteArray& data);

private Q_SLOTS:

    void slotError(const QString& msg);
    void slotFinished(QNetworkReply* reply);
    void slotUploadPhoto();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGoogleServicesPlugin

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
 * SPDX-FileCopyrightText: 2009      by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gptalker.h"

// Qt includes

#include <QMimeDatabase>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QUrlQuery>
#include <QApplication>
#include <QDir>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wstoolutils.h"
#include "digikam_version.h"
#include "gswindow.h"
#include "gpmpform.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

static const int NB_MAX_ITEM_UPLOAD = 50;

static bool gphotoLessThan(const GSFolder& p1, const GSFolder& p2)
{
    return (p1.title.toLower() < p2.title.toLower());
}

class Q_DECL_HIDDEN GPTalker::Private
{
public:

    enum State
    {
        GP_LOGOUT     = -1,
        GP_LISTALBUMS = 0,
        GP_GETUSER,
        GP_LISTPHOTOS,
        GP_ADDPHOTO,
        GP_UPDATEPHOTO,
        GP_UPLOADPHOTO,
        GP_GETPHOTO,
        GP_CREATEALBUM
    };

public:

    Private() = default;

public:

    QString                apiVersion       = QLatin1String("v1");

    QString                userInfoUrl      = QString::fromLatin1("https://www.googleapis.com/plus/%1/people/me").arg(apiVersion);
    QString                apiUrl           = QString::fromLatin1("https://photoslibrary.googleapis.com/%1/%2").arg(apiVersion);

    State                  state            = GP_LOGOUT;

    QString                albumIdToUpload  = QLatin1String("-1");
    QString                albumIdToImport;
    QString                previousImageId  = QLatin1String("-1");
    QString                currDescription;

    QStringList            descriptionList;
    QStringList            uploadTokenList;
    QList<GSFolder>        albumList;
    QList<GSPhoto>         photoList;
};

GPTalker::GPTalker(QWidget* const parent)
    : GSTalkerBase(parent,
                   QStringList() // to get user login (temporary until gphoto supports it officially)
                                 << QLatin1String("https://www.googleapis.com/auth/plus.login")
                                 // to add and download photo in the library
                                 << QLatin1String("https://www.googleapis.com/auth/photoslibrary")
                                 // to download photo created by digiKam on GPhoto
                                 << QLatin1String("https://www.googleapis.com/auth/photoslibrary.readonly.appcreateddata")
                                 // for shared albums
                                 << QLatin1String("https://www.googleapis.com/auth/photoslibrary.sharing"),
                   QLatin1String("GooglePhotos")),
      d(new Private)
{
    connect(m_service->networkAccessManager(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(signalError(QString)),
            this, SLOT(slotError(QString)));

    connect(this, SIGNAL(signalReadyToUpload()),
            this, SLOT(slotUploadPhoto()));
}

GPTalker::~GPTalker()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    WSToolUtils::removeTemporaryDir("google");

    delete d;
}

QStringList GPTalker::getUploadTokenList()
{
    return d->uploadTokenList;
}

/**
 * (Trung): Comments below are not valid anymore with google photos api
 * Google Photo's Album listing request/response
 * First a request is sent to the url below and then we might(?) get a redirect URL
 * We then need to send the GET request to the Redirect url.
 * This uses the authenticated album list fetching to get all the albums included the unlisted-albums
 * which is not returned for an unauthorised request as done without the Authorization header.
 */
void GPTalker::listAlbums(const QString& nextPageToken)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "list albums";

    QUrl url(d->apiUrl.arg(QLatin1String("albums")));

    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("pageSize"), QLatin1String("50"));

    if (nextPageToken.isEmpty())
    {
        d->albumList.clear();
    }
    else
    {
        query.addQueryItem(QLatin1String("pageToken"), nextPageToken);
    }

    url.setQuery(query);

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url for list albums" << url;

    m_reply  = m_service->get(url);

    d->state = Private::GP_LISTALBUMS;

    Q_EMIT signalBusy(true);
}

/**
 * We get user profile from Google Plus API
 * This is a temporary solution until Google Photo support API for user profile
 */
void GPTalker::getLoggedInUser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getLoggedInUser";

    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    QUrl url(d->userInfoUrl);

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url for list albums" << url;

    m_reply  = m_service->get(url);

    d->state = Private::GP_GETUSER;

    Q_EMIT signalBusy(true);
}

void GPTalker::listPhotos(const QString& albumId, const QString& nextPageToken)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    d->albumIdToImport = albumId;

    if (nextPageToken.isEmpty())
    {
        d->photoList.clear();
    }

    QUrl url(d->apiUrl.arg(QLatin1String("mediaItems:search")));

    QByteArray data;
    data += "{\"pageSize\": \"100\",";

    if (!nextPageToken.isEmpty())
    {
        data += "\"pageToken\": \"";
        data += nextPageToken.toLatin1();
        data += "\",";
    }

    data += "\"albumId\": \"";
    data += albumId.toLatin1();
    data += "\"}";

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << "data to list photos:" << data;

    m_reply  = m_service->post(url, data);

    d->state = Private::GP_LISTPHOTOS;

    Q_EMIT signalBusy(true);
}

void GPTalker::createAlbum(const GSFolder& album)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    // Create body in json

    QByteArray data;
    data += "{\"album\": ";
    data += "{\"title\": \"";
    data += album.title.toUtf8();
    data += "\"}}";

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << data;

    QUrl url(d->apiUrl.arg(QLatin1String("albums")));

    m_reply  = m_service->post(url, data);

    d->state = Private::GP_CREATEALBUM;

    Q_EMIT signalBusy(true);
}

/**
 * First a request is sent to the url below and then we will get an upload token
 * Upload token then will be sent with url in GPTlaker::uploadPhoto to create real photos on user account
 */
bool GPTalker::addPhoto(const QString& photoPath,
                        const GSPhoto& info,
                        const QString& albumId,
                        bool original,
                        bool rescale,
                        int maxDim,
                        int imageQuality)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    QUrl url(d->apiUrl.arg(QLatin1String("uploads")));

    // Save album ID and description to upload

    d->currDescription = info.description;
    d->albumIdToUpload = albumId;

    QString path(photoPath);

    QMimeDatabase mimeDB;

    if (!original && mimeDB.mimeTypeForFile(photoPath).name().startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (image.isNull())
        {
            return false;
        }

        path = WSToolUtils::makeTemporaryDir("google").filePath(QFileInfo(photoPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && ((image.width() > maxDim) || (image.height() > maxDim)))
        {
            image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image.save(path, "JPEG", imageQuality);

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (meta->load(photoPath))
        {
            meta->setItemDimensions(image.size());
            meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta->save(path, true);
        }
    }

    // Create the body for temporary upload

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray data = imageFile.readAll();
    imageFile.close();

    QString imageName = QUrl::fromLocalFile(path).fileName();

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    netRequest.setRawHeader("X-Goog-Upload-File-Name", imageName.toUtf8());
    netRequest.setRawHeader("X-Goog-Upload-Protocol", "raw");

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << imageName;

    m_reply  = m_service->networkAccessManager()->post(netRequest, data);

    d->state = Private::GP_ADDPHOTO;

    Q_EMIT signalBusy(true);

    return true;
}

bool GPTalker::updatePhoto(const QString& /*photoPath*/, GSPhoto& /*info*/,
                           bool /*rescale*/, int /*maxDim*/, int /*imageQuality*/)
{
/*
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    GPMPForm form;
    QString path = photoPath;

    QMimeDatabase mimeDB;

    if (mimeDB.mimeTypeForFile(photoPath).name().startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (image.isNull())
        {
            Q_EMIT signalBusy(false);
            return false;
        }

        path = WSToolUtils::makeTemporaryDir("google").filePath(QFileInfo(photoPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && (image.width() > maxDim || image.height() > maxDim))
        {
            image = image.scaled(maxDim,maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image.save(path, "JPEG", imageQuality);

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (meta->load(photoPath))
        {
            meta->setItemDimensions(image.size());
            meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta->save(path, true);
        }
    }

    QNetworkRequest netRequest(info.editUrl);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1() + "\nIf-Match: *");

    m_reply = m_netMngr->put(netRequest, form.formData());

    d->state = Private::GP_UPDATEPHOTO;
*/
    return true;
}

void GPTalker::getPhoto(const QString& imgPath)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    QUrl url(imgPath);

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << "link to get photo" << url;

    m_reply  = m_service->get(url);

    d->state = Private::GP_GETPHOTO;
}

void GPTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    d->descriptionList.clear();
    d->uploadTokenList.clear();

    Q_EMIT signalBusy(false);
}

void GPTalker::slotError(const QString& error)
{
    QString transError;
    int     errorNo = 0;

    if (!error.isEmpty())
    {
        errorNo = error.toInt();
    }

    switch (errorNo)
    {
        case 2:
        {
            transError=i18n("No photo specified");
            break;
        }

        case 3:
        {
            transError=i18n("General upload failure");
            break;
        }

        case 4:
        {
            transError=i18n("File-size was zero");
            break;
        }

        case 5:
        {
            transError=i18n("File-type was not recognized");
            break;
        }

        case 6:
        {
            transError=i18n("User exceeded upload limit");
            break;
        }

        case 96:
        {
            transError=i18n("Invalid signature");
            break;
        }

        case 97:
        {
            transError=i18n("Missing signature");
            break;
        }

        case 98:
        {
            transError=i18n("Login failed / Invalid auth token");
            break;
        }

        case 100:
        {
            transError=i18n("Invalid API Key");
            break;
        }

        case 105:
        {
            transError=i18n("Service currently unavailable");
            break;
        }

        case 108:
        {
            transError=i18n("Invalid Frob");
            break;
        }

        case 111:
        {
            transError=i18n("Format \"xxx\" not found");
            break;
        }

        case 112:
        {
            transError=i18n("Method \"xxx\" not found");
            break;
        }

        case 114:
        {
            transError=i18n("Invalid SOAP envelope");
            break;
        }

        case 115:
        {
            transError=i18n("Invalid XML-RPC Method Call");
            break;
        }

        case 116:
        {
            transError=i18n("The POST method is now required for all setters.");
            break;
        }

        default:
        {
            transError=i18n("Unknown error");
        }
    };

    QMessageBox::critical(QApplication::activeWindow(), i18nc("@title:window", "Error"),
                          i18n("Error occurred: %1\nUnable to proceed further.", transError + error));
}

void GPTalker::slotFinished(QNetworkReply* reply)
{
    Q_EMIT signalBusy(false);

    if (reply != m_reply)
    {
        return;
    }

    m_reply = nullptr;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "reply error:" << reply->error()
                                     << "-" << reply->errorString();

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == Private::GP_ADDPHOTO)
        {
            Q_EMIT signalAddPhotoDone(reply->error(), reply->errorString());
        }
        else if (reply->error() != QNetworkReply::OperationCanceledError)
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());
        }

        reply->deleteLater();
        return;
    }

    QByteArray buffer = reply->readAll();

    switch (d->state)
    {
        case (Private::GP_LOGOUT):
        {
            break;
        }

        case (Private::GP_GETUSER):
        {
            parseResponseGetLoggedInUser(buffer);
            break;
        }

        case (Private::GP_CREATEALBUM):
        {
            parseResponseCreateAlbum(buffer);
            break;
        }

        case (Private::GP_LISTALBUMS):
        {
            parseResponseListAlbums(buffer);
            break;
        }

        case (Private::GP_LISTPHOTOS):
        {
            parseResponseListPhotos(buffer);
            break;
        }

        case (Private::GP_ADDPHOTO):
        {
            parseResponseAddPhoto(buffer);
            break;
        }

        case (Private::GP_UPDATEPHOTO):
        {
            Q_EMIT signalAddPhotoDone(1, QString());
            break;
        }

        case (Private::GP_UPLOADPHOTO):
        {
            parseResponseUploadPhoto(buffer);
            break;
        }

        case (Private::GP_GETPHOTO):
        {
            // all we get is data of the image
            // qCDebug(DIGIKAM_WEBSERVICES_LOG) << buffer;

            QString header         = reply->header(QNetworkRequest::ContentDispositionHeader).toString();
            QStringList headerList = header.split(QLatin1Char(';'));
            QString fileName;

            if (
                (headerList.count() > 1)                          &&
                (headerList.at(0) == QLatin1String("attachment")) &&
                headerList.at(1).contains(QLatin1String("filename="))
               )
            {
                fileName = headerList.at(1).section(QLatin1Char('"'), 1, 1);
            }

            Q_EMIT signalGetPhotoDone(1, QString(), buffer, fileName);

            break;
        }
    }

    reply->deleteLater();
}

void GPTalker::slotUploadPhoto()
{
    /* Keep track of number of items will be uploaded, because
     * Google Photo API upload maximum NB_MAX_ITEM_UPLOAD items in at a time
     */
    int nbItemsUpload = 0;

    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    QUrl url(d->apiUrl.arg(QLatin1String("mediaItems:batchCreate")));

    QByteArray data;
    data += '{';

    if (d->albumIdToUpload != QLatin1String("-1"))
    {
        data += "\"albumId\": \"";
        data += d->albumIdToUpload.toLatin1();
        data += "\",";
    }

    data += "\"newMediaItems\": [";

    if (d->uploadTokenList.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "token list is empty";
    }

    while (!d->uploadTokenList.isEmpty() && nbItemsUpload < NB_MAX_ITEM_UPLOAD)
    {
        const QString& uploadToken = d->uploadTokenList.takeFirst();
        data += "{\"description\": \"";

        if (d->descriptionList.isEmpty())
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "description list is empty";
        }
        else
        {
            data += d->descriptionList.takeFirst().toUtf8();
        }

        data += "\",";
        data += "\"simpleMediaItem\": {";
        data += "\"uploadToken\": \"";
        data += uploadToken.toLatin1();
        data += "\"}}";

        if (d->uploadTokenList.length() > 0)
        {
            data += ',';
        }

        nbItemsUpload ++;
    }

    if (d->previousImageId == QLatin1String("-1"))
    {
        data += ']';
    }
    else
    {
        data += "],\"albumPosition\": {";
        data += "\"position\": \"AFTER_MEDIA_ITEM\",";
        data += "\"relativeMediaItemId\": \"";
        data += d->previousImageId.toLatin1();
        data += "\"}\r\n";
    }

    data += "}\r\n";

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << data;

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());

    m_reply  = m_service->networkAccessManager()->post(netRequest, data);

    d->state = Private::GP_UPLOADPHOTO;

    Q_EMIT signalBusy(true);
}

void GPTalker::parseResponseListAlbums(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListAlbums";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalListAlbumsDone(0, QString::fromLatin1("Code: %1 - %2").arg(err.error)
                                                                           .arg(err.errorString()),
                                    QList<GSFolder>());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonArray jsonArray    = jsonObject[QLatin1String("albums")].toArray();

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << "json array " << doc;

    /**
     * Google-photos allows user to post photos on their main page (not in any albums)
     * so this folder is created for that purpose
     */

    if (d->albumList.isEmpty())
    {
        GSFolder mainPage;
        d->albumList.append(mainPage);
    }

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        GSFolder album;

        QJsonObject obj     = value.toObject();
        album.id            = obj[QLatin1String("id")].toString();
        album.title         = obj[QLatin1String("title")].toString();
        album.url           = obj[QLatin1String("productUrl")].toString();
        album.isWriteable   = obj[QLatin1String("isWriteable")].toBool();

        d->albumList.append(album);
    }

    QString nextPageToken   = jsonObject[QLatin1String("nextPageToken")].toString();

    if (!nextPageToken.isEmpty())
    {
        listAlbums(nextPageToken);
        return;
    }

    std::sort(d->albumList.begin(), d->albumList.end(), gphotoLessThan);

    Q_EMIT signalListAlbumsDone(1, QLatin1String(""), d->albumList);
}

void GPTalker::parseResponseListPhotos(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotos";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalListPhotosDone(0, i18n("Failed to fetch photo-set list"), QList<GSPhoto>());

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error code:" << err.error
                                         << ", msg:" << err.errorString();
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonArray jsonArray    = jsonObject[QLatin1String("mediaItems")].toArray();

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QJsonObject obj = value.toObject();

        GSPhoto photo;

        photo.baseUrl        = obj[QLatin1String("baseUrl")].toString();
        photo.description    = obj[QLatin1String("description")].toString();
        photo.id             = obj[QLatin1String("id")].toString();
        photo.mimeType       = obj[QLatin1String("mimeType")].toString();
        photo.location       = obj[QLatin1String("Location")].toString(); // Not yet available in v1 but will be in the future

        QJsonObject metadata = obj[QLatin1String("mediaMetadata")].toObject();

        photo.creationTime   = metadata[QLatin1String("creationTime")].toString();
        photo.width          = metadata[QLatin1String("width")].toString();
        photo.height         = metadata[QLatin1String("height")].toString();

        QString option       = QLatin1String("=d");

        if (photo.mimeType.startsWith(QLatin1String("video/")))
        {
            option.append(QLatin1Char('v'));
        }

        photo.originalURL    = QUrl(photo.baseUrl + option);

        // qCDebug(DIGIKAM_WEBSERVICES_LOG) << photo.originalURL;

        d->photoList.append(photo);
    }

    QString nextPageToken    = jsonObject[QLatin1String("nextPageToken")].toString();

    if (!nextPageToken.isEmpty())
    {
        listPhotos(d->albumIdToImport, nextPageToken);
        return;
    }

    Q_EMIT signalListPhotosDone(1, QLatin1String(""), d->photoList);
}

void GPTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateAlbums";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalCreateAlbumDone(0, QString::fromLatin1("Code: %1 - %2").arg(err.error)
                                                                            .arg(err.errorString()),
                                     QString());
        return;
    }

    QJsonObject jsonObject  = doc.object();
    QString albumId         = jsonObject[QLatin1String("id")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "album Id"  << doc;

    Q_EMIT signalCreateAlbumDone(1, QLatin1String(""), albumId);
}

void GPTalker::parseResponseAddPhoto(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhoto";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "response" << data;

    d->uploadTokenList << QString::fromUtf8(data);
    d->descriptionList << d->currDescription;

    Q_EMIT signalAddPhotoDone(1, QString());
}

void GPTalker::parseResponseGetLoggedInUser(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseGetLoggedInUser";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    QString userName       = jsonObject[QLatin1String("displayName")].toString();

    Q_EMIT signalSetUserName(userName);

    listAlbums();
}

// TODO: Parse and return photoID

void GPTalker::parseResponseUploadPhoto(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUploadPhoto";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "doc" << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalUploadPhotoDone(0, err.errorString(), QStringList());

        return;
    }

    QJsonObject jsonObject  = doc.object();
    QJsonArray jsonArray    = jsonObject[QLatin1String("newMediaItemResults")].toArray();

    QStringList listPhotoId;

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QJsonObject obj = value.toObject();

        QJsonObject mediaItem = obj[QLatin1String("mediaItem")].toObject();
        listPhotoId << mediaItem[QLatin1String("id")].toString();
    }

    d->previousImageId = listPhotoId.last();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "list photo Id" << listPhotoId.join(QLatin1String(", "));

    Q_EMIT signalBusy(false);
    Q_EMIT signalUploadPhotoDone(1, QString(), listPhotoId);
}

} // namespace DigikamGenericGoogleServicesPlugin

#include "moc_gptalker.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 * SPDX-FileCopyrightText: 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "twittertalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QUrl>
#include <QUrlQuery>
#include <QFileInfo>
#include <QWidget>
#include <QScopedPointer>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>

/*
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
*/

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "twitterwindow.h"
#include "twittermpform.h"
#include "previewloadthread.h"
#include "networkmanager.h"
#include "o0settingsstore.h"
#include "o1requestor.h"

namespace DigikamGenericTwitterPlugin
{

QStringList imageFormat(QString::fromLatin1("jpg,png,gif,webp").split(QLatin1Char(',')));

class Q_DECL_HIDDEN TwTalker::Private
{
public:

    enum State
    {
        TW_USERNAME = 0,
        TW_LISTFOLDERS,
        TW_CREATEFOLDER,
        TW_ADDPHOTO,
        TW_CREATETWEET,
        TW_UPLOADINIT,
        TW_UPLOADAPPEND,
        TW_UPLOADSTATUSCHECK,
        TW_UPLOADFINALIZE
    };

public:

    Private() = default;

public:

    QString                clientId             = QLatin1String("lkRgRsucipXsUEvKh0ECblreC");
    QString                clientSecret         = QLatin1String("6EThTiPQHZTMo7F83iLHrfNO89fkDVvM9hVWaYH9D49xEOyMBe");
    QString                authUrl              = QLatin1String("https://api.twitter.com/oauth/authenticate");
    QString                requestTokenUrl      = QLatin1String("https://api.twitter.com/oauth/request_token");
    QString                accessTokenUrl       = QLatin1String("https://api.twitter.com/oauth/access_token");
    QString                redirectUrl          = QLatin1String("http://127.0.0.1:8000");                            // krazy:exclude=insecurenet
    QString                uploadUrl            = QLatin1String("https://upload.twitter.com/1.1/media/upload.json");
/*
    QString                scope                = QLatin1String("User.Read Files.ReadWrite");
*/
    QString                accessToken;
    QString                mediaUploadedPath;
    QString                mediaId;

    int                    segmentIndex         = 0;

    QWidget*               parent               = nullptr;

    QNetworkAccessManager* netMngr              = nullptr;

    QNetworkReply*         reply                = nullptr;

    State                  state                = TW_USERNAME;

    QMap<QString, QString> urlParametersMap;
/*
    QWebEngineView*        view                 = nullptr;
*/
    QSettings*             settings             = nullptr;

    O1Twitter*             o1Twitter            = nullptr;
    O1Requestor*           requestor            = nullptr;
};

TwTalker::TwTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->o1Twitter = new O1Twitter(this);
    d->o1Twitter->setClientId(d->clientId);
    d->o1Twitter->setClientSecret(d->clientSecret);
    d->o1Twitter->setLocalPort(8000);

    d->requestor = new O1Requestor(d->netMngr, d->o1Twitter, this);

    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Twitter"));
    d->o1Twitter->setStore(store);

    connect(d->o1Twitter, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o1Twitter, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1Twitter, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}

TwTalker::~TwTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir("twitter");

    delete d;
}

void TwTalker::link()
{
/*
    Q_EMIT signalBusy(true);

    QUrl url(d->requestTokenUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("OAuth oauth_callback= \"%1\"").arg(d->redirectUrl).toUtf8());
    QNetworkAccessManager requestMngr;
    QNetworkReply* reply;
    reply = requestMngr.post(netRequest);

    if (reply->error() != QNetworkReply::NoError)
    {
    }

    QByteArray buffer;
    buffer.append(reply->readAll());
    QString response = fromLatin1(buffer);

    QMultiMap<QString, QString> headers;

    // Discard the first line

    response = response.mid(response.indexOf('\n') + 1).trimmed();
    const auto lines = response.split('\n');

    for (QString line : lines)
    {
        int colon = line.indexOf(':');
        QString headerName = line.left(colon).trimmed();
        QString headerValue = line.mid(colon + 1).trimmed();

        headers.insert(headerName, headerValue);
    }

    QString oauthToken = headers[oauth_token];
    QSting oauthTokenSecret = headers[oauth_token_secret];

    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"),     d->clientId);
    query.addQueryItem(QLatin1String("scope"),         d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"),  d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), "token");
    url.setQuery(query);

    d->view = new QWebEngineView(d->parent);
    d->view->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    d->view->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    d->view->setWindowFlags(Qt::Dialog);
    d->view->load(url);
    d->view->show();

    connect(d->view, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));
*/

    Q_EMIT signalBusy(true);

    d->o1Twitter->link();
}

void TwTalker::unLink()
{
/*
    d->accessToken = QString();
    d->view->page()->profile()->cookieStore()->deleteAllCookies();

    Q_EMIT oneDriveLinkingSucceeded();
*/

    d->o1Twitter->unlink();

    d->settings->beginGroup(QLatin1String("Twitter"));
    d->settings->remove(QString());
    d->settings->endGroup();
}

void TwTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

QMap<QString, QString> TwTalker::ParseUrlParameters(const QString& url)
{
    QMap<QString, QString> urlParameters;

    if (url.indexOf(QLatin1Char('?')) == -1)
    {
        return urlParameters;
    }

    QString tmp           = url.right(url.length()-url.indexOf(QLatin1Char('?'))-1);
    tmp                   = tmp.right(tmp.length() - tmp.indexOf(QLatin1Char('#'))-1);
    QStringList paramlist = tmp.split(QLatin1Char('&'));

    for (int i = 0 ; i < paramlist.count() ; ++i)
    {
        QStringList paramarg = paramlist.at(i).split(QLatin1Char('='));
        urlParameters.insert(paramarg.at(0),paramarg.at(1));
    }

    return urlParameters;
}

void TwTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter fail";

    Q_EMIT signalBusy(false);
}

void TwTalker::slotLinkingSucceeded()
{
    if (!d->o1Twitter->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Twitter ok";

        Q_EMIT signalBusy(false);

        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Twitter ok";
    QVariantMap extraTokens = d->o1Twitter->extraTokens();

    if (!extraTokens.isEmpty())
    {
        //Q_EMIT extraTokensReady(extraTokens);

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Extra tokens in response:";
        const auto keys = extraTokens.keys();

        for (const QString& key : keys)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "\t"
                                             << key
                                             << ":"
                                             << (extraTokens.value(key).toString().left(3) + QLatin1String("..."));
        }
    }

    Q_EMIT signalLinkingSucceeded();

    getUserName();
}

bool TwTalker::authenticated()
{
    return d->o1Twitter->linked();
}

void TwTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(false);
}

bool TwTalker::addPhoto(const QString& imgPath,
                        const QString& /* uploadFolder */,
                        bool rescale,
                        int maxDim,
                        int imageQuality)
{

    QFileInfo imgFileInfo(imgPath);
    QString path;
    bool chunked = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << imgFileInfo.suffix();

    if (
        (imgFileInfo.suffix() != QLatin1String("gif")) &&
        (imgFileInfo.suffix() != QLatin1String("mp4"))
       )
    {
        QImage image     = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();
        qint64 imageSize = QFileInfo(imgPath).size();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "SIZE of image using qfileinfo:   " << imageSize;
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << " ";

        if (image.isNull())
        {
            Q_EMIT signalBusy(false);

            return false;
        }

        path = WSToolUtils::makeTemporaryDir("twitter").filePath(imgFileInfo.baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && ((image.width() > maxDim) || (image.height() > maxDim)))
        {
            image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image.save(path, "JPEG", imageQuality);

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (meta->load(imgPath))
        {
            meta->setItemDimensions(image.size());
            meta->setItemOrientation(DMetadata::ORIENTATION_NORMAL);
            meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta->save(path, true);
        }
    }
    else
    {
        path = imgPath;
        chunked = true;
    }

    if (chunked)
    {
        return addPhotoInit(path);
    }
    else
    {
        return addPhotoSingleUpload(path);
    }
}

bool TwTalker::addPhotoSingleUpload(const QString& imgPath)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "addPhotoSingleUpload";

    Q_EMIT signalBusy(true);

    TwMPForm form;

    if (!form.addFile(imgPath))
    {
        Q_EMIT signalBusy(false);
        return false;
    }

    form.finish();

    if (form.formData().isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Form DATA Empty:";
    }

    if (form.formData().isNull())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Form DATA null:";
    }

    QUrl url = QUrl(QLatin1String("https://upload.twitter.com/1.1/media/upload.json"));

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    d->reply = d->requestor->post(request, reqParams, form.formData());

    d->state = Private::TW_ADDPHOTO;

    return true;
}

bool TwTalker::addPhotoInit(const QString& imgPath)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "addPhotoInit";

    Q_EMIT signalBusy(true);

    TwMPForm form;
    QByteArray mediaType, mediaCategory;
    QFileInfo fileInfo(imgPath);
    QString fileFormat(fileInfo.suffix());

    form.addPair(form.createPair("command", "INIT"));
    form.addPair(form.createPair("total_bytes", QString::number(QFileInfo(imgPath).size()).toLatin1()));

    /* (Feb 2019)
     * Image file must be <= 5MB
     * Gif must be <= 15MB
     * Video must be <= 512MB
     */

    if      (imageFormat.indexOf(fileFormat) != -1)
    {
        mediaType = "image/jpeg";

        if (fileFormat == QLatin1String("gif"))
        {

            if (fileInfo.size() > 15728640)
            {
                Q_EMIT signalBusy(false);
                Q_EMIT signalAddPhotoFailed(i18n("File too big to upload"));

                return false;
            }

            mediaCategory = "TWEET_GIF";
        }
        else
        {
            if (fileInfo.size() > 5242880)
            {
                Q_EMIT signalBusy(false);
                Q_EMIT signalAddPhotoFailed(i18n("File too big to upload"));

                return false;
            }

            mediaCategory = "TWEET_IMAGE";
        }
    }
    else if (fileFormat == QLatin1String("mp4"))
    {
        if (fileInfo.size() > 536870912)
        {
            Q_EMIT signalBusy(false);
            Q_EMIT signalAddPhotoFailed(i18n("File too big to upload"));

            return false;
        }

        mediaType = "video/mp4";
        mediaCategory = "TWEET_VIDEO";
    }
    else
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Media format is not supported yet"));

        return false;
    }

    form.addPair(form.createPair("media_type", mediaType));
    form.addPair(form.createPair("media_category", mediaCategory));
    form.finish();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << form.formData();

    QUrl url(d->uploadUrl);

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    d->reply = d->requestor->post(request, reqParams, form.formData());

    d->mediaUploadedPath = imgPath;

    d->state = Private::TW_UPLOADINIT;

    return true;
}

bool TwTalker::addPhotoAppend(const QString& mediaId, int segmentIndex)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "addPhotoAppend: ";

    static TwMPForm form;

    if (segmentIndex == 0)
    {
        form.addPair(form.createPair("command", "APPEND"));
        form.addPair(form.createPair("media_id", mediaId.toLatin1()));
        form.addFile(d->mediaUploadedPath, true);
        d->segmentIndex = form.numberOfChunks() - 1;
    }

    QByteArray data(form.formData());
    data.append(form.createPair("segment_index", QString::number(segmentIndex).toLatin1()));
    data.append(form.createPair("media", form.getChunk(segmentIndex)));
    data.append(form.border());

    QUrl url(d->uploadUrl);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    d->reply = d->requestor->post(request, reqParams, data);

    d->state = Private::TW_UPLOADAPPEND;

    // Reset form for later uploads

    if (segmentIndex == d->segmentIndex)
    {
        form.reset();
    }

    return true;
}

bool TwTalker::addPhotoFinalize(const QString& mediaId)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "addPhotoFinalize: ";

    TwMPForm form;

    form.addPair(form.createPair("command", "FINALIZE"));
    form.addPair(form.createPair("media_id", mediaId.toLatin1()));
    form.finish();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << form.formData();

    QUrl url(d->uploadUrl);

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    d->reply = d->requestor->post(request, reqParams, form.formData());

    d->state = Private::TW_UPLOADFINALIZE;

    return true;
}

void TwTalker::getUserName()
{
    /*
     * The endpoint below allows to get more than just account name (e.g. profile avatar, links to tweets posted, etc.)
     * Look at debug message printed to console for further ideas and exploration
     */
    QUrl url(QLatin1String("https://api.twitter.com/1.1/account/verify_credentials.json"));

    QNetworkRequest request(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    d->reply = d->requestor->get(request, reqParams);
    d->state = Private::TW_USERNAME;

    Q_EMIT signalBusy(true);
}

void TwTalker::createTweet(const QString& mediaId)
{
    QUrl url = QUrl(QLatin1String("https://api.twitter.com/1.1/statuses/update.json"));

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
    reqParams << O0RequestParameter(QByteArray("status"), QByteArray(""));
    reqParams << O0RequestParameter(QByteArray("media_ids"), mediaId.toUtf8());
    QByteArray  postData                = O1::createQueryParameters(reqParams);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));
    d->reply = d->requestor->post(request, reqParams, postData);

    d->state = Private::TW_CREATETWEET;
}

void TwTalker::slotCheckUploadStatus()
{
    QUrl url = QUrl(d->uploadUrl);

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();
    reqParams << O0RequestParameter(QByteArray("command"), QByteArray("STATUS"));
    reqParams << O0RequestParameter(QByteArray("media_id"), d->mediaId.toUtf8());

    QUrlQuery query;
    query.addQueryItem(QLatin1String("command"), QLatin1String("STATUS"));
    query.addQueryItem(QLatin1String("media_id"), d->mediaId);

    url.setQuery(query);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << url.toString();

    QNetworkRequest request(url);
    d->reply = d->requestor->get(request, reqParams);
    d->state = Private::TW_UPLOADSTATUSCHECK;
}

void TwTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "TwTalker::slotFinished";

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::TW_CREATEFOLDER)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->readAll();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "status code: " << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
            Q_EMIT signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());

            reply->deleteLater();
            return;
        }
    }

    QByteArray buffer = reply->readAll();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "status code: " << reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    static int segmentIndex = 0;

    switch (d->state)
    {
        case Private::TW_LISTFOLDERS:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_LISTFOLDERS";
            parseResponseListFolders(buffer);
            break;
        }

        case Private::TW_CREATEFOLDER:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_CREATEFOLDER";
            parseResponseCreateFolder(buffer);
            break;
        }

        case Private::TW_ADDPHOTO:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_ADDPHOTO";
            parseResponseAddPhoto(buffer);
            break;
        }

        case Private::TW_USERNAME:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_USERNAME";
            parseResponseUserName(buffer);
            break;
        }

        case Private::TW_CREATETWEET:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_CREATETWEET";
            parseResponseCreateTweet(buffer);
            break;
        }

        case Private::TW_UPLOADINIT:
        {
            segmentIndex = 0;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_UPLOADINIT";
            parseResponseAddPhotoInit(buffer);
            break;
        }

        case Private::TW_UPLOADAPPEND:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_UPLOADAPPEND (at index " << segmentIndex << ")";
            segmentIndex++;
            parseResponseAddPhotoAppend(buffer, segmentIndex);
            break;
        }

        case Private::TW_UPLOADSTATUSCHECK:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_UPLOADSTATUSCHECK";
            parseCheckUploadStatus(buffer);
            break;
        }

        case Private::TW_UPLOADFINALIZE:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In TW_UPLOADFINALIZE";
            parseResponseAddPhotoFinalize(buffer);
            break;
        }

        default:
        {
            break;
        }
    }

    reply->deleteLater();
}

void TwTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhoto: " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    QJsonObject jsonObject = doc.object();
    QString mediaId        = jsonObject[QLatin1String("media_id_string")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "media id: " << mediaId;

    // We haven't Q_EMIT signalAddPhotoSucceeded() here yet, since we need to update the status first

    createTweet(mediaId);
}

void TwTalker::parseResponseAddPhotoInit(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoInit: " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    QJsonObject jsonObject = doc.object();
    d->mediaId             = jsonObject[QLatin1String("media_id_string")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "media id: " << d->mediaId;

    // We haven't Q_EMIT signalAddPhotoSucceeded() here yet, since we need to update the status first

    addPhotoAppend(d->mediaId);
}

void TwTalker::parseResponseAddPhotoAppend(const QByteArray& /*data*/, int segmentIndex)
{
    /* (Fev. 2019)
     * Currently, we don't parse data of response from addPhotoAppend, since the response is with HTTP 204
     * This is indeed an expected response code, because the response should be with an empty body.
     * However, in order to keep a compatible prototype of parseResponse methods and reserve for future change,
     * we should keep argument const QByteArray& data.
     */
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoAppend: ";

    if (segmentIndex <= d->segmentIndex)
    {
        addPhotoAppend(d->mediaId, segmentIndex);
    }
    else
    {
        addPhotoFinalize(d->mediaId);
    }
}

void TwTalker::parseResponseAddPhotoFinalize(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoFinalize: " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    QJsonObject jsonObject    = doc.object();
    QJsonValue processingInfo = jsonObject[QLatin1String("processing_info")];

    if (processingInfo != QJsonValue::Undefined)
    {
        const QString state = processingInfo.toObject().value(QLatin1String("state")).toString();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "state: " << state;

        if (state == QLatin1String("pending"))
        {
            QTimer::singleShot(processingInfo.toObject().value(QLatin1String("check_after_secs")).toInt() * 1000 /*msec*/,
                               this, SLOT(slotCheckUploadStatus()));
        }
    }
    else
    {
        // We haven't emit signalAddPhotoSucceeded() here yet, since we need to update the status first

        createTweet(d->mediaId);
    }
}

void TwTalker::parseCheckUploadStatus(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseCheckUploadStatus: " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    QJsonObject jsonObject     = doc.object();
    QJsonObject processingInfo = jsonObject[QLatin1String("processing_info")].toObject();
    QString state              = processingInfo[QLatin1String("state")].toString();

    if      (state == QLatin1String("in_progress"))
    {
        QTimer::singleShot(processingInfo[QLatin1String("check_after_secs")].toInt()*1000 /*msec*/, this, SLOT(slotCheckUploadStatus()));
    }
    else if (state == QLatin1String("failed"))
    {
        QJsonObject error = processingInfo[QLatin1String("error")].toObject();

        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo\n"
                                       "Code: %1, name: %2, message: %3",
                                       error[QLatin1String("code")].toInt(),
                                       error[QLatin1String("name")].toString(),
                                       error[QLatin1String("message")].toString()));
        return;
    }
    else // succeeded
    {
        // We haven't Q_EMIT signalAddPhotoSucceeded() here yet, since we need to update the status first

        createTweet(d->mediaId);
    }
}

void TwTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseUserName: "<<doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);

        return;
    }

    QJsonObject obj     = doc.object();
    QString name        = obj[QLatin1String("name")].toString();
    QString screenName  = obj[QLatin1String("screen_name")].toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "user full name: "<<name;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "user screen name: @" << screenName;

    Q_EMIT signalBusy(false);
    Q_EMIT signalSetUserName(QString::fromLatin1("%1 (@%2)").arg(name).arg(screenName));
}

void TwTalker::parseResponseCreateTweet(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateTweet: " << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalAddPhotoFailed(i18n("Failed to create tweet for photo uploaded"));

        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Tweet posted successfully!";

    Q_EMIT signalBusy(false);
    Q_EMIT signalAddPhotoSucceeded();
}

void TwTalker::parseResponseListFolders(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalListAlbumsFailed(i18n("Failed to list folders"));
        return;
    }

    QJsonObject jsonObject = doc.object();

    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json: " << doc;

    QJsonArray jsonArray   = jsonObject[QLatin1String("value")].toArray();

    //qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json response: " << jsonArray;

    QList<QPair<QString, QString> > list;
    list.append(qMakePair(QLatin1String(""), QLatin1String("root")));

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QString path;
        QString folderName;
        QJsonObject folder;

        QJsonObject obj = value.toObject();
        folder          = obj[QLatin1String("folder")].toObject();

        if (!folder.isEmpty())
        {
            folderName    = obj[QLatin1String("name")].toString();
            path          = QLatin1Char('/') + folderName;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder Name is" << folderName;
            list.append(qMakePair(path, folderName));
        }
    }

    Q_EMIT signalBusy(false);
    Q_EMIT signalListAlbumsDone(list);
}

void TwTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc1      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc1.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    Q_EMIT signalBusy(false);

    if (fail)
    {
      QJsonParseError err;
      QJsonDocument doc2 = QJsonDocument::fromJson(data, &err);
      qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseCreateFolder ERROR: " << doc2;

      Q_EMIT signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        Q_EMIT signalCreateFolderSucceeded();
    }
}

} // namespace DigikamGenericTwitterPlugin

#include "moc_twittertalker.cpp"

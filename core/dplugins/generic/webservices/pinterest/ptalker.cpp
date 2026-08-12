/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ptalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QFileInfo>
#include <QWidget>
#include <QMessageBox>
#include <QApplication>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>
#include <kwindowconfig.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "previewloadthread.h"
#include "networkmanager.h"
#include "webbrowserdlg.h"
#include "wstoolutils.h"
#include "pwindow.h"

namespace DigikamGenericPinterestPlugin
{

class Q_DECL_HIDDEN PTalker::Private
{
public:

    enum State
    {
        P_USERNAME = 0,
        P_LISTBOARDS,
        P_CREATEBOARD,
        P_ADDPIN,
        P_ACCESSTOKEN
    };

public:

    Private() = default;

public:

    QString                clientId         = QLatin1String("1477112");
    QString                clientSecret     = QLatin1String("69dc00477dd1c59430b15675d92ff30136126dcb");
    QString                authUrl          = QLatin1String("https://www.pinterest.com/oauth/");
    QString                tokenUrl         = QLatin1String("https://api.pinterest.com/v5/oauth/token");
    QString                redirectUrl      = QLatin1String("https://login.live.com/oauth20_desktop.srf");
    QString                scope            = QLatin1String("boards:read,boards:write,pins:read,pins:write,user_accounts:read");
    QString                serviceName      = QLatin1String("Pinterest");
    QString                serviceKey       = QLatin1String("access_token");

    QString                accessToken;
    QString                userName;

    QWidget*               parent           = nullptr;

    QNetworkAccessManager* netMngr          = nullptr;
    QNetworkReply*         reply            = nullptr;

    QSettings*             settings         = nullptr;

    State                  state            = P_USERNAME;

    QMap<QString, QString> urlParametersMap;

    WebBrowserDlg*         browser          = nullptr;
};

PTalker::PTalker(QWidget* const parent)
    : d(new Private)
{
    d->parent   = parent;
    d->netMngr  = NetworkManager::instance()->getNetworkManager(this);
    d->settings = WSToolUtils::getOauthSettings(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(pinterestLinkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(pinterestLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));
}

PTalker::~PTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir("pinterest");

    delete d;
}

void PTalker::link()
{
    Q_EMIT signalBusy(true);

    QUrl url(d->authUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"),     d->clientId);
    query.addQueryItem(QLatin1String("scope"),         d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"),  d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), QLatin1String("code"));
    url.setQuery(query);

    d->browser = new WebBrowserDlg(url, d->parent, true);
    d->browser->setModal(true);

    connect(d->browser, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));

    connect(d->browser, SIGNAL(closeView(bool)),
            this, SIGNAL(signalBusy(bool)));

    d->browser->show();
}

void PTalker::unLink()
{
    d->accessToken = QString();

    d->settings->beginGroup(d->serviceName);
    d->settings->remove(QString());
    d->settings->endGroup();

    Q_EMIT pinterestLinkingSucceeded();
}

void PTalker::slotCatchUrl(const QUrl& url)
{
    d->urlParametersMap = ParseUrlParameters(url.toString());
    QString code        = d->urlParametersMap.value(QLatin1String("code"));
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received URL from webview in link function:" << url ;

    if (!code.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "CODE Received";
        d->browser->close();
        getToken(code);
        Q_EMIT signalBusy(false);
    }
}

void PTalker::getToken(const QString& code)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Code:" << code;

    QUrlQuery query;
    query.addQueryItem(QLatin1String("grant_type"),   QLatin1String("authorization_code"));
    query.addQueryItem(QLatin1String("redirect_uri"), d->redirectUrl);
    query.addQueryItem(QLatin1String("code"),         code);

    QByteArray basic = d->clientId.toLatin1() + QByteArray(":") + d->clientSecret.toLatin1();
    basic            = basic.toBase64();

    QNetworkRequest netRequest(QUrl(d->tokenUrl));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Basic %1").arg(QLatin1String(basic)).toLatin1());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Accept", "application/json");

    d->reply = d->netMngr->post(netRequest, query.toString().toLatin1());

    d->state = Private::P_ACCESSTOKEN;
}

QMap<QString, QString> PTalker::ParseUrlParameters(const QString& url)
{
    QMap<QString, QString> urlParameters;

    if (url.indexOf(QLatin1Char('?')) == -1)
    {
        return urlParameters;
    }

    QString tmp           = url.right(url.length()-url.indexOf(QLatin1Char('?')) - 1);
    QStringList paramlist = tmp.split(QLatin1Char('&'));

    for (int i = 0 ; i < paramlist.count() ; ++i)
    {
        QStringList paramarg = paramlist.at(i).split(QLatin1Char('='));

        if (paramarg.count() == 2)
        {
            urlParameters.insert(paramarg.at(0), paramarg.at(1));
        }
    }

    return urlParameters;
}

void PTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Pinterest fail";

    Q_EMIT signalBusy(false);
}

void PTalker::slotLinkingSucceeded()
{
    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Pinterest ok";

        Q_EMIT signalBusy(false);

        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Pinterest ok";
    writeSettings();

    Q_EMIT signalLinkingSucceeded();
}

bool PTalker::authenticated()
{
    return (!d->accessToken.isEmpty());
}

void PTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(false);
}

void PTalker::createBoard(QString& boardName)
{
    QUrl url(QLatin1String("https://api.pinterest.com/v5/boards"));
    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    QByteArray postData = QString::fromUtf8("{\"name\": \"%1\"}").arg(boardName).toUtf8();
/*
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "createBoard:" << postData;
*/
    d->reply = d->netMngr->post(netRequest, postData);

    d->state = Private::P_CREATEBOARD;

    Q_EMIT signalBusy(true);
}

void PTalker::getUserName()
{
    QUrl url(QLatin1String("https://api.pinterest.com/v5/user_account"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());

    d->reply = d->netMngr->get(netRequest);
    d->state = Private::P_USERNAME;

    Q_EMIT signalBusy(true);
}

/**
 * Get list of boards by parsing json sent by pinterest
 */
void PTalker::listBoards(const QString& /*path*/)
{
    QUrl url(QLatin1String("https://api.pinterest.com/v5/boards"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::P_LISTBOARDS;

    Q_EMIT signalBusy(true);
}

bool PTalker::addPin(const QString& imgPath,
                     const QString& boardID,
                     bool rescale,
                     int maxDim,
                     int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

    if (image.isNull())
    {
        Q_EMIT signalBusy(false);

        return false;
    }

    QString path = WSToolUtils::makeTemporaryDir("pinterest").filePath(QFileInfo(imgPath)
                                                 .baseName().trimmed() + QLatin1String(".jpg"));

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

    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QUrl url(QLatin1String("https://api.pinterest.com/v5/pins"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    QByteArray postData;
    postData += "{\"board_id\": \"";
    postData += boardID.toLatin1();
    postData += "\",\"media_source\": {";
    postData += "\"source_type\": \"image_base64\",";
    postData += "\"content_type\": \"image/jpeg\",";
    postData += "\"data\": \"";
    postData += fileData.toBase64();
    postData += "\"}}";

    d->reply = d->netMngr->post(netRequest, postData);
    d->state = Private::P_ADDPIN;

    return true;
}

void PTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::P_CREATEBOARD)
        {
            Q_EMIT signalBusy(false);

            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());
/*
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error content: " << reply->readAll();
*/
            Q_EMIT signalNetworkError();

            reply->deleteLater();
            return;
        }
    }

    QByteArray buffer = reply->readAll();
/*
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "BUFFER" << buffer;
*/
    switch (d->state)
    {
        case Private::P_LISTBOARDS:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_LISTBOARDS";
            parseResponseListBoards(buffer);
            break;
        }

        case Private::P_CREATEBOARD:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_CREATEBOARD";
            parseResponseCreateBoard(buffer);
            break;
        }

        case Private::P_ADDPIN:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_ADDPIN";
            parseResponseAddPin(buffer);
            break;
        }

        case Private::P_USERNAME:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_USERNAME";
            parseResponseUserName(buffer);
            break;
        }

        case Private::P_ACCESSTOKEN:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In P_ACCESSTOKEN";
            parseResponseAccessToken(buffer);
            break;
        }

        default:
        {
            break;
        }
    }

    reply->deleteLater();
}

void PTalker::parseResponseAccessToken(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    d->accessToken         = jsonObject[QLatin1String("access_token")].toString();

    if (!d->accessToken.isEmpty())
    {
        qDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token Received:" << d->accessToken;
        Q_EMIT pinterestLinkingSucceeded();
    }
    else
    {
        Q_EMIT pinterestLinkingFailed();
    }

    Q_EMIT signalBusy(false);
}

void PTalker::parseResponseAddPin(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool success           = jsonObject.contains(QLatin1String("id"));

    Q_EMIT signalBusy(false);

    if (!success)
    {
        Q_EMIT signalAddPinFailed(i18n("Failed to upload Pin"));
    }
    else
    {
        Q_EMIT signalAddPinSucceeded();
    }
}

void PTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    d->userName            = jsonObject[QLatin1String("username")].toString();

    Q_EMIT signalBusy(false);
    Q_EMIT signalSetUserName(d->userName);
}

void PTalker::parseResponseListBoards(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalListBoardsFailed(i18n("Failed to list boards"));

        return;
    }

    QJsonObject jsonObject = doc.object();
/*
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json Listing Boards:" << doc;
*/
    QJsonArray jsonArray   = jsonObject[QLatin1String("items")].toArray();

    QList<QPair<QString, QString> > list;

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QString boardID;
        QString boardName;
        QJsonObject obj = value.toObject();
        boardID         = obj[QLatin1String("id")].toString();
        boardName       = obj[QLatin1String("name")].toString();

        list.append(qMakePair(boardID, boardName));
    }

    Q_EMIT signalBusy(false);
    Q_EMIT signalListBoardsDone(list);
}

void PTalker::parseResponseCreateBoard(const QByteArray& data)
{
    QJsonDocument doc1     = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc1.object();
    bool fail              = jsonObject.contains(QLatin1String("code"));

    Q_EMIT signalBusy(false);

    if (fail)
    {
        QJsonParseError err;
        QJsonDocument doc2 = QJsonDocument::fromJson(data, &err);
        Q_UNUSED(doc2);

        Q_EMIT signalCreateBoardFailed(jsonObject[QLatin1String("message")].toString());
    }
    else
    {
        Q_EMIT signalCreateBoardSucceeded();
    }
}

void PTalker::writeSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->settings->setValue(d->serviceKey, d->accessToken);
    d->settings->endGroup();
}

void PTalker::readSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->accessToken = d->settings->value(d->serviceKey).toString();
    d->settings->endGroup();

    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking...";
        link();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Already Linked";

        Q_EMIT pinterestLinkingSucceeded();
    }
}

} // namespace DigikamGenericPinterestPlugin

#include "moc_ptalker.cpp"

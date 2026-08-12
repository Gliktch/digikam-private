/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "odtalker.h"

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
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QUrlQuery>

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
#include "odwindow.h"
#include "odmpform.h"

namespace DigikamGenericOneDrivePlugin
{

class Q_DECL_HIDDEN ODTalker::Private
{
public:

    enum State
    {
        OD_USERNAME = 0,
        OD_LISTFOLDERS,
        OD_CREATEFOLDER,
        OD_ADDPHOTO
    };

public:

    Private() = default;

public:

    QString                         clientId        = QLatin1String("4c20a541-2ca8-4b98-8847-a375e4d33f34");
    QString                         clientSecret    = QLatin1String("wtdcaXADCZ0|tcDA7633|@*");
    QString                         authUrl         = QLatin1String("https://login.live.com/oauth20_authorize.srf");
    QString                         tokenUrl        = QLatin1String("https://login.live.com/oauth20_token.srf");
    QString                         scope           = QLatin1String("Files.ReadWrite User.Read");
    QString                         redirectUrl     = QLatin1String("https://login.live.com/oauth20_desktop.srf");
    QString                         serviceName     = QLatin1String("Onedrive");
    QString                         serviceTime     = QLatin1String("token_time");
    QString                         serviceKey      = QLatin1String("access_token");
    QString                         accessToken;

    QDateTime                       expiryTime;

    State                           state           = OD_USERNAME;

    QWidget*                        parent          = nullptr;

    QNetworkAccessManager*          netMngr         = nullptr;
    QNetworkReply*                  reply           = nullptr;

    QSettings*                      settings        = nullptr;

    WebBrowserDlg*                  browser         = nullptr;

    QList<QPair<QString, QString> > folderList;
    QList<QString>                  nextFolder;
};

ODTalker::ODTalker(QWidget* const parent)
    : d           (new Private)
{
    d->parent   = parent;
    d->netMngr  = NetworkManager::instance()->getNetworkManager(this);
    d->settings = WSToolUtils::getOauthSettings(this);

    connect(this, SIGNAL(oneDriveLinkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(this, SIGNAL(oneDriveLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

ODTalker::~ODTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir("onedrive");

    delete d;
}

void ODTalker::link()
{
    Q_EMIT signalBusy(true);

    QUrl url(d->authUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"), d->clientId);
    query.addQueryItem(QLatin1String("scope"), d->scope);
    query.addQueryItem(QLatin1String("redirect_uri"), d->redirectUrl);
    query.addQueryItem(QLatin1String("response_type"), QLatin1String("token"));
    url.setQuery(query);

    delete d->browser;

    d->browser = new WebBrowserDlg(url, d->parent, true);
    d->browser->setModal(true);

    connect(d->browser, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));

    connect(d->browser, SIGNAL(closeView(bool)),
            this, SIGNAL(signalBusy(bool)));

    d->browser->show();
}

void ODTalker::unLink()
{
    d->accessToken = QString();

    d->settings->beginGroup(d->serviceName);
    d->settings->remove(QString());
    d->settings->endGroup();

    Q_EMIT oneDriveLinkingSucceeded();
}

void ODTalker::slotCatchUrl(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received URL from webview:" << url;

    QString   str = url.toString();
    QUrlQuery query(str.section(QLatin1Char('#'), -1, -1));

    if (query.hasQueryItem(QLatin1String("access_token")))
    {
        d->accessToken = query.queryItemValue(QLatin1String("access_token"));
        int seconds    = query.queryItemValue(QLatin1String("expires_in")).toInt();
        d->expiryTime  = QDateTime::currentDateTime().addSecs(seconds);

        writeSettings();

        qDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token received";

        Q_EMIT oneDriveLinkingSucceeded();
    }
    else
    {
        Q_EMIT oneDriveLinkingFailed();
    }
}

void ODTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Onedrive fail";

    Q_EMIT signalBusy(false);
}

void ODTalker::slotLinkingSucceeded()
{
    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Onedrive";

        Q_EMIT signalBusy(false);

        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Onedrive";

    if (d->browser)
    {
        d->browser->close();
    }

    Q_EMIT signalLinkingSucceeded();
}

bool ODTalker::authenticated()
{
    return (!d->accessToken.isEmpty());
}

void ODTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(false);
}

void ODTalker::createFolder(QString& path)
{
    // path also has name of new folder so send path parameter accordingly

    QString name       = QUrl(path).fileName();
    QString folderPath = QUrl(path).adjusted(QUrl::RemoveFilename |
                                             QUrl::StripTrailingSlash).path();

    QUrl url;

    if (folderPath == QLatin1String("/"))
    {
        url = QUrl(QLatin1String("https://graph.microsoft.com/v1.0/me/drive/root/children"));
    }
    else
    {
        url = QUrl(QString::fromUtf8("https://graph.microsoft.com/v1.0/me/drive/root:/%1:/children").arg(folderPath));
    }

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"name\": \"%1\",\"folder\": {}}").arg(name).toUtf8();
    d->reply = d->netMngr->post(netRequest, postData);

    d->state = Private::OD_CREATEFOLDER;

    Q_EMIT signalBusy(true);
}

void ODTalker::getUserName()
{
    QUrl url(QLatin1String("https://graph.microsoft.com/v1.0/me"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);
    d->state = Private::OD_USERNAME;

    Q_EMIT signalBusy(true);
}

/**
 * Get list of folders by parsing json sent by onedrive
 */
void ODTalker::listFolders(const QString& folder)
{
    QString nextFolder;

    if (folder.isEmpty())
    {
        d->folderList.clear();
        d->nextFolder.clear();
    }
    else
    {
        nextFolder = QLatin1Char(':') + folder + QLatin1Char(':');
    }

    QUrl url(QString::fromLatin1("https://graph.microsoft.com/v1.0/me/drive/root%1/"
                                 "children?select=name,folder,path,parentReference").arg(nextFolder));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::OD_LISTFOLDERS;

    Q_EMIT signalBusy(true);
}

bool ODTalker::addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    ODMPForm form;
    QString path = imgPath;

    QMimeDatabase mimeDB;

    if (mimeDB.mimeTypeForFile(imgPath).name().startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

        if (image.isNull())
        {
            Q_EMIT signalBusy(false);

            return false;
        }

        path = WSToolUtils::makeTemporaryDir("onedrive").filePath(QFileInfo(imgPath)
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
    }

    if (!form.addFile(path))
    {
        Q_EMIT signalBusy(false);

        return false;
    }

    QString uploadPath = uploadFolder + QUrl(imgPath).fileName();
    QUrl url(QString::fromLatin1("https://graph.microsoft.com/v1.0/me/drive/root:%1:/content").arg(uploadPath));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->accessToken).toUtf8());

    d->reply = d->netMngr->put(netRequest, form.formData());

    d->state = Private::OD_ADDPHOTO;

    return true;
}

void ODTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::OD_CREATEFOLDER)
        {
            Q_EMIT signalTransferCancel();

            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());

            reply->deleteLater();

            return;
        }
    }

    QByteArray buffer = reply->readAll();

    switch (d->state)
    {
        case Private::OD_LISTFOLDERS:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_LISTFOLDERS";
            parseResponseListFolders(buffer);
            break;
        }

        case Private::OD_CREATEFOLDER:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_CREATEFOLDER";
            parseResponseCreateFolder(buffer);
            break;
        }

        case Private::OD_ADDPHOTO:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_ADDPHOTO";
            parseResponseAddPhoto(buffer);
            break;
        }

        case Private::OD_USERNAME:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In OD_USERNAME";
            parseResponseUserName(buffer);
            break;
        }

        default:
        {
            break;
        }
    }

    reply->deleteLater();
}

void ODTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool success           = jsonObject.contains(QLatin1String("size"));

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));
    }
    else
    {
        Q_EMIT signalAddPhotoSucceeded();
    }
}

void ODTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc  = QJsonDocument::fromJson(data);
    const QString name = doc.object().value(QLatin1String("displayName")).toString();

    Q_EMIT signalBusy(false);
    Q_EMIT signalSetUserName(name);
}

void ODTalker::parseResponseListFolders(const QByteArray& data)
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
/*
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Json: " << doc;
*/
    QJsonArray jsonArray   = jsonObject[QLatin1String("value")].toArray();

    if (d->folderList.isEmpty())
    {
        d->folderList.append(qMakePair(QLatin1String(""), QLatin1String("root")));
    }

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QString path;
        QString listName;
        QString folderPath;
        QString folderName;
        QJsonObject folder;
        QJsonObject parent;

        QJsonObject obj = value.toObject();
        folder          = obj[QLatin1String("folder")].toObject();
        parent          = obj[QLatin1String("parentReference")].toObject();

        if (!folder.isEmpty())
        {
            folderPath  = parent[QLatin1String("path")].toString();
            folderName  = obj[QLatin1String("name")].toString();

            path        = folderPath.section(QLatin1String("root:"), -1, -1) +
                                             QLatin1Char('/') + folderName;
            path        = QUrl(path).toString();
            listName    = path.section(QLatin1Char('/'), 1);

            d->folderList.append(qMakePair(path, listName));

            if (folder[QLatin1String("childCount")].toInt() > 0)
            {
                d->nextFolder << path;
            }
        }
    }

    if (!d->nextFolder.isEmpty())
    {
        listFolders(d->nextFolder.takeLast());
    }
    else
    {
        std::sort(d->folderList.begin(), d->folderList.end());

        Q_EMIT signalBusy(false);
        Q_EMIT signalListAlbumsDone(d->folderList);
    }
}

void ODTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc1     = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc1.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    Q_EMIT signalBusy(false);

    if (fail)
    {
        QJsonParseError err;
        QJsonDocument doc2 = QJsonDocument::fromJson(data, &err);
        Q_UNUSED(doc2);

        Q_EMIT signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        Q_EMIT signalCreateFolderSucceeded();
    }
}

void ODTalker::writeSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->settings->setValue(d->serviceTime, d->expiryTime);
    d->settings->setValue(d->serviceKey,  d->accessToken);
    d->settings->endGroup();
}

void ODTalker::readSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->expiryTime  = d->settings->value(d->serviceTime).toDateTime();
    d->accessToken = d->settings->value(d->serviceKey).toString();
    d->settings->endGroup();

    if      (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Linking...";
        link();
    }
    else if (QDateTime::currentDateTime() > d->expiryTime)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token has expired";
        d->accessToken = QString();
        link();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Already Linked";

        Q_EMIT oneDriveLinkingSucceeded();
    }
}

} // namespace DigikamGenericOneDrivePlugin

#include "moc_odtalker.cpp"

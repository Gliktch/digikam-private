/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2018-2019 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <dbtalker.h>

// Qt includes

#include <QMimeDatabase>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QFileInfo>
#include <QWidget>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "previewloadthread.h"
#include "wstoolutils.h"
#include "dmetadata.h"
#include "dbwindow.h"
#include "dbmpform.h"
#include "dbitem.h"
#include "o2.h"
#include "o0globals.h"
#include "o0settingsstore.h"
#include "networkmanager.h"

using namespace Digikam;

namespace DigikamGenericDropBoxPlugin
{

class Q_DECL_HIDDEN DBTalker::Private
{
public:

    enum State
    {
        DB_USERNAME = 0,
        DB_LISTFOLDERS,
        DB_CREATEFOLDER,
        DB_ADDPHOTO
    };

public:

    explicit Private(QWidget* const p)
        : parent(p)
    {
    }

public:

    QString                         apikey      = QLatin1String("mv2pk07ym9bx3r8");
    QString                         secret      = QLatin1String("f33sflc8jhiozqu");
    QString                         authUrl     = QLatin1String("https://www.dropbox.com/oauth2/authorize");
    QString                         tokenUrl    = QLatin1String("https://api.dropboxapi.com/oauth2/token");
    QList<QPair<QString, QString> > folderList;

    QWidget*                        parent      = nullptr;
    QNetworkAccessManager*          netMngr     = nullptr;

    QNetworkReply*                  reply       = nullptr;

    QSettings*                      settings    = nullptr;

    State                           state       = DB_USERNAME;

    O2*                             o2          = nullptr;
};

DBTalker::DBTalker(QWidget* const parent)
    : d           (new Private(parent))
{
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    d->o2      = new O2(this);

    d->o2->setClientId(d->apikey);
    d->o2->setClientSecret(d->secret);
    d->o2->setRefreshTokenUrl(d->tokenUrl);
    d->o2->setRequestUrl(d->authUrl);
    d->o2->setTokenUrl(d->tokenUrl);
    d->o2->setLocalPort(8000);

    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(QLatin1String("Dropbox"));
    d->o2->setStore(store);

    connect(d->o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o2, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));
}

DBTalker::~DBTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir("dropbox");

    delete d;
}

void DBTalker::link()
{
    Q_EMIT signalBusy(true);

    d->o2->link();
}

void DBTalker::unLink()
{
    d->o2->unlink();

    d->settings->beginGroup(QLatin1String("Dropbox"));
    d->settings->remove(QString());
    d->settings->endGroup();
}

void DBTalker::reauthenticate()
{
    d->o2->unlink();

    // Wait until user account is unlinked completely
    while (authenticated());

    d->o2->link();
}

void DBTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Dropbox fail";

    Q_EMIT signalBusy(false);
}

void DBTalker::slotLinkingSucceeded()
{
    if (!d->o2->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Dropbox ok";

        Q_EMIT signalBusy(false);

        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Dropbox ok";

    Q_EMIT signalLinkingSucceeded();
}

void DBTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser...";
    QDesktopServices::openUrl(url);
}

bool DBTalker::authenticated()
{
    return d->o2->linked();
}

/** Creates folder at specified path
 */
void DBTalker::createFolder(const QString& path)
{
    //path also has name of new folder so send path parameter accordingly
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "createFolder:" << path;

    QUrl url(QLatin1String("https://api.dropboxapi.com/2/files/create_folder_v2"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"path\": \"%1\"}").arg(path).toUtf8();

    d->reply = d->netMngr->post(netRequest, postData);

    d->state = Private::DB_CREATEFOLDER;

    Q_EMIT signalBusy(true);
}

/** Get username of dropbox user
 */
void DBTalker::getUserName()
{
    QUrl url(QLatin1String("https://api.dropboxapi.com/2/users/get_current_account"));

    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());

    d->reply = d->netMngr->post(netRequest, QByteArray());

    d->state = Private::DB_USERNAME;

    Q_EMIT signalBusy(true);
}

/** Get list of folders by parsing json sent by dropbox
 */
void DBTalker::listFolders(const QString& cursor)
{
    QUrl url(QLatin1String("https://api.dropboxapi.com/2/files/list_folder"));
    QByteArray postData;

    if (cursor.isEmpty())
    {
        d->folderList.clear();
        postData = QString::fromUtf8("{\"path\": \"\",\"recursive\": true}").toUtf8();
    }
    else
    {
        url.setPath(url.path() + QLatin1String("/continue"));
        postData = QString::fromUtf8("{\"cursor\": \"%1\"}").arg(cursor).toUtf8();
    }

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());

    d->reply = d->netMngr->post(netRequest, postData);

    d->state = Private::DB_LISTFOLDERS;

    Q_EMIT signalBusy(true);
}

bool DBTalker::addPhoto(const QString& imgPath, const QString& uploadFolder,
                        bool original, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    QString path = imgPath;

    QMimeDatabase mimeDB;

    if (!original && mimeDB.mimeTypeForFile(imgPath).name().startsWith(QLatin1String("image/")))
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

        if (image.isNull())
        {
            image.load(imgPath);
        }

        if (image.isNull())
        {
            Q_EMIT signalBusy(false);

            return false;
        }

        path = WSToolUtils::makeTemporaryDir("dropbox").filePath(QFileInfo(imgPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && (image.width() > maxDim || image.height() > maxDim))
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

    DBMPForm form;

    if (!form.addFile(path))
    {
        Q_EMIT signalBusy(false);

        return false;
    }

    QString uploadPath = uploadFolder + QUrl(QUrl::fromLocalFile(imgPath)).fileName();
    QUrl url(QLatin1String("https://content.dropboxapi.com/2/files/upload"));

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/octet-stream"));
    netRequest.setRawHeader("Authorization", QString::fromLatin1("Bearer %1").arg(d->o2->token()).toUtf8());

    QByteArray postData = QString::fromUtf8("{\"path\": \"%1\",\"mode\": \"add\"}").arg(uploadPath).toUtf8();
    netRequest.setRawHeader("Dropbox-API-Arg", postData);

    d->reply = d->netMngr->post(netRequest, form.formData());

    d->state = Private::DB_ADDPHOTO;
    return true;
}

void DBTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(false);
}

void DBTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state != Private::DB_CREATEFOLDER)
        {
            Q_EMIT signalBusy(false);

            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());

            reply->deleteLater();
            return;
        }
    }

    QByteArray buffer = reply->readAll();

    switch (d->state)
    {
        case Private::DB_LISTFOLDERS:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In DB_LISTFOLDERS";
            parseResponseListFolders(buffer);
            break;
        }

        case Private::DB_CREATEFOLDER:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In DB_CREATEFOLDER";
            parseResponseCreateFolder(buffer);
            break;
        }

        case Private::DB_ADDPHOTO:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In DB_ADDPHOTO";
            parseResponseAddPhoto(buffer);
            break;
        }

        case Private::DB_USERNAME:
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In DB_USERNAME";
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

void DBTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool success           = jsonObject.contains(QLatin1String("size"));

    Q_EMIT signalBusy(false);

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));
    }
    else
    {
        Q_EMIT signalAddPhotoSucceeded();
    }
}

void DBTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonDocument doc            = QJsonDocument::fromJson(data);
    const QJsonObject jsonObject = doc.object().value(QLatin1String("name")).toObject();
    const QString name           = jsonObject.value(QLatin1String("display_name")).toString();

    Q_EMIT signalBusy(false);
    Q_EMIT signalSetUserName(name);
}

void DBTalker::parseResponseListFolders(const QByteArray& data)
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
    QJsonArray jsonArray   = jsonObject[QLatin1String("entries")].toArray();

    if (d->folderList.isEmpty())
    {
        d->folderList.append(qMakePair(QLatin1String(""), QLatin1String("root")));
    }

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QString path;
        QString folder;

        QJsonObject obj = value.toObject();
        path            = obj[QLatin1String("path_display")].toString();
        folder          = obj[QLatin1String(".tag")].toString();

        if (folder == QLatin1String("folder"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Path is" << path;
            QString listName = path.section(QLatin1Char('/'), 1);
            d->folderList.append(qMakePair(path, listName));
        }
    }

    if (jsonObject[QLatin1String("has_more")].toBool())
    {
        QString cursor = jsonObject[QLatin1String("cursor")].toString();

        if (!cursor.isEmpty())
        {
            listFolders(cursor);
            return;
        }
    }

    std::sort(d->folderList.begin(), d->folderList.end());

    Q_EMIT signalBusy(false);
    Q_EMIT signalListAlbumsDone(d->folderList);
}

void DBTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonDocument doc      = QJsonDocument::fromJson(data);
    QJsonObject jsonObject = doc.object();
    bool fail              = jsonObject.contains(QLatin1String("error"));

    Q_EMIT signalBusy(false);

    if (fail)
    {
        Q_EMIT signalCreateFolderFailed(jsonObject[QLatin1String("error_summary")].toString());
    }
    else
    {
        Q_EMIT signalCreateFolderSucceeded();
    }
}

} // namespace DigikamGenericDropBoxPlugin

#include "moc_dbtalker.cpp"

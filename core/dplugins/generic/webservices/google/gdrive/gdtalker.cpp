/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gdtalker.h"

// Qt includes

#include <QMimeDatabase>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wstoolutils.h"
#include "digikam_version.h"
#include "gswindow.h"
#include "gdmpform.h"
#include "digikam_debug.h"
#include "previewloadthread.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

static bool gdriveLessThan(const GSFolder& p1, const GSFolder& p2)
{
    return (p1.title.toLower() < p2.title.toLower());
}

class Q_DECL_HIDDEN GDTalker::Private
{
public:

    enum State
    {
        GD_LOGOUT      = -1,
        GD_LISTFOLDERS = 0,
        GD_CREATEFOLDER,
        GD_ADDPHOTO,
        GD_USERNAME,
    };

public:

    Private() = default;

public:

    QString                apiUrl           = QLatin1String("https://www.googleapis.com/drive/v2/%1");
    QString                uploadUrl        = QLatin1String("https://www.googleapis.com/upload/drive/v2/files");
    QString                rootid           = QLatin1String("root");
    QString                rootfoldername   = QLatin1String("GoogleDrive Root");
    QString                username;
    State                  state;
    QStringList            listPhotoId;
};

GDTalker::GDTalker(QWidget* const parent)
    : GSTalkerBase(parent, QStringList(QLatin1String("https://www.googleapis.com/auth/drive")), QLatin1String("GoogleDrive")),
      d           (new Private)
{
    connect(m_service->networkAccessManager(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(this, SIGNAL(signalReadyToUpload()),
            this, SLOT(slotUploadPhoto()));
}

GDTalker::~GDTalker()
{
    if (m_reply)
    {
        m_reply->abort();
    }

    WSToolUtils::removeTemporaryDir("google");

    delete d;
}

/**
 * Gets username
 */
void GDTalker::getUserName()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getUserName";

    QUrl url(d->apiUrl.arg(QLatin1String("about")));

    m_reply  = m_service->get(url);

    d->state = Private::GD_USERNAME;

    Q_EMIT signalBusy(true);
}

/**
 * Gets list of folder of user in json format
 */
void GDTalker::listFolders()
{
    QUrl url(d->apiUrl.arg(QLatin1String("files")));

    QUrlQuery q;
    q.addQueryItem(QLatin1String("q"), QLatin1String("mimeType = 'application/vnd.google-apps.folder'"));

    url.setQuery(q);

    m_reply  = m_service->get(url);

    d->state = Private::GD_LISTFOLDERS;

    Q_EMIT signalBusy(true);
}

/**
 * Creates folder inside any folder(of which id is passed)
 */
void GDTalker::createFolder(const QString& title, const QString& id)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    QUrl url(d->apiUrl.arg(QLatin1String("files")));
    QByteArray data;
    data += "{\"title\":\"";
    data += title.toLatin1();
    data += "\",\r\n";
    data += "\"parents\":";
    data += "[{";
    data += "\"id\":\"";
    data += id.toLatin1();
    data += "\"}],\r\n";
    data += "\"mimeType\":";
    data += "\"application/vnd.google-apps.folder\"";
    data += "}\r\n";

    // qCDebug(DIGIKAM_WEBSERVICES_LOG) << "data:" << data;

    m_reply  = m_service->post(url, data);

    d->state = Private::GD_CREATEFOLDER;

    Q_EMIT signalBusy(true);
}

bool GDTalker::addPhoto(const QString& imgPath, const GSPhoto& info,
                        const QString& id, bool original, bool rescale, int maxDim, int imageQuality)
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
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

        path = WSToolUtils::makeTemporaryDir("google").filePath(QFileInfo(imgPath)
                                             .baseName().trimmed() + QLatin1String(".jpg"));

        if (rescale && ((image.width() > maxDim) || (image.height() > maxDim)))
        {
            image = image.scaled(maxDim,maxDim,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        }

        image.save(path, "JPEG", imageQuality);

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (meta->load(imgPath))
        {
            meta->setItemDimensions(image.size());
            meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
            meta->save(path, true);
        }
    }

    GDMPForm form;
    form.addPair(QUrl::fromLocalFile(path).fileName(), info.description, path, id);

    if (!form.addFile(path))
    {
        Q_EMIT signalBusy(false);
        return false;
    }

    form.finish();

    QUrl url(d->uploadUrl);

    QUrlQuery q;
    q.addQueryItem(QLatin1String("uploadType"), QLatin1String("multipart"));

    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());
    netRequest.setRawHeader("Authorization", m_bearerAccessToken.toLatin1());
    netRequest.setRawHeader("Host", "www.googleapis.com");

    m_reply  = m_service->networkAccessManager()->post(netRequest, form.formData());

    d->state = Private::GD_ADDPHOTO;

    return true;
}

void GDTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        Q_EMIT signalBusy(false);
        QMessageBox::critical(QApplication::activeWindow(),
                              i18nc("@title:window", "Error"), reply->errorString());

        reply->deleteLater();
        return;
    }

    QByteArray buffer =  reply->readAll();

    switch (d->state)
    {
        case (Private::GD_LOGOUT):
        {
            break;
        }

        case (Private::GD_LISTFOLDERS):
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_LISTFOLDERS";
            parseResponseListFolders(buffer);
            break;
        }

        case (Private::GD_CREATEFOLDER):
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_CREATEFOLDER";
            parseResponseCreateFolder(buffer);
            break;
        }

        case (Private::GD_ADDPHOTO):
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_ADDPHOTO"; // << buffer;
            parseResponseAddPhoto(buffer);
            break;
        }

        case (Private::GD_USERNAME):
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GD_USERNAME"; // << buffer;
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

void GDTalker::slotUploadPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << d->listPhotoId.join(QLatin1String(", "));

    Q_EMIT signalUploadPhotoDone(1, QString(), d->listPhotoId);
}

void GDTalker::parseResponseUserName(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);

        return;
    }

    QJsonObject jsonObject = doc.object();
    qCDebug(DIGIKAM_WEBSERVICES_LOG)<<"User Name is: " << jsonObject[QLatin1String("name")].toString();
    QString temp           = jsonObject[QLatin1String("name")].toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in parseResponseUserName";

    Q_EMIT signalBusy(false);
    Q_EMIT signalSetUserName(temp);
}

void GDTalker::parseResponseListFolders(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        Q_EMIT signalListAlbumsDone(0,i18n("Failed to list folders"),QList<GSFolder>());

        return;
    }

    QJsonObject jsonObject = doc.object();
    QJsonArray jsonArray   = jsonObject[QLatin1String("items")].toArray();

    QList<GSFolder> albumList;
    GSFolder fps;
    fps.id    = d->rootid;
    fps.title = d->rootfoldername;
    albumList.append(fps);

    for (const QJsonValue& value : std::as_const(jsonArray))
    {
        QJsonObject obj      = value.toObject();

        // Verify if album is in trash

        QJsonObject labels   = obj[QLatin1String("labels")].toObject();
        bool        trashed  = labels[QLatin1String("trashed")].toBool();

        // Verify if album is editable
        bool        editable = obj[QLatin1String("editable")].toBool();

        /* Verify if album is visualized in a folder inside My Drive
         * If parents is empty, album is shared by another person and not added to My Drive yet
         */

        QJsonArray  parents  = obj[QLatin1String("parents")].toArray();

        fps.id          = obj[QLatin1String("id")].toString();
        fps.title       = obj[QLatin1String("title")].toString();

        if (editable && !trashed && !parents.isEmpty())
        {
            albumList.append(fps);
        }
    }

    std::sort(albumList.begin(), albumList.end(), gdriveLessThan);

    Q_EMIT signalBusy(false);
    Q_EMIT signalListAlbumsDone(1, QString(), albumList);
}

void GDTalker::parseResponseCreateFolder(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);

        return;
    }

    QJsonObject jsonObject = doc.object();
    QString temp           = jsonObject[QLatin1String("alternateLink")].toString();
    bool success           = false;

    if (!(QString::compare(temp, QLatin1String(""), Qt::CaseInsensitive) == 0))
    {
        success = true;
    }

    Q_EMIT signalBusy(false);

    if (!success)
    {
        Q_EMIT signalCreateFolderDone(0,i18n("Failed to create folder"));
    }
    else
    {
        Q_EMIT signalCreateFolderDone(1,QString());
    }
}

void GDTalker::parseResponseAddPhoto(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);

        return;
    }

    QJsonObject jsonObject = doc.object();
    QString altLink        = jsonObject[QLatin1String("alternateLink")].toString();
    QString photoId        = jsonObject[QLatin1String("id")].toString();
    bool success           = false;

    if (!(QString::compare(altLink, QLatin1String(""), Qt::CaseInsensitive) == 0))
    {
        success = true;
    }

    Q_EMIT signalBusy(false);

    if (!success)
    {
        Q_EMIT signalAddPhotoDone(0, i18n("Failed to upload photo"));
    }
    else
    {
        d->listPhotoId << photoId;

        Q_EMIT signalAddPhotoDone(1, QString());
    }
}

void GDTalker::cancel()
{
    if (m_reply)
    {
        m_reply->abort();
        m_reply = nullptr;
    }

    Q_EMIT signalBusy(false);
}

} // namespace DigikamGenericGoogleServicesPlugin

#include "moc_gdtalker.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-31
 * Description : Online version downloader.
 *
 * SPDX-FileCopyrightText: 2020-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "onlineversiondwnl.h"

// Qt includes

#include <QDir>
#include <QSysInfo>
#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QByteArray>
#include <QPointer>
#include <QEventLoop>
#include <QApplication>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QCryptographicHash>
#include <QNetworkAccessManager>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "onlineversionchecker.h"
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN OnlineVersionDwnl::Private
{
public:

    Private() = default;

public:

    bool                   preRelease       = false;    ///< Flag to check pre-releases
    bool                   updateWithDebug  = false;    ///< Flag to use version with debug symbols
    int                    redirects        = 0;        ///< Count of redirected url

    QString                downloadUrl;                 ///< Root url for current download
    QString                checksums;                   ///< Current download sha256 sums
    QString                currentUrl;                  ///< Full url of current file to download
    QString                error;                       ///< Error string about current download
    QString                file;                        ///< Info about file to download (version string, or filename)
    QString                downloaded;                  ///< Local file path to downloaded data

    QNetworkReply*         reply            = nullptr;  ///< Current network request reply
    QNetworkAccessManager* manager          = nullptr;  ///< Network manager instance
};

OnlineVersionDwnl::OnlineVersionDwnl(QObject* const parent,
                                     bool checkPreRelease,
                                     bool updateWithDebug)
    : QObject(parent),
      d      (new Private)
{
    d->preRelease      = checkPreRelease;
    d->updateWithDebug = updateWithDebug;

    if (d->preRelease)
    {
        d->downloadUrl = QLatin1String("https://files.kde.org/digikam/");
    }
    else
    {
        d->downloadUrl = QLatin1String("https://download.kde.org/stable/digikam/");
    }

    d->manager         = new QNetworkAccessManager(this);
    d->manager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    connect(d->manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotDownloaded(QNetworkReply*)));
}

OnlineVersionDwnl::~OnlineVersionDwnl()
{
    cancelDownload();

    delete d;
}

QString OnlineVersionDwnl::downloadUrl() const
{
    return d->downloadUrl;
}

void OnlineVersionDwnl::cancelDownload()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }
}

void OnlineVersionDwnl::startDownload(const QString& version)
{
    QUrl url;

    if (d->preRelease)
    {
        if (d->updateWithDebug)
        {
            QString base = version.section(QLatin1Char('.'), 0, -2);
            QString suf  = version.section(QLatin1Char('.'), -1);
            d->file      = base + QLatin1String("-debug.") + suf;
        }
        else
        {
            d->file = version;
        }

        d->currentUrl = d->downloadUrl + d->file + QLatin1String(".sha256");
        url           = QUrl(d->currentUrl);
    }
    else
    {
        QString arch;
        QString bundle;
        QString qtVersion;
        QString dir;
        QString debug = d->updateWithDebug ? QLatin1String("-debug") : QString();

        if (!OnlineVersionChecker::bundleProperties(arch, bundle, qtVersion, dir))
        {
            Q_EMIT signalDownloadError(i18n("Unsupported Architecture: %1", QSysInfo::buildAbi()));

            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported architecture";

            return;
        }

        QString os    =

#ifdef Q_OS_MACOS

                        QLatin1String("MacOS-");

#else

                        QString();

#endif

        d->file       = QString::fromLatin1("digiKam-%1-%2-%3%4%5.%6")
                            .arg(version)
                            .arg(qtVersion)
                            .arg(os)
                            .arg(arch)
                            .arg(debug)
                            .arg(bundle);

        d->currentUrl = d->downloadUrl + QString::fromLatin1("%1/").arg(version) + d->file + QLatin1String(".sha256");
        url           = QUrl(d->currentUrl);
    }

    d->redirects = 0;
    download(url);
}

void OnlineVersionDwnl::download(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Downloading: " << url;

    d->redirects++;
    d->reply = d->manager->get(QNetworkRequest(url));

    connect(d->reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SIGNAL(signalDownloadProgress(qint64,qint64)));

    connect(d->reply, SIGNAL(sslErrors(QList<QSslError>)),
            d->reply, SLOT(ignoreSslErrors()));
}

void OnlineVersionDwnl::slotDownloaded(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    // mark for deletion

    reply->deleteLater();
    d->reply = nullptr;

    if (
        (reply->error() != QNetworkReply::NoError)             &&
        (reply->error() != QNetworkReply::InsecureRedirectError)
       )
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << reply->errorString();

        Q_EMIT signalDownloadError(reply->errorString());

        return;
    }

    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (
        redirectUrl.isValid()         &&
        (reply->url() != redirectUrl) &&
        (d->redirects < 10)
       )
    {
        download(redirectUrl);

        return;
    }

    // Check if checksum arrive in first

    if (reply->url().url().endsWith(QLatin1String(".sha256")))
    {
        QByteArray data = reply->readAll();

        if (data.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Checksum file is empty";

            Q_EMIT signalDownloadError(i18n("Checksum file is empty."));

            return;
        }

        QTextStream in(&data);
        QString line    = in.readLine();  // first line and section 0 contains the checksum.
        QString sums    = line.section(QLatin1Char(' '), 0, 0);

        if (sums.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Checksum is invalid";
            Q_EMIT signalDownloadError(i18n("Checksum is invalid."));

            return;
        }

        d->checksums = sums;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Checksum is" << d->checksums;

        d->redirects = 0;
        download(QUrl(d->currentUrl.remove(QLatin1String(".sha256"))));

        return;
    }

    // Whole file to download is here

    QByteArray data = reply->readAll();

    if (data.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Downloaded file is empty";

        Q_EMIT signalDownloadError(i18n("Downloaded file is empty."));

        return;
    }

    // Compute checksum in a separated thread

    Q_EMIT signalComputeChecksum();

    QString hash;
    QPointer<QEventLoop> loop = new QEventLoop(this);
    QFutureWatcher<void> fwatcher;

    connect(&fwatcher, SIGNAL(finished()),
            loop, SLOT(quit()));

    connect(static_cast<QDialog*>(parent()), SIGNAL(rejected()),
            &fwatcher, SLOT(cancel()));

    fwatcher.setFuture(QtConcurrent::run([&hash, &data]()
        {
            ActionThreadBase::setCurrentThreadName(QLatin1String("VersionComputeHash"));       // To customize thread name

            QCryptographicHash sha256(QCryptographicHash::Sha256);
            sha256.addData(data);
            hash = QString::fromLatin1(sha256.result().toHex());
        }
    ));

    loop->exec();

    if (d->checksums != hash)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Checksums error";

        Q_EMIT signalDownloadError(i18n("Checksums error."));

        return;
    }

    // Checksum is fine, now save data to disk

    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    path         = QDir::toNativeSeparators(path + QLatin1String("/") + d->file);

    QFile file(path);

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(data);
        file.close();

        QFile::setPermissions(path, QFile::permissions(path) | QFileDevice::ExeUser);
        d->downloaded = path;

        qCDebug(DIGIKAM_GENERAL_LOG) << "Download is complete: " << path;

        Q_EMIT signalDownloadError(QString());  // No error: download is complete.
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open " << path;

        Q_EMIT signalDownloadError(i18n("Cannot open target file."));
    }
}

QString OnlineVersionDwnl::downloadedPath() const
{
    return d->downloaded;
}

} // namespace Digikam

#include "moc_onlineversiondwnl.cpp"

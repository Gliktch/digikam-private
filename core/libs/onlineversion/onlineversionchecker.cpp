/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-31
 * Description : Online version checker
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "onlineversionchecker.h"

// Qt includes

#include <QLocale>
#include <QSysInfo>
#include <QTextStream>
#include <QNetworkAccessManager>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN OnlineVersionChecker::Private
{
public:

    Private() = default;

public:

    bool                   preRelease         = false;                                  ///< Flag to check pre-releases
    int                    redirects          = 0;                                      ///< Count of redirected url

    QString                curVersion         = QLatin1String(digikam_version_short);   ///< Current application version string
    QString                preReleaseFileName;                                          ///< Pre-release file name get from remote server

    QDateTime              curBuildDt         = digiKamBuildDate();                     ///< Current application build date

    QNetworkReply*         reply              = nullptr;                                ///< Current network request reply
    QNetworkAccessManager* manager            = nullptr;                                ///< Network manager instance
};

OnlineVersionChecker::OnlineVersionChecker(QObject* const parent, bool checkPreRelease)
    : QObject(parent),
      d      (new Private)
{
    d->preRelease = checkPreRelease;
    d->manager    = new QNetworkAccessManager(this);
    d->manager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    connect(d->manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotDownloadFinished(QNetworkReply*)));
}

OnlineVersionChecker::~OnlineVersionChecker()
{
    cancelCheck();

    delete d;
}

void OnlineVersionChecker::cancelCheck()
{
    if (d->reply)
    {
        d->reply->abort();
    }
}

void OnlineVersionChecker::setCurrentVersion(const QString& version)
{
    d->curVersion = version;
}

void OnlineVersionChecker::setCurrentBuildDate(const QDateTime& dt)
{
    d->curBuildDt = dt;
}

QString OnlineVersionChecker::preReleaseFileName() const
{
    return d->preReleaseFileName;
}

QString OnlineVersionChecker::lastCheckDate()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Updates"));
    QDateTime dt              = group.readEntry(QLatin1String("Last Check For New Version"), QDateTime());
    QString dts               = QLocale().toString(dt, QLocale::ShortFormat);

    return (!dts.isEmpty() ? dts : i18nc("@info: check of new online version never done yet", "never"));
}

void OnlineVersionChecker::checkForNewVersion()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Updates"));
    group.writeEntry(QLatin1String("Last Check For New Version"), QDateTime::currentDateTime());
    config->sync();

    QUrl rUrl;

    if (d->preRelease)
    {
        rUrl = QUrl(QLatin1String("https://files.kde.org/digikam/FILES"));
    }
    else
    {
        rUrl = QUrl(QLatin1String("https://digikam.org/release.yml"));
    }

    d->redirects = 0;
    download(rUrl);
}

void OnlineVersionChecker::download(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Downloading: " << url;

    d->redirects++;
    d->reply = d->manager->get(QNetworkRequest(url));

    connect(d->reply, SIGNAL(sslErrors(QList<QSslError>)),
            d->reply, SLOT(ignoreSslErrors()));

    if (d->reply->error())
    {
        Q_EMIT signalNewVersionCheckError(d->reply->errorString());
    }
}

void OnlineVersionChecker::slotDownloadFinished(QNetworkReply* reply)
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

        Q_EMIT signalNewVersionCheckError(reply->errorString());

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

    QString data     = QString::fromUtf8(reply->readAll());

    if (data.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No data returned from the remote connection.";

        Q_EMIT signalNewVersionCheckError(i18n("No data returned from the remote connection."));

        return;
    }

    if (d->preRelease)
    {
        // NOTE: pre-release files list from files.kde.org is a simple text
        // file of remote directory contents where we will extract build date string.

        QString arch;
        QString ext;
        QString qtVersion;
        QString dir;

        if (!OnlineVersionChecker::bundleProperties(arch, ext, qtVersion, dir))
        {
            Q_EMIT signalNewVersionCheckError(i18n("Unsupported Architecture: %1", QSysInfo::buildAbi()));

            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported architecture";

            return;
        }

        QTextStream in(&data);
        QString line;

        do
        {
            line = in.readLine();

            if (line.contains(ext) && line.contains(arch))
            {
                d->preReleaseFileName = line.simplified();
                break;
            }
        }
        while (!line.isNull());

        QStringList sections = d->preReleaseFileName.split(QLatin1Char('-'));

        if (sections.size() < 5)
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid file name format returned from the remote connection.";

            Q_EMIT signalNewVersionCheckError(i18n("Invalid file name format returned from the remote connection."));

            return;
        }

        // Two possibles places exists where to find the date in file name.

        // Check 1 - the file-name include a pre release suffix as -beta or -rc

        QString dtStr      = sections[3];
        QDateTime onlineDt = asDateTimeUTC(QDateTime::fromString(dtStr, QLatin1String("yyyyMMddTHHmmss")));

        if (!onlineDt.isValid())
        {
            // Check 2 - the file name do not include a pre release suffix

            dtStr    = sections[2];
            onlineDt = asDateTimeUTC(QDateTime::fromString(dtStr, QLatin1String("yyyyMMddTHHmmss")));
        }

        if (!onlineDt.isValid())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid pre-release date from the remote list.";

            Q_EMIT signalNewVersionCheckError(i18n("Invalid pre-release date from the remote list."));

            return;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Pre-release file Name :" << preReleaseFileName();
        qCDebug(DIGIKAM_GENERAL_LOG) << "Pre-release build date:" << onlineDt;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Current build date:"     << d->curBuildDt;

        if (onlineDt > d->curBuildDt)
        {
            Q_EMIT signalNewVersionAvailable(dtStr);            // Forward pre-release build date from remote file.
        }
        else
        {
            Q_EMIT signalNewVersionCheckError(QString());             // Report error to GUI
        }
    }
    else
    {
        // NOTE: stable files list from digikam.org is a Yaml config file where we will extract version string.

        QString tag            = QLatin1String("version: ");
        int start              = data.indexOf(tag) + tag.size();
        QString rightVer       = data.mid(start);
        int end                = rightVer.indexOf(QLatin1Char('\n'));
        QString onlineVer      = rightVer.mid(0, end);
        QStringList onlineVals = onlineVer.split(QLatin1Char('.'));

        if (onlineVals.size() != 3)
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid format returned from the remote connection.";

            Q_EMIT signalNewVersionCheckError(i18n("Invalid format returned from the remote connection."));

            return;
        }

        QStringList currVals = d->curVersion.split(QLatin1Char('.'));

        qCDebug(DIGIKAM_GENERAL_LOG) << "Online Version:" << onlineVer;

        if (
            digiKamMakeIntegerVersion(onlineVals[0].toInt(), onlineVals[1].toInt(), onlineVals[2].toInt()) >
            digiKamMakeIntegerVersion(currVals[0].toInt(),   currVals[1].toInt(),   currVals[2].toInt())
           )
        {
            Q_EMIT signalNewVersionAvailable(onlineVer);
        }
        else
        {
            Q_EMIT signalNewVersionCheckError(QString());
        }
    }
}

bool OnlineVersionChecker::bundleProperties(QString& arch, QString& ext,
                                            QString& qtVersion, QString& dir)
{

#if defined Q_OS_MACOS

    ext       = QLatin1String("pkg");

#   if defined Q_PROCESSOR_X86_64

    arch      = QLatin1String("x86-64");
    qtVersion = QLatin1String("Qt5");
    dir       = QLatin1String("/legacy");

#   elif defined Q_PROCESSOR_ARM

    arch      = QLatin1String("arm64");
    qtVersion = QLatin1String("Qt6");
    dir       = QLatin1String("/");

#   endif

#endif

#if defined Q_OS_WIN

    ext  = QLatin1String("exe");

#   if defined Q_PROCESSOR_X86_64

    arch = QLatin1String("Win64");

#   elif defined Q_PROCESSOR_ARM

/*  Windows Arm is not yet supported
    arch = QLatin1String("arm-64");
*/

#   elif defined Q_PROCESSOR_X86_32

    // 32 bits is not supported

#   endif

#   if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    qtVersion = QLatin1String("Qt6");
    dir       = QLatin1String("/");

#   else

    qtVersion = QLatin1String("Qt5");
    dir       = QLatin1String("/legacy");

#   endif

#endif

#if defined Q_OS_LINUX

    ext  = QLatin1String("appimage");

#   ifdef Q_PROCESSOR_X86_64

    arch = QLatin1String("x86-64");

#   elif defined Q_PROCESSOR_ARM

/*  Linux Arm is not yet supported
    arch = QLatin1String("arm-64");
*/

#   elif defined Q_PROCESSOR_X86_32

    // 32 bits is not supported

#   endif

#   if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    qtVersion = QLatin1String("Qt6");
    dir       = QLatin1String("/");

#   else

    qtVersion = QLatin1String("Qt5");
    dir       = QLatin1String("/legacy");

#   endif

#endif

    return (
            !ext.isEmpty()       &&
            !arch.isEmpty()      &&
            !qtVersion.isEmpty() &&
            !dir.isEmpty()
           );
}

} // namespace Digikam

#include "moc_onlineversionchecker.cpp"

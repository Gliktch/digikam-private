/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-08
 * Description : Service menu operation methods
 *
 * SPDX-FileCopyrightText: 2014-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dservicemenu.h"

// Qt includes

#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QProcess>
#include <QMimeType>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QRegularExpression>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kservice_version.h>

#if KSERVICE_VERSION > QT_VERSION_CHECK(5, 81, 0)
#   include <kapplicationtrader.h>
#else
#   include <kmimetypetrader.h>
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dfileoperations.h"

#ifdef Q_OS_WIN
#   include <windows.h>
#   include <shellapi.h>
#endif

namespace Digikam
{

bool DServiceMenu::runFiles(const KService::Ptr& service,
                            const QList<QUrl>& urls)
{
    return (runFiles(service->exec(), urls, service, DServiceInfo()));
}

bool DServiceMenu::runFiles(const DServiceInfo& serviceInfo,
                            const QList<QUrl>& urls)
{
    return (runFiles(serviceInfo.exec, urls, KService::Ptr(), serviceInfo));
}

bool DServiceMenu::runFiles(const QString& appCmd,
                            const QList<QUrl>& urls,
                            const KService::Ptr& service,
                            const DServiceInfo& serviceInfo)
{
    QStringList cmdList = splitStringAtSpaces(appCmd);
    QList<QUrl> urlList = urls;

    if (cmdList.isEmpty() || urlList.isEmpty())
    {
        return false;
    }

    if (!appCmd.contains(QLatin1String("%f"), Qt::CaseInsensitive) &&
        !appCmd.contains(QLatin1String("%u"), Qt::CaseInsensitive) &&
        !appCmd.contains(QLatin1String("%d"), Qt::CaseInsensitive))
    {
        cmdList << QLatin1String("%f");
    }

    QString exec;
    QString name;
    QString icon;
    QString term;

    QStringList dirs;
    QStringList files;
    QStringList cmdArgs;
    QStringList termOpts;

    bool useTerminal = false;
    bool openNewRun  = false;

    if       (service)
    {
        icon        = service->icon();
        name        = service->desktopEntryName();

        useTerminal = service->terminal();
        termOpts    = splitStringAtSpaces(service->terminalOptions());
    }
    else if (!serviceInfo.isEmpty())
    {
        icon        = serviceInfo.icon;
        name        = serviceInfo.name;

        useTerminal = serviceInfo.term;
        termOpts    = splitStringAtSpaces(serviceInfo.topt);
    }

#ifdef Q_OS_LINUX

    if (useTerminal)
    {
        term = QStandardPaths::findExecutable(QLatin1String("konsole"));

        if (term.isEmpty())
        {
            term = QStandardPaths::findExecutable(QLatin1String("xterm"));
            termOpts.replaceInStrings(QLatin1String("--noclose"),
                                      QLatin1String("-hold"));
        }

        useTerminal = !term.isEmpty();
    }

#endif // Q_OS_LINUX

    QProcess* const process = new QProcess();
    QProcessEnvironment env = adjustedEnvironmentForAppImage();

    for (const QUrl& url : std::as_const(urlList))
    {
        dirs  << QDir::toNativeSeparators(url.adjusted(QUrl::RemoveFilename).toLocalFile());
        files << QDir::toNativeSeparators(url.toLocalFile());
    }

    for (const QString& cmdString : std::as_const(cmdList))
    {
        QString cmd = cmdString;

        if (cmd.startsWith(QLatin1Char('"')) && cmd.endsWith(QLatin1Char('"')))
        {
            cmd.remove(0, 1).chop(1);
        }

        if (exec.isEmpty() && cmd.contains(QLatin1Char('=')))
        {
            QStringList envList = cmd.split(QLatin1Char('='), Qt::SkipEmptyParts);

            if (envList.count() == 2)
            {
                env.insert(envList[0], envList[1]);
            }

            continue;
        }
        else if (exec.isEmpty())
        {
            exec = cmd;
            continue;
        }

        if      (cmd == QLatin1String("%c"))
        {
            cmdArgs << name;
        }
        else if (cmd == QLatin1String("%i"))
        {
            cmdArgs << icon;
        }
        else if (cmd == QLatin1String("%f"))
        {
            cmdArgs << files.first();
            openNewRun = true;
        }
        else if (cmd == QLatin1String("%F"))
        {
            cmdArgs << files;
        }
        else if (cmd == QLatin1String("%u"))
        {
            cmdArgs << files.first();
            openNewRun = true;
        }
        else if (cmd == QLatin1String("%U"))
        {
            cmdArgs << files;
        }
        else if (cmd == QLatin1String("%d"))
        {
            cmdArgs << dirs.first();
            openNewRun = true;
        }
        else if (cmd == QLatin1String("%D"))
        {
            cmdArgs << dirs;
        }
        else
        {
            cmdArgs << cmd;
        }
    }

    process->setProcessEnvironment(env);

    if (useTerminal)
    {
        termOpts << QLatin1String("-e") << exec << cmdArgs;
        process->start(term, termOpts);
    }
    else
    {

#ifdef Q_OS_WIN

        process->start(DFileOperations::findExecutable(exec), cmdArgs);

#else

        process->start(exec, cmdArgs);

#endif

    }

    bool ret = true;
    ret     &= process->waitForStarted();

    if (openNewRun)
    {
        urlList.removeFirst();

        if (!urlList.isEmpty())
        {
            ret &= runFiles(appCmd, urlList, service, serviceInfo);
        }
    }

    return ret;
}

KService::List DServiceMenu::servicesForOpenWith(const QList<QUrl>& urls)
{
    // This code is inspired by KonqMenuActions:
    // kdebase/apps/lib/konq/konq_menuactions.cpp

    QStringList    mimeTypes;
    KService::List offers;

    for (const QUrl& item : std::as_const(urls))
    {
        const QString mimeType = QMimeDatabase().mimeTypeForFile(item.toLocalFile(),
                                                                 QMimeDatabase::MatchExtension).name();

        if (!mimeTypes.contains(mimeType))
        {
            mimeTypes << mimeType;
        }
    }

    if (!mimeTypes.isEmpty())
    {
        // Query trader

        const QString firstMimeType      = mimeTypes.takeFirst();
        const QString constraintTemplate = QLatin1String("'%1' in ServiceTypes");
        QStringList constraints;

        for (const QString& mimeType : std::as_const(mimeTypes))
        {
            constraints << constraintTemplate.arg(mimeType);
        }

#if KSERVICE_VERSION > QT_VERSION_CHECK(5, 81, 0)

        offers = KApplicationTrader::queryByMimeType(firstMimeType);

#else

        offers = KMimeTypeTrader::self()->query(firstMimeType,
                                                QLatin1String("Application"),
                                                constraints.join(QLatin1String(" and ")));

#endif

        // remove duplicate service entries

        QSet<QString> seenApps;

        for (KService::List::iterator it = offers.begin() ; it != offers.end() ; )
        {
            const QString appName((*it)->name());

            if (!seenApps.contains(appName))
            {
                seenApps.insert(appName);
                ++it;
            }
            else
            {
                it = offers.erase(it);
            }
        }
    }

    return offers;
}

QList<DServiceInfo> DServiceMenu::servicesForOpen(const QList<QUrl>& urls)
{
    QStringList neededMimeTypes;

    for (const QUrl& item : std::as_const(urls))
    {
        QString mimeType = QMimeDatabase().mimeTypeForFile(item.toLocalFile(),
                                                           QMimeDatabase::MatchExtension).name();
        mimeType         = mimeType.left(mimeType.indexOf(QLatin1Char('/')));

        if (!neededMimeTypes.contains(mimeType))
        {
            neededMimeTypes << mimeType;
        }
    }

    QMap<QString, DServiceInfo> servicesMap;

    QStringList appFolders = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                       QLatin1String("applications"), QStandardPaths::LocateDirectory);

    for (const QString& path : std::as_const(appFolders))
    {
        QDir appDir(path, QLatin1String("*.desktop"), QDir::NoSort, QDir::Files | QDir::NoDotAndDotDot);

        const auto list = appDir.entryList();

        for (const QString& file : list)
        {
            KConfig config(path + QLatin1Char('/') + file);
            KConfigGroup group = config.group(QLatin1String("Desktop Entry"));

            if (group.readEntry(QLatin1String("Type"), QString()) != QLatin1String("Application"))
            {
                continue;
            }

            QString name      = group.readEntry(QLatin1String("Name"),            QString());

            if (name.isEmpty() || servicesMap.contains(name))
            {
                continue;
            }

            QStringList mimes = group.readEntry(QLatin1String("MimeType"),        QString()).split(QLatin1Char(';'),
                                                                                                   Qt::SkipEmptyParts);
#ifdef Q_OS_WIN

            QString exec      = group.readEntry(QLatin1String("X-Exec"),          QString());
            QString icon      = group.readEntry(QLatin1String("X-Icon"),          QString());

#else

            QString exec      = group.readEntry(QLatin1String("Exec"),            QString());
            QString icon      = group.readEntry(QLatin1String("Icon"),            QString());

#endif

            QString topt      = group.readEntry(QLatin1String("TerminalOptions"), QString());
            bool    term      = group.readEntry(QLatin1String("Terminal"),        false);

            if (mimes.isEmpty() || exec.isEmpty())
            {
                continue;
            }

            bool typeFound = false;

            for (int i = 0 ; i < mimes.size() ; ++i)
            {
                for (int j = 0 ; j < neededMimeTypes.size() ; ++j)
                {
                    if (mimes.at(i).startsWith(neededMimeTypes.at(j)))
                    {

#ifdef Q_OS_WIN

                        QStringList cmdList = splitStringAtSpaces(exec);

                        if (cmdList.isEmpty() || DFileOperations::findExecutable(cmdList.constFirst()).isEmpty())
                        {
                            break;
                        }

#endif

                        DServiceInfo sinfo(name, exec, icon, topt, term);
                        servicesMap.insert(name, sinfo);
                        typeFound = true;
                        break;
                    }
                }

                if (typeFound)
                {
                    break;
                }
            }
        }
    }

    return servicesMap.values();
}

QIcon DServiceMenu::getIconFromService(const DServiceInfo& sinfo)
{
    QIcon icon(QIcon::fromTheme(sinfo.icon));

#if defined Q_OS_WIN && (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (icon.isNull())
    {
        QString execPath    = DFileOperations::findExecutable(sinfo.icon);
        HINSTANCE hInstance = GetModuleHandle(NULL);                               // krazy:exclude=null
        HICON hicon         = ExtractIconW(hInstance, (LPCWSTR)execPath.utf16(), 0);
        QPixmap exePixmap   = QPixmap::fromImage(QImage::fromHICON(hicon));

        return QIcon(exePixmap);
    }

#endif

    return icon;
}

QStringList DServiceMenu::splitStringAtSpaces(const QString& string)
{
    QRegularExpression split(QLatin1String(" +(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)"));

    return (string.split(split, Qt::SkipEmptyParts));
}

//-----------------------------------------------------------------------------

DServiceInfo::DServiceInfo()
{
}

DServiceInfo::DServiceInfo(const QString& _name,
                           const QString& _exec,
                           const QString& _icon,
                           const QString& _topt,
                           bool           _term)
    : name(_name),
      exec(_exec),
      icon(_icon),
      topt(_topt),
      term(_term)
{
}

DServiceInfo::DServiceInfo(const DServiceInfo& other)
    : name(other.name),
      exec(other.exec),
      icon(other.icon),
      topt(other.topt),
      term(other.term)
{
}

DServiceInfo::~DServiceInfo()
{
}

bool DServiceInfo::isEmpty() const
{
    return (name.isEmpty() || exec.isEmpty());
}

DServiceInfo& DServiceInfo::operator=(const DServiceInfo& other)
{
    name = other.name;
    exec = other.exec;
    icon = other.icon;
    topt = other.topt;
    term = other.term;

    return *this;
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media server manager
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmediaservermngr.h"

// Qt includes

#include <QApplication>
#include <QStringList>
#include <QUrl>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QStandardPaths>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#   include <QTextCodec>
#endif

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dnotificationwrapper.h"

using namespace Digikam;

namespace DigikamGenericMediaServerPlugin
{

class Q_DECL_HIDDEN DMediaServerMngrCreator
{
public:

    DMediaServerMngr object;
};

Q_GLOBAL_STATIC(DMediaServerMngrCreator, creator)

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DMediaServerMngr::Private
{
public:

    Private() = default;

public:

    /// Configuration XML file to store albums map to share in case of restoring between sessions.
    QString              mapsConf;

    /// Server instance pointer.
    DMediaServer*        server                     = nullptr;

    /// The current albums collection to share.
    MediaServerMap       collectionMap;

    const QString configGroupName                   = QLatin1String("DLNA Settings");
    const QString configStartServerOnStartupEntry   = QLatin1String("Start MediaServer At Startup");
};

DMediaServerMngr::DMediaServerMngr()
    : QObject(),
      d      (new Private)
{
    d->mapsConf = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                  QLatin1String("/mediaserver.xml");
}

DMediaServerMngr::~DMediaServerMngr()
{
    delete d;
}

DMediaServerMngr* DMediaServerMngr::instance()
{
    return &creator->object;
}

QString DMediaServerMngr::configGroupName() const
{
    return d->configGroupName;
}

QString DMediaServerMngr::configStartServerOnStartupEntry() const
{
    return d->configStartServerOnStartupEntry;
}

void DMediaServerMngr::cleanUp()
{
    delete d->server;
    d->server = nullptr;
}

bool DMediaServerMngr::loadAtStartup()
{
    KSharedConfig::Ptr config    = KSharedConfig::openConfig();
    KConfigGroup dlnaConfigGroup = config->group(configGroupName());
    bool startServerOnStartup    = dlnaConfigGroup.readEntry(configStartServerOnStartupEntry(), false);
    bool result                  = true;

    if (startServerOnStartup)
    {
        // Restore the old sharing configuration and start the server.

        result &= load();
        result &= startMediaServer();

        mediaServerNotification(result);

        return result;
    }

    return false;
}

void DMediaServerMngr::saveAtShutdown()
{
    KSharedConfig::Ptr config    = KSharedConfig::openConfig();
    KConfigGroup dlnaConfigGroup = config->group(configGroupName());
    bool startServerOnStartup    = dlnaConfigGroup.readEntry(configStartServerOnStartupEntry(), false);

    if (startServerOnStartup)
    {
        // Save the current sharing configuration for the next session.

        save();
    }

    cleanUp();
}

void DMediaServerMngr::mediaServerNotification(bool started)
{
    DNotificationWrapper(QLatin1String("mediaserverloadstartup"),
                         started ? i18n("Media Server have been started")
                                 : i18n("Media Server cannot be started!"),
                         qApp->activeWindow(), qApp->applicationName());
}

void DMediaServerMngr::setItemsList(const QString& aname, const QList<QUrl>& urls)
{
    d->collectionMap.clear();
    d->collectionMap.insert(aname, urls);
}

QList<QUrl> DMediaServerMngr::itemsList() const
{
    QList<QUrl> ret;

    if (!d->collectionMap.isEmpty())
    {
        QList<QList<QUrl> > ulst = d->collectionMap.values();

        for (const QList<QUrl>& urls : std::as_const(ulst))
        {
            ret << urls;
        }
    }

    return ret;
}

void DMediaServerMngr::setCollectionMap(const MediaServerMap& map)
{
    d->collectionMap = map;
}

MediaServerMap DMediaServerMngr::collectionMap() const
{
    return d->collectionMap;
}

bool DMediaServerMngr::startMediaServer()
{
    if (!d->server)
    {
        d->server = new DMediaServer();

        if (!d->server->init())
        {
            cleanUp();

            return false;
        }
    }

    if (d->collectionMap.isEmpty())
    {
        cleanUp();

        return false;
    }

    d->server->addAlbumsOnServer(d->collectionMap);

    return true;
}

bool DMediaServerMngr::isRunning() const
{
    return (d->server ? true : false);
}

int DMediaServerMngr::albumsShared() const
{
    if (d->collectionMap.isEmpty())
    {
        return 0;
    }

    return d->collectionMap.count();
}

int DMediaServerMngr::itemsShared() const
{
    return itemsList().count();
}

bool DMediaServerMngr::save()
{
    QDomDocument doc(QLatin1String("mediaserverlist"));
    doc.setContent(QString::fromUtf8("<!DOCTYPE XMLQueueList><mediaserverlist version=\"1.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();

    auto end = d->collectionMap.cend();

    for (auto it = d->collectionMap.cbegin() ; it != end ; ++it)
    {
        QDomElement elm = doc.createElement(QLatin1String("album"));
        elm.setAttribute(QLatin1String("title"), it.key());

        // ----------------------

        QDomElement data;
        const auto& urls = it.value();

        for (const QUrl& url : urls)
        {
            data = doc.createElement(QLatin1String("path"));
            data.setAttribute(QLatin1String("value"), url.toLocalFile());
            elm.appendChild(data);
        }

        docElem.appendChild(elm);
    }

    QFile file(d->mapsConf);

    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot open XML file to store MediaServer list";
        qCDebug(DIGIKAM_MEDIASRV_LOG) << file.fileName();

        return false;
    }

    QTextStream stream(&file);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)

    // In Qt5 only. Qt6 uses UTF-8 by default.

    stream.setCodec(QTextCodec::codecForName("UTF-8"));

#endif

    stream.setAutoDetectUnicode(true);
    stream << doc.toString(4);
    file.close();

    return true;
}

bool DMediaServerMngr::load()
{
    QFile file(d->mapsConf);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot open XML file to load MediaServer list";

            return false;
        }

        QDomDocument doc(QLatin1String("mediaserverlist"));

        if (!doc.setContent(&file))
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot load MediaServer list XML file";
            file.close();

            return false;
        }

        QDomElement    docElem = doc.documentElement();
        MediaServerMap map;
        QList<QUrl>    urls;
        QString        album;

        for (QDomNode n = docElem.firstChild() ; !n.isNull() ; n = n.nextSibling())
        {
            QDomElement e = n.toElement();

            if (e.isNull())
            {
                continue;
            }

            if (e.tagName() != QLatin1String("album"))
            {
                continue;
            }

            album = e.attribute(QLatin1String("title"));
            urls.clear();

            for (QDomNode n2 = e.firstChild() ; !n2.isNull() ; n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();

                if (e2.isNull())
                {
                    continue;
                }

                QString name2  = e2.tagName();
                QString val2   = e2.attribute(QLatin1String("value"));

                if (name2 == QLatin1String("path"))
                {
                    urls << QUrl::fromLocalFile(val2);
                }
            }

            map.insert(album, urls);
        }

        setCollectionMap(map);
        file.close();

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace DigikamGenericMediaServerPlugin

#include "moc_dmediaservermngr.cpp"

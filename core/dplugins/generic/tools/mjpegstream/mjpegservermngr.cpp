/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG server manager
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegservermngr.h"

// Qt includes

#include <QApplication>
#include <QStringList>
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
#include "mjpegframethread.h"

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class Q_DECL_HIDDEN MjpegServerMngrCreator
{
public:

    MjpegServerMngr object;
};

Q_GLOBAL_STATIC(MjpegServerMngrCreator, creator)

// ---------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN MjpegServerMngr::Private
{
public:

    Private() = default;

public:

    /// Configuration XML file to store albums map to share in case of restoring between sessions.
    QString              mapsConf;

    /// MJPEG Server instance pointer.
    MjpegServer*         server                     = nullptr;

    /// Frames generateur thread.
    MjpegFrameThread*    thread                     = nullptr;

    /// The current albums collection to share.
    MjpegServerMap       collectionMap;

    /// The MJPEG stream settings.
    MjpegStreamSettings  settings;

    const QString configGroupName                   = QLatin1String("MJPEG Settings");
    const QString configStartServerOnStartupEntry   = QLatin1String("Start MjpegServer At Startup");
};

MjpegServerMngr::MjpegServerMngr()
    : QObject(),
      d      (new Private)
{
    d->mapsConf = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                  QLatin1String("/mjpegserver.xml");
}

MjpegServerMngr::~MjpegServerMngr()
{
    delete d;
}

MjpegServerMngr* MjpegServerMngr::instance()
{
    return &creator->object;
}

QString MjpegServerMngr::configGroupName() const
{
    return d->configGroupName;
}

QString MjpegServerMngr::configStartServerOnStartupEntry() const
{
    return d->configStartServerOnStartupEntry;
}

void MjpegServerMngr::cleanUp()
{
    if (d->thread)
    {
        d->thread->cancel();
        delete d->thread;
        d->thread = nullptr;
    }

    if (d->server)
    {
        d->server->stop();
        delete d->server;
        d->server = nullptr;
    }
}

bool MjpegServerMngr::loadAtStartup()
{
    KSharedConfig::Ptr config     = KSharedConfig::openConfig();
    KConfigGroup mjpegConfigGroup = config->group(configGroupName());
    bool startServerOnStartup     = mjpegConfigGroup.readEntry(configStartServerOnStartupEntry(), false);
    bool result                   = true;

    if (startServerOnStartup)
    {
        // Restore the old sharing configuration and start the server.

        result &= load();
        result &= startMjpegServer();

        mjpegServerNotification(result);

        return result;
    }

    return false;
}

void MjpegServerMngr::saveAtShutdown()
{
    KSharedConfig::Ptr config     = KSharedConfig::openConfig();
    KConfigGroup mjpegConfigGroup = config->group(configGroupName());
    bool startServerOnStartup     = mjpegConfigGroup.readEntry(configStartServerOnStartupEntry(), false);

    if (startServerOnStartup)
    {
        // Save the current sharing configuration for the next session.

        save();
    }

    cleanUp();
}

void MjpegServerMngr::mjpegServerNotification(bool started)
{
    DNotificationWrapper(QLatin1String("mjpegserverloadstartup"),
                         started ? i18n("MJPEG Server have been started")
                                 : i18n("MJPEG Server cannot be started!"),
                         qApp->activeWindow(), qApp->applicationName());
}

void MjpegServerMngr::setItemsList(const QString& aname, const QList<QUrl>& urls)
{
    d->collectionMap.clear();
    d->collectionMap.insert(aname, urls);
}

QList<QUrl> MjpegServerMngr::itemsList() const
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

void MjpegServerMngr::setCollectionMap(const MjpegServerMap& map)
{
    d->collectionMap = map;
}

MjpegServerMap MjpegServerMngr::collectionMap() const
{
    return d->collectionMap;
}

void MjpegServerMngr::setSettings(const MjpegStreamSettings& set)
{
    d->settings = set;
}

MjpegStreamSettings MjpegServerMngr::settings() const
{
    return d->settings;
}

bool MjpegServerMngr::startMjpegServer()
{
    if (!d->server)
    {
        d->server = new MjpegServer(QString(), d->settings.port);
        d->server->setRate(d->settings.rate);
        d->server->start();
    }

    if (d->collectionMap.isEmpty())
    {
        cleanUp();

        return false;
    }

    d->thread = new MjpegFrameThread(this);
    d->settings.setCollectionMap(d->collectionMap);
    d->thread->createFrameJob(d->settings);

    connect(d->thread, SIGNAL(signalFrameChanged(QByteArray)),
            d->server, SLOT(slotWriteFrame(QByteArray)));

    d->thread->start();

    return true;
}

bool MjpegServerMngr::isRunning() const
{
    return (d->server ? true : false);
}

int MjpegServerMngr::albumsShared() const
{
    if (d->collectionMap.isEmpty())
    {
        return 0;
    }

    return d->collectionMap.count();
}

int MjpegServerMngr::itemsShared() const
{
    return itemsList().count();
}

bool MjpegServerMngr::save()
{
    QDomDocument doc(QLatin1String("mjpegserverlist"));
    doc.setContent(QString::fromUtf8("<!DOCTYPE XMLQueueList><mjpegserverlist version=\"1.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();
    auto end            = d->collectionMap.cend();

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
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot open XML file to store MjpegServer list";
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

bool MjpegServerMngr::load()
{
    QFile file(d->mapsConf);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot open XML file to load MjpegServer list";

            return false;
        }

        QDomDocument doc(QLatin1String("mjpegserverlist"));

        if (!doc.setContent(&file))
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot load MjpegServer list XML file";
            file.close();

            return false;
        }

        QDomElement    docElem = doc.documentElement();
        MjpegServerMap map;
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

} // namespace DigikamGenericMjpegStreamPlugin

#include "moc_mjpegservermngr.cpp"

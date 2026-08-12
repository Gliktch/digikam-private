/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "HttpDownloadManager.h"

// Qt includes

#include <QList>
#include <QMap>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QCoreApplication>

// Local includes

#include "DownloadPolicy.h"
#include "DownloadQueueSet.h"
#include "HttpJob.h"
#include "StoragePolicy.h"
#include "digikam_debug.h"

namespace Marble
{

// Time before a failed download job is requeued in ms

const quint32 requeueTime = 60000;

class Q_DECL_HIDDEN HttpDownloadManager::Private
{
public:

    Private(HttpDownloadManager* parent, StoragePolicy* policy);
    ~Private();

public:

    void connectDefaultQueueSets();
    void connectQueueSet(DownloadQueueSet*);
    bool hasDownloadPolicy(const DownloadPolicy& policy) const;
    void finishJob(const QByteArray&, const QString&, const QString& id);
    void requeue();
    void startRetryTimer();

    DownloadQueueSet* findQueues(const QString& hostName, const DownloadUsage usage);

public:

    HttpDownloadManager*                                m_downloadManager   = nullptr;
    QTimer                                              m_requeueTimer;

    /**
     * Contains per download policy a queue set containing of
     * - a queue where jobs are waiting for being activated (=downloaded)
     * - a queue containing currently being downloaded
     * - a queue for retries of failed downloads
     */
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> > m_queueSets;

    QMap<DownloadUsage, DownloadQueueSet*>              m_defaultQueueSets;
    StoragePolicy* const                                m_storagePolicy     = nullptr;
    QNetworkAccessManager                               m_networkAccessManager;
    bool                                                m_acceptJobs;
};

HttpDownloadManager::Private::Private(HttpDownloadManager* parent, StoragePolicy* policy)
    : m_downloadManager(parent),
      m_requeueTimer(),
      m_storagePolicy(policy),
      m_networkAccessManager(),
      m_acceptJobs(true)
{
    // setup default download policy and associated queue set

    DownloadPolicy defaultBrowsePolicy;
    defaultBrowsePolicy.setMaximumConnections(20);
    m_defaultQueueSets[ DownloadBrowse ] = new DownloadQueueSet(defaultBrowsePolicy);
    DownloadPolicy defaultBulkDownloadPolicy;
    defaultBulkDownloadPolicy.setMaximumConnections(2);
    m_defaultQueueSets[ DownloadBulk ] = new DownloadQueueSet(defaultBulkDownloadPolicy);
}

HttpDownloadManager::Private::~Private()
{
    QMap<DownloadUsage, DownloadQueueSet*>::iterator pos       = m_defaultQueueSets.begin();
    QMap<DownloadUsage, DownloadQueueSet*>::iterator const end = m_defaultQueueSets.end();

    for ( ; pos != end ; ++pos)
    {
        delete pos.value();
    }
}

DownloadQueueSet* HttpDownloadManager::Private::findQueues(const QString& hostName,
                                                           const DownloadUsage usage)
{
    DownloadQueueSet* result                                                = nullptr;
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::iterator pos       = m_queueSets.begin();
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::iterator const end = m_queueSets.end();

    for ( ; pos != end ; ++pos)
    {
        if ((*pos).first.matches(hostName, usage))
        {
            result = (*pos).second;
            break;
        }
    }

    if (!result)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "No download policy found for" << hostName << usage
                                    << ", using default policy.";
        result = m_defaultQueueSets[ usage ];
    }

    return result;
}

HttpDownloadManager::HttpDownloadManager(StoragePolicy* policy)
    : d(new Private(this, policy))
{
    d->m_requeueTimer.setInterval(requeueTime);

    connect(&d->m_requeueTimer, SIGNAL(timeout()),
            this, SLOT(requeue()));

    d->connectDefaultQueueSets();
}

HttpDownloadManager::~HttpDownloadManager()
{
    delete d;
}

void HttpDownloadManager::setDownloadEnabled(const bool enable)
{
/*
    TODO: PORT_QT6
    d->m_networkAccessManager.setNetworkAccessible(enable ? QNetworkAccessManager::Accessible
                                                          : QNetworkAccessManager::NotAccessible);
*/
    d->m_acceptJobs                                                         = enable;
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::iterator pos       = d->m_queueSets.begin();
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::iterator const end = d->m_queueSets.end();

    for ( ; pos != end ; ++pos)
    {
        pos->second->purgeJobs();
    }
}

void HttpDownloadManager::addDownloadPolicy(const DownloadPolicy& policy)
{
    if (d->hasDownloadPolicy(policy))
    {
        return;
    }

    DownloadQueueSet* const queueSet = new DownloadQueueSet(policy, this);
    d->connectQueueSet(queueSet);
    d->m_queueSets.append(QPair<DownloadPolicyKey, DownloadQueueSet*>
                          (queueSet->downloadPolicy().key(), queueSet));
}

void HttpDownloadManager::addJob(const QUrl& sourceUrl, const QString& destFileName,
                                 const QString& id, const DownloadUsage usage)
{
    if (!d->m_acceptJobs)
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << Q_FUNC_INFO << "Working offline, not adding job";
        return;
    }

    DownloadQueueSet* const queueSet = d->findQueues(sourceUrl.host(), usage);

    if (queueSet->canAcceptJob(sourceUrl, destFileName))
    {
        HttpJob* const job = new HttpJob(sourceUrl, destFileName, id, &d->m_networkAccessManager);
        job->setUserAgentPluginId(QString::fromUtf8("QNamNetworkPlugin"));
        job->setDownloadUsage(usage);
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "adding job " << sourceUrl;
        queueSet->addJob(job);
    }
}

void HttpDownloadManager::Private::finishJob(const QByteArray& data, const QString& destinationFileName,
                                             const QString& id)
{
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "Emitting downloadComplete( QByteArray, " << id << ")";

    Q_EMIT m_downloadManager->downloadComplete(data, id);

    if (m_storagePolicy)
    {
        const bool saved = m_storagePolicy->updateFile(destinationFileName, data);

        if (saved)
        {
            qCDebug(DIGIKAM_GEOENGINE_LOG) << "Emitting downloadComplete( " << destinationFileName << ", " << id << ")";

            Q_EMIT m_downloadManager->downloadComplete(destinationFileName, id);
        }
        else
        {
            qCWarning(DIGIKAM_GEOENGINE_LOG) << "Could not save:" << destinationFileName;
        }
    }
}

void HttpDownloadManager::Private::requeue()
{
    m_requeueTimer.stop();

    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::iterator pos = m_queueSets.begin();
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::iterator const end = m_queueSets.end();

    for ( ; pos != end ; ++pos)
    {
        (*pos).second->retryJobs();
    }
}

void HttpDownloadManager::Private::startRetryTimer()
{
    if (!m_requeueTimer.isActive())
    {
        m_requeueTimer.start();
    }
}

void HttpDownloadManager::Private::connectDefaultQueueSets()
{
    QMap<DownloadUsage, DownloadQueueSet*>::iterator pos = m_defaultQueueSets.begin();
    QMap<DownloadUsage, DownloadQueueSet*>::iterator const end = m_defaultQueueSets.end();

    for ( ; pos != end ; ++pos)
    {
        connectQueueSet(pos.value());
    }
}

void HttpDownloadManager::Private::connectQueueSet(DownloadQueueSet* queueSet)
{
    connect(queueSet, SIGNAL(jobFinished(QByteArray,QString,QString)),
            m_downloadManager, SLOT(finishJob(QByteArray,QString,QString)));

    connect(queueSet, SIGNAL(jobRetry()),
            m_downloadManager, SLOT(startRetryTimer()));

    connect(queueSet, SIGNAL(jobRedirected(QUrl,QString,QString,DownloadUsage)),
            m_downloadManager, SLOT(addJob(QUrl,QString,QString,DownloadUsage)));

    // relay jobAdded/jobRemoved signals (interesting for progress bar)

    connect(queueSet, SIGNAL(jobAdded()),
            m_downloadManager, SIGNAL(jobAdded()));

    connect(queueSet, SIGNAL(jobRemoved()),
            m_downloadManager, SIGNAL(jobRemoved()));

    connect(queueSet, SIGNAL(progressChanged(int,int)),
            m_downloadManager, SIGNAL(progressChanged(int,int)));
}

bool HttpDownloadManager::Private::hasDownloadPolicy(const DownloadPolicy& policy) const
{
    bool found                                                                    = false;
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::const_iterator pos       = m_queueSets.constBegin();
    QList<QPair<DownloadPolicyKey, DownloadQueueSet*> >::const_iterator const end = m_queueSets.constEnd();

    for ( ; pos != end ; ++pos)
    {
        if ((*pos).second->downloadPolicy() == policy)
        {
            found = true;
            break;
        }
    }

    return found;
}

QByteArray HttpDownloadManager::userAgent(const QString& platform, const QString& component)
{
    QString result         = QString::fromUtf8("Mozilla/5.0 (compatible; Marble/%1; %2; %3; %4; %5)");
    bool const smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
    QString const device   = smallScreen ? QString::fromUtf8("MobileDevice") : QString::fromUtf8("DesktopDevice");
    QString const app      = QCoreApplication::applicationName();
    result                 = result.arg(MARBLE_VERSION_STRING, device, platform, component, app);

    return result.toLatin1();
}

} // namespace Marble

#include "moc_HttpDownloadManager.cpp"

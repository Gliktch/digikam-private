/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust thread.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustthread.h"

// Qt includes

#include <QFileInfo>
#include <QReadWriteLock>

// Local includes

#include "dmetadata.h"
#include "timeadjusttask.h"

namespace DigikamGenericTimeAdjustPlugin
{

class Q_DECL_HIDDEN TimeAdjustThread::Private
{

public:

    Private() = default;

    TimeAdjustContainer    settings;  ///< Settings from GUI.

    QReadWriteLock         lock;

    QMap<QUrl, int>        itemsMap;
    QHash<QUrl, QDateTime> timeDateCache;

    bool                   clearTimeCache = false;

    DInfoInterface*        iface          = nullptr;
};


TimeAdjustThread::TimeAdjustThread(QObject* const parent, DInfoInterface* const iface)
    : ActionThreadBase(parent),
      d               (new Private)
{
    d->iface = iface;
}

TimeAdjustThread::~TimeAdjustThread()
{
    // cancel the thread

    cancel();

    // wait for the thread to finish

    wait();

    delete d;
}

void TimeAdjustThread::setUpdatedDates(const QMap<QUrl, int>& itemsMap)
{
    d->itemsMap = itemsMap;

    ActionJobCollection collection;
    const auto urls = itemsMap.keys();

    for (const QUrl& url : urls)
    {
        TimeAdjustTask* const t = new TimeAdjustTask(url, this);
        t->setSettings(d->settings);

        connect(t, SIGNAL(signalProcessStarted(QUrl)),
                this, SIGNAL(signalProcessStarted(QUrl)));

        connect(t, SIGNAL(signalProcessEnded(QUrl,QDateTime,QDateTime,int)),
                this, SIGNAL(signalProcessEnded(QUrl,QDateTime,QDateTime,int)));

        connect(t, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)),
                this, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)));

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void TimeAdjustThread::setPreviewDates(const QMap<QUrl, int>& itemsMap)
{
    d->itemsMap = itemsMap;

    ActionJobCollection collection;
    const auto urls = itemsMap.keys();

    for (const QUrl& url : urls)
    {
        TimePreviewTask* const t = new TimePreviewTask(url, this);
        t->setSettings(d->settings);

        connect(t, SIGNAL(signalPreviewReady(QUrl,QDateTime,QDateTime)),
                this, SIGNAL(signalPreviewReady(QUrl,QDateTime,QDateTime)));

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void TimeAdjustThread::setSettings(const TimeAdjustContainer& settings)
{
    d->settings = settings;

    if (d->clearTimeCache)
    {
        d->timeDateCache.clear();
        d->clearTimeCache = false;
    }
}

QDateTime TimeAdjustThread::readTimestamp(const QUrl& url) const
{
    {
        QReadLocker locker(&d->lock);

        if (d->timeDateCache.contains(url))
        {
            return d->timeDateCache.value(url);
        }
    }

    QDateTime dateTime;

    switch (d->settings.dateSource)
    {
        case TimeAdjustContainer::APPDATE:
        {
            dateTime = readApplicationTimestamp(url);
            break;
        }

        case TimeAdjustContainer::FILENAME:
        {
            dateTime = readFileNameTimestamp(url);
            break;
        }

        case TimeAdjustContainer::FILEDATE:
        {
            dateTime = readFileTimestamp(url);
            break;
        }

        case TimeAdjustContainer::METADATADATE:
        {
            dateTime = readMetadataTimestamp(url);
            break;
        }

        default:  // CUSTOMDATE
        {
            dateTime = QDateTime(d->settings.customDate.date(),
                                 d->settings.customTime.time());

            break;
        }
    }

    {
        QWriteLocker locker(&d->lock);

        d->timeDateCache.insert(url, dateTime);
    }

    return dateTime;
}

QDateTime TimeAdjustThread::readApplicationTimestamp(const QUrl& url) const
{
    DItemInfo info(d->iface->itemInfo(url));

    if (info.dateTime().isValid())
    {
        return info.dateTime();
    }

    return QDateTime();
}

QDateTime TimeAdjustThread::readFileNameTimestamp(const QUrl& url) const
{
    return d->settings.getDateTimeFromString(url.fileName());
}

QDateTime TimeAdjustThread::readFileTimestamp(const QUrl& url) const
{
    QFileInfo fileInfo(url.toLocalFile());

    if (d->settings.fileDateSource == TimeAdjustContainer::FILECREATED)
    {
        return fileInfo.birthTime();
    }

    return fileInfo.lastModified();
}

QDateTime TimeAdjustThread::readMetadataTimestamp(const QUrl& url) const
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (d->settings.metadataSource != TimeAdjustContainer::XMPCREATED)
    {
        meta->setUseXMPSidecar4Reading(false);
    }

    if (!meta->load(url.toLocalFile(), true))
    {
        return QDateTime();
    }

    QDateTime dateTime;

    switch (d->settings.metadataSource)
    {
        case TimeAdjustContainer::EXIFIPTCXMP:
        {
            dateTime = meta->getItemDateTime();
            break;
        }

        case TimeAdjustContainer::EXIFCREATED:
        {
            dateTime = QDateTime::fromString(meta->getExifTagString("Exif.Image.DateTime"),
                                             Qt::ISODate);
            break;
        }

        case TimeAdjustContainer::EXIFORIGINAL:
        {
            dateTime = QDateTime::fromString(meta->getExifTagString("Exif.Photo.DateTimeOriginal"),
                                             Qt::ISODate);
            break;
        }

        case TimeAdjustContainer::EXIFDIGITIZED:
        {
            dateTime = QDateTime::fromString(meta->getExifTagString("Exif.Photo.DateTimeDigitized"),
                                             Qt::ISODate);
            break;
        }

        case TimeAdjustContainer::IPTCCREATED:
        {
            // we have to truncate the timezone from the time, otherwise it cannot be converted to a QTime

            dateTime = QDateTime(QDate::fromString(meta->getIptcTagString("Iptc.Application2.DateCreated"),
                                                   Qt::ISODate),
                                 QTime::fromString(meta->getIptcTagString("Iptc.Application2.TimeCreated").left(8),
                                                   Qt::ISODate));
            break;
        }

        case TimeAdjustContainer::XMPCREATED:
        {
            dateTime = QDateTime::fromString(meta->getXmpTagString("Xmp.xmp.CreateDate"),
                                             Qt::ISODate);

            break;
        }

        case TimeAdjustContainer::FUZZYCREATED:
        {
            dateTime = d->settings.getDateTimeFromString(meta->getExifTagString("Exif.Image.DateTime"));
            break;
        }

        case TimeAdjustContainer::FUZZYORIGINAL:
        {
            dateTime = d->settings.getDateTimeFromString(meta->getExifTagString("Exif.Photo.DateTimeOriginal"));
            break;
        }

        case TimeAdjustContainer::FUZZYDIGITIZED:
        {
            dateTime = d->settings.getDateTimeFromString(meta->getExifTagString("Exif.Photo.DateTimeDigitized"));
            break;
        }

        default:
        {
            // dateTime stays invalid

            break;
        }
    };

    return dateTime;
}

int TimeAdjustThread::indexForUrl(const QUrl& url) const
{
    return d->itemsMap.value(url);
}

void TimeAdjustThread::slotSrcTimestampChanged()
{
    d->clearTimeCache = true;
}

} // namespace DigikamGenericTimeAdjustPlugin

#include "moc_timeadjustthread.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file loading and managing
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "trackmanager.h"

// Qt includes

#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>

// Local includes

#include "trackreader.h"

namespace Digikam
{

// TrackManager::TrackPoint ---------------------------------------------------

bool TrackManager::TrackPoint::EarlierThan(const TrackPoint& a, const TrackPoint& b)
{
    return (a.dateTime < b.dateTime);
}

// TrackManager ---------------------------------------------------------------

class Q_DECL_HIDDEN TrackManager::Private
{
public:

    Private() = default;

    QFutureWatcher<TrackReader::TrackReadResult>* trackLoadFutureWatcher = nullptr;
    QFuture<TrackReader::TrackReadResult>         trackLoadFuture;
    TrackManager::Track::List                     trackPendingList;
    TrackManager::Track::List                     trackList;
    QList<QPair<QUrl, QString> >                  loadErrorFiles;

    Id                                            nextTrackId            = 1;
    int                                           nextTrackColor         = 0;
    bool                                          visibility             = true;
};

TrackManager::TrackManager(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
}

TrackManager::~TrackManager()
{
}

bool TrackManager::getVisibility() const
{
    return d->visibility;
}

void TrackManager::setVisibility(const bool value)
{
    if (d->visibility == value)
    {
        return;
    }

    d->visibility = value;

    Q_EMIT signalVisibilityChanged(value);
}

void TrackManager::clear()
{
    /// @todo send a signal

    d->trackList.clear();
    d->nextTrackColor = 0;
}

const TrackManager::Track& TrackManager::getTrack(const int index) const
{
    return d->trackList.at(index);
}

void TrackManager::loadTrackFiles(const QList<QUrl>& urls)
{
    d->trackLoadFutureWatcher = new QFutureWatcher<TrackReader::TrackReadResult>(this);

    connect(d->trackLoadFutureWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotTrackFilesReadyAt(int,int)));

    connect(d->trackLoadFutureWatcher, SIGNAL(finished()),
            this, SLOT(slotTrackFilesFinished()));

    d->trackLoadFuture = QtConcurrent::mapped(urls, TrackReader::loadTrackFile);
    d->trackLoadFutureWatcher->setFuture(d->trackLoadFuture);

    // results are reported to slotTrackFilesReadyAt
}

void TrackManager::slotTrackFilesReadyAt(int beginIndex, int endIndex)
{
/*
    const int nFilesBefore = d->trackList.count();
*/
    // note that endIndex is exclusive!

    for (int i = beginIndex ; i < endIndex ; ++i)
    {
        const TrackReader::TrackReadResult nextFile = d->trackLoadFuture.resultAt(i);

        if (nextFile.isValid)
        {
            Track nextTrack = nextFile.track;
            nextTrack.id    = getNextFreeTrackId();
            nextTrack.color = getNextFreeTrackColor();
            d->trackPendingList << nextTrack;
        }
        else
        {
            d->loadErrorFiles << QPair<QUrl, QString>(nextFile.track.url, nextFile.loadError);
        }
    }
}

void TrackManager::slotTrackFilesFinished()
{
    d->trackLoadFutureWatcher->deleteLater();

    d->trackList << d->trackPendingList;
    QList<TrackChanges> trackChanges;

    for (const Track& track : std::as_const(d->trackPendingList))
    {
        trackChanges << TrackChanges(track.id, ChangeAdd);
    }

    d->trackPendingList.clear();

    Q_EMIT signalAllTrackFilesReady();
    Q_EMIT signalTracksChanged(trackChanges);
}

TrackManager::Track::List TrackManager::getTrackList() const
{
    return d->trackList;
}

int TrackManager::trackCount() const
{
    return d->trackList.count();
}

QList<QPair<QUrl, QString> > TrackManager::readLoadErrors()
{
    const QList<QPair<QUrl, QString> > result = d->loadErrorFiles;
    d->loadErrorFiles.clear();

    return result;
}

quint64 TrackManager::getNextFreeTrackId()
{
    const quint64 nextId = d->nextTrackId;
    ++(d->nextTrackId);

    return nextId;
}

TrackManager::Track TrackManager::getTrackById(const quint64 trackId) const
{
    for (const Track& track : std::as_const(d->trackList))
    {
        if (track.id == trackId)
        {   // cppcheck-suppress useStlAlgorithm
            return track;
        }
    }

    return TrackManager::Track();
}

QColor TrackManager::getNextFreeTrackColor()
{
    QList<QColor> colorList;
    colorList << Qt::red << Qt::blue << Qt::green << Qt::magenta;

    if (d->nextTrackColor >= colorList.count())
    {
        d->nextTrackColor = 0;
    }

    const QColor nextColor = colorList.at(d->nextTrackColor);
    ++(d->nextTrackColor);

    return nextColor;
}

} // namespace Digikam

#include "moc_trackmanager.cpp"

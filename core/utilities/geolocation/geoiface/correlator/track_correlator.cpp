/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Correlator for tracks and images
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "track_correlator.h"

// Local includes

#include "digikam_debug.h"
#include "track_correlator_thread.h"

namespace Digikam
{

class Q_DECL_HIDDEN TrackCorrelator::Private
{
public:

    Private() = default;

    TrackManager*           trackManager = nullptr;
    TrackCorrelatorThread*  thread       = nullptr;
};

TrackCorrelator::TrackCorrelator(TrackManager* const trackManager,
                                 QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->trackManager = trackManager;

    qRegisterMetaType<Digikam::TrackCorrelator::Correlation::List>("Digikam::TrackCorrelator::Correlation::List");
}

TrackCorrelator::~TrackCorrelator()
{
}

/**
 * @brief GPS-correlate items
 */
void TrackCorrelator::correlate(const Correlation::List& itemsToCorrelate,
                                const CorrelationOptions& options)
{
    d->thread                   = new TrackCorrelatorThread(this);
    d->thread->options          = options;
    d->thread->fileList         = d->trackManager->getTrackList();
    d->thread->itemsToCorrelate = itemsToCorrelate;

    connect(d->thread, SIGNAL(signalItemsCorrelated(Digikam::TrackCorrelator::Correlation::List)),
            this, SLOT(slotThreadItemsCorrelated(Digikam::TrackCorrelator::Correlation::List)),
            Qt::QueuedConnection);

    connect(d->thread, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()),
            Qt::QueuedConnection);

    d->thread->start();
}

void TrackCorrelator::slotThreadItemsCorrelated(const Correlation::List& correlatedItems)
{
    Q_EMIT signalItemsCorrelated(correlatedItems);
}

void TrackCorrelator::slotThreadFinished()
{
    const bool threadCanceled = d->thread->canceled;
    delete d->thread;
    d->thread                 = nullptr;

    if (threadCanceled)
    {
        Q_EMIT signalCorrelationCanceled();
    }
    else
    {
        Q_EMIT signalAllItemsCorrelated();
    }
}

void TrackCorrelator::cancelCorrelation()
{
    if (d->thread)
    {
        d->thread->doCancel = true;
    }
}

} // namespace Digikam

#include "moc_track_correlator.cpp"

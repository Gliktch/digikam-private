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

#include "MarbleClock.h"

// Qt includes

#include <QDateTime>
#include <QTimer>

// Local includes

#include "digikam_debug.h"

namespace Marble
{

class Q_DECL_HIDDEN MarbleClockPrivate
{
public:

    MarbleClock* q = nullptr;
    int          m_speed;
    QTimer       m_timer;
    QDateTime    m_datetime;        // stores the UTC time
    QDateTime    m_lasttime;
    int          m_timezoneInSec;
    int          m_updateInterval;

    explicit MarbleClockPrivate(MarbleClock* parent);

    void timerTimeout();
};

MarbleClockPrivate::MarbleClockPrivate(MarbleClock* parent) :
    q(parent),
    m_speed(1),
    m_datetime(QDateTime::currentDateTimeUtc()),
    m_lasttime(QDateTime::currentDateTimeUtc()),
    m_timezoneInSec(0),
    m_updateInterval(60)
{
    // nothing to do
}

void MarbleClockPrivate::timerTimeout()
{
    // calculate real period elapsed since last call
    QDateTime curenttime(QDateTime::currentDateTimeUtc());
    int msecdelta = m_lasttime.msecsTo(curenttime);
    m_lasttime = curenttime;

    // update m_datetime at m_speed pace
    m_datetime = m_datetime.addMSecs(msecdelta * m_speed);

    // trigger round minute update (at m_speed pace)
    Q_EMIT q->timeChanged();

    // sleeptime is the time to sleep until next round minute, at m_speed pace
    int sleeptime = (m_updateInterval * 1000 - (qreal)(m_datetime.time().msec() + m_datetime.time().second() * 1000)) / m_speed;

    if (sleeptime < 1000)
    {
        // don't trigger more often than 1s
        sleeptime = 1000;
    }

    m_timer.start(sleeptime);

    //qCDebug(DIGIKAM_GEOENGINE_LOG) << "MarbleClock: will sleep for " << sleeptime;
}

MarbleClock::MarbleClock(QObject* parent)
    : QObject(parent), d(new MarbleClockPrivate(this))

{
    connect(&d->m_timer, SIGNAL(timeout()),
            this,    SLOT(timerTimeout()));
    d->timerTimeout();
}


MarbleClock::~MarbleClock()
{
    delete d;
}

qreal MarbleClock::dayFraction() const
{
    qreal fraction = d->m_datetime.time().second();
    fraction = fraction / 60.0 + d->m_datetime.time().minute();
    fraction = fraction / 60.0 + d->m_datetime.time().hour();
    fraction = fraction / 24.0;
    return fraction;
}

void MarbleClock::setDateTime(const QDateTime& datetime)
{
    d->m_datetime = datetime;
    d->timerTimeout();
}

QDateTime MarbleClock::dateTime() const
{
    return d->m_datetime;
}

void MarbleClock::setUpdateInterval(int seconds)
{
    d->m_updateInterval = seconds;
    Q_EMIT updateIntervalChanged(seconds);
}

int MarbleClock::updateInterval() const
{
    return d->m_updateInterval;
}

int MarbleClock::speed() const
{
    return d->m_speed;
}

void MarbleClock::setSpeed(int speed)
{
    d->m_speed = speed;
    d->timerTimeout();
}

int MarbleClock::timezone() const
{
    return d->m_timezoneInSec;
}

void MarbleClock::setTimezone(int timezoneInSec)
{
    d->m_timezoneInSec = timezoneInSec;
}

} // namespace Marble

#include "moc_MarbleClock.cpp"

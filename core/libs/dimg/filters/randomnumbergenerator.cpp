/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-03
 * Description : Generating random numbers
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2021      by Anjani Kumar <anjanik012 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "randomnumbergenerator.h"

// Qt includes

#include <QDateTime>
#include <QFile>
#include <QUuid>
#include <QRandomGenerator>

// C++ includes

#include <random>

namespace Digikam
{

NonDeterministicRandomData::NonDeterministicRandomData(int s)
{

#ifndef Q_OS_WIN

    {
        // Try urandom for UNIX platforms.

        QFile urandom(QLatin1String("/dev/urandom"));

        if (urandom.exists() && urandom.open(QIODevice::ReadOnly))
        {
            resize(s);

            if (urandom.read(data(), s) == s)
            {
                urandom.close();
                return;
            }

            urandom.close();
        }
    }

#endif

    /*
     * Fallback, mostly for Windows, where UUID generation
     * is supposed to be very good.
     */
    if (isEmpty())
    {
        reserve(s);

        while (size() < s)
        {
            append(QByteArray::fromHex(QUuid::createUuid()
                        .toString()
                        .remove(QLatin1Char('{'))
                        .remove(QLatin1Char('}'))
                        .remove(QLatin1Char('-'))
                        .toLatin1())
                  );
        }

        resize(s);
    }
}

// ---------------------------------------------------------------------------------

class Q_DECL_HIDDEN RandomNumberGenerator::Private
{
public:

    enum
    {
        InitialSeed = 5489  ///< guaranteed constant initial seed, do not change
    };

public:

    Private() = default;

    quint32          seed   = InitialSeed;
    QRandomGenerator engine = QRandomGenerator(InitialSeed);
};

RandomNumberGenerator::RandomNumberGenerator()
    : d(new Private)
{
}

RandomNumberGenerator::~RandomNumberGenerator()
{
    delete d;
}

quint32 RandomNumberGenerator::nonDeterministicSeed()
{
    NonDeterministicRandomData rseed(sizeof(quint32));

    return *reinterpret_cast<quint32*>(rseed.data());
}

quint32 RandomNumberGenerator::timeSeed()
{
    uint rseed;
    rseed = quintptr(&rseed) + QDateTime::currentSecsSinceEpoch();

    return rseed;
}

quint32 RandomNumberGenerator::seedNonDeterministic()
{
    d->seed = nonDeterministicSeed();
    d->engine.seed(d->seed);

    return d->seed;
}

quint32 RandomNumberGenerator::seedByTime()
{
    d->seed = timeSeed();
    d->engine.seed(d->seed);

    return d->seed;
}

void RandomNumberGenerator::seed(quint32 seed)
{
    d->seed = seed;
    d->engine.seed(seed);
}

void RandomNumberGenerator::reseed()
{
    seed(d->seed);
}

quint32 RandomNumberGenerator::currentSeed() const
{
    return d->seed;
}

int RandomNumberGenerator::number(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(d->engine);
}

double RandomNumberGenerator::number(double min, double max)
{
    std::uniform_real_distribution<double> dist(min, max);
    return dist(d->engine);
}

bool RandomNumberGenerator::yesOrNo(double p)
{
    std::bernoulli_distribution dist(p);
    return dist(d->engine);
}

} // namespace Digikam

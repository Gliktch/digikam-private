/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-17
 * Description : test parsing gpx data
 *
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "geoparsing_utest.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "geodataparser_time.h"

using namespace DigikamGenericGeolocationEditPlugin;

QTEST_GUILESS_MAIN(TestGPXParsing)

/**
 * @brief Test how well QDateTime deals with various string representations
 *
 * The behavior of QDateTime::fromString changed in some Qt version, so here
 * we can test what the current behavior is and quickly detect if Qt changes
 * again.
 */
void TestGPXParsing::testQDateTimeParsing()
{
    {
        // strings ending with a 'Z' are taken to be in UTC, regardless of milliseconds

        QDateTime time1 = QDateTime::fromString(QLatin1String("2009-03-11T13:39:55.622Z"), Qt::ISODate);
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QDateTime time2 = QDateTime::fromString(QLatin1String("2009-03-11T13:39:55Z"), Qt::ISODate);
        QCOMPARE(time2.timeSpec(), Qt::UTC);
    }

    {
        // eCoach in N900 records GPX files with this kind of date format:
        // 2010-01-14T09:26:02.287+02:00

        QDateTime time1 = QDateTime::fromString(QLatin1String("2010-01-14T09:26:02.287+02:00"), Qt::ISODate);

        /// @todo What about the timezone?

        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(9, 26, 2, 287));

        // when we omit the time zone data, parsing succeeds
        // time is interpreted as local time

        QDateTime time2 = QDateTime::fromString(QLatin1String("2010-01-14T09:26:02.287")/*"+02:00"*/, Qt::ISODate);
        QCOMPARE(time2.date(), QDate(2010, 01, 14));
        QCOMPARE(time2.time(), QTime(9, 26, 2, 287));
        QCOMPARE(time2.timeSpec(), Qt::LocalTime);
    }
}

/**
 * @brief Test our custom parsing function
 */
void TestGPXParsing::testCustomParsing()
{
    {
        // this should work as usual:

        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2009-03-11T13:39:55.622Z"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2009, 03, 11));
        QCOMPARE(time1.time(), QTime(13, 39, 55, 622));
    }

    {
        // eCoach in N900: 2010-01-14T09:26:02.287+02:00

        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2010-01-14T09:26:02.287+02:00"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(7, 26, 02, 287));
    }

    {
        // test negative time zone offset: 2010-01-14T09:26:02.287+02:00

        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2010-01-14T09:26:02.287-02:00"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(11, 26, 02, 287));
    }

    {
        // test negative time zone offset with minutes: 2010-01-14T09:26:02.287+03:15

        const QDateTime time1 = GeoDataParserParseTime(QLatin1String("2010-01-14T09:26:02.287-03:15"));
        QCOMPARE(time1.timeSpec(), Qt::UTC);
        QCOMPARE(time1.date(), QDate(2010, 01, 14));
        QCOMPARE(time1.time(), QTime(12, 41, 02, 287));
    }
}

#include "moc_geoparsing_utest.cpp"

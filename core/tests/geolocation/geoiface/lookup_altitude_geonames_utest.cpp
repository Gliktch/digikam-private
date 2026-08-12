/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-07
 * Description : Test for the geonames based altitude lookup class
 *
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lookup_altitude_geonames_utest.h"

// Local includes

#include "lookupaltitudegeonames.h"
#include "lookupfactory.h"
#include "geoifacecommon.h"

using namespace Digikam;

void TestLookupAltitudeGeonames::testNoOp()
{
}

void TestLookupAltitudeGeonames::testSimpleLookup()
{
    LookupAltitude* const myLookup = LookupFactory::getAltitudeLookup(QString::fromLatin1("geonames"), this);

    QSignalSpy spyRequestsReady(myLookup, SIGNAL(signalRequestsReady(QList<int>)));
    QSignalSpy spyLookupDone(myLookup, SIGNAL(signalDone()));

    LookupAltitude::Request::List requestsList;
    const int nRequests = 30;

    // add different requests
    for (qreal i = 0; i < nRequests; ++i)
    {
        LookupAltitude::Request myRequest;
        myRequest.coordinates = GeoCoordinates(52.0, 6.0+i);
        requestsList << myRequest;
    }

    // add those same requests again, expecting them to be merged into the existing requests:
    for (qreal i = 0; i < nRequests; ++i)
    {
        LookupAltitude::Request myRequest;
        myRequest.coordinates = GeoCoordinates(52.0, 6.0+i);
        requestsList << myRequest;
    }

    myLookup->addRequests(requestsList);
    myLookup->startLookup();

    // wait until the files are loaded:
    while (spyLookupDone.isEmpty())
    {
        QTest::qWait(100);
    }

    QCOMPARE(spyRequestsReady.count(), 2);
}

QTEST_GUILESS_MAIN(TestLookupAltitudeGeonames)

#include "moc_lookup_altitude_geonames_utest.cpp"

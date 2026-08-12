/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-28
 * Description : Test loading and saving of data in GPSItemContainer.
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gpsimageitem_utest.h"

// C++ includes

#include <memory>

// Qt includes

#include <QDateTime>
#include <QScopedPointer>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
#include "gpsdatacontainer.h"
#include "gpsitemcontainer.h"
#include "dtestdatadir.h"

using namespace Digikam;

void TestGPSItemContainer::initTestCase()
{
    // initialize Exiv2 before doing any multitasking

    MetaEngine::initializeExiv2();
}

void TestGPSItemContainer::cleanupTestCase()
{
}

/**
 * @return the path of the directory containing the test data.
 */
QString GetTestDataDirectory()
{
    QString filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/geolocation/editor"))
                           .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << filesPath;

    return filesPath;
}

GPSItemContainer* ItemFromFile(const QUrl& url)
{
    std::unique_ptr<GPSItemContainer> imageItem(new GPSItemContainer(url));

    if (imageItem->loadImageData())
    {
        return imageItem.release();
    }

    return nullptr;
}

/**
 * @brief Dummy test that does nothing
 */
void TestGPSItemContainer::testNoOp()
{
}

void TestGPSItemContainer::testBasicLoading()
{
    {
        // test failure on not-existing file

        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1String("not-existing"));
        QScopedPointer<GPSItemContainer> imageItem(ItemFromFile(testDataDir));
        QVERIFY(!imageItem);
    }

    {
        // load a file without GPS info

        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1String("exiftest-nogps.png"));
        QScopedPointer<GPSItemContainer> imageItem(ItemFromFile(testDataDir));
        QVERIFY(imageItem);

        const GPSDataContainer container = imageItem->gpsData();
        QVERIFY(!container.hasCoordinates());
        QVERIFY(!container.hasAltitude());
        QVERIFY(!container.hasNSatellites());
        QVERIFY(!container.hasDop());
        QVERIFY(!container.hasFixType());
    }

    {
        // load a file with geo:5,15,25

        QUrl testDataDir = QUrl::fromLocalFile(GetTestDataDirectory() + QLatin1String("exiftest-5_15_25.jpg"));
        QScopedPointer<GPSItemContainer> imageItem(ItemFromFile(testDataDir));
        QVERIFY(imageItem);

        const GPSDataContainer container = imageItem->gpsData();
        QVERIFY(container.hasCoordinates());
        QVERIFY(container.hasAltitude());
        QVERIFY(container.getCoordinates().lat() == 5.0);
        QVERIFY(container.getCoordinates().lon() == 15.0);
        QVERIFY(container.getCoordinates().alt() == 25.0);
        QVERIFY(!container.hasNSatellites());
        QVERIFY(!container.hasDop());
        QVERIFY(!container.hasFixType());
    }
}

QTEST_GUILESS_MAIN(TestGPSItemContainer)

#include "moc_gpsimageitem_utest.cpp"

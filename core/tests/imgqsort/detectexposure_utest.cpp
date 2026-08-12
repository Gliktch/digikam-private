/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : an unit-test to detect image quality level
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "detectexposure_utest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QDebug>

// Local includes


using namespace Digikam;

QTEST_MAIN(ImgQSortTestDetecteExposure)

ImgQSortTestDetecteExposure::ImgQSortTestDetecteExposure(QObject* const parent)
    : ImgQSortTest(parent)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestDetecteExposure::testParseTestImagesForExposureDetection()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("exposureDetection"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTEXPOSURE);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetecteExposure::testParseTestImagesForExposureDetection_backlight()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("exposureBacklight"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTEXPOSURE);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetecteExposure::testParseTestImagesForExposureDetection_sun()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("exposureSun"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTEXPOSURE);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

#include "moc_detectexposure_utest.cpp"

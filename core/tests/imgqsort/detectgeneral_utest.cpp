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

#include "detectgeneral_utest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QFileInfoList>
#include <QDebug>

// Local includes

#include "digikam_globals.h"
#include "imagequalitysettings.h"
#include "dpluginloader.h"

using namespace Digikam;

QTEST_MAIN(ImgQSortTestGeneral)

ImgQSortTestGeneral::ImgQSortTestGeneral(QObject* const)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestGeneral::testParseTestImagesForGeneralNormalImage()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("normalImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTIONGENERAL);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestGeneral::testParseTestImagesForSunImage()
{
    CustomDetection customSettings { true, true, true, false, false };

    if (!m_hasExifTool)
    {
        m_dataTestCases.remove(QLatin1String("sunImage"),
                               PairImageQuality(QLatin1String("exposure_sun_1.jpg"), 1));
        m_dataTestCases.insert(QLatin1String("sunImage"),
                               PairImageQuality(QLatin1String("exposure_sun_1.jpg"), 2));
    }

    QHash<QString, bool> results = testParseTestImages(QLatin1String("sunImage"),
                                                       ImgQSortTest_ParseTestImagesCustomDetection, customSettings);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestGeneral::testParseTestImagesForGeneral_failCase()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("failCase"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTIONGENERAL);

    for (const auto& test_case : results.keys())
    {
        QEXPECT_FAIL("", "Will fix in the next release", Continue);

        QVERIFY(results.value(test_case));
    }
}

#include "moc_detectgeneral_utest.cpp"

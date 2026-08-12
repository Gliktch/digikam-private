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

#include "detectnoise_utest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QFileInfoList>
#include <QDebug>
#include <QDir>

// Local includes

#include "digikam_debug.h"

using namespace Digikam;

QTEST_MAIN(ImgQSortTestDetectNoise)

ImgQSortTestDetectNoise::ImgQSortTestDetectNoise(QObject* const parent)
    : ImgQSortTest(parent)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestDetectNoise::testParseTestImagesForNoiseDetection()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("noiseDetection"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTNOISE);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectNoise::testParseTestImagesForImageHighSO()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("highISO"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTNOISE);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectNoise::testParseTestImagesForVariousTypeNoise()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("variousTypesNoise"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTNOISE);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectNoise::testParseTestImagesForVariousTypeNoiseFailCase()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("variousTypesNoiseFailCase"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTNOISE);

    for (const auto& test_case : results.keys())
    {
        QEXPECT_FAIL("", "Will fix in the next release", Continue);

        QVERIFY(results.value(test_case));
    }
}

#include "moc_detectnoise_utest.cpp"

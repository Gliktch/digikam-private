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

#include "detectblur_utest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QFileInfoList>
#include <QDebug>
#include <QDir>

// Local includes

#include "digikam_globals.h"
#include "imagequalitysettings.h"
#include "dpluginloader.h"
#include "digikam_debug.h"

using namespace Digikam;

QTEST_MAIN(ImgQSortTestDetectBlur)

ImgQSortTestDetectBlur::ImgQSortTestDetectBlur(QObject* const parent)
    : ImgQSortTest(parent)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestDetectBlur::testParseTestImagesForBlurDetection()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("blurDetection"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTBLUR);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectBlur::testParseTestImagesForBlurDetection_SharpImage()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("sharpImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTBLUR);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectBlur::testParseTestImagesForBlurDetection_MotionBlurImage()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("motionBlurImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTBLUR);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectBlur::testParseTestImagesForBlurDetection_DefocusImage()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("defocusImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTBLUR);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectBlur::testParseTestImagesForBlurDetection_BlurBackGroundImage()
{
    if (!m_hasExifTool)
    {
        m_dataTestCases.remove(QLatin1String("blurBackGroundImage"));
        m_dataTestCases.insert(QLatin1String("blurBackGroundImage"),
                               PairImageQuality(QLatin1String("blur_blurbackground_1.jpg"), 2));
        m_dataTestCases.insert(QLatin1String("blurBackGroundImage"),
                               PairImageQuality(QLatin1String("blur_blurbackground_2.jpg"), 2));
        m_dataTestCases.insert(QLatin1String("blurBackGroundImage"),
                               PairImageQuality(QLatin1String("blur_blurbackground_3.jpg"), 2));
    }

    QHash<QString, bool> results = testParseTestImages(QLatin1String("blurBackGroundImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTBLUR);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectBlur::testParseTestImagesForBlurDetection_FailCase()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("blurDetectionFailTest"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTBLUR);

    for (const auto& test_case : results.keys())
    {
        QEXPECT_FAIL("", "Will fix in the next release", Continue);

        QVERIFY(results.value(test_case));
    }
}

#include "moc_detectblur_utest.cpp"

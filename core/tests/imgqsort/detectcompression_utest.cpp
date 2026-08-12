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

#include "detectcompression_utest.h"

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

QTEST_MAIN(ImgQSortTestDetectCompression)

ImgQSortTestDetectCompression::ImgQSortTestDetectCompression(QObject* const parent)
    : ImgQSortTest(parent)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestDetectCompression::testParseTestImagesForCompressionDetection()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("compressionDetection"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTCOMPRESSION);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestDetectCompression::testParseTestImagesForCompressionDetection_failCase()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("compressionDetection_failCase"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTCOMPRESSION);

    for (const auto& test_case : results.keys())
    {
        QEXPECT_FAIL("", "Will fix in the next release", Continue);

        QVERIFY(results.value(test_case));
    }
}

#include "moc_detectcompression_utest.cpp"

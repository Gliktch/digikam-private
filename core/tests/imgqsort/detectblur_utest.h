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

#pragma once

// Qt includes

#include <QObject>
#include <QMultiMap>
#include <QString>

// Local includes

#include "imgqsorttest.h"

using namespace Digikam;

class ImgQSortTestDetectBlur : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestDetectBlur(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForBlurDetection();
    void testParseTestImagesForBlurDetection_SharpImage();
    void testParseTestImagesForBlurDetection_MotionBlurImage();
    void testParseTestImagesForBlurDetection_DefocusImage();
    void testParseTestImagesForBlurDetection_BlurBackGroundImage();
    void testParseTestImagesForBlurDetection_FailCase();
};

DataTestCases const dataTestCases =
{
    {QLatin1String("blurDetection"),         PairImageQuality(QLatin1String("test_blurred_1.jpg"),        2)},
    {QLatin1String("blurDetection"),         PairImageQuality(QLatin1String("test_blurred_2.jpg"),        2)},
    {QLatin1String("blurDetection"),         PairImageQuality(QLatin1String("test_blurred_5.jpg"),        1)},
    {QLatin1String("blurDetection"),         PairImageQuality(QLatin1String("test_blurred_9.jpg"),        1)},

    {QLatin1String("sharpImage"),            PairImageQuality(QLatin1String("blur_rock_1.jpg"),           3)},
    {QLatin1String("sharpImage"),            PairImageQuality(QLatin1String("blur_tree_1.jpg"),           3)},
    {QLatin1String("sharpImage"),            PairImageQuality(QLatin1String("blur_street_1.jpg"),         3)},

    {QLatin1String("motionBlurImage"),       PairImageQuality(QLatin1String("blur_sky_2.jpg"),            2)},
    {QLatin1String("motionBlurImage"),       PairImageQuality(QLatin1String("blur_rock_2.jpg"),           2)},
    {QLatin1String("motionBlurImage"),       PairImageQuality(QLatin1String("blur_tree_2.jpg"),           2)},
    {QLatin1String("motionBlurImage"),       PairImageQuality(QLatin1String("blur_street_2.jpg"),         2)},

    {QLatin1String("defocusImage"),          PairImageQuality(QLatin1String("blur_sky_3.jpg"),            1)},
    {QLatin1String("defocusImage"),          PairImageQuality(QLatin1String("blur_street_3.jpg"),         1)},

    {QLatin1String("blurBackGroundImage"),   PairImageQuality(QLatin1String("blur_blurbackground_1.jpg"), 3)}, // other result without ExifTool
    {QLatin1String("blurBackGroundImage"),   PairImageQuality(QLatin1String("blur_blurbackground_2.jpg"), 3)}, // other result without ExifTool
    {QLatin1String("blurBackGroundImage"),   PairImageQuality(QLatin1String("blur_blurbackground_3.jpg"), 3)}, // other result without ExifTool

    {QLatin1String("blurDetectionFailTest"), PairImageQuality(QLatin1String("blur_rock_3.jpg"),           1)}, // False case : Pending instead of Rejected
    {QLatin1String("blurDetectionFailTest"), PairImageQuality(QLatin1String("blur_tree_3.jpg"),           1)}, // False case : Pending instead of Rejected
    {QLatin1String("blurDetectionFailTest"), PairImageQuality(QLatin1String("blur_sky_1.jpg"),            3)}, // False case : Pending instead of Accepted
};

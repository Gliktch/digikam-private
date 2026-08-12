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
#include <QDir>
#include <QMultiMap>
#include <QString>

// Local includes

#include "imgqsorttest.h"

class ImgQSortTestGeneral : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestGeneral(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForGeneralNormalImage();
    void testParseTestImagesForSunImage();
    void testParseTestImagesForGeneral_failCase();
};

DataTestCases const dataTestCases =
{
    {QLatin1String("normalImage"),      PairImageQuality(QLatin1String("general_normal_image_1.jpg"),     2)},
    {QLatin1String("normalImage"),      PairImageQuality(QLatin1String("general_normal_image_2.jpg"),     2)},
    {QLatin1String("normalImage"),      PairImageQuality(QLatin1String("general_normal_image_3.jpg"),     2)},
    {QLatin1String("normalImage"),      PairImageQuality(QLatin1String("general_normal_image_4.jpg"),     2)},
    {QLatin1String("normalImage"),      PairImageQuality(QLatin1String("general_normal_image_5.jpg"),     2)},
    {QLatin1String("normalImage"),      PairImageQuality(QLatin1String("general_normal_image_6.jpg"),     2)},

    {QLatin1String("sunImage"),         PairImageQuality(QLatin1String("exposure_sun_1.jpg"),             1)}, // other result without ExifTool
    {QLatin1String("sunImage"),         PairImageQuality(QLatin1String("exposure_sun_2.jpg"),             2)},
    {QLatin1String("sunImage"),         PairImageQuality(QLatin1String("exposure_sun_3.jpg"),             2)},

    {QLatin1String("failCase"),         PairImageQuality(QLatin1String("general_bad_image_2.jpg"),        1)},
    {QLatin1String("failCase"),         PairImageQuality(QLatin1String("general_bad_image_4.jpg"),        1)},
    {QLatin1String("failCase"),         PairImageQuality(QLatin1String("general_quite_good_image_1.jpg"), 3)},
    {QLatin1String("failCase"),         PairImageQuality(QLatin1String("general_quite_good_image_6.jpg"), 3)},
};

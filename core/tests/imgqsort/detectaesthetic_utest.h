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
#include <QString>

// Local includes

#include "imgqsorttest.h"

using namespace Digikam;

class ImgQSortTestDetectQesthetic : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestDetectQesthetic(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForAestheticDetection();
};

DataTestCases const dataTestCases =
{
    {QLatin1String("aestheticDetection"),         PairImageQuality(QLatin1String("aesthetic_1.jpg"),         3)},
    {QLatin1String("aestheticDetection"),         PairImageQuality(QLatin1String("aesthetic_2.jpg"),         3)},
    {QLatin1String("aestheticDetection"),         PairImageQuality(QLatin1String("aesthetic_3.jpg"),         3)},

    {QLatin1String("aestheticDetection"),         PairImageQuality(QLatin1String("general_bad_image_1.jpg"), 1)},
    {QLatin1String("aestheticDetection"),         PairImageQuality(QLatin1String("general_bad_image_2.jpg"), 1)},
    {QLatin1String("aestheticDetection"),         PairImageQuality(QLatin1String("general_bad_image_3.jpg"), 1)},
};

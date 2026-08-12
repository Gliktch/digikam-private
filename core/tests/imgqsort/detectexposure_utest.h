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

#include "imgqsorttest_shared.h"
#include "imgqsorttest.h"

namespace Digikam
{

class ImgQSortTestDetecteExposure : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestDetecteExposure(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForExposureDetection_sun();
    void testParseTestImagesForExposureDetection_backlight();
    void testParseTestImagesForExposureDetection();
};

// pair name image - quality expected
using PairImageQuality = QPair<QString, int>;
using DataTestCases    = QMultiMap<QString, PairImageQuality>;

DataTestCases const dataTestCases =
{
    {QLatin1String("exposureBacklight"), PairImageQuality(QLatin1String("exposure_backlight_1.jpg"), 1)},
    {QLatin1String("exposureBacklight"), PairImageQuality(QLatin1String("exposure_backlight_2.jpg"), 1)},
    {QLatin1String("exposureBacklight"), PairImageQuality(QLatin1String("exposure_backlight_3.jpg"), 1)},

    {QLatin1String("exposureSun"),       PairImageQuality(QLatin1String("exposure_sun_1.jpg"),       2)},
    {QLatin1String("exposureSun"),       PairImageQuality(QLatin1String("exposure_sun_2.jpg"),       2)},
    {QLatin1String("exposureSun"),       PairImageQuality(QLatin1String("exposure_sun_3.jpg"),       1)},

    {QLatin1String("exposureDetection"), PairImageQuality(QLatin1String("test_overexposed_5.jpg"),   2)},
    {QLatin1String("exposureDetection"), PairImageQuality(QLatin1String("test_overexposed_9.jpg"),   1)},
    {QLatin1String("exposureDetection"), PairImageQuality(QLatin1String("test_underexposed_1.jpg"),  1)},
    {QLatin1String("exposureDetection"), PairImageQuality(QLatin1String("test_underexposed_5.jpg"),  2)},

};

} // namespace Digikam

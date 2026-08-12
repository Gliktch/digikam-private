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

class ImgQSortTestDetectNoise : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestDetectNoise(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForNoiseDetection();
    void testParseTestImagesForImageHighSO();
    void testParseTestImagesForVariousTypeNoise();
    void testParseTestImagesForVariousTypeNoiseFailCase();
};

DataTestCases const dataTestCases =
{
    {QLatin1String("noiseDetection"),               PairImageQuality(QLatin1String("test_noised_1.jpg"),          3)},
    {QLatin1String("noiseDetection"),               PairImageQuality(QLatin1String("test_noised_2.jpg"),          3)},
    {QLatin1String("noiseDetection"),               PairImageQuality(QLatin1String("test_noised_5.jpg"),          2)},
    {QLatin1String("noiseDetection"),               PairImageQuality(QLatin1String("test_noised_9.jpg"),          1)},

    {QLatin1String("highISO"),                      PairImageQuality(QLatin1String("noise_book_1.jpg"),           3)},
    {QLatin1String("highISO"),                      PairImageQuality(QLatin1String("noise_book_2.jpg"),           1)},
    {QLatin1String("highISO"),                      PairImageQuality(QLatin1String("noise_graffi_1.jpg"),         3)},
    {QLatin1String("highISO"),                      PairImageQuality(QLatin1String("noise_graffi_2.jpg"),         1)},

    {QLatin1String("variousTypesNoise"),            PairImageQuality(QLatin1String("noise_bird_nor.png"),         3)},
    {QLatin1String("variousTypesNoise"),            PairImageQuality(QLatin1String("noise_bird_gaussian.png"),    1)},
    {QLatin1String("variousTypesNoise"),            PairImageQuality(QLatin1String("noise_bird_rayleigh.png"),    1)},
    {QLatin1String("variousTypesNoise"),            PairImageQuality(QLatin1String("noise_bird_speckle.png"),     1)},

    {QLatin1String("variousTypesNoiseFailCase"),    PairImageQuality(QLatin1String("noise_bird_salt_pepper.png"), 1)},
    {QLatin1String("variousTypesNoiseFailCase"),    PairImageQuality(QLatin1String("noise_bird_bandpass.png"),    1)},
};

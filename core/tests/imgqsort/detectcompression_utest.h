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

namespace Digikam
{

class ImgQSortTestDetectCompression : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestDetectCompression(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForCompressionDetection();
    void testParseTestImagesForCompressionDetection_failCase();
};

DataTestCases const dataTestCases =
{
    {QLatin1String("compressionDetection"),          PairImageQuality(QLatin1String("test_compressed_2.jpg"), 2)},
    {QLatin1String("compressionDetection"),          PairImageQuality(QLatin1String("test_compressed_4.jpg"), 2)},
    {QLatin1String("compressionDetection"),          PairImageQuality(QLatin1String("test_compressed_9.jpg"), 3)},

    {QLatin1String("compressionDetection_failCase"), PairImageQuality(QLatin1String("test_compressed_1.jpg"), 1)}, //False case : Pending instead of Accepted
};

} // namespace Digikam

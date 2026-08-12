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

#include <QMultiMap>
#include <QString>
#include <QFileInfoList>
#include <QObject>
#include <QDir>
#include <QTest>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

enum DetectionType
{
    DETECTBLUR = 0,
    DETECTNOISE,
    DETECTCOMPRESSION,
    DETECTEXPOSURE,
    DETECTAESTHETIC,
    DETECTIONGENERAL
};

struct CustomDetection
{
    bool detectBlur;
    bool detectNoise;
    bool detectExposure;
    bool detectCompression;
    bool detectAesthetic;
};

QHash<QString, int> ImgQSortTest_ParseTestImagesDefautDetection(DetectionType type, const QFileInfoList& list);

QHash<QString, int> ImgQSortTest_ParseTestImagesCustomDetection(const CustomDetection& customSetting, const QFileInfoList& list);

} // namespace Digikam

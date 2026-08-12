/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNN Model Manager definition
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

namespace Digikam
{

typedef enum _DNNLoaderType
{
    DNNLoaderNet,
    DNNLoaderConfig,
    DNNLoaderYuNet,
    DNNLoaderSFace
} DNNLoaderType;

typedef enum _DNNModelUsage
{
    DNNUsageFaceDetection,
    DNNUsageFaceRecognition,
    DNNUsageRedeyeDetection,
    DNNUsageObjectDetection,
    DNNUsageImageClassification,
    DNNUsageAesthetics,
    DNNUsageAutoRotate
} DNNModelUsage;

typedef QList<DNNModelUsage> DNNModelUsageList;

const int DNN_MODEL_THRESHOLD_NOT_SET = 1000;

} // namespace Digikam

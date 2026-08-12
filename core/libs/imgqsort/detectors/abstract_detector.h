/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser - Abstract class for detector
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>

// Local includes

#include "dimg.h"
#include "digikam_opencv.h"

namespace Digikam
{

class AbstractDetector : public QObject
{
    Q_OBJECT

public:

    explicit AbstractDetector(QObject* const parent = nullptr);
    virtual ~AbstractDetector() = default;

    virtual float detect(const cv::Mat& image) const = 0;

public:

    static cv::Mat prepareForDetection(const DImg& inputImage);
};

} // namespace Digikam

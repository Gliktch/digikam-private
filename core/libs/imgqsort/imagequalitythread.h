/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Thread to run detectors
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "abstract_detector.h"
#include "imagequalitycalculator.h"

namespace Digikam
{

class ImageQualityThread : public QThread
{
    Q_OBJECT

public:

    explicit ImageQualityThread(QObject* const parent,
                                AbstractDetector* const detector,
                                const cv::Mat& image,
                                ImageQualityCalculator* const calculator,
                                float weight_quality);
    ~ImageQualityThread() = default;

public:

    void run() override;

private:

    AbstractDetector*       m_detector   = nullptr;
    ImageQualityCalculator* m_calculator = nullptr;
    cv::Mat                 m_image;
    float                   m_weight     = 0.0F;
};

// -------------------------------------------------------------------------------------------

class ImageQualityThreadPool: public QObject
{
    Q_OBJECT

public:

    explicit ImageQualityThreadPool(QObject* const parent,
                                    ImageQualityCalculator* const calculator);
    ~ImageQualityThreadPool();

public:

    void addDetector(const cv::Mat& image,
                     float weight_quality,
                     AbstractDetector* const detector);

    void start();
    void end();

private:

    ImageQualityCalculator*    m_calculator = nullptr;
    QList<ImageQualityThread*> m_threads;
};

} // namespace Digikam

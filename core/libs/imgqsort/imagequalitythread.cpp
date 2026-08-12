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

#include "imagequalitythread.h"

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{

ImageQualityThread::ImageQualityThread(QObject* const parent,
                                       AbstractDetector* const detector,
                                       const cv::Mat& image,
                                       ImageQualityCalculator* const calculator,
                                       float weight_quality)
    : QThread     (parent),
      m_detector  (detector),
      m_calculator(calculator),
      m_image     (image),
      m_weight    (weight_quality)
{
}

void ImageQualityThread::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("ImageQualityThread"));       // To customize thread name

    float damageLevel = m_detector->detect(m_image);
    m_calculator->addDetectionResult(QString(), damageLevel, m_weight);
}

//--------------------------------------------------------------------------

ImageQualityThreadPool::ImageQualityThreadPool(QObject* const parent,
                                               ImageQualityCalculator* const calculator)
    : QObject     (parent),
      m_calculator(calculator)
{
}

ImageQualityThreadPool::~ImageQualityThreadPool()
{
    end();
/*
    for (auto& thread : std::as_const(m_threads))
    {
        delete thread;
    }
*/
}


void ImageQualityThreadPool::addDetector(const cv::Mat& image,
                                         float weight_quality,
                                         AbstractDetector* const detector)
{
    ImageQualityThread* const thread = new ImageQualityThread(this,
                                                              detector,
                                                              image,
                                                              m_calculator,
                                                              weight_quality);
/*
    connect(thread, &QThread::finished,
            thread, &QObject::deleteLater);
*/
    m_threads.push_back(thread);
}

void ImageQualityThreadPool::start()
{
    for (const auto& thread : std::as_const(m_threads))
    {
        thread->start();
    }
}

void ImageQualityThreadPool::end()
{
    for (auto& thread : std::as_const(m_threads))
    {
        thread->quit();
        thread->wait();
    }
}

} // namespace Digikam

#include "moc_imagequalitythread.cpp"

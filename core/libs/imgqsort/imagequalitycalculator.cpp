/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Calculor
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagequalitycalculator.h"

// Qt includes

#include <QRecursiveMutex>

namespace Digikam
{

class Q_DECL_HIDDEN ImageQualityCalculator::Private
{
public:

    Private() = default;

public:

    float                  threshold_punish = 0.9F;
    float                  weight_punish    = 20.0F;

    QRecursiveMutex        mutex;

    QList<ResultDetection> detectionResults;
};

ImageQualityCalculator::ImageQualityCalculator()
    : d(new Private)
{
    d->detectionResults = QList<ResultDetection>();
}

ImageQualityCalculator::~ImageQualityCalculator()
{
    delete d;
}

void ImageQualityCalculator::addDetectionResult(const QString& name,
                                                const float score,
                                                const float weight) const
{
    ResultDetection result;

    result.detetionType = name;
    result.weight       = weight;
    result.score        = score;

    QMutexLocker locker(&d->mutex);

    d->detectionResults.append(result);
}

void ImageQualityCalculator::normalizeWeight() const
{
    float sum = 0.0F;

    for (const auto& result : std::as_const(d->detectionResults))
    {
        sum += result.weight;
    }

    for (auto& result : d->detectionResults)
    {
        if (sum != 0.0F)
        {
            result.weight /= sum;
        }
    }
}

float ImageQualityCalculator::calculateQuality() const
{
    QMutexLocker locker(&d->mutex);

    if (!numberDetectors())
    {
        return (-1.0F);
    }

    adjustWeightByQualityLevel();

    normalizeWeight();

    float damage = 0.0F;

    for (const auto& result : std::as_const(d->detectionResults))
    {
        damage += result.score * result.weight;
    }

    return ((1.0F - damage) * 100.0F);
}

int ImageQualityCalculator::numberDetectors() const
{
    return d->detectionResults.count();
}

void ImageQualityCalculator::adjustWeightByQualityLevel() const
{
    for (auto& result : d->detectionResults)
    {
        if (result.score > d->threshold_punish)
        {
            result.weight *= d->weight_punish;
        }
    }
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Integrated, multithread object detection / recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autotagsclassifiersoftmax.h"

// Qt includes

#include <QList>

// local includes

#include "digikam_debug.h"

namespace Digikam
{

AutotagsClassifierSoftmax::AutotagsClassifierSoftmax(float _threshold, const QString& predefinedClassFile)
    : AutotagsClassifierBase(_threshold, predefinedClassFile)
{
}

int AutotagsClassifierSoftmax::predict(const cv::Mat& target, const QList<int>& exclusionLabelList) const
{
    Q_UNUSED(exclusionLabelList);

    Q_ASSERT(target.rows == 1);

    float* input = reinterpret_cast<float*>(target.data);
    float  m     = -INFINITY;

    for (int i = 0 ; i < target.size[1] ; i++)
    {
        if (input[i] > m)
        {
            m = input[i];
        }
    }

    float sum    = 0.0F;

    for (int i = 0 ; i < target.size[1] ; i++)
    {
        sum += expf(input[i] - m);
    }

    float offset = m;

    if (sum > 0.0F)
    {
        offset += logf(sum);
    }

    for (int i = 0 ; i < target.size[1] ; i++)
    {
        input[i] = expf(input[i] - offset);
    }

    double final_prob = 0.0;
    cv::Point classIdPoint;
    cv::minMaxLoc(target.reshape(1, 1), 0, &final_prob, 0, &classIdPoint);
    int label_id      = classIdPoint.x;

    if (final_prob > threshold)
    {
        qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "AutotagsClassifierSoftmax::predict - Class ID:"
                                            << label_id << predefinedClasses[label_id]
                                            << "Confidence:" << final_prob;
        return label_id;
    }
    else
    {
        qCDebug(DIGIKAM_AUTOTAGSENGINE_LOG) << "AutotagsClassifierSoftmax::predict - Class ID:"
                                            << label_id << predefinedClasses[label_id]
                                            << "Confidence:" << final_prob << "below threshold";
    }

    return -1;
}

int AutotagsClassifierSoftmax::predict(const cv::UMat& target, const QList<int>& exclusionLabelList) const
{
    return predict(target.getMat(cv::ACCESS_READ), exclusionLabelList);
}

QList<int> AutotagsClassifierSoftmax::predictMulti(const QList<cv::Mat>& targets) const
{
    QList<int> result;

    int label = predict(targets[0]);

    if (-1 != label)
    {
        result << label;
    }

    return result;
}

QList<int> AutotagsClassifierSoftmax::predictMulti(const QList<cv::UMat>& targets) const
{
    QList<int> result;

    int label = predict(targets[0]);

    if (-1 != label)
    {
        result << label;
    }

    return result;
}

} // namespace Digikam

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

#pragma once

// Qt includes

#include <QSize>

// Local includes

#include "autotagsclassifierbase.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AutotagsClassifierYolo : public AutotagsClassifierBase
{
    Q_OBJECT

public:

    enum YoloVersion
    {
        YOLOv5,
        YOLOv6,
        YOLOv7,
        YOLOv8,
        YOLOv9,
        YOLOv10,
        YOLOv11,
    };

public:

    explicit AutotagsClassifierYolo(float _threshold, const QString& predefinedClassFile);
    virtual ~AutotagsClassifierYolo()                                               override = default;

    virtual int predict(const cv::Mat& target,
                        const QList<int>& exclusionLabelList = QList<int>()) const  override
    {
        Q_UNUSED(target);
        Q_UNUSED(exclusionLabelList);

        return -1;
    }

    virtual int predict(const cv::UMat& target,
                        const QList<int>& exclusionLabelList = QList<int>()) const  override
    {
        Q_UNUSED(target);
        Q_UNUSED(exclusionLabelList);

        return -1;
    }

    virtual QList<int> predictMulti(const QList<cv::Mat>& targets)  const           override;
    virtual QList<int> predictMulti(const QList<cv::UMat>& targets) const           override
    {
        Q_UNUSED(targets);

        return QList<int>();
    }

    void setParams(YoloVersion version, QSize size);

private:

    YoloVersion yoloVersion                                                     = YOLOv7;
    bool        ready                                                           = false;
    QSize       imageSize                                                       = QSize(640, 640);
    float       objThreshold                                                    = 0.35F;
    float       confidenceThreshold                                             = 0.33F;
    float       nmsThreshold                                                    = 0.5F;
    float       scoreThreshold                                                  = 0.2F;
    int         outputNumbox                                                    = 0;
    int         outputNumprob                                                   = 0;

private:

    /// @note disabled
    AutotagsClassifierYolo()                                                                 = delete;
    AutotagsClassifierYolo(const AutotagsClassifierYolo&)                                    = delete;
    explicit AutotagsClassifierYolo(QObject*)                                                = delete;
};

} // Digikam

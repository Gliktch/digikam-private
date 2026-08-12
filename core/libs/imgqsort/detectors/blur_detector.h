/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Blur detection basic factor
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstract_detector.h"

namespace Digikam
{

class BlurDetector : public AbstractDetector
{
    Q_OBJECT

public:

    explicit BlurDetector(const DImg& image);
    ~BlurDetector()                                                   override;

    float detect(const cv::Mat& image)                          const override;

private:

    cv::Mat edgeDetection(const cv::Mat& image)                 const;
    cv::Mat detectDefocusMap(const cv::Mat& edgesMap)           const;
    cv::Mat detectMotionBlurMap(const cv::Mat& edgesMap)        const;
    bool    isMotionBlur(const cv::Mat& frag)                   const;

    bool    haveFocusRegion(const DImg& image)                  const;
    cv::Mat detectBackgroundRegion(const cv::Mat& image)        const;
    cv::Mat getWeightMap(const cv::Mat& image)                  const;

    /// @note disabled
    explicit BlurDetector(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

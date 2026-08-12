/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Compression detection basic factor
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

class CompressionDetector : public AbstractDetector
{
    Q_OBJECT

public:

    explicit CompressionDetector();
    ~CompressionDetector()                                        override;

    float detect(const cv::Mat& image)                      const override;

private:

    template <typename Function>
    cv::Mat checkEdgesBlock(const cv::Mat& gray_image,
                            int blockSize,
                            Function accessEdges)           const;
    cv::Mat detectMonoColorRegion(const cv::Mat& image)     const;

    float normalize(const float number);

    /// @note disabled
    explicit CompressionDetector(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

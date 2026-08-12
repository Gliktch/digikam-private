/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - noise detection basic factor
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

class NoiseDetector : public AbstractDetector
{
    Q_OBJECT

public:

    typedef QList<cv::Mat> Mat3D;

public:

    explicit NoiseDetector();
    ~NoiseDetector()                                                                  override;

    float detect(const cv::Mat& image)                                          const override;

public:

    static const Mat3D filtersHaar;

private:

    Mat3D   decompose_by_filter(const cv::Mat& image, const Mat3D& filters)     const;
    void    calculate_variance_kurtosis(const Mat3D& channels,
                                        cv::Mat& variance,
                                        cv::Mat& kurtosis)                      const;
    float   noise_variance(const cv::Mat& variance, const cv::Mat& kurtosis)    const;
    float   normalize(const float number)                                       const;

    cv::Mat raw_moment(const NoiseDetector::Mat3D& mat, int order)              const;
    cv::Mat pow_mat(const cv::Mat& mat, float ordre)                            const;
    float   mean_mat(const cv::Mat& mat)                                        const;

    /// @note disabled
    explicit NoiseDetector(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

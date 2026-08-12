/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Exposure detection basic factor
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

class ExposureDetector : public AbstractDetector
{
    Q_OBJECT

public:

    explicit ExposureDetector();
    ~ExposureDetector()                                       override;

    float detect(const cv::Mat& image)                  const override;

private:

    float percent_underexposed(const cv::Mat& image)    const;
    float percent_overexposed(const cv::Mat& image)     const;

    int count_by_condition(const cv::Mat& image,
                           int minVal, int maxVal)      const;

    /// @note disabled
    explicit ExposureDetector(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

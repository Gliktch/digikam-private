/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - Aesthetic detection based on deep learning
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMutex>

// Local includes

#include "abstract_detector.h"

namespace Digikam
{

class AestheticDetector : public AbstractDetector
{
    Q_OBJECT

public:

    explicit AestheticDetector();
    ~AestheticDetector()                                              override = default;

    float detect(const cv::Mat& image)                          const override;

private:

    cv::Mat preprocess(const cv::Mat& image)                    const;
    float postProcess(const cv::Mat& modelOutput)               const;

    /// @note disabled
    explicit AestheticDetector(QObject*);

public:

    static bool s_loadModel();
    static void s_unloadModel();
    static bool s_isEmptyModel();

private:

    static cv::dnn::Net s_model;
    static QMutex       s_modelMutex;
};

} // namespace Digikam

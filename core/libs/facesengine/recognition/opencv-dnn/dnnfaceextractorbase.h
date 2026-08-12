/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-06-01
 * Description : Face recognition using deep learning.
 *               The internal DNN library interface.
 *
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020      by Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <vector>

// Qt include

#include <QJsonArray>

// Local includes

#include "digikam_opencv.h"
#include "digikam_export.h"
#include "dnnmodelbase.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT DNNFaceExtractorBase
{

public:

    DNNFaceExtractorBase();
    virtual ~DNNFaceExtractorBase();

public:

    /**
     * @brief Read pretrained neural network for face recognition.
     */
    virtual bool loadModels()                                                               = 0;

    /**
     * @brief Cover the UI threshold to a float using the conversion
     * factor built into the model
     */
    virtual float getThreshold(int uiThreshold = DNN_MODEL_THRESHOLD_NOT_SET) const         = 0;

    virtual cv::Mat alignFace(const cv::Mat& inputImage) const                              = 0;
    virtual const QPair<cv::Mat, cv::Mat> getFaceEmbedding(const cv::Mat& faceImage) const  = 0;

    /**
     * @brief UMat versions.
     */
    virtual cv::UMat alignFace(const cv::UMat& inputImage) const                            = 0;
    virtual const QPair<cv::Mat, cv::Mat> getFaceEmbedding(const cv::UMat& faceImage) const = 0;

    /**
     * @brief Calculate different between 2 vectors.
     */
    static double cosineDistance(const std::vector<float>& v1, const std::vector<float>& v2);
    static double L2squareDistance(const std::vector<float>& v1, const std::vector<float>& v2);
    static double L2squareNormDistance(const std::vector<float>& v1, const std::vector<float>& v2);

    /**
     * @brief Convert face embedding between different formats.
     */
    static cv::Mat              vectortomat(const std::vector<float>& vector);
    static QJsonArray           encodeVector(const std::vector<float>& vector);
    static std::vector<float>   decodeVector(const QJsonArray& json);

private:

    //// @note disabled
    DNNFaceExtractorBase(const DNNFaceExtractorBase&)                               = delete;
    DNNFaceExtractorBase& operator=(const DNNFaceExtractorBase&)                    = delete;
};

} // namespace Digikam

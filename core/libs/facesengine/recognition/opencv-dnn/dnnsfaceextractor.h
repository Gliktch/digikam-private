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

// Local includes

#include "dnnfaceextractorbase.h"
#include "dnnmodelyunet.h"

namespace Digikam
{

// TODO: remove export macro after testing

class DIGIKAM_GUI_EXPORT DNNSFaceExtractor: public DNNFaceExtractorBase
{

public:

    DNNSFaceExtractor();
    ~DNNSFaceExtractor()                                                                        override;

public:

    /**
     * Read pretrained neural network for face recognition.
     */
    bool loadModels()                                                                           override;

    /**
     * Cover the UI threshold to a float using the conversion
     * factor built into the model.
     */
    float getThreshold(int uiThreshold = DNN_MODEL_THRESHOLD_NOT_SET)                   const   override;

    virtual cv::Mat alignFace(const cv::Mat& inputImage)                                const   override;
    virtual const QPair<cv::Mat, cv::Mat> getFaceEmbedding(const cv::Mat& faceImage)    const   override;

    virtual cv::UMat alignFace(const cv::UMat& inputImage)                              const   override;
    virtual const QPair<cv::Mat, cv::Mat> getFaceEmbedding(const cv::UMat& faceImage)   const   override;

private:

    //// @note disabled
    DNNSFaceExtractor(const DNNSFaceExtractor&)                                 = delete;
    DNNSFaceExtractor& operator=(const DNNSFaceExtractor&)                      = delete;

private:

    class Private;
    Private* const d                                                            = nullptr;
};

} // namespace Digikam

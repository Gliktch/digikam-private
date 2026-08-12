/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Performs face detection and recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "facepipelinebase.h"
#include "dnnmodelbase.h"
#include "dnnfacedetectoryunet.h"
#include "dnnmodelmanager.h"
#include "dnnmodelsface.h"

namespace Digikam
{

class FacePipelineDetectRecognize : public FacePipelineBase
{
    Q_OBJECT

public:

    explicit FacePipelineDetectRecognize(const FaceScanSettings& _settings);
    ~FacePipelineDetectRecognize()                              override;

public:

    bool start()                                                override;

protected:

    bool finder()                                               override;
    bool loader()                                               override;
    bool extractor()                                            override;
    bool classifier()                                           override;

    bool trainer()                                              override
    {
        return false;
    }

    bool writer()                                               override;

    void addMoreWorkers()                                       override;

private:

    DNNFaceDetectorYuNet* faceDetector                          = nullptr;
    DNNModelSFace*        faceExtractor                         = nullptr;

private:

    /// @note disabled
    FacePipelineDetectRecognize()                                               = delete;
    FacePipelineDetectRecognize(QObject* const)                                 = delete;
    FacePipelineDetectRecognize(const FacePipelineDetectRecognize&)             = delete;
    FacePipelineDetectRecognize& operator=(const FacePipelineDetectRecognize&)  = delete;
};

} // namespace Digikam

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
#include "facedetector.h"

namespace Digikam
{

class FacePipelineRetrain : public FacePipelineBase
{
    Q_OBJECT

public:

    explicit FacePipelineRetrain(const FaceScanSettings& _settings);
    ~FacePipelineRetrain()                                      override = default;

    bool start()                                                override;

protected:

    bool finder()                                               override;
    bool loader()                                               override;
    bool extractor()                                            override;
    bool classifier()                                           override
    {
        return false;
    }

    bool trainer()                                              override
    {
        return false;
    }

    bool writer()                                               override;

    void addMoreWorkers()                                       override;

private:

    FaceDetector detector;

private:

    /// @note disabled
    FacePipelineRetrain()                                       = delete;
    FacePipelineRetrain(QObject* const)                         = delete;
    FacePipelineRetrain(const FacePipelineRetrain&)             = delete;
    FacePipelineRetrain& operator=(const FacePipelineRetrain&)  = delete;
};

} // namespace Digikam

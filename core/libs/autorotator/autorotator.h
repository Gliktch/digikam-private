/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-03-10
 * Description : Autorotator class to automatically rotate images
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

 // local includes

#include "digikam_opencv.h"
#include "digikam_export.h"
#include "dimg.h"
#include "dnnmodelnet.h"
#include "metaengine_rotation.h"

namespace Digikam
{

class DIGIKAM_EXPORT AutoRotator
{
public:

    AutoRotator();
    ~AutoRotator()          = default;

    /**
     * @brief rotationAngle
     * @param img               The DImg file must be in the same rotation as what is displayed in the image viewer
     * @return                  The free rotation angle
     */
    float                                    rotationAngle(const DImg& img,
                                                           bool copyDImg = true);

    /**
     * @brief rotationOrientation
     * @param img               The DImg file must be in the same rotation as what is displayed in the image viewer
     * @param sensitivity       The sensitivity of the of the base angle
     * @return                  The rotation action to be taken
     */
    MetaEngineRotation::TransformationAction rotationOrientation(const DImg& img,
                                                                 int sensitivity,
                                                                 bool copyDImg = true);

    /**
     * @brief rotationOrientation
     * @param img               The image file path
     * @param sensitivity       The sensitivity of the of the base angle
     * @return                  The rotation action to be taken
     */
    MetaEngineRotation::TransformationAction rotationOrientation(const QString& img,
                                                                 int sensitivity);

protected:

    DNNModelNet*    model   = nullptr;

private:

    bool            loadModel();
    cv::Mat         Preprocess(const DImg& img);
    bool            shouldRotate(int degrees, int sensitivity, int angle);
    float           privateRotationAngle(const DImg& img);

private:

    const int       baseArc = 50;
};

} // namespace Digikam

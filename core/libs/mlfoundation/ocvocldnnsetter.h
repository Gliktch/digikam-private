/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-25-04
 * Description : Enable/Disable OpenCL in OpenCV when not compatible with DNN
 *
 * SPDX-FileCopyrightText: 2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * @brief This class is used to lock OpenCL in OpenCV when using DNN.
 * It is used to prevent OpenCL from being used when DNN is not compatible with it.
 *
 * It's used much like a QMutexLocker. Simply create an instance of this class
 * and it will set OpenCV to use the correct OpenCL settings for DNN usage. When the
 * instance is destroyed, it will revert the OpenCL settings back to the global value.
 *
 * @note OpenCV will use OpenCL only if using a cv::UMat. If using a cv::Mat, OpenCL
 * will not be used so there is no need to use this class with cv::Mat.
 */

class DIGIKAM_EXPORT OpenCVOpenCLDNNSetter
{
public:

    OpenCVOpenCLDNNSetter();
    ~OpenCVOpenCLDNNSetter();

    void unlockOpenCL();
    void relockOpenCL();

private:

    bool locked = false;

private:

    /// @note disabled
    OpenCVOpenCLDNNSetter(const OpenCVOpenCLDNNSetter&)            = delete;
    OpenCVOpenCLDNNSetter& operator=(const OpenCVOpenCLDNNSetter&) = delete;

private:

    class Private;
    static Private* d;
};

} // namespace Digikam

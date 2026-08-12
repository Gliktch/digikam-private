/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelYuNet specific for create FaceDetectorYN instances
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnmodelyunet.h"

// Qt includes

#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "dnnmodelconfig.h"

namespace Digikam
{

DNNModelYuNet::DNNModelYuNet(const DNNModelInfoContainer& _info)
    : DNNModelBase(_info)
{
}

cv::Ptr<cv::FaceDetectorYN>& DNNModelYuNet::getNet()
{
    QMutexLocker lock(&loaderMutex);

    if (!modelLoaded)
    {
        if (loadModel())
        {
            modelLoaded = true;
        }
    }

    return net;
}

bool DNNModelYuNet::loadModel()
{
    // Load the model from disk and create cv::dnn:Net instance.

    // Verify filename.

    if (checkFilename())
    {
        // Create the cv::dnn::Net instance with the config.

        return callLoader();

    }

    return false;
}

bool DNNModelYuNet::callLoader()
{
    float conf_threshold             = 0.3F;
    float nms_threshold              = 0.3F;
    int top_k                        = 5000;

    QString modelPath                = getModelPath();
    QPair<int, int> backendAndTarget = getBackendAndTarget();

    net = cv::FaceDetectorYN::create(
                                     modelPath.toStdString(),
                                     "",
                                     cv::Size(info.imageSize, info.imageSize),
                                     conf_threshold,
                                     nms_threshold,
                                     top_k,
                                     backendAndTarget.first,
                                     backendAndTarget.second
                                    );

    return true;
}

} // namespace Digikam

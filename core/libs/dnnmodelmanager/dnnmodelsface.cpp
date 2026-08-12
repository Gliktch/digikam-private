/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelSFace specific for create FaceRecognizerSF instances
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnmodelsface.h"

// Qt includes

#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "dnnmodelconfig.h"

namespace Digikam
{

DNNModelSFace::DNNModelSFace(const DNNModelInfoContainer& _info)
    : DNNModelBase(_info)
{
}

cv::Ptr<cv::FaceRecognizerSF>& DNNModelSFace::getNet()
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

bool DNNModelSFace::loadModel()
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

bool DNNModelSFace::callLoader()
{

    QString modelPath = getModelPath();
    QPair<int, int> backendAndTarget = getBackendAndTarget();

    net = cv::FaceRecognizerSF::create(
                                       modelPath.toStdString(),
                                       "",
                                       backendAndTarget.first,
                                       backendAndTarget.second
                                      );

    return true;
}

} // namespace Digikam

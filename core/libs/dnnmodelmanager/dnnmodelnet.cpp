/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelNet for generic OpenCV nets
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnmodelnet.h"

// Qt includes

#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "dnnmodelconfig.h"

namespace Digikam
{

DNNModelNet::DNNModelNet(const DNNModelInfoContainer& _info)
    : DNNModelBase(_info)
{
}

cv::dnn::Net& DNNModelNet::getNet()
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

bool DNNModelNet::loadModel()
{
    // Load the model from disk and create cv::dnn:Net instance.

    QString configPath;

    // Verify filename.

    if (checkFilename())
    {
        // Load config model if needed.

        if (0 < info.configName.size())
        {
            const DNNModelConfig* const configModel = static_cast<DNNModelConfig*>(DNNModelManager::instance()->getModel(info.configName, info.usage[0]));
            configPath                              = configModel->getModelPath();
        }

        // Create the cv::dnn::Net instance with the config.

        return callLoader(configPath);
    }

    return false;
}

bool DNNModelNet::callLoader(const QString& configPath)
{
    QString modelPath                = getModelPath();
    QPair<int, int> backendAndTarget = getBackendAndTarget();

    if (0 < configPath.size())
    {

        net = cv::dnn::readNet(modelPath.toStdString(),
                               configPath.toStdString());
    }
    else
    {
        net = cv::dnn::readNet(modelPath.toStdString());
    }

    net.setPreferableBackend(backendAndTarget.first);
    net.setPreferableTarget(backendAndTarget.second);

    return true;
}

} // namespace Digikam

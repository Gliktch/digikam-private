/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelConfig for generic OpenCV config
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnnmodelconfig.h"

// Qt includes

#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DNNModelConfig::DNNModelConfig(const DNNModelInfoContainer& _info)
    : DNNModelBase(_info)
{
}

bool DNNModelConfig::loadModel()
{
    if (checkFilename())
    {
        // Do nothing. Config don't have a cv::dnn::Net pointer.

        return modelLoaded = true;  // Set modelLoaded and return true.
    }

    return false;
}

} // namespace Digikam

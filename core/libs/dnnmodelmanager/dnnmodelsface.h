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

#pragma once

// Qt includes

#include <QMutex>

// Local includes

#include "digikam_opencv.h"
#include "digikam_export.h"
#include "dnnmodelmanager.h"
#include "dnnmodelbase.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNNModelSFace: public DNNModelBase
{

public:

    // ---------- public methods ----------

    explicit DNNModelSFace(const DNNModelInfoContainer& _info);
    virtual ~DNNModelSFace() override   = default;

public:

    // ---------- public members ----------

    cv::Ptr<cv::FaceRecognizerSF>& getNet();

private:

    cv::Ptr<cv::FaceRecognizerSF> net;

private:

    DNNModelSFace()                     = delete;

    virtual bool loadModel() override;                  ///< Must be overridden in child class.
    bool callLoader();
};

} // namespace Digikam

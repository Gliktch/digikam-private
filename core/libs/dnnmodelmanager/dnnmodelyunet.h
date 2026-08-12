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

class DIGIKAM_EXPORT DNNModelYuNet: public DNNModelBase
{

public:

    // ---------- public methods ----------

    explicit DNNModelYuNet(const DNNModelInfoContainer& _info);
    virtual ~DNNModelYuNet() override   = default;

public:

    // ---------- public members ----------

    cv::Ptr<cv::FaceDetectorYN>& getNet();

private:

    cv::Ptr<cv::FaceDetectorYN> net;

private:

    DNNModelYuNet()                     = delete;

    virtual bool loadModel() override;                  ///< Must be overridden in child class.
    bool callLoader();
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam DNNModelNet base class
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMutex>
#include <QVersionNumber>
#include <QPair>

// Local includes

#include "digikam_opencv.h"
#include "digikam_export.h"
#include "filesdownloader.h"
#include "dnnmodeldefinitions.h"
#include "dnnmodelinfocontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNNModelBase
{

public:

    explicit DNNModelBase(const DNNModelInfoContainer& _info);
    virtual ~DNNModelBase() = default;

    // ---------- public members ----------

    /**
     * @param uiThreshold is the slider value from the UI.
     * @return float threshold to be used by processing (FaceDetector, FaceRecognizer, etc...).
     */
    float                       getThreshold(int uiThreshold = DNN_MODEL_THRESHOLD_NOT_SET)    const;
    DownloadInfo                getDownloadInformation()                                       const;

    /**
     * @return path to the model, or null string if path cannot be found.
     */
    const QString               getModelPath()                                                 const;

public:

    bool                        modelLoaded                 = false;    ///< Indicate if the model has been loaded.
    const DNNModelInfoContainer info;                                   ///< Information about the model.
    QMutex                      mutex;                                  ///< Mutex to single-thread model during critical processing functions.

protected:

    QMutex                      loaderMutex;

    bool                        checkFilename()             const;
    const QPair<int, int>       getBackendAndTarget()       const;

private:

    DNNModelBase()           = delete;
    virtual bool loadModel() = 0;                                       ///< Must be overridden in child class.
};

} // namespace Digikam

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

#pragma once

// Qt includes

#include <QMutex>

// Local includes

#include "digikam_opencv.h"
#include "digikam_export.h"
#include "dnnmodelbase.h"
#include "dnnmodelmanager.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNNModelConfig: public DNNModelBase
{

public:

    // ---------- public methods ----------

    explicit DNNModelConfig(const DNNModelInfoContainer& _info);
    virtual ~DNNModelConfig() override  = default;

public:

    // ---------- public members ----------

private:

    DNNModelConfig()                    = delete;

    virtual bool loadModel() override;                  ///< Must be overridden in child class.
};

} // namespace Digikam

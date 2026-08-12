/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Performs autotags object detection and recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "autotagspipelinebase.h"
#include "dnnmodelbase.h"
#include "dnnmodelmanager.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT AutotagsPipelineObject : public AutotagsPipelineBase
{
    Q_OBJECT

public:

    explicit AutotagsPipelineObject(const AutotagsScanSettings& _settings);
    ~AutotagsPipelineObject()                                   override;

    bool start()                                                override;

protected:

    bool finder()                                               override;
    bool loader()                                               override;
    bool extractor()                                            override;
    bool classifier()                                           override;
    bool trainer()                                              override { return false; }
    bool writer()                                               override;

    void addMoreWorkers()                                       override;

private:

    /// @note disabled
    AutotagsPipelineObject()                                    = delete;
    AutotagsPipelineObject(QObject*)                            = delete;
    AutotagsPipelineObject(const AutotagsPipelineObject&)       = delete;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Performs face detection and recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "facepipelinebase.h"
#include "faceclassifier.h"

namespace Digikam
{

class FacePipelineReset : public FacePipelineBase
{
    Q_OBJECT

public:

    explicit FacePipelineReset(const FaceScanSettings& _settings);
    ~FacePipelineReset()                                        override = default;

public:

    bool start()                                                override;

protected:

    bool finder()                                               override;

    bool loader()                                               override
    {
        return false;
    }

    bool extractor()                                            override
    {
        return false;
    }

    bool classifier()                                           override
    {
        return false;
    }

    bool trainer()                                              override
    {
        return false;
    }

    bool writer()                                               override;

    void addMoreWorkers()                                       override;

private:

    /// @note disabled
    FacePipelineReset()                                         = delete;
    FacePipelineReset(QObject* const)                           = delete;
    FacePipelineReset(const FacePipelineReset&)                 = delete;
    FacePipelineReset& operator=(const FacePipelineReset&)      = delete;
};

} // namespace Digikam

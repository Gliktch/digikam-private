/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Performs object detection and recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QRectF>

// Local includes

#include "digikam_opencv.h"
#include "mlpipelinepackagefoundation.h"
#include "faceutils.h"
#include "dimg.h"
#include "iteminfo.h"

namespace Digikam
{

class AutotagsPipelinePackageBase : public MLPipelinePackageFoundation
{
public:

    enum EditPipelineAction
    {
        Confirm,
        Remove,
        EditTag,
        EditRegion,
        AddManually
    };

public:

    AutotagsPipelinePackageBase()                                               = default;
    explicit AutotagsPipelinePackageBase(qlonglong _imageId);

    virtual ~AutotagsPipelinePackageBase() override                             = default;

public:

    ItemInfo                info;
    QImage                  thumbnail;
    DImg                    image;
    cv::Mat                 features;
    QList<cv::Mat>          featuresList;
    QList<int>              labelList;
    QList<QString>          tagList;

private:

    /// @note disabled
    AutotagsPipelinePackageBase(const AutotagsPipelinePackageBase&)            = delete;
};

} // namespace Digikam

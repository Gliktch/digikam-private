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

// Qt includes

#include <QImage>
#include <QRectF>

// Local includes

#include "digikam_opencv.h"
#include "mlpipelinepackagefoundation.h"
#include "faceutils.h"
#include "iteminfo.h"
#include "dimg.h"


namespace Digikam
{

class FacePipelinePackageBase : public MLPipelinePackageFoundation
{
public:

    enum EditPipelineAction
    {
        Confirm,
        Remove,
        RemoveAll,
        EditTag,
        EditRegion,
        AddManually,
        DeleteRejectedFaceTagList
    };

public:

    FacePipelinePackageBase()                                               = default;
    explicit FacePipelinePackageBase(qlonglong _imageId,
                                     int _serialNumber = -1);

    explicit FacePipelinePackageBase(qlonglong _imageId,
                                     const FaceTagsIface& _face,
                                     int _serialNumber = -1);

    explicit FacePipelinePackageBase(const ItemInfo& _info,
                                     EditPipelineAction _action,
                                     int _serialNumber = -1);

    explicit FacePipelinePackageBase(const ItemInfo& _info,
                                     const FaceTagsIface& _face,
                                     int _tagId,
                                     const TagRegion& _region,
                                     const DImg& _image,
                                     EditPipelineAction _action,
                                     bool _retrain,
                                     int _serialNumber = -1);

    virtual ~FacePipelinePackageBase() override;

public:

    ItemInfo                info;
    FaceTagsIface           face;
    QImage                  thumbnail;
    DImg                    image;
    QList<QRectF>           faceRects;
    cv::Mat                 features;
    int                     label           = -1;
    int                     tagId           = -1;
    TagRegion               region;
    EditPipelineAction      action          = EditPipelineAction::Confirm;
    bool                    useForTraining  = false;
    bool                    retrain         = false;
    QList<cv::Mat>          featuresList;
    QList<int>              labelList;
    QList<FaceTagsIface>    faceList;
    QList<int>              exclusionIdentityIds;
    int                     serialNumber    = -1;

private:

    /// @note disabled
    FacePipelinePackageBase(const FacePipelinePackageBase&)                 = delete;
    FacePipelinePackageBase& operator=(const FacePipelinePackageBase&)      = delete;
};

} // namespace Digikam

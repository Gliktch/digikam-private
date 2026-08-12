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

#include "facepipelinepackagebase.h"

// digiKam includes

#include "digikam_debug.h"

namespace Digikam
{

FacePipelinePackageBase::FacePipelinePackageBase(qlonglong _imageId,
                                                 int _serialNumber)
    : info        (_imageId),
      features    (cv::Mat()),
      serialNumber(_serialNumber)
{
}

FacePipelinePackageBase::FacePipelinePackageBase(qlonglong _imageId,
                                                 const FaceTagsIface& _face,
                                                 int _serialNumber)
    : info        (_imageId),
      face        (_face),
      features    (cv::Mat()),
      serialNumber(_serialNumber)
{
}

FacePipelinePackageBase::FacePipelinePackageBase(const ItemInfo& _info,
                                                 EditPipelineAction _action,
                                                 int _serialNumber)
    : info        (_info),
      features    (cv::Mat()),
      action      (_action),
      serialNumber(_serialNumber)
{
}

FacePipelinePackageBase::FacePipelinePackageBase(const ItemInfo& _info,
                                                 const FaceTagsIface& _face,
                                                 int _tagId,
                                                 const TagRegion& _region,
                                                 const DImg& _image,
                                                 EditPipelineAction _action,
                                                 bool _retrain,
                                                 int _serialNumber)
    : info        (_info),
      face        (_face),
      image       (_image),
      features    (cv::Mat()),
      tagId       (_tagId),
      region      (_region),
      action      (_action),
      retrain     (_retrain),
      serialNumber(_serialNumber)
{
}

FacePipelinePackageBase::~FacePipelinePackageBase()
{
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "FacePipelinePackageBase::~FacePipelinePackageBase: "
                                        "Deleting package with serial number"
                                     << serialNumber;
}

} // namespace Digikam

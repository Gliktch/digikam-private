/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-05-02
 * Description : AI tools pipeline package class
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "aitoolspipelinepackage.h"

// Qt includes

#include <QFileInfo>

namespace Digikam
{

class Q_DECL_HIDDEN AIToolsPipelinePackage::Private
{
public:

    explicit Private()
    {
    }
};

AIToolsPipelinePackage::AIToolsPipelinePackage(AITool aiTool, const ItemInfo& info, ProgressItem* const progress)
    : MLPipelinePackageFoundation(),
      info                       (info),
      rotationTransformation     (MetaEngineRotation::NoTransformation),
      progress                   (progress),
      tool                       (aiTool),
      d                          (new Private)
{
}

AIToolsPipelinePackage::AIToolsPipelinePackage(AITool aiTool, const DImg& img, ProgressItem* const progress)
    : MLPipelinePackageFoundation(),
      rotationTransformation     (MetaEngineRotation::NoTransformation),
      image                      (img),
      progress                   (progress),
      tool                       (aiTool),
      d                          (new Private)
{
}

AIToolsPipelinePackage::~AIToolsPipelinePackage()
{
    delete d;
}

} // namespace Digikam

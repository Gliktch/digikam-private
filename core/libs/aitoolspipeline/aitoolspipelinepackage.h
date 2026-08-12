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

#pragma once

// Qt includes

#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"
#include "mlpipelinepackagefoundation.h"
#include "dimg.h"
#include "progressmanager.h"
#include "iteminfo.h"
#include "metaengine_rotation.h"

namespace Digikam
{

/**
 * @class AIToolsPipelinePackage
 * @brief A package class for the AIToolsPipeline, containing image data and processing information
 */
class DIGIKAM_GUI_EXPORT AIToolsPipelinePackage : public MLPipelinePackageFoundation
{
public:

    /**
     * @brief Enum defining the AI tools available in the pipeline
     */
    enum AITool
    {
        AutoRotate = 0,    ///< Auto rotation tool
        /**
         * @note add more tools here in the future
         */
        Unknown            ///< Unknown or undefined tool
    };

    /**
     * @brief Constructor with AITool, ItemInfo and optional ProgressItem
     * @param aiTool The AI tool to use for processing
     * @param info Information about the item to process
     * @param progress Optional progress item for tracking progress (nullptr by default)
     */
    explicit AIToolsPipelinePackage(AITool aiTool, const ItemInfo& info, ProgressItem* const progress = nullptr);

    /**
     * @brief Constructor with AITool, DImg and optional ProgressItem
     * @param aiTool The AI tool to use for processing
     * @param img The image to process
     * @param progress Optional progress item for tracking progress (nullptr by default)
     */
    explicit AIToolsPipelinePackage(AITool aiTool, const DImg& img, ProgressItem* const progress = nullptr);

    /**
     * @brief Destructor
     */
    ~AIToolsPipelinePackage() override;

    /**
     * @brief Public properties
     */
    ItemInfo                                    info;
    MetaEngineRotation::TransformationAction    rotationTransformation;
    DImg                                        image;
    ProgressItem*                               progress;
    AITool                                      tool;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

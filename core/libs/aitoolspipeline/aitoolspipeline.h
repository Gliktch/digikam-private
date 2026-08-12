/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-05-02
 * Description : AI tools pipeline class
 *
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "mlpipelinefoundation.h"
#include "iteminfo.h"
#include "metaengine_rotation.h"
#include "aitoolspipelinepackage.h"

namespace Digikam
{

/**
 * @class AIToolsPipeline
 * @brief A class to handle AI tools processing pipeline
 */

class DIGIKAM_GUI_EXPORT AIToolsPipeline : public MLPipelineFoundation
{
    Q_OBJECT

public:

    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit AIToolsPipeline(QObject* const parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AIToolsPipeline()      override;

    /**
     * @brief Returns the global AIToolsPipeline instance
     * @return Static instance of AIToolsPipeline
     */
    static AIToolsPipeline* instance();

    /**
     * @brief Start the pipeline processing
     * @return True if the pipeline was started successfully
     */
    bool start()            override;

    /**
     * @brief Auto-rotate an image using AI-based rotation detection
     * @param info The item information for the image to process
     * @param progress Optional progress item for tracking progress
     * @return True if the item was successfully submitted for processing
     */
    bool autoRotate(const ItemInfo& info, ProgressItem* const progress = nullptr);

protected:

    /**
     * @brief Process an item with the AI tools pipeline
     * @param info The item information to process
     * @param tool The AI tool to use for processing
     * @param progress Optional progress item for tracking progress
     * @return True if the item was successfully submitted for processing
     */
    bool process(const ItemInfo& info,
                 AIToolsPipelinePackage::AITool tool,
                 ProgressItem* const progress = nullptr);

    /**
     * @brief Process a pre-loaded image with the AI tools pipeline
     * @param image The pre-loaded image to process
     * @param tool The AI tool to use for processing
     * @param progress Optional progress item for tracking progress
     * @return True if the image was successfully submitted for processing
     */
    bool process(const DImg& image,
                 AIToolsPipelinePackage::AITool tool,
                 ProgressItem* const progress = nullptr);

    /**
     * @brief Implementation of the finder stage
     * @return True if the stage completed successfully
     */
    bool finder()           override { return true; }

    /**
     * @brief Implementation of the loader stage
     * @return True if the stage completed successfully
     */
    bool loader()           override;

    /**
     * @brief Implementation of the extractor stage
     * @return True if the stage completed successfully
     */
    bool extractor()        override { return true; }

    /**
     * @brief Implementation of the classifier stage
     * @return True if the stage completed successfully
     */
    bool classifier()       override;

    /**
     * @brief Implementation of the trainer stage
     * @return True if the stage completed successfully
     */
    bool trainer()          override { return true; }

    /**
     * @brief Implementation of the writer stage
     * @return True if the stage completed successfully
     */
    bool writer()           override;

    /**
     * @brief Implementation of the addMoreWorkers method
     */
    void addMoreWorkers()   override;

    /**
     * @brief Check if there's an existing progress item for AIToolPipeline or create a new one
     * @return The existing or newly created ProgressItem
     */
    ProgressItem* getProgressItem();

private Q_SLOTS:

    /**
     * @brief Handle scheduled signal from MLPipelineFoundation
     */
    void slotScheduled();

    /**
     * @brief Handle started signal from MLPipelineFoundation
     * @param message Start message
     */
    void slotStarted(const QString& message);

    /**
     * @brief Handle processed signal from MLPipelineFoundation
     * @param package Package that was processed
     */
    void slotProcessed(const MLPipelinePackageNotify::Ptr& package);

    /**
     * @brief Handle skipped signal from MLPipelineFoundation
     * @param package Package that was skipped
     */
    void slotSkipped(const MLPipelinePackageNotify::Ptr& package);

    /**
     * @brief Handle finished signal from MLPipelineFoundation
     */
    void slotFinished();

    /**
     * @brief Cancels all batch processing and clears Loader and Classifier queues
     */
    void slotBatchCancel();

Q_SIGNALS:

    /**
     * @brief Signal emitted when processing has started
     * @param filePath Path of the image being processed
     */
    void signalProcessingStarted(const QString& filePath);

    /**
     * @brief Signal emitted when processing has finished
     * @param filePath Path of the image that was processed
     * @param success Whether the processing was successful
     */
    void signalProcessingFinished(const QString& filePath, bool success);

    /**
     * @brief Signal emitted to transform an image
     * @param infos List of ItemInfos to transform
     * @param action Transformation action to apply
     */
    void signalTransform(const QList<ItemInfo>& infos,
                         MetaEngineRotation::TransformationAction action);

private:

    class Private;
    Private* const d = nullptr;

    friend class AIToolsPipelineCreator;
};

} // namespace Digikam

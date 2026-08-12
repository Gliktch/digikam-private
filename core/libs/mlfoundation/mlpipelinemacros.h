/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-01-03
 * Description : Macros for simplifying the machine learning pipeline code
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

/**
 * @brief: these macros are used to simplify coding of ML pipeline stages. Whenever possible, use these macros to
 * ensure that the pipeline stages are consistent and easy to read.
 */

#define MLPIPELINE_FINDER_START(threadPriority, nextStage)                                              \
            MLPipelineQueue *thisQueue = nullptr, *nextQueue = nullptr;                                 \
            stageStart(threadPriority, MLPipelineStage::Finder, nextStage, thisQueue, nextQueue);       \
            bool moreCpu = false;                                                                       \
            QElapsedTimer timer;                                                                        \
            pipelinePerformanceStart(MLPipelineStage::Finder, timer);

#define MLPIPELINE_FINDER_END(nextStage)                                            \
            emitSignalUpdateItemCount(totalItemCount);                              \
            pipelinePerformanceEnd(MLPipelineStage::Finder, totalItemCount, timer); \
            stageEnd(MLPipelineStage::Finder, nextStage);                           \
            return true;

#define MLPIPELINE_STAGE_START(threadPriority, thisStage, nextStage)                \
            MLPipelineQueue *thisQueue = nullptr, *nextQueue = nullptr;             \
            stageStart(threadPriority, thisStage, nextStage, thisQueue, nextQueue); \
            QElapsedTimer timer;

#define MLPIPELINE_STAGE_END(thisStage, nextStage) \
            stageEnd(thisStage, nextStage);        \
            return true;

#define MLPIPELINE_LOOP_START(thisStage, thisQueue)                              \
            while (!cancelled)                                                   \
            {                                                                    \
                package = nullptr;                                               \
                try                                                              \
                {                                                                \
                    MLPipelinePackageFoundation* mlpackage = dequeue(thisQueue); \
                    if (queueEndSignal() == mlpackage) { break; }                \
                    pipelinePerformanceStart(thisStage, timer);

#define MLPIPELINE_LOOP_END(thisStage, pipelineStageName)                                                                                                                \
                pipelinePerformanceEnd(thisStage, timer);                                                                                                                \
            }                                                                                                                                                            \
            catch(const std::exception& e)                                                                                                                               \
            {                                                                                                                                                            \
                qCCritical(DIGIKAM_MLPIPELINEFOUNDATION_LOG) << pipelineStageName << e.what() << " Restarting...";                                                       \
                notify(MLPipelineNotification::notifySkipped, i18n("Error"), QLatin1String(e.what()), QString(), 0, QIcon::fromTheme(QStringLiteral("error")));          \
                if (package)                                                                                                                                             \
                {                                                                                                                                                        \
                     delete package;                                                                                                                                     \
                     package = nullptr;                                                                                                                                  \
                }                                                                                                                                                        \
            }                                                                                                                                                            \
            catch(...)                                                                                                                                                   \
            {                                                                                                                                                            \
                qCCritical(DIGIKAM_MLPIPELINEFOUNDATION_LOG) << pipelineStageName << "  Restarting...";                                                                  \
                notify(MLPipelineNotification::notifySkipped, i18n("Error"), QLatin1String(pipelineStageName), QString(), 0, QIcon::fromTheme(QStringLiteral("error"))); \
                if (package)                                                                                                                                             \
                {                                                                                                                                                        \
                    delete package;                                                                                                                                      \
                    package = nullptr;                                                                                                                                   \
                }                                                                                                                                                        \
            }                                                                                                                                                            \
        }

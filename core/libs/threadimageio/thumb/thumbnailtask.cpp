/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Multithreaded loader for thumbnails
 *
 * SPDX-FileCopyrightText: 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbnailtask.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QVariant>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "dmetadata.h"
#include "iccmanager.h"
#include "jpegutils.h"
#include "metaenginesettings.h"
#include "thumbnailloadthread.h"
#include "thumbnailcreator.h"

namespace Digikam
{

ThumbnailLoadingTask::ThumbnailLoadingTask(LoadSaveThread* const thread, const LoadingDescription& description)
    : SharedLoadingTask(thread,
                        description,
                        LoadSaveThread::AccessModeRead,
                        LoadingTaskStatusLoading)
{
    // Thread must be a ThumbnailLoadThread, crashes otherwise.
    // Not a clean but pragmatic solution.

    const ThumbnailLoadThread* const thumbThread = static_cast<ThumbnailLoadThread*>(thread);
    m_creator                                    = thumbThread->thumbnailCreator();
}

void ThumbnailLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        m_thread->taskHasFinished();

        return;
    }

    if (m_loadingDescription.previewParameters.onlyPregenerate())
    {
        setupCreator();

        switch (m_loadingDescription.previewParameters.type)
        {
            case LoadingDescription::PreviewParameters::Thumbnail:
            {
                m_creator->pregenerate(m_loadingDescription.thumbnailIdentifier());
                break;
            }

            case LoadingDescription::PreviewParameters::DetailThumbnail:
            {
                m_creator->pregenerateDetail(m_loadingDescription.thumbnailIdentifier(),
                                             m_loadingDescription.previewParameters.extraParameter.toRect());
                break;
            }

            default:
            {
                break;
            }
        }

        m_thread->taskHasFinished();

        // do not Q_EMIT any signal

        return;
    }

    LoadingCache* const cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images

        const QImage* const cachedImage = cache->retrieveThumbnail(m_loadingDescription.cacheKey());

        if (cachedImage)
        {
            m_qimage = *cachedImage;
        }

        if (m_qimage.isNull())
        {
            // find possible running loading process
            // do not wait on other loading processes?

            LoadingProcess* const usedProcess = cache->retrieveLoadingProcess(m_loadingDescription.cacheKey());

            if (usedProcess)
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.

                usedProcess->addListener(this);

                // break loop when either the loading has completed, or this task is being stopped

                // cppcheck-suppress knownConditionTrueFalse
                while ((m_loadingTaskStatus != LoadingTaskStatusStopping) && !usedProcess->completed())
                {
                    lock.timedWait();
                }

                // remove listener from process

                usedProcess->removeListener(this);

                // wake up the process which is waiting until all listeners have removed themselves

                lock.wakeAll();
            }
        }
    }

    if (continueQuery() && m_qimage.isNull())
    {
        {
            LoadingCache::CacheLock lock(cache);

            // Neither in cache, nor currently loading in different thread.
            // Load it here and now, add this LoadingProcess to cache list.

            cache->addLoadingProcess(this);

            // Notify other processes that we are now loading this image.
            // They might be interested - see notifyNewLoadingProcess below

            cache->notifyNewLoadingProcess(this, m_loadingDescription);
        }

        // Load or create thumbnail

        setupCreator();

        switch (m_loadingDescription.previewParameters.type)
        {
            case LoadingDescription::PreviewParameters::Thumbnail:
            {
                m_qimage = m_creator->load(m_loadingDescription.thumbnailIdentifier(),
                                           m_loadingDescription.previewParameters.onlyFromStorage());
                break;
            }

            case LoadingDescription::PreviewParameters::DetailThumbnail:
            {
                m_qimage = m_creator->loadDetail(m_loadingDescription.thumbnailIdentifier(),
                                                 m_loadingDescription.previewParameters.extraParameter.toRect(),
                                                 m_loadingDescription.previewParameters.onlyFromStorage());
                break;
            }

            default:
            {
                break;
            }
        }

        if (continueQuery() && !m_qimage.isNull())
        {
            postProcess();
        }

        {
            LoadingCache::CacheLock lock(cache);

            // remove this from the list of loading processes in cache

            cache->removeLoadingProcess(this);

            // put valid image into cache of loaded images

            if (continueQuery() && !m_qimage.isNull())
            {
                cache->putThumbnail(m_loadingDescription.cacheKey(), m_qimage,
                                    m_loadingDescription.filePath);

                // dispatch image to all listeners

                for (int i = 0 ; i < m_listeners.count() ; ++i)
                {
                    ThumbnailLoadingTask* const task = dynamic_cast<ThumbnailLoadingTask*>(m_listeners.at(i));

                    if (task)
                    {
                        task->setThumbResult(m_loadingDescription, m_qimage);
                    }
                }
            }

            // indicate that loading has finished so that listeners can stop waiting

            m_completed = true;

            // wake all listeners waiting on cache condVar, so that they remove themselves

            lock.wakeAll();

            // wait until all listeners have removed themselves

            while (m_listeners.count() != 0)
            {
                lock.timedWait();
            }
        }
    }

    if (!continueQuery())
    {
        m_qimage = QImage();
    }

    m_thread->taskHasFinished();
    m_thread->thumbnailLoaded(m_loadingDescription, m_qimage);
}

void ThumbnailLoadingTask::setupCreator()
{
    m_creator->setThumbnailSize(m_loadingDescription.previewParameters.size);
    m_creator->setExifRotate(MetaEngineSettings::instance()->settings().exifRotate);
    m_creator->setLoadingProperties(this, m_loadingDescription.rawDecodingSettings);
}

void ThumbnailLoadingTask::setThumbResult(const LoadingDescription& loadingDescription, const QImage& qimage)
{
    // This is called from another process's execute while this task is waiting on usedProcess.
    // Note that loadingDescription need not equal m_loadingDescription (may be superior)

    LoadingDescription tempDescription       = loadingDescription;

    // these are taken from our own description

    tempDescription.postProcessingParameters = m_loadingDescription.postProcessingParameters;
    m_loadingDescription                     = tempDescription;
    m_qimage                                 = qimage;
}

void ThumbnailLoadingTask::postProcess()
{
    m_loadingDescription.postProcessingParameters.profile().description();

    switch (m_loadingDescription.postProcessingParameters.colorManagement)
    {
        case LoadingDescription::NoColorConversion:
        {
            break;
        }

        case LoadingDescription::ConvertToSRGB:
        {
            // Thumbnails are stored in sRGB

            break;
        }

        case LoadingDescription::ConvertForDisplay:
        {
            IccManager::transformForDisplay(m_qimage, m_loadingDescription.postProcessingParameters.profile());
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported postprocessing parameter for thumbnail loading:"
                                         << m_loadingDescription.postProcessingParameters.colorManagement;
            break;
        }
    }
}

} // namespace Digikam

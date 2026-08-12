/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * SPDX-FileCopyrightText: 2005-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadsavetask.h"

// Local includes

#include "digikam_debug.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "loadsavethread.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"

namespace Digikam
{

LoadSaveTask::LoadSaveTask(LoadSaveThread* const thread)
    : m_thread(thread)
{
}

// --------------------------------------------------------------------------------------------

LoadingTask::LoadingTask(LoadSaveThread* const thread,
                         const LoadingDescription& description,
                         LoadingTaskStatus loadingTaskStatus)
    : LoadSaveTask        (thread),
      m_loadingDescription(description),
      m_loadingTaskStatus (loadingTaskStatus)
{
}

LoadingTask::LoadingTaskStatus LoadingTask::status() const
{
    return m_loadingTaskStatus;
}

QString LoadingTask::filePath() const
{
    return m_loadingDescription.filePath;
}

const LoadingDescription& LoadingTask::loadingDescription() const
{
    return m_loadingDescription;
}

void LoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        m_thread->taskHasFinished();

        return;
    }

    DImg img(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
    m_thread->taskHasFinished();
    m_thread->imageLoaded(m_loadingDescription, img);
}

LoadingTask::TaskType LoadingTask::type()
{
    return TaskTypeLoading;
}

void LoadingTask::progressInfo(float progress)
{
    if (m_loadingTaskStatus == LoadingTaskStatusLoading)
    {
        if (m_thread && m_thread->querySendNotifyEvent())
        {
            m_thread->loadingProgress(m_loadingDescription, progress);
        }
    }
}

bool LoadingTask::continueQuery()
{
    return (m_loadingTaskStatus != LoadingTaskStatusStopping);
}

void LoadingTask::setStatus(LoadingTaskStatus status)
{
    m_loadingTaskStatus = status;
}

//---------------------------------------------------------------------------------------------------

SharedLoadingTask::SharedLoadingTask(LoadSaveThread* const thread, const LoadingDescription& description,
                                     LoadSaveThread::AccessMode mode, LoadingTaskStatus loadingTaskStatus)
    : LoadingTask (thread, description, loadingTaskStatus),
      m_accessMode(mode)
{
    if (m_accessMode == LoadSaveThread::AccessModeRead && needsPostProcessing())
    {
        m_accessMode = LoadSaveThread::AccessModeReadWrite;
    }
}

void SharedLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        m_thread->taskHasFinished();

        return;
    }

    // send StartedLoadingEvent from each single Task, not via LoadingProcess list

    m_thread->imageStartedLoading(m_loadingDescription);

    LoadingCache* const cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images

        DImg* cachedImg        = nullptr;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();

        for (const QString& key : std::as_const(lookupKeys))
        {
            if ((cachedImg = cache->retrieveImage(key)))
            {
                if      (!comparePostProcessing(cachedImg))
                {
                    cachedImg = nullptr;
                }
                else if (m_loadingDescription.needCheckRawDecoding())
                {
                    if (cachedImg->rawDecodingSettings() == m_loadingDescription.rawDecodingSettings)
                    {
                        break;
                    }
                    else
                    {
                        cachedImg = nullptr;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successful

            m_img = *cachedImg;
        }
        else
        {
            // find possible running loading process

            LoadingProcess* usedProcess = nullptr;

            for (QStringList::const_iterator it = lookupKeys.constBegin() ; it != lookupKeys.constEnd() ; ++it)
            {
                if ((usedProcess = cache->retrieveLoadingProcess(*it)))
                {
                    break;
                }
            }

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

                // m_img is now set to the result
            }
        }
    }

    if (continueQuery() && m_img.isNull())
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

        // load image

        m_img = DImg(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);

        if (continueQuery() && !m_img.isNull())
        {
            postProcess();
        }

        {
            LoadingCache::CacheLock lock(cache);

            // remove this from the list of loading processes in cache

            cache->removeLoadingProcess(this);

            // put valid image into cache of loaded images

            if (continueQuery() && !m_img.isNull())
            {
                cache->putImage(m_loadingDescription.cacheKey(), m_img,
                                m_loadingDescription.filePath);

                // dispatch image to all listeners

                for (int i = 0 ; i < m_listeners.count() ; ++i)
                {
                    LoadingProcessListener* const l = m_listeners.at(i);

                    if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
                    {
                        // If a listener requested ReadWrite access, it gets a deep copy.
                        // DImg is explicitly shared.

                        l->setResult(m_loadingDescription, m_img.copy());
                    }
                    else
                    {
                        l->setResult(m_loadingDescription, m_img);
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

    // following the golden rule to avoid deadlocks, do this when CacheLock is not held

    if      (continueQuery() && !m_img.isNull())
    {
        if (accessMode() == LoadSaveThread::AccessModeReadWrite)
        {
            m_img.detach();
        }
    }
    else if (continueQuery())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot load image for" << m_loadingDescription.filePath;
    }
    else
    {
        m_img = DImg();
    }

    m_thread->taskHasFinished();
    m_thread->imageLoaded(m_loadingDescription, m_img);
}

void SharedLoadingTask::setResult(const LoadingDescription& loadingDescription, const DImg& img)
{
    // This is called from another process's execute while this task is waiting on usedProcess.
    // Note that loadingDescription need not equal m_loadingDescription (may be superior)

    LoadingDescription tempDescription       = loadingDescription;

    // these are taken from our own description

    tempDescription.postProcessingParameters = m_loadingDescription.postProcessingParameters;
    m_loadingDescription                     = tempDescription;
    m_img                                    = img;
}

bool SharedLoadingTask::needsPostProcessing() const
{
    return m_loadingDescription.postProcessingParameters.needsProcessing();
}

bool SharedLoadingTask::comparePostProcessing(DImg* const img) const
{
    int colorManagement = 0;

    if (img->hasAttribute(QLatin1String("colorManagement")))
    {
        colorManagement = img->attribute(QLatin1String("colorManagement")).toInt();
    }

    return (colorManagement == (int)m_loadingDescription.postProcessingParameters.colorManagement);
}

void SharedLoadingTask::postProcess()
{
    // ---- Color management ---- //

    switch (m_loadingDescription.postProcessingParameters.colorManagement)
    {
        case LoadingDescription::NoColorConversion:
        {
            break;
        }

        case LoadingDescription::ApplyTransform:
        {
            IccTransform trans = m_loadingDescription.postProcessingParameters.transform();
            trans.apply(m_img);
            m_img.setIccProfile(trans.outputProfile());
            break;
        }

        case LoadingDescription::ConvertForEditor:
        {
            IccManager manager(m_img);
            manager.transformDefault();
            break;
        }

        case LoadingDescription::ConvertToSRGB:
        {
            IccManager manager(m_img);
            manager.transformToSRGB();
            break;
        }

        case LoadingDescription::ConvertForDisplay:
        {
            IccManager manager(m_img);
            manager.transformForDisplay(m_loadingDescription.postProcessingParameters.profile());
            break;
        }

        case LoadingDescription::ConvertForOutput:
        {
            IccManager manager(m_img);
            manager.transformForOutput(m_loadingDescription.postProcessingParameters.profile());
            break;
        }
    }

    m_img.setAttribute(QLatin1String("colorManagement"),
                       (int)m_loadingDescription.postProcessingParameters.colorManagement);
}

void SharedLoadingTask::progressInfo(float progress)
{
    if (m_loadingTaskStatus == LoadingTaskStatusLoading)
    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        for (int i = 0 ; i < m_listeners.size() ; ++i)
        {
            LoadingProcessListener* const l  = m_listeners.at(i);
            LoadSaveNotifier* const notifier = l->loadSaveNotifier();

            if (notifier && l->querySendNotifyEvent())
            {
                notifier->loadingProgress(m_loadingDescription, progress);
            }
        }
    }
}

bool SharedLoadingTask::completed() const
{
    return m_completed;
}

QString SharedLoadingTask::cacheKey() const
{
    return m_loadingDescription.cacheKey();
}

void SharedLoadingTask::addListener(LoadingProcessListener* const listener)
{
    m_listeners << listener;
}

void SharedLoadingTask::removeListener(LoadingProcessListener* const listener)
{
    m_listeners.removeOne(listener);
}

void SharedLoadingTask::notifyNewLoadingProcess(LoadingProcess* const process,
                                                const LoadingDescription& description)
{
    // Ok, we are notified that another task has been started in another thread.
    // We are of course only interested if the task loads the same file,
    // and we are right now loading a reduced version, and the other task is loading the full version.
    // In this case, we notify our own thread (a signal to the API user is emitted) of this.
    // The fact that we are receiving the method call shows that this task is registered with the LoadingCache,
    // somewhere in between the calls to addLoadingProcess(this) and removeLoadingProcess(this) above.

    if ((process != static_cast<LoadingProcess*>(this))              &&
        m_loadingDescription.isReducedVersion()                      &&
        m_loadingDescription.equalsIgnoreReducedVersion(description) &&
        !description.isReducedVersion())
    {
        for (int i = 0 ; i < m_listeners.size() ; ++i)
        {
            if (m_listeners.at(i)->loadSaveNotifier())
            {
                m_listeners.at(i)->loadSaveNotifier()->
                    moreCompleteLoadingAvailable(m_loadingDescription, description);
            }
        }
    }
}

bool SharedLoadingTask::querySendNotifyEvent() const
{
    return (m_thread && m_thread->querySendNotifyEvent());
}

LoadSaveNotifier* SharedLoadingTask::loadSaveNotifier() const
{
    return m_thread;
}

LoadSaveThread::AccessMode SharedLoadingTask::accessMode() const
{
    return m_accessMode;
}

DImg SharedLoadingTask::img() const
{
    return m_img;
}

//---------------------------------------------------------------------------------------------------

SavingTask::SavingTask(LoadSaveThread* const thread,
                       const DImg& img,
                       const QString& filePath,
                       const QString& format)
    : LoadSaveTask(thread),
        m_filePath(filePath),
        m_format  (format),
        m_img     (img)
{
}

SavingTask::SavingTaskStatus SavingTask::status() const
{
    return m_savingTaskStatus;
}

QString SavingTask::filePath() const
{
    return m_filePath;
}

void SavingTask::execute()
{
    m_thread->imageStartedSaving(m_filePath);
    bool success = m_img.save(m_filePath, m_format, this);
    m_thread->taskHasFinished();
    m_thread->imageSaved(m_filePath, success);
}

LoadingTask::TaskType SavingTask::type()
{
    return TaskTypeSaving;
}

void SavingTask::progressInfo(float progress)
{
    if (m_thread->querySendNotifyEvent())
    {
        m_thread->savingProgress(m_filePath, progress);
    }
}

bool SavingTask::continueQuery()
{
    return (m_savingTaskStatus != SavingTaskStatusStopping);
}

void SavingTask::setStatus(SavingTaskStatus status)
{
    m_savingTaskStatus = status;
}

}   //namespace Digikam

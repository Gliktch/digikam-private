/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * SPDX-FileCopyrightText: 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadsavethread.h"

// Local includes

#include "metaengine_rotation.h"
#include "dmetadata.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"
#include "loadsavetask.h"

namespace Digikam
{

class Q_DECL_HIDDEN LoadSaveThread::Private
{
public:

    Private() = default;

public:

    bool                             running            = true;
    bool                             blockNotification  = false;

    QTime                            notificationTime;

    LoadSaveTask*                    lastTask           = nullptr;

    static LoadSaveFileInfoProvider* infoProvider;
};

LoadSaveFileInfoProvider* LoadSaveThread::Private::infoProvider = nullptr;

//---------------------------------------------------------------------------------------------------

LoadSaveThread::LoadSaveThread(QObject* const parent)
    : DynamicThread(parent),
      d            (new Private)
{
}

LoadSaveThread::~LoadSaveThread()
{
    shutDown();
    delete d;
}

void LoadSaveThread::setInfoProvider(LoadSaveFileInfoProvider* const infoProvider)
{
    Private::infoProvider = infoProvider;
}

LoadSaveFileInfoProvider* LoadSaveThread::infoProvider()
{
    return Private::infoProvider;
}

void LoadSaveThread::load(const LoadingDescription& description)
{
    QMutexLocker lock(threadMutex());
    m_todo << new LoadingTask(this, description);
    start(lock);
}

void LoadSaveThread::save(const DImg& image, const QString& filePath, const QString& format)
{
    QMutexLocker lock(threadMutex());
    m_todo << new SavingTask(this, image, filePath, format);
    start(lock);
}

void LoadSaveThread::run()
{
    while (runningFlag())
    {
        {
            QMutexLocker lock(threadMutex());

            delete d->lastTask;
            d->lastTask   = nullptr;
            delete m_currentTask;
            m_currentTask = nullptr;

            if (!m_todo.isEmpty())
            {
                m_currentTask = m_todo.takeFirst();

                if (m_notificationPolicy == NotificationPolicyTimeLimited)
                {
                    // set timing values so that first event is sent only
                    // after an initial time span.

                    d->notificationTime  = QTime::currentTime();
                    d->blockNotification = true;
                }
            }
            else
            {
                stop(lock);
            }
        }

        if (m_currentTask)
        {
            m_currentTask->execute();
        }
    }
}

void LoadSaveThread::taskHasFinished()
{
    // This function is called by the tasks _before_ they send their _final_ message.
    // This is to guarantee the user of the API that at least the final message
    // is sent after load() has been called.
    // We set m_currentTask to 0 here. If a new task is appended, base classes usually check
    // that m_currentTask is not currently loading the same task.
    // Now it might happen that m_currentTask has already emitted its final signal,
    // but the new task is rejected afterwards when m_currentTask is still the task
    // that has actually already finished (execute() in the loop above is of course not under mutex).
    // So we set m_currentTask to 0 immediately before the final message is emitted,
    // so that anyone who finds this task running as m_current task will get a message.

    QMutexLocker lock(threadMutex());
    d->lastTask   = m_currentTask;
    m_currentTask = nullptr;
}

void LoadSaveThread::imageStartedLoading(const LoadingDescription& loadingDescription)
{
    notificationReceived();

    Q_EMIT signalImageStartedLoading(loadingDescription);
}

void LoadSaveThread::loadingProgress(const LoadingDescription& loadingDescription, float progress)
{
    notificationReceived();

    Q_EMIT signalLoadingProgress(loadingDescription, progress);
}

void LoadSaveThread::imageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    notificationReceived();

    Q_EMIT signalImageLoaded(loadingDescription, img);
}

void LoadSaveThread::moreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                                  const LoadingDescription& newLoadingDescription)
{
    notificationReceived();

    Q_EMIT signalMoreCompleteLoadingAvailable(oldLoadingDescription, newLoadingDescription);
}

void LoadSaveThread::imageStartedSaving(const QString& filePath)
{
    notificationReceived();

    Q_EMIT signalImageStartedSaving(filePath);
}

void LoadSaveThread::savingProgress(const QString& filePath, float progress)
{
    notificationReceived();

    Q_EMIT signalSavingProgress(filePath, progress);
}

void LoadSaveThread::imageSaved(const QString& filePath, bool success)
{
    notificationReceived();

    Q_EMIT signalImageSaved(filePath, success);
}

void LoadSaveThread::thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img)
{
    notificationReceived();

    Q_EMIT signalQImageThumbnailLoaded(loadingDescription, img);
}

void LoadSaveThread::notificationReceived()
{
    switch (m_notificationPolicy)
    {
        case NotificationPolicyDirect:
        {
            d->blockNotification = false;
            break;
        }

        case NotificationPolicyTimeLimited:
        {
            break;
        }
    }
}

void LoadSaveThread::setNotificationPolicy(NotificationPolicy notificationPolicy)
{
    m_notificationPolicy = notificationPolicy;
    d->blockNotification = false;
}

bool LoadSaveThread::querySendNotifyEvent() const
{
    // This function is called from the thread to ask for permission to send a notify event.

    switch (m_notificationPolicy)
    {
        case NotificationPolicyDirect:
        {
            // Note that m_blockNotification is not protected by a mutex. However, if there is a
            // race condition, the worst case is that one event is not sent, which is no problem.

            if (d->blockNotification)
            {
                return false;
            }
            else
            {
                d->blockNotification = true;

                return true;
            }

            break;
        }

        case NotificationPolicyTimeLimited:
        {
            // Current default time value: 100 millisecs.

            if (d->blockNotification)
            {
                d->blockNotification = d->notificationTime.msecsTo(QTime::currentTime()) < 100;
            }

            if (d->blockNotification)
            {
                return false;
            }
            else
            {
                d->notificationTime  = QTime::currentTime();
                d->blockNotification = true;

                return true;
            }

            break;
        }
    }

    return false;
}

int LoadSaveThread::exifOrientation(const QString& filePath,
                                    const DMetadata& metadata,
                                    bool isRaw,
                                    bool fromRawEmbeddedPreview)
{
    int dbOrientation = MetaEngine::ORIENTATION_UNSPECIFIED;

    if (infoProvider())
    {
        dbOrientation = infoProvider()->orientationHint(filePath);
    }

    int exifOrientation = metadata.getItemOrientation();

    // Raw files are already rotated properly by Raw engine. Only perform auto-rotation with JPEG/PNG/TIFF file.
    // We don't have a feedback from Raw engine about auto-rotated RAW file during decoding.

    if (isRaw && !fromRawEmbeddedPreview)
    {
        // Did the user apply any additional rotation over the metadata flag?

        if ((dbOrientation == MetaEngine::ORIENTATION_UNSPECIFIED) || (dbOrientation == exifOrientation))
        {
            return MetaEngine::ORIENTATION_NORMAL;
        }

        // Assume A is the orientation as from metadata, B is an additional operation applied by the user,
        // C is the current orientation in the database.
        // A*B = C and B = A_inv * C

        QTransform A     = MetaEngineRotation::toTransform((MetaEngine::ImageOrientation)exifOrientation);
        QTransform C     = MetaEngineRotation::toTransform((MetaEngine::ImageOrientation)dbOrientation);
        QTransform A_inv = A.inverted();
        QTransform B     = A_inv * C;
        MetaEngineRotation m(B.m11(), B.m12(), B.m21(), B.m22());

        return m.exifOrientation();
    }

    if (dbOrientation != MetaEngine::ORIENTATION_UNSPECIFIED)
    {
        return dbOrientation;
    }

    return exifOrientation;
}

} // namespace Digikam

#include "moc_loadsavethread.cpp"

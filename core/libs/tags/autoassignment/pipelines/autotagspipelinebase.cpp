/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Integrated, multithread object detection / recognition
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autotagspipelinebase.h"

// Qt includes

#include <QList>

// Local includes

#include "digikam_debug.h"
#include "digikam_opencv.h"
#include "sharedqueue.h"
#include "autotagspipelinepackagebase.h"
#include "thumbnailloadthread.h"
#include "coredb.h"

namespace Digikam
{

AutotagsPipelineBase::AutotagsPipelineBase(const AutotagsScanSettings& _settings)
    : MLPipelineFoundation(),
      settings            (_settings),
      albumRoots          (CoreDbAccess().db()->getAlbumRoots())
{
}

bool AutotagsPipelineBase::start()
{
    if (settings.bqmMode)
    {
        bqmSemaphore.release();
    }

    return MLPipelineFoundation::start();
}

void AutotagsPipelineBase::bqmSendOne(std::unique_ptr<DMetadata>& _bqmMeta,
                                      const ItemInfo& info,
                                      const QUrl& outputUrl,
                                      const DImg& image)
{
    if (settings.bqmMode)
    {
        bqmOutputUrl = outputUrl;
        bqmMeta.reset(_bqmMeta.release());

        // create a package for the image ID
        // the package should look like it just came from the finder stage

        AutotagsPipelinePackageBase* const package = new AutotagsPipelinePackageBase();
        package->info                              = info;
        package->image                             = image;
        bqmOutputUrl                               = outputUrl;

        // acquire the semaphore

        bqmSemaphore.acquire();

        if (package->image.isNull())
        {
            // if the image is null, send to Loader stage

            // wait for the queue to be ready

            while (nullptr == queues[MLPipelineStage::Loader])
            {
                QThread::msleep(10);
            }

            // send the package to the finder queue

            enqueue(queues[MLPipelineStage::Loader], package);
        }
        else
        {
            // if we have an image, send to the extractor stage

            // create a thumbnail for the notification

            package->thumbnailIcon = QIcon(package->image.smoothScale(48, 48, Qt::KeepAspectRatio).convertToPixmap());

            // wait for the queue to be ready

            while (nullptr == queues[MLPipelineStage::Extractor])
            {
                QThread::msleep(10);
            }

            // send the package to the finder queue

            enqueue(queues[MLPipelineStage::Extractor], package);
        }

        // Try to acquire the semaphore again.
        // This will block until the package is processed and the semaphore is released

        bqmSemaphore.acquire();

        // release the semaphore for the next image

        bqmSemaphore.release();
    }
}

void AutotagsPipelineBase::notify(MLPipelineNotification notification,
                                  const QString& _name,
                                  const QString& _path,
                                  const QString& _displayData,
                                  int _processed,
                                  const QImage& _thumbnail)
{
    // if we are in batch queue mode, release the semaphore to free up bqmSendOne call

    if (settings.bqmMode)
    {
        bqmSemaphore.release();
    }

    MLPipelineFoundation::notify(notification, _name, _path, _displayData, _processed, _thumbnail);
}

void AutotagsPipelineBase::notify(MLPipelineNotification notification,
                                  const QString& _name,
                                  const QString& _path,
                                  const QString& _displayData,
                                  int _processed,
                                  const DImg& _thumbnail)
{
    // if we are in batch queue mode, release the semaphore to free up bqmSendOne call

    if (settings.bqmMode)
    {
        bqmSemaphore.release();
    }

    MLPipelineFoundation::notify(notification, _name, _path, _displayData, _processed, _thumbnail);
}

void AutotagsPipelineBase::notify(MLPipelineNotification notification,
                                  const QString& _name,
                                  const QString& _path,
                                  const QString& _displayData,
                                  int _processed,
                                  const QIcon& _thumbnail)
{
    // if we are in batch queue mode, release the semaphore to free up bqmSendOne call

    if (settings.bqmMode)
    {
        bqmSemaphore.release();
    }

    MLPipelineFoundation::notify(notification, _name, _path, _displayData, _processed, _thumbnail);
}

bool AutotagsPipelineBase::enqueue(MLPipelineQueue* thisQueue, MLPipelinePackageFoundation* package)
{
    // calculate the package size.  Only big items need to be checked

    if (nullptr != package)
    {
        package->size = (
                         static_cast<AutotagsPipelinePackageBase*>(package)->image.size().width()  *
                         static_cast<AutotagsPipelinePackageBase*>(package)->image.size().height() * 4
                        );
    }

    return MLPipelineFoundation::enqueue(thisQueue, package);
}

} // namespace Digikam

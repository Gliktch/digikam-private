/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Thread actions task for image quality sorter.
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagequalitytask.h"

// Local includes

#include "digikam_debug.h"
#include "imagequalitysettings.h"
#include "imagequalityparser.h"
#include "previewloadthread.h"
#include "maintenancedata.h"
#include "scancontroller.h"
#include "metadatahub.h"
#include "iteminfo.h"
#include "dimg.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageQualityTask::Private
{
public:

    Private() = default;

public:

    ImageQualitySettings quality;
    ImageQualityParser*   imgqsort  = nullptr;

    MaintenanceData*      data      = nullptr;
};

// -------------------------------------------------------

ImageQualityTask::ImageQualityTask()
    : ActionJob(),
      d        (new Private)
{
}

ImageQualityTask::~ImageQualityTask()
{
    slotCancel();
    cancel();

    delete d;
}

void ImageQualityTask::setQuality(const ImageQualitySettings& quality)
{
    d->quality = quality;
}

void ImageQualityTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void ImageQualityTask::slotCancel()
{
    if (d->imgqsort)
    {
        d->imgqsort->cancelAnalyse();
    }
}

void ImageQualityTask::run()
{
    ActionJob::run();       // To customize thread name

    // While we have data (using this as check for non-null).

    while (d->data)
    {
        if (m_cancel)
        {
            return;
        }

        QString path   = d->data->getImagePath();

        if (path.isEmpty())
        {
            break;
        }

        PickLabel pick = NoPickLabel;
        ItemInfo info  = ItemInfo::fromLocalFile(path);

        // Get item preview to perform quality analysis. No need to load whole image, this will be slower.
        // 1024 pixels size image must be enough to get suitable Quality results.

        DImg dimg      = PreviewLoadThread::loadFastSynchronously(path, 1024);

        // cppcheck-suppress knownConditionTrueFalse
        if (!dimg.isNull() && !m_cancel)
        {
            // Run Quality analysis backend and store Pick Label result to database.
            // Backend Input : d->quality as Quality analysis settings,
            //                 dimg       as reduced size image data to parse,
            //                 path       as file path to patch database properties.
            // Result        : Backend must scan Quality of image depending of settings and compute a Quality estimation accordingly.
            //                 Finally, using file path, database Pick Label properties is assigned through ItemInfo interface.
            // Warning       : All code here will run in a separated thread and must be re-entrant/thread-safe. Only pure computation
            //                 must be processed. GUI calls are prohibited. ItemInfo and DImg can be used safety in thread.

            d->imgqsort = new ImageQualityParser(dimg, d->quality, &pick);
            d->imgqsort->startAnalyse();

            info.setPickLabel(pick);

            MetadataHub hub;
            hub.load(info);

            ScanController::FileMetadataWrite writeScope(info);
            writeScope.changed(hub.writeToMetadata(info, MetadataHub::WRITE_PICKLABEL));

            // delete image data after to set label.

            delete d->imgqsort;
            d->imgqsort = nullptr;
        }

        // Dispatch progress to Progress Manager.

        QImage qimg = dimg.smoothScale(48, 48, Qt::KeepAspectRatio).copyQImage();

        Q_EMIT signalFinished(info, qimg, pick);
    }

    Q_EMIT signalDone();
}

} // namespace Digikam

#include "moc_imagequalitytask.cpp"

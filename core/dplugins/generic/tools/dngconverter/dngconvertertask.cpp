/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-24
 * Description : DNG Converter thread
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngconvertertask.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "drawinfo.h"
#include "drawdecoder.h"
#include "dmetadata.h"
#include "digikam_debug.h"

namespace DigikamGenericDNGConverterPlugin
{

class Q_DECL_HIDDEN DNGConverterTask::Private
{
public:

    Private() = default;

public:

    bool               backupOriginalRawFile    = false;
    bool               compressLossLess         = true;
    bool               updateFileDate           = false;
    bool               cancel                   = false;

    int                previewMode              = DNGWriter::FULL_SIZE;

    QUrl               url;
    DNGConverterAction action                   = NONE;

    DNGWriter          dngProcessor;
};

DNGConverterTask::DNGConverterTask(QObject* const parent, const QUrl& fileUrl, const DNGConverterAction& action)
    : ActionJob(parent),
      d        (new Private)
{
    d->url    = fileUrl;
    d->action = action;
}

DNGConverterTask::~DNGConverterTask()
{
    slotCancel();
    delete d;
}

void DNGConverterTask::setBackupOriginalRawFile(bool b)
{
    d->backupOriginalRawFile = b;
}

void DNGConverterTask::setCompressLossLess(bool b)
{
    d->compressLossLess = b;
}

void DNGConverterTask::setUpdateFileDate(bool b)
{
    d->updateFileDate = b;
}

void DNGConverterTask::setPreviewMode(int mode)
{
    d->previewMode = mode;
}

void DNGConverterTask::run()
{
    ActionJob::run();       // To customize thread name

    if (d->cancel)
    {
        return;
    }

    switch (d->action)
    {
        case IDENTIFY:
        {
            // Identify Camera model.

            DRawInfo info;
            QString identify = i18n("Cannot identify Raw image");

            if (DRawDecoder::rawFileIdentify(info, d->url.toLocalFile()))
            {
                if (info.isDecodable)
                {
                    identify = info.make + QLatin1String("-") + info.model;
                }
            }

            DNGConverterActionData ad;
            ad.action        = d->action;
            ad.fileUrl       = d->url;
            ad.message       = identify;
            ad.result        = DNGWriter::PROCESS_COMPLETE;

            Q_EMIT signalFinished(ad);

            break;
        }

        case PROCESS:
        {
            DNGConverterActionData ad1;
            ad1.action   = PROCESS;
            ad1.fileUrl  = d->url;
            ad1.starting = true;

            Q_EMIT signalStarting(ad1);

            QString destPath;

            QFileInfo fi(d->url.toLocalFile());
            destPath     = fi.absolutePath() + QLatin1String("/.digikam-dngconverter-tmp-") +
                           QString::number(QDateTime::currentSecsSinceEpoch()) + QString(d->url.fileName());

            d->dngProcessor.reset();
            d->dngProcessor.setInputFile(d->url.toLocalFile());
            d->dngProcessor.setOutputFile(destPath);
            d->dngProcessor.setBackupOriginalRawFile(d->backupOriginalRawFile);
            d->dngProcessor.setCompressLossLess(d->compressLossLess);
            d->dngProcessor.setUpdateFileDate(d->updateFileDate);
            d->dngProcessor.setPreviewMode(d->previewMode);
            int ret      = d->dngProcessor.convert();

            DNGConverterActionData ad2;
            ad2.action   = PROCESS;
            ad2.fileUrl  = d->url;
            ad2.destPath = destPath;
            ad2.result   = ret;

            Q_EMIT signalFinished(ad2);

            break;
        }

        default:
        {
            qCCritical(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action specified";
            break;
        }
    }

    Q_EMIT signalDone();
}

void DNGConverterTask::slotCancel()
{
    d->cancel = true;
    d->dngProcessor.cancel();
}

} // namespace DigikamGenericDNGConverterPlugin

#include "moc_dngconvertertask.cpp"

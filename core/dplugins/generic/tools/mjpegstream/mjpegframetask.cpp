/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : a MJPEG frame generator.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mjpegframetask.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QString>
#include <QBuffer>
#include <QApplication>
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "previewloadthread.h"
#include "frameutils.h"
#include "frameosd.h"
#include "vidslidesettings.h"

namespace DigikamGenericMjpegStreamPlugin
{

class Q_DECL_HIDDEN MjpegFrameTask::Private
{
public:

    explicit Private(const MjpegStreamSettings& set)
        : settings(set)
    {
        VidSlideSettings::VidType type = (VidSlideSettings::VidType)settings.outSize;

        /**
         * NOTE: QIcon depend of X11 under Linux which is not re-rentrant.
         * Load this image here in first from main thread.
         */
        brokenImg = QIcon::fromTheme(QLatin1String("view-preview")).pixmap(VidSlideSettings::videoSizeFromType(type)).toImage();
        endImg    = QIcon::fromTheme(QLatin1String("window-close")).pixmap(VidSlideSettings::videoSizeFromType(type)).toImage();
    }

public:

    MjpegStreamSettings settings;               ///< The MJPEG stream settings.
    QImage              brokenImg;              ///< Image to push as frame if current item from list cannot be loaded.
    QImage              endImg;                 ///< Image to push as frame when stream is complete.
    bool                failedToLoad = false;   ///< determinate if image is loaded
};

MjpegFrameTask::MjpegFrameTask(const MjpegStreamSettings& settings)
    : ActionJob(nullptr),
      d        (new Private(settings))
{
}

MjpegFrameTask::~MjpegFrameTask()
{
    delete d;
}

QByteArray MjpegFrameTask::imageToJPEGArray(const QImage& frame) const
{
    QByteArray outbuf;
    QBuffer buffer(&outbuf);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", d->settings.quality);

    return outbuf;
}

QImage MjpegFrameTask::loadImageFromPreviewCache(const QString& path) const
{
    QImage qimg;
    qCDebug(DIGIKAM_GENERAL_LOG) << "MjpegStream: Generate frame for" << path;

    DImg dimg = PreviewLoadThread::loadHighQualitySynchronously(path);

    if (dimg.isNull())
    {
        // Generate an error frame.

        qimg            = d->brokenImg;
        d->failedToLoad = true;
        qCWarning(DIGIKAM_GENERAL_LOG) << "MjpegStream: Failed to load" << path;
    }
    else
    {
        // Generate real preview frame.

        qimg = dimg.copyQImage();
    }

    // Resize output image to the wanted dimensions.

    VidSlideSettings::VidType type = (VidSlideSettings::VidType)d->settings.outSize;
    qimg                           = FrameUtils::makeScaledImage(qimg, VidSlideSettings::videoSizeFromType(type));

    return qimg;
}

void MjpegFrameTask::run()
{
    ActionJob::run();       // To customize thread name

    QImage qiimg;   // Current image in stream.
    QImage qtimg;   // Current transition image.
    QImage qoimg;   // Next image in stream.

    FrameOsd osd;

    VidSlideSettings::VidType type     = (VidSlideSettings::VidType)d->settings.outSize;
    QSize JPEGsize                     = VidSlideSettings::videoSizeFromType(type);
    int imgFrames                      = d->settings.delay * d->settings.rate;
    bool oneLoopDone                   = false;

    TransitionMngr transmngr;
    transmngr.setOutputSize(JPEGsize);

    EffectMngr effmngr;
    effmngr.setOutputSize(JPEGsize);
    effmngr.setFrames(imgFrames);               // Ex: 30 frames at 10 img/s => 3 s of effect

    do
    {
        // To stream in loop forever.

        for (int i = 0 ; ((i < d->settings.inputImages.count()) && !m_cancel) ; ++i)
        {
            // One loop stream all items one by one from the ordered list

            if ((i == 0) && !oneLoopDone)
            {
                // If we use transition, the first item at the first loop must be merged from a black image.

                qiimg = FrameUtils::makeFramedImage(QString(), JPEGsize);
            }

            QString ofile;

            if (i < d->settings.inputImages.count())
            {
                // The current item to pass to the next stage from a transition

                ofile = d->settings.inputImages[i].toLocalFile();
            }

            qoimg      = loadImageFromPreviewCache(ofile);

            // Apply transition between images

            transmngr.setInImage(qiimg);
            transmngr.setOutImage(qoimg);
            transmngr.setTransition(d->settings.transition);

            int ttmout = 0;

            do
            {
                // Loop over all stages to make the transition

                qtimg = transmngr.currentFrame(ttmout);

                Q_EMIT signalFrameChanged(imageToJPEGArray(qtimg));

                QThread::msleep(lround(1000.0 / d->settings.rate));
            }
            while ((ttmout != -1) && !m_cancel);

            // Append OSD overlay on frame generated by effect.

            QUrl itemUrl = d->settings.inputImages[i];

            // Apply effect on frame

            int count  = 0;
            int itmout = 0;
            effmngr.setImage(qoimg);
            effmngr.setEffect(d->settings.effect);

            do
            {
                // Loop over all stages to make the effect

                qiimg = effmngr.currentFrame(itmout);

                if (!d->failedToLoad)
                {
                    if ((JPEGsize.width() >= 1024) && (JPEGsize.height() >= 576))
                    {
                        osd.insertOsdToFrame(qiimg,
                                             itemUrl,
                                             d->settings.osdSettings,
                                             d->settings.iface);
                    }
                }
                else
                {
                    osd.insertMessageOsdToFrame(qiimg,
                                                JPEGsize,
                                                QLatin1String("Failed to load image"));
                }

                Q_EMIT signalFrameChanged(imageToJPEGArray(qiimg));

                count++;

                QThread::msleep(lround(1000.0 / d->settings.rate));
            }
            while ((count < imgFrames) && !m_cancel);

            d->failedToLoad = false;
            oneLoopDone     = true;        // At least one loop is done.
        }
    }
    while (!m_cancel && d->settings.loop);

    osd.insertMessageOsdToFrame(d->endImg,
                                JPEGsize,
                                QLatin1String("End of stream"));

    Q_EMIT signalFrameChanged(imageToJPEGArray(d->endImg));

    qCDebug(DIGIKAM_GENERAL_LOG) << "MjpegStream: end of stream";

    Q_EMIT signalDone();
}

} // namespace DigikamGenericMjpegStreamPlugin

#include "moc_mjpegframetask.cpp"

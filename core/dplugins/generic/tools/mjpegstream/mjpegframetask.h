/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : a MJPEG frame generator.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QColor>
#include <QPoint>
#include <QFont>
#include <QDateTime>

// Local includes

#include "actionthread.h"
#include "mjpegserver.h"
#include "mjpegstreamsettings.h"

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class MjpegFrameTask : public ActionJob
{
    Q_OBJECT

public:

    /**
     * Standard constructor using MJPEG stream settings.
     */
    explicit MjpegFrameTask(const MjpegStreamSettings& settings);
    ~MjpegFrameTask()   override;

Q_SIGNALS:

    /**
     * Output JPEG frames from generator thread (emitted to server thread).
     */
    void signalFrameChanged(const QByteArray& frame);

private:

    /**
     * Helper monvert a QImage to a byte-aaray of JPEG data file.
     */
    QByteArray imageToJPEGArray(const QImage& frame) const;

    /**
     * Load image from Preview cache from path with desired output size.
     */
    QImage loadImageFromPreviewCache(const QString& path) const;

    /**
     * Loop from separated main thread to render periodically frames for MJPEG stream.
     * This include transition between images and effect to render items.
     * Results are sent by signalFrameChanged().
     */
    void run()          override;

    MjpegFrameTask(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMjpegStreamPlugin

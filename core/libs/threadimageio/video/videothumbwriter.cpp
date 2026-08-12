/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : video thumbnails extraction based on ffmpeg
 *
 * SPDX-FileCopyrightText: 2010      by Dirk Vanden Boer <dirk dot vdb at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2018 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "videothumbwriter.h"

namespace Digikam
{

VideoFrame::VideoFrame(int width, int height, int lineSize)
    : width   (width),
      height  (height),
      lineSize(lineSize)
{
}

// ------------------------------------------------------

void VideoThumbWriter::writeFrame(VideoFrame& frame, QImage& image)
{
    QImage previewImage(frame.width, frame.height, QImage::Format_RGB888);

    for (quint32 y = 0 ; y < frame.height ; ++y)
    {
        // Copy each line...

        memcpy(previewImage.scanLine(y),
               &frame.frameData[y * frame.lineSize],
               frame.width * 3);
    }

    image = previewImage;
}

} // namespace Digikam

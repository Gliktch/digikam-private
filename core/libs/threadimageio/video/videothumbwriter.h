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

#pragma once

// Qt includes

#include <QtGlobal>
#include <QImage>
#include <QVector>

namespace Digikam
{

class VideoFrame
{
public:

    VideoFrame()  = default;
    VideoFrame(int width, int height, int lineSize);
    ~VideoFrame() = default;

public:

    quint32         width       = 0;
    quint32         height      = 0;
    quint32         lineSize    = 0;
    QVector<quint8> frameData;
};

// -----------------------------------------------------------------

class VideoThumbWriter
{
public:

    VideoThumbWriter()  = default;
    ~VideoThumbWriter() = default;

    void writeFrame(VideoFrame& frame, QImage& image);

private:

    /// @note disabled
    VideoThumbWriter(const VideoThumbWriter&)            = delete;
    VideoThumbWriter& operator=(const VideoThumbWriter&) = delete;
};

} // namespace Digikam

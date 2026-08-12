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

#include <QString>

// Local includes

#include "videothumbwriter.h"

namespace Digikam
{

class VideoThumbDecoder
{
public:

    explicit VideoThumbDecoder(const QString& filename);
    ~VideoThumbDecoder();

public:

    QString getCodec()       const;
    int     getWidth()       const;
    int     getHeight()      const;
    int     getDuration()    const;
    bool    getInitialized() const;

    void seek(int timeInSeconds);
    bool decodeVideoFrame()  const;
    void getScaledVideoFrame(int scaledSize,
                             bool maintainAspectRatio,
                             VideoFrame& videoFrame);

    void initialize(const QString& filename);
    void destroy();

private:

    /// @note disabled
    VideoThumbDecoder(const VideoThumbDecoder&)            = delete;
    VideoThumbDecoder& operator=(const VideoThumbDecoder&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

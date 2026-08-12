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

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class VideoFrame;

class DIGIKAM_EXPORT VideoStripFilter
{
public:

    VideoStripFilter()  = default;
    ~VideoStripFilter() = default;

    void process(VideoFrame& videoFrame);

private:

    /// @note disabled
    VideoStripFilter(const VideoStripFilter&)            = delete;
    VideoStripFilter& operator=(const VideoStripFilter&) = delete;
};

} // namespace Digikam

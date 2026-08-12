/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : video information container
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "videoinfocontainer.h"

// Qt includes

#include <QDataStream>

namespace Digikam
{

bool VideoInfoContainer::operator==(const VideoInfoContainer& t) const
{
    bool b1  = (aspectRatio        == t.aspectRatio);
    bool b2  = (duration           == t.duration);
    bool b3  = (frameRate          == t.frameRate);
    bool b4  = (videoCodec         == t.videoCodec);
    bool b5  = (audioBitRate       == t.audioBitRate);
    bool b6  = (audioChannelType   == t.audioChannelType);
    bool b7  = (audioCodec         == t.audioCodec);

    return (b1 && b2 && b3 && b4 && b5 && b6 && b7);
}

bool VideoInfoContainer::isEmpty() const
{
    if (
        aspectRatio.isEmpty()            &&
        duration.isEmpty()               &&
        frameRate.isEmpty()              &&
        videoCodec.isEmpty()             &&
        audioBitRate.isEmpty()           &&
        audioChannelType.isEmpty()       &&
        audioCodec.isEmpty()
       )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VideoInfoContainer::isNull() const
{
    return (
            aspectRatio.isEmpty()            &&
            duration.isEmpty()               &&
            frameRate.isEmpty()              &&
            videoCodec.isEmpty()             &&
            audioBitRate.isEmpty()           &&
            audioChannelType.isEmpty()       &&
            audioCodec.isEmpty()
           );
}

QDataStream& operator<<(QDataStream& ds, const VideoInfoContainer& info)
{
    ds << info.aspectRatio;
    ds << info.duration;
    ds << info.frameRate;
    ds << info.videoCodec;
    ds << info.audioBitRate;
    ds << info.audioChannelType;
    ds << info.audioCodec;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, VideoInfoContainer& info)
{
    ds >> info.aspectRatio;
    ds >> info.duration;
    ds >> info.frameRate;
    ds >> info.videoCodec;
    ds >> info.audioBitRate;
    ds >> info.audioChannelType;
    ds >> info.audioCodec;

    return ds;
}

QDebug operator<<(QDebug dbg, const VideoInfoContainer& t)
{
    dbg.nospace() << "VideoInfoContainer::aspectRatio: "
                  << t.aspectRatio << ", ";
    dbg.nospace() << "VideoInfoContainer::duration: "
                  << t.duration << ", ";
    dbg.nospace() << "VideoInfoContainer::frameRate: "
                  << t.frameRate << ", ";
    dbg.nospace() << "VideoInfoContainer::videoCodec: "
                  << t.videoCodec << ", ";
    dbg.nospace() << "VideoInfoContainer::audioBitRate: "
                  << t.audioBitRate << ", ";
    dbg.nospace() << "VideoInfoContainer::audioChannelType: "
                  << t.audioChannelType << ", ";
    dbg.nospace() << "VideoInfoContainer::audioCodec: "
                  << t.audioCodec << ", ";

    return dbg.space();
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-03-17
 * Description : Methods to list FFMPEG features.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ffmpegconfighelper.h"

// FFMpeg includes

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace Digikam
{

FFMpegProperties FFMpegConfigHelper::getVideoCodecsProperties()
{
    FFMpegProperties props;

    const AVCodec* vcodec = nullptr;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(58, 10, 100)

    void* it = nullptr;

    while ((vcodec = av_codec_iterate(&it)))

#else

    avcodec_register_all();

    while ((vcodec = av_codec_next(vcodec)))

#endif

    {
        if (vcodec->type != AVMEDIA_TYPE_VIDEO)
        {
            continue;
        }

        QString name = QString::fromUtf8(vcodec->name);
        QStringList vals;

        vals << QString::fromUtf8(vcodec->long_name);

        if (av_codec_is_decoder(vcodec))
        {
            vals << QLatin1String("R");
        }
        else
        {
            vals << QLatin1String("X");
        }

        if (av_codec_is_encoder(vcodec))
        {
            vals << QLatin1String("W");
        }
        else
        {
            vals << QLatin1String("X");
        }

        props.insert(name, vals);
    }

    return props;
}

FFMpegProperties FFMpegConfigHelper::getAudioCodecsProperties()
{
    FFMpegProperties props;

    const AVCodec* acodec = nullptr;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(58, 10, 100)

    void* it              = nullptr;

    while ((acodec = av_codec_iterate(&it)))

#else

    avcodec_register_all();

    while ((acodec = av_codec_next(acodec)))

#endif

    {
        if (acodec->type != AVMEDIA_TYPE_AUDIO)
        {
            continue;
        }

        QString name = QString::fromUtf8(acodec->name);
        QStringList vals;

        vals << QString::fromUtf8(acodec->long_name);

        if (av_codec_is_decoder(acodec))
        {
            vals << QLatin1String("R");
        }
        else
        {
            vals << QLatin1String("X");
        }

        if (av_codec_is_encoder(acodec))
        {
            vals << QLatin1String("W");
        }
        else
        {
            vals << QLatin1String("X");
        }

        props.insert(name, vals);
    }

    return props;
}

FFMpegProperties FFMpegConfigHelper::getExtensionsProperties()
{
    FFMpegProperties props;
    QStringList ext, exts;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(58, 10, 100)

    const AVOutputFormat* outfmt = nullptr;
    void* it                     = nullptr;

    while ((outfmt = av_muxer_iterate(&it)))

#else

    av_register_all();                  // Must register all input/output formats
    AVOutputFormat* outfmt = nullptr;

    while ((outfmt = av_oformat_next(outfmt)))

#endif

    {
        if (outfmt->extensions)
        {
            ext << QString::fromUtf8(outfmt->extensions).split(QLatin1Char(','), Qt::SkipEmptyParts);
        }
    }

    for (const QString& val : std::as_const(ext))
    {
        exts.append(val.trimmed());
    }

    exts.removeDuplicates();

    for (const QString& val : std::as_const(exts))
    {

        outfmt = nullptr;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(58, 10, 100)

        it     = nullptr;

        while ((outfmt = av_muxer_iterate(&it)))

#else

        while ((outfmt = av_oformat_next(outfmt)))

#endif

        {
            if (QString::fromUtf8(outfmt->extensions) == val)
            {
                props.insert(val, QStringList() << QString::fromUtf8(outfmt->long_name));
                continue;
            }
        }
    }

    return props;
}

} // namespace Digikam

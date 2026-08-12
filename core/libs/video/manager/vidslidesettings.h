/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QStringList>
#include <QList>
#include <QUrl>
#include <QSize>
#include <QMap>
#include <QTime>
#include <QStandardPaths>

// Local includes

#include "digikam_export.h"
#include "filesaveconflictbox.h"
#include "transitionmngr.h"
#include "effectmngr.h"
#include "frameosdsettings.h"
#include "dinfointerface.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT VidSlideSettings
{
public:

    /**
     * Images selection mode
     */
    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    /**
     * Video player to use
     */
    enum VidPlayer
    {
        NOPLAYER = 0,
        INTERNAL,
        DESKTOP
    };

    /**
     * Video Codecs
     */
    enum VidCodec
    {
        X264 = 0,    ///< https://en.wikipedia.org/wiki/X264
        MPEG4,       ///< https://en.wikipedia.org/wiki/MPEG-4
        MPEG2,       ///< https://en.wikipedia.org/wiki/MPEG-2
        MJPEG,       ///< https://en.wikipedia.org/wiki/Motion_JPEG
        FLASH,       ///< https://en.wikipedia.org/wiki/Adobe_Flash
        WEBMVP8,     ///< https://en.wikipedia.org/wiki/VP8
        THEORA,      ///< https://en.wikipedia.org/wiki/Theora
        WMV7,        ///< https://en.wikipedia.org/wiki/Windows_Media_Video
        WMV8,        ///< https://en.wikipedia.org/wiki/Windows_Media_Video
        WMV9         ///< https://en.wikipedia.org/wiki/Windows_Media_Video
    };

    /**
     * Video Standards
     */
    enum VidStd
    {
        PAL = 0,    ///< 25 FPS
        NTSC        ///< 29.97 FPS
    };

    /**
     * Video types (size of target screen)
     * See https://en.wikipedia.org/wiki/List_of_common_resolutions#Digital_TV_standards
     *     https://en.wikipedia.org/wiki/Aspect_ratio_(image)
     */
    enum VidType
    {
        QVGA = 0,   ///< 320   x 180  - 16:9    - Computer Graphics
        VCD1,       ///< 352   x 240  - 7:5     - Digital TV
        VCD2,       ///< 352   x 288  - 6:5     - Digital TV
        CVD1,       ///< 352   x 480  - 11:15   - Digital TV
        CVD2,       ///< 352   x 576  - 11:18   - Digital TV
        HVGA,       ///< 480   x 270  - 16:9    - Computer Graphics
        SVCD1,      ///< 480   x 480  - 1:1     - Digital TV
        SDTV1,      ///< 528   x 480  - 11:10   - Digital TV
        SDTV2,      ///< 544   x 480  - 17:15   - Digital TV
        EDTV1,      ///< 544   x 576  - 17;18   - Digital TV
        SVCD2,      ///< 480   x 576  - 5:6     - Digital TV
        EGA,        ///< 640   x 350  - 16:9    - Computer Graphics
        VGA,        ///< 640   x 480  - 4:3     - Computer Graphics
        SDTV3,      ///< 704   x 480  - 22:15   - Digital TV
        EDTV2,      ///< 704   x 576  - 11;9    - Digital TV
        DVD1,       ///< 720   x 480  - 3:2     - Digital TV
        DVD2,       ///< 720   x 576  - 5:4     - Digital TV
        WVGA,       ///< 800   x 450  - 16:9    - Computer Graphics
        SVGA,       ///< 800   x 600  - 4:3     - Computer Graphics
        DVGA,       ///< 960   x 640  - 3:2     - Computer Graphics
        XVGA,       ///< 1024  x 576  - 16:9    - Computer Graphics
        HDTV,       ///< 1280  x 720  - 16:9    - Digital TV
        WXGA1,      ///< 1280  x 768  - 5:3     - Computer Graphics
        WXGA2,      ///< 1280  x 800  - 8:5     - Computer Graphics
        SXGA,       ///< 1280  x 1024 - 5:4     - Computer Graphics
        SXGAPLUS,   ///< 1400  x 1050 - 4:3     - Computer Graphics
        WSXGA,      ///< 1440  x 900  - 8:5     - Computer Graphics
        HDPLUS,     ///< 1600  x 900  - 16:9    - Digital TV
        UXGA,       ///< 1600  x 1200 - 4:3     - Computer Graphics
        WSXGAPLUS,  ///< 1680  x 1050 - 8:5     - Computer Graphics
        BLUERAY,    ///< 1920  x 1080 - 19:9    - Digital TV
        WUXGA,      ///< 1920  x 1200 - 8:5     - Computer Graphics
        TXGA,       ///< 1920  x 1440 - 7:5     - Computer Graphics
        QXGA,       ///< 2048  x 1536 - 4:3     - Computer Graphics
        UWFHD,      ///< 2560  < 1080 - 21:9    - Computer Graphics
        WQHD,       ///< 2560  x 1440 - 16:9    - Computer Graphics
        WQXGA,      ///< 2560  x 1600 - 8:5     - Computer Graphics
        QSXGA,      ///< 2560  x 2048 - 5:4     - Computer Graphics
        QSXGAPLUS,  ///< 2800  x 2100 - 4:3     - Computer Graphics
        WQXGAPLUS,  ///< 3200  x 1800 - 16:9    - Computer Graphics
        WQSXGA,     ///< 3200  x 2048 - 25:16   - Computer Graphics
        QUXGA,      ///< 3200  x 2400 - 4:3     - Computer Graphics
        UHD4K,      ///< 3840  x 2160 - 19:9    - Digital TV
        WQUXGA,     ///< 3840  x 2400 - 8:5     - Computer Graphics
        HXGA,       ///< 4096  x 3072 - 4:3     - Computer Graphics
        UHD5K,      ///< 5120  x 2880 - 16:9    - Computer Graphics
        WHXGA,      ///< 5120  x 3200 - 8:5     - Computer Graphics
        HSXGA,      ///< 5120  x 4096 - 5:4     - Computer Graphics
        UHD6K,      ///< 6016  x 3384 - 16:9    - Computer Graphics
        WHSXGA,     ///< 6400  x 4096 - 25:16   - Computer Graphics
        HUXGA,      ///< 6400  x 4800 - 4:3     - Computer Graphics
        UHD8K,      ///< 7680  x 4320 - 16:9    - Digital TV
        WHUXGA,     ///< 7680  x 4800 - 8:5     - Computer Graphics
        UW10K,      ///< 10240 x 4320 - 21:9    - Computer Graphics
        UW16K       ///< 15360 x 8640 - 16:9    - Computer Graphics
    };

    /**
     * Video rates in bits per seconds.
     */
    enum VidBitRate
    {
        VBR04 = 0,  ///< 400000
        VBR05,      ///< 500000
        VBR10,      ///< 1000000
        VBR12,      ///< 1200000
        VBR15,      ///< 1500000
        VBR20,      ///< 2000000
        VBR25,      ///< 2500000
        VBR30,      ///< 3000000
        VBR40,      ///< 4000000
        VBR45,      ///< 4500000
        VBR50,      ///< 5000000
        VBR60,      ///< 6000000
        VBR80       ///< 8000000
    };

    /**
     * Video Container Formats
     */
    enum VidFormat
    {
        AVI = 0,    ///< https://en.wikipedia.org/wiki/Audio_Video_Interleave
        MKV,        ///< https://en.wikipedia.org/wiki/Matroska
        MP4,        ///< https://en.wikipedia.org/wiki/MPEG-4_Part_14
        MPG         ///< https://en.wikipedia.org/wiki/MPEG-2
    };

public:

    VidSlideSettings()  = default;
    ~VidSlideSettings() = default;

    /**
     * Read and write settings in config file between sessions.
     */
    void  readSettings(const KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

    QSize   videoSize()             const;              ///< Return the current video size.
    int     videoBitRate()          const;              ///< Return the current video bit rate.
    qreal   videoFrameRate()        const;              ///< Return the current video frame rate.
    QString videoCodec()            const;              ///< Return the current video ffmpeg codec name.
    QString videoFormat()           const;              ///< Return the current video format extension.

    /**
     * Helper methods to fill combobox from GUI.
     */
    static QMap<VidType,    QString> videoTypeNames();
    static QMap<VidBitRate, QString> videoBitRateNames();
    static QMap<VidStd,     QString> videoStdNames();
    static QMap<VidCodec,   QString> videoCodecNames();
    static QMap<VidFormat,  QString> videoFormatNames();
    static QMap<VidPlayer,  QString> videoPlayerNames();

    static QSize videoSizeFromType(VidType type);       ///< Return the current size from a type of video.
    static bool isVideoTVFormat(VidType type);          ///< Return true if type is a video TV format. If false is returned type is computer graphics screen format.

    QStringList defaultFFmpegSearchPaths() const;

public:

    Selection                         selMode           = IMAGES;               ///< Items selection mode

    // -- Generator settings ------

    QList<QUrl>                       inputImages;                              ///< Images stream.
    QString                           audioTrack;                               ///< Soundtrack stream.

    TransitionMngr::TransType         transition        = TransitionMngr::None; ///< Transition type between images.

    int                               imgFrames         = 125;                  ///< Amount of frames by image to encode in video (ex: 125 frames = 5 s at 25 img/s).

    int                               abitRate          = 64000;                ///< Encoded Audio stream bit rate in bit/s.
    VidBitRate                        vbitRate          = VBR12;                ///< Encoded Video stream bit rate in bit/s.
    VidStd                            vStandard         = PAL;                  ///< Encoded Video standard => frame rate in img/s.
    VidType                           vType             = BLUERAY;              ///< Encoded video type.
    VidCodec                          vCodec            = X264;                 ///< Encoded video codec.
    VidFormat                         vFormat           = MP4;                  ///< Encoded video container format.
    EffectMngr::EffectType            vEffect           = EffectMngr::None;     ///< Encoded video effect while displaying images.

    /**
     * Rule to follow if video file already exists.
     */
    FileSaveConflictBox::ConflictRule conflictRule      = FileSaveConflictBox::OVERWRITE;

    /**
     * Encoded video stream directory.
     */
    QString                           outputDir         = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);

    // ---------------------

    QString                           tempDir;                                  ///< To store temporary frames.
    QString                           outputFile;                               ///< Path to encoded video.
    QString                           filesList;                                ///< Path to list of frame files to encode.
    QString                           outputVideo;                              ///< Target video file encoded at end.
    VidPlayer                         outputPlayer      = INTERNAL;             ///< Open video stream in player at end.
    QString                           ffmpegPath;                               ///< Path to FFmpeg binary.
    bool                              equalize          = false;                ///< Equalize filter to applying while encoding video from frames.
    int                               strength          = 5;                    ///< Equalization strength factor.

    // -- FFMpeg features --------

    QMap<QString, QString>            ffmpegCodecs;                             ///< Map of FFmpeg codec names and features.
    QMap<QString, QString>            ffmpegFormats;                            ///< Map of FFmpeg format names and features.
    QTime                             soundtrackLength;                         ///< Duration of the soundtrack.

    FrameOsdSettings                  osdSettings;                              ///< On Screen Display parameters.
    DInfoInterface*                   iface             = nullptr;              ///< Plugin host interface to handle item properties.
};

} // namespace Digikam

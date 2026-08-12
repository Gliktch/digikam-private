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

#include "vidslidesettings.h"

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

void VidSlideSettings::readSettings(const KConfigGroup& group)
{
    selMode      = (Selection)group.readEntry("SelMode",
                   (int)IMAGES);
    imgFrames    = group.readEntry("ImgFrames",
                   125);
    vStandard    = (VidStd)group.readEntry("VStandard",
                   (int)PAL);
    vbitRate     = (VidBitRate)group.readEntry("VBitRate",
                   (int)VBR12);
    vCodec       = (VidCodec)group.readEntry("VCodec",
                   (int)X264);
    vType        = (VidType)group.readEntry("VType",
                   (int)BLUERAY);
    vFormat      = (VidFormat)group.readEntry("VFormat",
                   (int)MP4);
    vEffect      = (EffectMngr::EffectType)group.readEntry("VEffect",
                   (int)EffectMngr::None);
    abitRate     = group.readEntry("ABitRate",
                   64000);
    transition   = (TransitionMngr::TransType)group.readEntry("Transition",
                   (int)TransitionMngr::None);
    conflictRule = (FileSaveConflictBox::ConflictRule)group.readEntry("ConflictRule",
                   (int)FileSaveConflictBox::OVERWRITE);
    outputDir    = group.readEntry("OutputDir",
                   QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    audioTrack   = group.readEntry("AudioTrack",
                   QString());
    outputPlayer = (VidPlayer)group.readEntry("OutputPlayer",
                   (int)INTERNAL);
    ffmpegPath   = group.readEntry("FFmpegPath",
                   defaultFFmpegSearchPaths().constFirst());
    equalize     = group.readEntry("Equalize",
                   false);
    strength     = group.readEntry("Strength",
                   5);
}

void VidSlideSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",      (int)selMode);
    group.writeEntry("ImgFrames",    imgFrames);
    group.writeEntry("VStandard",    (int)vStandard);
    group.writeEntry("VBitRate",     (int)vbitRate);
    group.writeEntry("VCodec",       (int)vCodec);
    group.writeEntry("VType",        (int)vType);
    group.writeEntry("VFormat",      (int)vFormat);
    group.writeEntry("VEffect",      (int)vEffect);
    group.writeEntry("Transition",   (int)transition);
    group.writeEntry("ABitRate",     abitRate);
    group.writeEntry("ConflictRule", (int)conflictRule);
    group.writeEntry("OutputDir",    outputDir);
    group.writeEntry("AudioTrack",   audioTrack);
    group.writeEntry("OutputPlayer", (int)outputPlayer);
    group.writeEntry("FFmpegPath",   ffmpegPath);
    group.writeEntry("Equalize",     equalize);
    group.writeEntry("Strength",     strength);
}

QSize VidSlideSettings::videoSize() const
{
    return videoSizeFromType(vType);
}

QSize VidSlideSettings::videoSizeFromType(VidType type)
{
    QSize s;

    switch (type)
    {
        case QVGA:
        {
            s = QSize(320, 180);
            break;
        }

        case VCD1:
        {
            s = QSize(352, 240);
            break;
        }

        case VCD2:
        {
            s = QSize(352, 288);
            break;
        }

        case CVD1:
        {
            s = QSize(352, 480);
            break;
        }

        case CVD2:
        {
            s = QSize(352, 576);
            break;
        }

        case HVGA:
        {
            s = QSize(480, 270);
            break;
        }

        case SVCD1:
        {
            s = QSize(480, 480);
            break;
        }

        case SDTV1:
        {
            s = QSize(528, 480);
            break;
        }

        case SDTV2:
        {
            s = QSize(544, 480);
            break;
        }

        case EDTV1:
        {
            s = QSize(544, 576);
            break;
        }

        case SVCD2:
        {
            s = QSize(480, 576);
            break;
        }

        case EGA:
        {
            s = QSize(640, 350);
            break;
        }

        case VGA:
        {
            s = QSize(640, 480);
            break;
        }

        case SDTV3:
        {
            s = QSize(704, 480);
            break;
        }

        case EDTV2:
        {
            s = QSize(704, 576);
            break;
        }

        case DVD1:
        {
            s = QSize(720, 480);
            break;
        }

        case DVD2:
        {
            s = QSize(720, 576);
            break;
        }

        case WVGA:
        {
            s = QSize(800, 450);
            break;
        }

        case SVGA:
        {
            s = QSize(800, 600);
            break;
        }

        case DVGA:
        {
            s = QSize(960, 640);
            break;
        }

        case XVGA:
        {
            s = QSize(1024, 576);
            break;
        }

        case HDTV:
        {
            s = QSize(1280, 720);
            break;
        }

        case WXGA1:
        {
            s = QSize(1280, 768);
            break;
        }

        case WXGA2:
        {
            s = QSize(1280, 800);
            break;
        }

        case SXGA:
        {
            s = QSize(1280, 1024);
            break;
        }

        case SXGAPLUS:
        {
            s = QSize(1400, 1050);
            break;
        }

        case WSXGA:
        {
            s = QSize(1440, 900);
            break;
        }

        case UXGA:
        {
            s = QSize(1600, 1200);
            break;
        }

        case HDPLUS:
        {
            s = QSize(1600, 900);
            break;
        }

        case WSXGAPLUS:
        {
            s = QSize(1680, 1050);
            break;
        }

        case WUXGA:
        {
            s = QSize(1920, 1200);
            break;
        }

        case TXGA:
        {
            s = QSize(1920, 1440);
            break;
        }

        case QXGA:
        {
            s = QSize(2048, 1536);
            break;
        }

        case UWFHD:
        {
            s = QSize(2560, 1080);
            break;
        }

        case WQHD:
        {
            s = QSize(2560, 1440);
            break;
        }

        case WQXGA:
        {
            s = QSize(2560, 1600);
            break;
        }

        case QSXGA:
        {
            s = QSize(2560, 2048);
            break;
        }

        case QSXGAPLUS:
        {
            s = QSize(2800, 2100);
            break;
        }

        case WQXGAPLUS:
        {
            s = QSize(3200, 1800);
            break;
        }

        case WQSXGA:
        {
            s = QSize(3200, 2048);
            break;
        }

        case QUXGA:
        {
            s = QSize(3200, 2400);
            break;
        }

        case UHD4K:
        {
            s = QSize(3840, 2160);
            break;
        }

        case WQUXGA:
        {
            s = QSize(3840, 2400);
            break;
        }

        case HXGA:
        {
            s = QSize(4096, 3072);
            break;
        }

        case UHD5K:
        {
            s = QSize(5120, 2880);
            break;
        }

        case WHXGA:
        {
            s = QSize(5120, 3200);
            break;
        }

        case HSXGA:
        {
            s = QSize(5120, 4096);
            break;
        }

        case UHD6K:
        {
            s = QSize(6016, 3384);
            break;
        }

        case WHSXGA:
        {
            s = QSize(6400, 4096);
            break;
        }

        case HUXGA:
        {
            s = QSize(6400, 4800);
            break;
        }

        case UHD8K:
        {
            s = QSize(7680, 4320);
            break;
        }

        case WHUXGA:
        {
            s = QSize(7680, 4800);
            break;
        }

        case UW10K:
        {
            s = QSize(10240, 4320);
            break;
        }

        case UW16K:
        {
            s = QSize(15360, 8640);
            break;
        }

        default: // BLUERAY
        {
            s = QSize(1920, 1080);
            break;
        }
    }

    return s;
}

bool VidSlideSettings::isVideoTVFormat(VidType type)
{
    switch (type)
    {
        case VCD1:
        case VCD2:
        case CVD1:
        case CVD2:
        case SVCD1:
        case SDTV1:
        case SDTV2:
        case EDTV1:
        case SVCD2:
        case SDTV3:
        case EDTV2:
        case DVD1:
        case DVD2:
        case HDTV:
        case BLUERAY:
        case UHD4K:
        case UHD8K:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}

QMap<VidSlideSettings::VidType, QString> VidSlideSettings::videoTypeNames()
{
    QMap<VidType, QString> types;

    types[QVGA]      = i18nc("Video Type: QVGA",      "QVGA - 320x180 - 16:9");
    types[VCD1]      = i18nc("Video Type: VCD1",      "VCD - 352x240 - 7:5");
    types[VCD2]      = i18nc("Video Type: VCD2",      "VCD - 352x288 - 6:5");
    types[CVD1]      = i18nc("Video Type: CVD1",      "CVD - 352x480 - 4:3");
    types[CVD2]      = i18nc("Video Type: CVD2",      "CVD - 352x576 - 16:9");
    types[HVGA]      = i18nc("Video Type: HVGA",      "HVGA - 480x270 - 16:9");
    types[SVCD1]     = i18nc("Video Type: SVCD1",     "SVCD - 480x480 - 1:1");
    types[SDTV1]     = i18nc("Video Type: SDTV1",     "SDTV - 528x480 - 11:10");
    types[SDTV2]     = i18nc("Video Type: SDTV2",     "SDTV - 544x480 - 17:15");
    types[EDTV1]     = i18nc("Video Type: EDTV1",     "EDTV - 544x576 - 17:18");
    types[SVCD2]     = i18nc("Video Type: SVCD2",     "SVCD - 480x576 - 5:6");
    types[EGA]       = i18nc("Video Type: EGA",       "EGA - 640x350 - 16:9");
    types[VGA]       = i18nc("Video Type: VGA",       "VGA - 640x480 - 4:3");
    types[SDTV3]     = i18nc("Video Type: SDTV3",     "SDTV - 704x480 - 22:15");
    types[EDTV2]     = i18nc("Video Type: EDTV2",     "EDTV - 704x576 - 11:9");
    types[DVD1]      = i18nc("Video Type: DVD",       "DVD - 720x480 - 3:2");
    types[DVD2]      = i18nc("Video Type: DVD",       "DVD - 720x576 - 5:4");
    types[WVGA]      = i18nc("Video Type: WVGA",      "WVGA - 800x450 - 16:9");
    types[SVGA]      = i18nc("Video Type: SVGA",      "SVGA - 800x600 - 4:3");
    types[DVGA]      = i18nc("Video Type: DVGA",      "DVGA - 960x640 - 3:2");
    types[XVGA]      = i18nc("Video Type: XVGA",      "XVGA - 1024x576 - 16:9");
    types[HDTV]      = i18nc("Video Type: HDTV",      "HDTV - 1280x720 - 16:9");
    types[WXGA1]     = i18nc("Video Type: WXGA1",     "WXGA - 1280x768 - 5:3");
    types[WXGA2]     = i18nc("Video Type: WXGA2",     "WXGA - 1280x800 - 8:5");
    types[SXGA]      = i18nc("Video Type: SXGA",      "SXGA - 1280x1024 - 5:4");
    types[SXGAPLUS]  = i18nc("Video Type: SXGAPLUS",  "SXGAPLUS - 1400x1050 - 4:3");
    types[WSXGA]     = i18nc("Video Type: WSXGA",     "WSXGA - 1440x900 - 8:5");
    types[HDPLUS]    = i18nc("Video Type: HDPLUS",    "HDPLUS - 1600x900 - 16:9");
    types[WSXGAPLUS] = i18nc("Video Type: WSXGAPLUS", "WSXGAPLUS - 1680x1050 - 8:5");
    types[UXGA]      = i18nc("Video Type: UXGA",      "UXGA - 1600x1200 - 4:3");
    types[BLUERAY]   = i18nc("Video Type: BLUERAY",   "BLUERAY - 1920x1080 - 16:9");
    types[WUXGA]     = i18nc("Video Type: WUXGA",     "WUXGA - 1920x1200 - 8:5");
    types[TXGA]      = i18nc("Video Type: TXGA",      "TXGA - 1920x1440 - 7:5");
    types[QXGA]      = i18nc("Video Type: QXGA",      "QXGA - 2048x1536 - 4:3");
    types[UWFHD]     = i18nc("Video Type: UWFHD",     "UWFHD - 2560x1080 - 21:9");
    types[WQHD]      = i18nc("Video Type: WQHD",      "WQHD - 2560x1440 - 16:9");
    types[WQXGA]     = i18nc("Video Type: WQXGA",     "WQXGA - 2560x1600 - 8:5");
    types[QSXGA]     = i18nc("Video Type: QSXGA",     "QSXGA - 2560x2048 - 5:4");
    types[QSXGAPLUS] = i18nc("Video Type: QSXGAPLUS", "QSXGAPLUS - 2800x2100 - 4:3");
    types[WQXGAPLUS] = i18nc("Video Type: WQXGAPLUS", "WQXGAPLUS - 3200x1800 - 16:9");
    types[WQSXGA]    = i18nc("Video Type: WQSXGA",    "WQSXGA - 3200x2048 - 25:16");
    types[QUXGA]     = i18nc("Video Type: QUXGA",     "QUXGA - 3200x2400 - 4:3");
    types[UHD4K]     = i18nc("Video Type: UHD4K",     "UHD4K - 3840x2160 - 16:9");
    types[WQUXGA]    = i18nc("Video Type: WQUXGA",    "WQUXGA - 3840x2400 - 8:5");
    types[HXGA]      = i18nc("Video Type: HXGA",      "HXGA - 4096x3072 - 4:3");
    types[UHD5K]     = i18nc("Video Type: UHD5K",     "UHD5K - 5120x2880 - 16:9");
    types[WHXGA]     = i18nc("Video Type: WHXGA",     "WHXGA - 5120x3200 - 8:5");
    types[HSXGA]     = i18nc("Video Type: HSXGA",     "HSXGA - 5120x4096 - 5:4");
    types[UHD6K]     = i18nc("Video Type: UHD6K",     "UHD6K - 6016x3384 - 16:9");
    types[WHSXGA]    = i18nc("Video Type: WHSXGA",    "WHSXGA - 6400x4096 - 25:16");
    types[HUXGA]     = i18nc("Video Type: HUXGA",     "HUXGA - 6400x4800 - 4:3");
    types[UHD8K]     = i18nc("Video Type: UHD8K",     "UHD8K - 7680x4320 - 16:9");
    types[WHUXGA]    = i18nc("Video Type: WHUXGA",    "WHUXGA - 7680x4800 - 8:5");
    types[UW10K]     = i18nc("Video Type: UW10K",     "UW10K - 10240x4320 - 21:9");
    types[UW16K]     = i18nc("Video Type: UW16K",     "UW16K - 15360x8640 - 16:9");

    return types;
}

int VidSlideSettings::videoBitRate() const
{
    int b;

    switch (vbitRate)
    {
        case VBR04:
        {
            b = 400000;
            break;
        }

        case VBR05:
        {
            b = 500000;
            break;
        }

        case VBR10:
        {
            b = 1000000;
            break;
        }

        case VBR15:
        {
            b = 1500000;
            break;
        }

        case VBR20:
        {
            b = 2000000;
            break;
        }

        case VBR25:
        {
            b = 2500000;
            break;
        }

        case VBR30:
        {
            b = 3000000;
            break;
        }

        case VBR40:
        {
            b = 4000000;
            break;
        }

        case VBR45:
        {
            b = 4500000;
            break;
        }

        case VBR50:
        {
            b = 5000000;
            break;
        }

        case VBR60:
        {
            b = 6000000;
            break;
        }

        case VBR80:
        {
            b = 8000000;
            break;
        }

        default: // VBR12
        {
            b = 1200000;
            break;
        }
    }

    return b;
}

qreal VidSlideSettings::videoFrameRate() const
{
    qreal fr;

    switch (vStandard)
    {
        case NTSC:
        {
            fr = 29.97;
            break;
        }

        default: // PAL
        {
            fr = 25.0;
            break;
        }
    }

    return fr;
}

QString VidSlideSettings::videoCodec() const
{
    QString cod;

    switch (vCodec)
    {
        case MJPEG:
        {
            cod = QLatin1String("mjpeg");
            break;
        }

        case MPEG2:
        {
            cod = QLatin1String("mpeg2video");
            break;
        }

        case MPEG4:
        {
            cod = QLatin1String("mpeg4");
            break;
        }

        case WEBMVP8:
        {
            cod = QLatin1String("libvpx");
            break;
        }

        case FLASH:
        {
            cod = QLatin1String("flv");
            break;
        }

        case THEORA:
        {
            cod = QLatin1String("libtheora");
            break;
        }

        case WMV7:
        {
            cod = QLatin1String("wm1");
            break;
        }

        case WMV8:
        {
            cod = QLatin1String("wm2");
            break;
        }

        case WMV9:
        {
            cod = QLatin1String("wm3");
            break;
        }

        default: // X264
        {
            cod = QLatin1String("libx264");
            break;
        }
    }

    return cod;
}

QString VidSlideSettings::videoFormat() const
{
    QString frm;

    switch (vFormat)
    {
        case AVI:
        {
            frm = QLatin1String("avi");
            break;
        }

        case MKV:
        {
            frm = QLatin1String("mkv");
            break;
        }

        case MPG:
        {
            frm = QLatin1String("mpg");
            break;
        }

        default: // MP4
        {
            frm = QLatin1String("mp4");
            break;
        }
    }

    return frm;
}

QMap<VidSlideSettings::VidBitRate, QString> VidSlideSettings::videoBitRateNames()
{
    QMap<VidBitRate, QString> br;

    br[VBR04]  = i18nc("Video Bit Rate 400000",  "400k");
    br[VBR05]  = i18nc("Video Bit Rate 500000",  "500k");
    br[VBR10]  = i18nc("Video Bit Rate 1000000", "1000k");
    br[VBR12]  = i18nc("Video Bit Rate 1200000", "1200k");
    br[VBR15]  = i18nc("Video Bit Rate 1500000", "1500k");
    br[VBR20]  = i18nc("Video Bit Rate 2000000", "2000k");
    br[VBR25]  = i18nc("Video Bit Rate 2500000", "2500k");
    br[VBR30]  = i18nc("Video Bit Rate 3000000", "3000k");
    br[VBR40]  = i18nc("Video Bit Rate 4000000", "4000k");
    br[VBR45]  = i18nc("Video Bit Rate 4500000", "4500k");
    br[VBR50]  = i18nc("Video Bit Rate 5000000", "5000k");
    br[VBR60]  = i18nc("Video Bit Rate 6000000", "6000k");
    br[VBR80]  = i18nc("Video Bit Rate 8000000", "8000k");

    return br;
}

QMap<VidSlideSettings::VidStd, QString> VidSlideSettings::videoStdNames()
{
    QMap<VidStd, QString> std;

    std[PAL]  = i18nc("Video Standard PAL",  "PAL - 25 FPS");
    std[NTSC] = i18nc("Video Standard NTSC", "NTSC - 29.97 FPS");

    return std;
}

QMap<VidSlideSettings::VidCodec, QString> VidSlideSettings::videoCodecNames()
{
    QMap<VidCodec, QString> codecs;

    // NOTE: Some video codecs are currently disabled due to incompatibility
    //       with bits rate, frames rate, or video sizes. This need some investiguations.

    codecs[X264]    = i18nc("Video Codec X264",    "High Quality H.264 AVC/MPEG-4 AVC");
    codecs[MPEG4]   = i18nc("Video Codec MPEG4",   "DivX/XVid/MPEG-4");
    codecs[MPEG2]   = i18nc("Video Codec MPEG2",   "MPEG-2 Video");
/*
    codecs[MJPEG]   = i18nc("Video Codec MJPEG",   "Motion JPEG");
    codecs[WEBMVP8] = i18nc("Video Codec WEBMVP8", "WebM-VP8");
    codecs[THEORA]  = i18nc("Video Codec THEORA",  "Theora-VP3");
    codecs[FLASH]   = i18nc("Video Codec FLASH",   "Flash Video/Sorenson H.263");
    codecs[WMV7]    = i18nc("Video Codec WMV7",    "Window Media Video 7");
    codecs[WMV8]    = i18nc("Video Codec WMV8",    "Window Media Video 8");
    codecs[WMV9]    = i18nc("Video Codec WMV9",    "Window Media Video 9");
*/
    return codecs;
}

QMap<VidSlideSettings::VidFormat, QString> VidSlideSettings::videoFormatNames()
{
    QMap<VidFormat, QString> frm;

    frm[AVI] = i18nc("Video Standard AVI", "AVI - Audio Video Interleave");
    frm[MKV] = i18nc("Video Standard MKV", "MKV - Matroska");
    frm[MP4] = i18nc("Video Standard MP4", "MP4 - MPEG-4");
    frm[MPG] = i18nc("Video Standard MPG", "MPG - MPEG-2");

    return frm;
}

QMap<VidSlideSettings::VidPlayer, QString> VidSlideSettings::videoPlayerNames()
{
    QMap<VidPlayer, QString> pla;

    pla[NOPLAYER] = i18nc("Video Effect NOPLAYER",   "None");
    pla[INTERNAL] = i18nc("Video Standard INTERNAL", "Internal");
    pla[DESKTOP]  = i18nc("Video Standard DESKTOP",  "Default from Desktop");

    return pla;
}

QStringList VidSlideSettings::defaultFFmpegSearchPaths() const
{
    QStringList defPaths;

#ifdef Q_OS_MACOS

    defPaths << QLatin1String("/usr/local/bin");

    // digiKam Bundle PKG install path

    defPaths << macOSBundlePrefix() + QLatin1String("bin");

    // Std Macports install path

    defPaths << QLatin1String("/opt/local/bin");

#endif

#ifdef Q_OS_WIN

    defPaths << QLatin1String("C:/Program Files/digiKam");

#endif

#ifdef Q_OS_UNIX

    defPaths << QLatin1String("/usr/bin");
    defPaths << QLatin1String("/usr/local/bin");
    defPaths << QLatin1String("/bin");

#endif

    return defPaths;
}

} // namespace Digikam

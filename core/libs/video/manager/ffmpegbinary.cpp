/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect FFmpeg binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ffmpegbinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

FFmpegBinary::FFmpegBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("ffmpeg"),
                   QLatin1String("FFmpeg"),
                   QLatin1String("https://ffmpeg.org/download.html"),
                   QString(),
                   QStringList(QLatin1String("-version")),
                   i18n("A complete, cross-platform solution to record, convert and stream audio and video.")
                  )
{
    setup();
}

QString FFmpegBinary::ffmpegToolBin()
{

#ifdef Q_OS_WIN

    return QLatin1String("ffmpeg.exe");

#else

    return QLatin1String("ffmpeg");

#endif

}

} // namespace Digikam

#include "moc_ffmpegbinary.cpp"

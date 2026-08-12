/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : FFmpeg launcher to encode frames as video
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ffmpeglauncher.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QDir>
#include <QDateTime>
#include <QEventLoop>
#include <QStringList>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"

namespace Digikam
{

FFmpegLauncher::FFmpegLauncher(QObject* const parent)
    : ProcessLauncher(parent)
{
}

void FFmpegLauncher::setSettings(VidSlideSettings* const settings)
{
    m_settings = settings;
}

void FFmpegLauncher::encodeFrames()
{
    // Run FFmpeg CLI to encode temporary JPEG frames
    // https://shotstack.io/learn/use-ffmpeg-to-convert-images-to-video/
    // ffmpeg -f concat -i fileslist.txt -b:v 200000 -r 25 -vcodec mpeg4 -y output.mp4

    qCDebug(DIGIKAM_GENERAL_LOG) << "Start encoding with FFmpeg";

    setWorkingDirectory(m_settings->outputDir);
    setProgram(m_settings->ffmpegPath);
    QStringList args;

    // Frames input

    args << QLatin1String("-f")
         << QLatin1String("concat")
         << QLatin1String("-i")
         << QDir::toNativeSeparators(m_settings->filesList);      // File list of frames to encode.

    // Audio input

    if (!m_settings->audioTrack.isEmpty())
    {
        args //<< QLatin1String("-stream_loop")
             //<< QLatin1String("-1")
             << QLatin1String("-i")
             << QDir::toNativeSeparators(m_settings->audioTrack)  // Audio file to use as soundtrack.
             << QLatin1String("-c:a")
             << QLatin1String("copy")                             // Do not re-encode
             << QLatin1String("-shortest");
    }

    // Metadata (not supported by all media containers - work fine with MP4)

    args << QLatin1String("-metadata")
         << QString::fromLatin1("date=\"%1\"").arg(QDateTime::currentDateTime().toString(Qt::ISODate))
         << QLatin1String("-metadata")
         << QString::fromLatin1("description=\"Encoded with digiKam VideoSlideShow tool version %1\"").arg(digiKamVersion());

    // Egualize video luminosity.
    // https://ffmpeg.org/ffmpeg-filters.html#tmidequalizer

    if (m_settings->equalize)
    {
        args << QLatin1String("-vf")
             << QString::fromLatin1("tmidequalizer=sigma=%1")
                                    .arg(m_settings->strength / 10.0);
    }

    // Video encoding

    args << QLatin1String("-b:v")                                 // Video bits-rate/
         << QString::number(m_settings->videoBitRate())
         << QLatin1String("-r")                                   // Video frames-rate.
         << QString::number(ceil(m_settings->videoFrameRate()))
         << QLatin1String("-vcodec")                              // Video codec.
         << m_settings->videoCodec()
         << QLatin1String("-y")                                   // Overwrite target.
         << QDir::toNativeSeparators(m_settings->outputFile);     // Target video stream.

    setArguments(args);
    startProcess();
}

QMap<QString, QString> FFmpegLauncher::supportedCodecs()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Get FFmpeg supported codecs";

    setConsoleTraces(false);
    setProgram(m_settings->ffmpegPath);
    setArguments(QStringList() << QLatin1String("-v")
                               << QLatin1String("quiet")
                               << QLatin1String("-codecs"));

    QEventLoop loop;

    connect(this, &ProcessLauncher::signalComplete,
            &loop, &QEventLoop::quit);

    startProcess();
    loop.exec();

    QMap<QString, QString> codecMap;        // name, features
    QString out        = output().section(QLatin1String("-------"), -1);
    QStringList codecs = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : std::as_const(codecs))
    {
        QStringList sections = line.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);

        if (sections.size() >= 2)
        {
            codecMap.insert(sections[1], sections[0]);
        }
    }

    return codecMap;
}

QMap<QString, QString> FFmpegLauncher::supportedFormats()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Get FFmpeg supported formats";

    setConsoleTraces(false);
    setProgram(m_settings->ffmpegPath);
    setArguments(QStringList() << QLatin1String("-v")
                               << QLatin1String("quiet")
                               << QLatin1String("-formats"));

    QEventLoop loop;

    connect(this, &ProcessLauncher::signalComplete,
            &loop, &QEventLoop::quit);

    startProcess();
    loop.exec();

    QMap<QString, QString> formatMap;        // name, features
    QString out         = output().section(QLatin1String("--"), -1);
    QStringList formats = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : std::as_const(formats))
    {
        QStringList sections = line.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);

        if (sections.size() >= 2)
        {
            formatMap.insert(sections[1], sections[0]);
        }
    }

    return formatMap;
}

QTime FFmpegLauncher::soundTrackLength(const QString& audioPath)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Get soundtrack length with FFmpeg";

    setConsoleTraces(false);
    setProgram(m_settings->ffmpegPath);
    setArguments(QStringList() << QLatin1String("-i")
                               << QDir::toNativeSeparators(audioPath));

    QEventLoop loop;

    connect(this, &ProcessLauncher::signalComplete,
            &loop, &QEventLoop::quit);

    startProcess();
    loop.exec();

    QStringList lines = output().split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : std::as_const(lines))
    {
        if (line.contains(QLatin1String("Duration")))
        {
            QStringList sections = line.simplified().split(QLatin1Char(','), Qt::SkipEmptyParts);

            if (!sections.isEmpty())
            {
                QStringList sections2 = sections[0].split(QLatin1String(" "), Qt::SkipEmptyParts);

                if (sections2.size() == 2)
                {
                    return QTime::fromString(sections2[1], QLatin1String("hh:mm:ss.zz"));
                }
            }
        }
    }

    return QTime();
}

} // namespace Digikam

#include "moc_ffmpeglauncher.cpp"

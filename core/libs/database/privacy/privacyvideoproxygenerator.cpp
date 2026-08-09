/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyvideoproxygenerator.h"

// C++ includes

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

// Qt includes

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QStandardPaths>

// Local includes

#include "privacyprocessrunner.h"

namespace Digikam
{

namespace
{

const QSize ProxyPixelSize(320, 180);
constexpr qsizetype MaximumProbeBytes = 1024 * 1024;
constexpr qsizetype MaximumProxyBytes = 8 * 1024 * 1024;
constexpr int ProbeTimeoutMs          = 30000;
constexpr int EncodeTimeoutMs         = 60000;
constexpr double MaximumProbeSeconds  = 21.0;

struct ContainerProfile
{
    QByteArray  canonicalName;
    QString     muxer;
    QString     codec;
    QStringList codecArguments;
    QSet<QString> acceptedProbeNames;
};

enum class SourceProbeStatus
{
    Success,
    Failed,
    ContainerMismatch
};

std::optional<ContainerProfile> profileForFileName(const QString& fileName)
{
    const QString suffix = QFileInfo(fileName).suffix().toLower();

    if (suffix == QLatin1String("mp4"))
    {
        return ContainerProfile {
            QByteArray("mp4"), QLatin1String("mp4"), QLatin1String("libx264"),
            { QLatin1String("-preset"), QLatin1String("ultrafast"),
              QLatin1String("-crf"), QLatin1String("38"),
              QLatin1String("-movflags"),
              QLatin1String("frag_keyframe+empty_moov+default_base_moof") },
            { QLatin1String("mov"), QLatin1String("mp4") }
        };
    }

    if (suffix == QLatin1String("mov"))
    {
        return ContainerProfile {
            QByteArray("mov"), QLatin1String("mov"), QLatin1String("libx264"),
            { QLatin1String("-preset"), QLatin1String("ultrafast"),
              QLatin1String("-crf"), QLatin1String("38"),
              QLatin1String("-movflags"),
              QLatin1String("frag_keyframe+empty_moov+default_base_moof") },
            { QLatin1String("mov"), QLatin1String("mp4") }
        };
    }

    if (suffix == QLatin1String("m4v"))
    {
        return ContainerProfile {
            QByteArray("m4v"), QLatin1String("m4v"), QLatin1String("mpeg4"),
            { QLatin1String("-q:v"), QLatin1String("12") },
            { QLatin1String("m4v") }
        };
    }

    if ((suffix == QLatin1String("3gp")) ||
        (suffix == QLatin1String("3g2")))
    {
        return ContainerProfile {
            suffix.toLatin1(), suffix, QLatin1String("mpeg4"),
            { QLatin1String("-q:v"), QLatin1String("12"),
              QLatin1String("-movflags"),
              QLatin1String("frag_keyframe+empty_moov+default_base_moof") },
            { QLatin1String("mov"), QLatin1String("mp4") }
        };
    }

    if (suffix == QLatin1String("mkv"))
    {
        return ContainerProfile {
            QByteArray("matroska"), QLatin1String("matroska"),
            QLatin1String("libx264"),
            { QLatin1String("-preset"), QLatin1String("ultrafast"),
              QLatin1String("-crf"), QLatin1String("38") },
            { QLatin1String("matroska") }
        };
    }

    if (suffix == QLatin1String("webm"))
    {
        return ContainerProfile {
            QByteArray("webm"), QLatin1String("webm"),
            QLatin1String("libvpx-vp9"),
            { QLatin1String("-deadline"), QLatin1String("good"),
              QLatin1String("-cpu-used"), QLatin1String("5"),
              QLatin1String("-b:v"), QLatin1String("0"),
              QLatin1String("-crf"), QLatin1String("40") },
            { QLatin1String("webm") }
        };
    }

    if ((suffix == QLatin1String("avi")) ||
        (suffix == QLatin1String("divx")))
    {
        return ContainerProfile {
            QByteArray("avi"), QLatin1String("avi"), QLatin1String("mpeg4"),
            { QLatin1String("-q:v"), QLatin1String("12") },
            { QLatin1String("avi") }
        };
    }

    if ((suffix == QLatin1String("mpeg")) ||
        (suffix == QLatin1String("mpg"))  ||
        (suffix == QLatin1String("mpe")))
    {
        return ContainerProfile {
            QByteArray("mpeg"), QLatin1String("mpeg"),
            QLatin1String("mpeg2video"),
            { QLatin1String("-q:v"), QLatin1String("12") },
            { QLatin1String("mpeg") }
        };
    }

    if (suffix == QLatin1String("vob"))
    {
        return ContainerProfile {
            QByteArray("vob"), QLatin1String("vob"),
            QLatin1String("mpeg2video"),
            { QLatin1String("-q:v"), QLatin1String("12") },
            { QLatin1String("mpeg") }
        };
    }

    if (suffix == QLatin1String("m2v"))
    {
        return ContainerProfile {
            QByteArray("mpegvideo"), QLatin1String("mpeg2video"),
            QLatin1String("mpeg2video"),
            { QLatin1String("-q:v"), QLatin1String("12") },
            { QLatin1String("mpegvideo") }
        };
    }

    if ((suffix == QLatin1String("mts")) ||
        (suffix == QLatin1String("m2ts")))
    {
        return ContainerProfile {
            QByteArray("mpegts"), QLatin1String("mpegts"),
            QLatin1String("libx264"),
            { QLatin1String("-preset"), QLatin1String("ultrafast"),
              QLatin1String("-crf"), QLatin1String("38") },
            { QLatin1String("mpegts") }
        };
    }

    if ((suffix == QLatin1String("wmv")) ||
        (suffix == QLatin1String("asf")))
    {
        return ContainerProfile {
            QByteArray("asf"), QLatin1String("asf"), QLatin1String("wmv2"),
            { QLatin1String("-q:v"), QLatin1String("12") },
            { QLatin1String("asf") }
        };
    }

    if ((suffix == QLatin1String("ogv")) || (suffix == QLatin1String("ogg")))
    {
        return ContainerProfile {
            QByteArray("ogg"), QLatin1String("ogg"), QLatin1String("libtheora"),
            { QLatin1String("-q:v"), QLatin1String("2") },
            { QLatin1String("ogg") }
        };
    }

    return std::nullopt;
}

bool isSafeSourcePath(const QString& path)
{
    if (path.isEmpty() || !QDir::isAbsolutePath(path) ||
        (QDir::cleanPath(path) != path) || path.contains(QChar::Null))
    {
        return false;
    }

    const QFileInfo info(path);

    return (info.isFile() && !info.isSymLink() && (info.size() >= 0) &&
            (info.canonicalFilePath() == info.absoluteFilePath()));
}

bool isSingleFileName(const QString& fileName)
{
    return (!fileName.isEmpty() && !fileName.contains(QChar::Null) &&
            !fileName.contains(QLatin1Char('/')) &&
            !fileName.contains(QLatin1Char('\\')) &&
            (QFileInfo(fileName).fileName() == fileName));
}

bool parseFiniteNumber(const QJsonValue& value, double* const number)
{
    if (!number)
    {
        return false;
    }

    bool ok = false;
    const double parsed = value.isDouble() ? value.toDouble()
                                          : value.toString().toDouble(&ok);

    if (value.isDouble())
    {
        ok = true;
    }

    if (!ok || !std::isfinite(parsed))
    {
        return false;
    }

    *number = parsed;

    return true;
}

PrivacyProcessSpec processSpec(const QString& program,
                               const QStringList& arguments,
                               int timeoutMs,
                               qsizetype maximumStdout)
{
    PrivacyProcessSpec spec;
    const QProcessEnvironment system = QProcessEnvironment::systemEnvironment();
    QProcessEnvironment environment;
    environment.insert(QLatin1String("LANG"), QLatin1String("C"));
    environment.insert(QLatin1String("LC_ALL"), QLatin1String("C"));
    environment.insert(QLatin1String("PATH"), QLatin1String("/usr/bin:/bin"));

    if (system.contains(QLatin1String("LD_LIBRARY_PATH")))
    {
        environment.insert(QLatin1String("LD_LIBRARY_PATH"),
                           system.value(QLatin1String("LD_LIBRARY_PATH")));
    }

    spec.program          = program;
    spec.arguments        = arguments;
    spec.environment      = environment;
    spec.startTimeoutMs   = 5000;
    spec.finishTimeoutMs  = timeoutMs;
    spec.maximumStdout    = maximumStdout;
    spec.maximumStderr    = 16384;
    spec.sensitiveOutput  = true;

    return spec;
}

QStringList quietProbeArguments()
{
    return {
        QLatin1String("-hide_banner"),
        QLatin1String("-loglevel"), QLatin1String("error")
    };
}

QStringList quietFfmpegArguments()
{
    QStringList arguments = quietProbeArguments();
    arguments.prepend(QLatin1String("-nostdin"));

    return arguments;
}

QJsonDocument successfulJson(PrivacyProcessRunner& runner,
                             const PrivacyProcessSpec& spec,
                             const QByteArray& input = {})
{
    PrivacyProcessResult process = runner.run(spec, input);

    if (!process.succeeded() || process.standardOutput.isEmpty())
    {
        return QJsonDocument();
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(process.standardOutput,
                                                            &error);

    if ((error.error != QJsonParseError::NoError) || !document.isObject())
    {
        return QJsonDocument();
    }

    return document;
}

SourceProbeStatus probeSource(PrivacyProcessRunner& runner,
                              const PrivacyVideoToolPaths& tools,
                              const QString& sourcePath,
                              const ContainerProfile& profile,
                              double* const durationSeconds)
{
    QStringList arguments = quietProbeArguments();
    arguments << QLatin1String("-show_entries")
              << QLatin1String("format=format_name,duration:stream=codec_type,duration")
              << QLatin1String("-of") << QLatin1String("json")
              << sourcePath;
    const QJsonDocument document = successfulJson(
        runner, processSpec(tools.ffprobe, arguments, ProbeTimeoutMs,
                            MaximumProbeBytes));

    if (document.isNull())
    {
        return SourceProbeStatus::Failed;
    }

    const QJsonObject root = document.object();
    const QJsonObject format = root.value(QLatin1String("format")).toObject();
    const QStringList formatNames = format.value(QLatin1String("format_name"))
                                          .toString().split(QLatin1Char(','));
    bool containerMatches = false;

    for (const QString& name : formatNames)
    {
        containerMatches = containerMatches || profile.acceptedProbeNames.contains(name);
    }

    if (!containerMatches)
    {
        return SourceProbeStatus::ContainerMismatch;
    }

    double duration = 0.0;

    if (!parseFiniteNumber(format.value(QLatin1String("duration")), &duration) ||
        (duration <= 0.0))
    {
        const QJsonArray streams = root.value(QLatin1String("streams")).toArray();

        for (const QJsonValue& streamValue : streams)
        {
            const QJsonObject stream = streamValue.toObject();

            if ((stream.value(QLatin1String("codec_type")).toString() ==
                 QLatin1String("video")) &&
                parseFiniteNumber(stream.value(QLatin1String("duration")), &duration) &&
                (duration > 0.0))
            {
                break;
            }
        }
    }

    if (!std::isfinite(duration) || (duration <= 0.0))
    {
        return SourceProbeStatus::Failed;
    }

    *durationSeconds = duration;

    return SourceProbeStatus::Success;
}

QList<double> probeKeyframes(PrivacyProcessRunner& runner,
                             const PrivacyVideoToolPaths& tools,
                             const QString& sourcePath)
{
    QStringList arguments = quietProbeArguments();
    arguments << QLatin1String("-select_streams") << QLatin1String("v:0")
              << QLatin1String("-read_intervals")
              << QString::fromLatin1("0%+%1").arg(MaximumProbeSeconds, 0, 'f', 0)
              << QLatin1String("-show_packets")
              << QLatin1String("-show_entries")
              << QLatin1String("packet=pts_time,flags")
              << QLatin1String("-of") << QLatin1String("json")
              << sourcePath;
    const QJsonDocument document = successfulJson(
        runner, processSpec(tools.ffprobe, arguments, ProbeTimeoutMs,
                            MaximumProbeBytes));
    QList<double> keyframes;

    if (document.isNull())
    {
        return keyframes;
    }

    const QJsonArray packets = document.object()
                                      .value(QLatin1String("packets")).toArray();

    for (const QJsonValue& packetValue : packets)
    {
        const QJsonObject packet = packetValue.toObject();

        if (!packet.value(QLatin1String("flags")).toString().contains(QLatin1Char('K')))
        {
            continue;
        }

        double timestamp = 0.0;

        if (parseFiniteNumber(packet.value(QLatin1String("pts_time")), &timestamp) &&
            (timestamp >= 0.0) && (timestamp <= MaximumProbeSeconds) &&
            (keyframes.isEmpty() ||
             (std::abs(keyframes.constLast() - timestamp) > 0.000001)))
        {
            keyframes << timestamp;
        }
    }

    std::sort(keyframes.begin(), keyframes.end());

    return keyframes;
}

QString decimalTimestamp(double seconds)
{
    return QString::number(seconds, 'f', 6);
}

QStringList outputArguments(const ContainerProfile& profile)
{
    QStringList arguments = {
        QLatin1String("-map_metadata"), QLatin1String("-1"),
        QLatin1String("-map_chapters"), QLatin1String("-1"),
        QLatin1String("-an"), QLatin1String("-sn"), QLatin1String("-dn"),
        QLatin1String("-frames:v"), QLatin1String("1"),
        QLatin1String("-threads"), QLatin1String("1"),
        QLatin1String("-c:v"), profile.codec,
        QLatin1String("-pix_fmt"), QLatin1String("yuv420p")
    };
    arguments << profile.codecArguments
              << QLatin1String("-f") << profile.muxer
              << QLatin1String("pipe:1");

    return arguments;
}

QByteArray encodePresentation(PrivacyProcessRunner& runner,
                              const PrivacyVideoToolPaths& tools,
                              const ContainerProfile& profile,
                              const QString& sourcePath,
                              std::optional<double> keyframeSeconds)
{
    QStringList arguments = quietFfmpegArguments();

    if (keyframeSeconds.has_value())
    {
        arguments << QLatin1String("-ss") << decimalTimestamp(*keyframeSeconds)
                  << QLatin1String("-i") << sourcePath
                  << QLatin1String("-map") << QLatin1String("0:v:0")
                  << QLatin1String("-vf")
                  << QLatin1String("scale=24:24:force_original_aspect_ratio=increase,"
                                   "crop=24:24,scale=320:180:flags=bilinear,"
                                   "eq=brightness=-0.12:saturation=0.70");
    }
    else
    {
        arguments << QLatin1String("-f") << QLatin1String("lavfi")
                  << QLatin1String("-i")
                  << QLatin1String("color=c=black:s=320x180:r=1")
                  << QLatin1String("-map") << QLatin1String("0:v:0");
    }

    arguments << outputArguments(profile);
    PrivacyProcessResult process = runner.run(
        processSpec(tools.ffmpeg, arguments, EncodeTimeoutMs, MaximumProxyBytes), {});

    if (!process.succeeded() || process.standardOutput.isEmpty() ||
        (process.standardOutput.size() > MaximumProxyBytes))
    {
        return {};
    }

    return std::move(process.standardOutput);
}

bool validateProxy(PrivacyProcessRunner& runner,
                   const PrivacyVideoToolPaths& tools,
                   const ContainerProfile& profile,
                   const QByteArray& encoded)
{
    QStringList arguments = quietProbeArguments();
    arguments << QLatin1String("-count_frames")
              << QLatin1String("-show_entries")
              << QLatin1String("format=format_name,duration:"
                               "stream=codec_type,width,height,nb_read_frames")
              << QLatin1String("-of") << QLatin1String("json")
              << QLatin1String("-i") << QLatin1String("pipe:0");
    const QJsonDocument document = successfulJson(
        runner, processSpec(tools.ffprobe, arguments, ProbeTimeoutMs,
                            MaximumProbeBytes), encoded);

    if (document.isNull())
    {
        return false;
    }

    const QJsonObject root = document.object();
    const QStringList formatNames = root.value(QLatin1String("format")).toObject()
                                        .value(QLatin1String("format_name"))
                                        .toString().split(QLatin1Char(','));
    bool containerMatches = false;

    for (const QString& name : formatNames)
    {
        containerMatches = containerMatches || profile.acceptedProbeNames.contains(name);
    }

    if (!containerMatches)
    {
        return false;
    }

    const QJsonArray streams = root.value(QLatin1String("streams")).toArray();

    if (streams.size() != 1)
    {
        return false;
    }

    const QJsonObject stream = streams.at(0).toObject();

    return ((stream.value(QLatin1String("codec_type")).toString() ==
             QLatin1String("video")) &&
            (stream.value(QLatin1String("width")).toInt() == ProxyPixelSize.width()) &&
            (stream.value(QLatin1String("height")).toInt() == ProxyPixelSize.height()) &&
            (stream.value(QLatin1String("nb_read_frames")).toString() ==
             QLatin1String("1")));
}

} // namespace

bool PrivacyVideoToolPaths::isValid() const
{
    return (!ffmpeg.isEmpty() && !ffprobe.isEmpty() &&
            QDir::isAbsolutePath(ffmpeg) && QDir::isAbsolutePath(ffprobe) &&
            (QDir::cleanPath(ffmpeg) == ffmpeg) &&
            (QDir::cleanPath(ffprobe) == ffprobe));
}

PrivacyVideoToolPaths PrivacyVideoToolPaths::discover()
{
    PrivacyVideoToolPaths paths;
    const QString applicationDirectory = QCoreApplication::applicationDirPath();
    const QStringList searchPaths = applicationDirectory.isEmpty()
                                  ? QStringList()
                                  : QStringList { applicationDirectory };
    paths.ffmpeg  = QStandardPaths::findExecutable(QLatin1String("ffmpeg"), searchPaths);
    paths.ffprobe = QStandardPaths::findExecutable(QLatin1String("ffprobe"), searchPaths);

    if (paths.ffmpeg.isEmpty())
    {
        paths.ffmpeg = QStandardPaths::findExecutable(QLatin1String("ffmpeg"));
    }

    if (paths.ffprobe.isEmpty())
    {
        paths.ffprobe = QStandardPaths::findExecutable(QLatin1String("ffprobe"));
    }

    return paths;
}

bool PrivacyVideoProxyResult::isValid() const
{
    if ((outcome == PrivacyVideoProxyOutcome::Failed) ||
        (error != PrivacyVideoProxyError::None) || encodedBytes.isEmpty() ||
        containerFormat.isEmpty() || (sha256.size() != 32) ||
        (pixelSize != ProxyPixelSize))
    {
        return false;
    }

    if (outcome == PrivacyVideoProxyOutcome::GeneratedSameContainer)
    {
        return ((fallbackReason == PrivacyVideoProxyFallbackReason::None) ||
                ((fallbackReason ==
                  PrivacyVideoProxyFallbackReason::KeyframeEncodeFailed) &&
                 (renderedPresentation == PrivacyVideoProxyPresentation::Blurred) &&
                 sourcePixelsUsed && (selectedKeyframeSeconds >= 0.0)));
    }

    return ((outcome == PrivacyVideoProxyOutcome::GeneratedGenericFallback) &&
            (renderedPresentation == PrivacyVideoProxyPresentation::Generic) &&
            (fallbackReason != PrivacyVideoProxyFallbackReason::None) &&
            !sourcePixelsUsed && (selectedKeyframeSeconds < 0.0));
}

PrivacyVideoProxyGenerator::PrivacyVideoProxyGenerator(
    PrivacyProcessRunner& runner, PrivacyVideoToolPaths toolPaths)
    : m_runner(runner),
      m_toolPaths(std::move(toolPaths))
{
}

QSize PrivacyVideoProxyGenerator::fixedPixelSize()
{
    return ProxyPixelSize;
}

QByteArray PrivacyVideoProxyGenerator::canonicalContainerForFileName(
    const QString& fileName)
{
    const std::optional<ContainerProfile> profile = profileForFileName(fileName);

    return profile.has_value() ? profile->canonicalName : QByteArray();
}

bool PrivacyVideoProxyGenerator::isSameContainerCandidate(const QString& fileName)
{
    return profileForFileName(fileName).has_value();
}

PrivacyVideoProxyResult PrivacyVideoProxyGenerator::generate(
    const PrivacyVideoProxyRequest& request) const
{
    PrivacyVideoProxyResult result;

    if (!isSafeSourcePath(request.sourcePath) ||
        !isSingleFileName(request.publicFileName) ||
        ((request.presentation != PrivacyVideoProxyPresentation::Generic) &&
         (request.presentation != PrivacyVideoProxyPresentation::Blurred)))
    {
        return result;
    }

    const std::optional<ContainerProfile> profile =
        profileForFileName(request.publicFileName);

    if (!profile.has_value())
    {
        result.error = PrivacyVideoProxyError::UnsupportedContainer;

        return result;
    }

    if (!m_toolPaths.isValid())
    {
        result.error = PrivacyVideoProxyError::ToolUnavailable;

        return result;
    }

    QByteArray encoded;
    std::optional<double> selectedKeyframe;
    bool encodedOutputInvalid = false;
    const auto encodeValidated = [this, &request, &profile,
                                  &encodedOutputInvalid](
        std::optional<double> keyframeSeconds)
    {
        QByteArray candidate = encodePresentation(
            m_runner, m_toolPaths, *profile, request.sourcePath,
            keyframeSeconds);

        if (!candidate.isEmpty() &&
            !validateProxy(m_runner, m_toolPaths, *profile, candidate))
        {
            encodedOutputInvalid = true;
            candidate.clear();
        }

        return candidate;
    };

    if (request.presentation == PrivacyVideoProxyPresentation::Blurred)
    {
        double duration = 0.0;

        const SourceProbeStatus probeStatus = probeSource(
            m_runner, m_toolPaths, request.sourcePath, *profile, &duration);

        if (probeStatus != SourceProbeStatus::Success)
        {
            result.fallbackReason =
                (probeStatus == SourceProbeStatus::ContainerMismatch)
                ? PrivacyVideoProxyFallbackReason::SourceContainerMismatch
                : PrivacyVideoProxyFallbackReason::SourceProbeFailed;
        }
        else
        {
            const QList<double> keyframes = probeKeyframes(
                m_runner, m_toolPaths, request.sourcePath);

            if (keyframes.isEmpty())
            {
                result.fallbackReason = PrivacyVideoProxyFallbackReason::NoUsableKeyframe;
            }
            else
            {
                const double target = std::min(10.0, duration / 2.0);
                const auto nearest = std::min_element(
                    keyframes.cbegin(), keyframes.cend(),
                    [target](double first, double second)
                    {
                        return std::abs(first - target) < std::abs(second - target);
                });
                selectedKeyframe = *nearest;
                encoded = encodeValidated(selectedKeyframe);

                if (encoded.isEmpty())
                {
                    result.fallbackReason =
                        PrivacyVideoProxyFallbackReason::KeyframeEncodeFailed;
                    const std::optional<double> secondKeyframe =
                        (keyframes.size() >= 2) ? std::optional<double>(keyframes.at(1))
                                               : std::nullopt;

                    if (secondKeyframe.has_value() &&
                        (std::abs(*secondKeyframe - *selectedKeyframe) > 0.000001))
                    {
                        selectedKeyframe = secondKeyframe;
                        encoded = encodeValidated(selectedKeyframe);
                    }
                }
            }
        }
    }

    if (encoded.isEmpty())
    {
        selectedKeyframe.reset();
        encoded = encodeValidated(std::nullopt);
        result.renderedPresentation = PrivacyVideoProxyPresentation::Generic;
        result.sourcePixelsUsed = false;

        if (encoded.isEmpty())
        {
            result.error = encodedOutputInvalid
                         ? PrivacyVideoProxyError::EncodedOutputInvalid
                         : PrivacyVideoProxyError::EncodeFailed;

            return result;
        }

        result.outcome = (result.fallbackReason == PrivacyVideoProxyFallbackReason::None)
                       ? PrivacyVideoProxyOutcome::GeneratedSameContainer
                       : PrivacyVideoProxyOutcome::GeneratedGenericFallback;
    }
    else
    {
        result.outcome = PrivacyVideoProxyOutcome::GeneratedSameContainer;
        result.renderedPresentation = PrivacyVideoProxyPresentation::Blurred;
        result.sourcePixelsUsed = true;
    }

    result.encodedBytes = std::move(encoded);
    result.containerFormat = profile->canonicalName;
    result.sha256 = QCryptographicHash::hash(result.encodedBytes,
                                             QCryptographicHash::Sha256);
    result.pixelSize = ProxyPixelSize;
    result.selectedKeyframeSeconds = selectedKeyframe.value_or(-1.0);
    result.error = PrivacyVideoProxyError::None;

    return result;
}

} // namespace Digikam

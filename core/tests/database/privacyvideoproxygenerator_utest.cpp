/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ includes

#include <memory>

// Qt includes

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacyprocessrunner.h"
#include "privacyvideoproxygenerator.h"

using namespace Digikam;

namespace
{

class FakeProcessHandle : public PrivacyProcessHandle
{
public:

    FakeProcessHandle(bool started, PrivacyProcessStatus status, int exitCode,
                      QByteArray standardOutput, bool sensitive)
        : m_started(started)
    {
        m_result.status = status;
        m_result.exitCode = exitCode;
        m_result.standardOutput = std::move(standardOutput);
        m_result.sensitiveOutput = sensitive;
    }

    bool started() const override
    {
        return m_started;
    }

    bool isRunning() override
    {
        return false;
    }

    PrivacyProcessResult waitForFinished(int) override
    {
        return std::move(m_result);
    }

    void terminate() override
    {
    }

private:

    bool                 m_started = false;
    PrivacyProcessResult m_result;
};

QByteArray jsonBytes(const QJsonObject& object)
{
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QString argumentAfter(const QStringList& arguments, const QString& option)
{
    const qsizetype index = arguments.indexOf(option);

    return ((index >= 0) && ((index + 1) < arguments.size()))
           ? arguments.at(index + 1) : QString();
}

QString lastArgumentAfter(const QStringList& arguments, const QString& option)
{
    const qsizetype index = arguments.lastIndexOf(option);

    return ((index >= 0) && ((index + 1) < arguments.size()))
           ? arguments.at(index + 1) : QString();
}

QString probeNameForMuxer(const QString& muxer)
{
    if ((muxer == QLatin1String("mp4")) || (muxer == QLatin1String("mov")) ||
        (muxer == QLatin1String("3gp")) || (muxer == QLatin1String("3g2")))
    {
        return QLatin1String("mov,mp4,m4a,3gp,3g2,mj2");
    }

    if ((muxer == QLatin1String("mpeg")) || (muxer == QLatin1String("vob")))
    {
        return QLatin1String("mpeg");
    }

    if (muxer == QLatin1String("mpeg2video"))
    {
        return QLatin1String("mpegvideo");
    }

    return muxer;
}

class FakeProcessRunner : public PrivacyProcessRunner
{
public:

    std::unique_ptr<PrivacyProcessHandle> start(
        const PrivacyProcessSpec& spec, const QByteArray& standardInput) override
    {
        specs << spec;
        inputs << standardInput;

        if (spec.program == ffprobePath)
        {
            if (spec.arguments.contains(QLatin1String("-count_frames")))
            {
                const QList<QByteArray> parts = standardInput.split('|');
                const QString muxer = (parts.size() >= 2)
                                    ? QString::fromLatin1(parts.at(1)) : QString();
                QJsonObject stream {
                    { QLatin1String("codec_type"), QLatin1String("video") },
                    { QLatin1String("width"), validationFails ? 640 : 320 },
                    { QLatin1String("height"), 180 },
                    { QLatin1String("nb_read_frames"), QLatin1String("1") }
                };
                QJsonObject root {
                    { QLatin1String("format"), QJsonObject {
                        { QLatin1String("format_name"), probeNameForMuxer(muxer) }
                    } },
                    { QLatin1String("streams"), QJsonArray { stream } }
                };

                return success(spec, jsonBytes(root));
            }

            if (spec.arguments.contains(QLatin1String("-show_packets")))
            {
                if (keyframeProbeFails)
                {
                    return failure(spec);
                }

                QJsonArray packets;

                for (double timestamp : keyframes)
                {
                    packets.append(QJsonObject {
                        { QLatin1String("pts_time"),
                          QString::number(timestamp, 'f', 6) },
                        { QLatin1String("flags"), QLatin1String("K__") }
                    });
                }

                return success(spec, jsonBytes(QJsonObject {
                    { QLatin1String("packets"), packets }
                }));
            }

            if (sourceProbeFails)
            {
                return failure(spec);
            }

            return success(spec, jsonBytes(QJsonObject {
                { QLatin1String("format"), QJsonObject {
                    { QLatin1String("format_name"), sourceFormat },
                    { QLatin1String("duration"),
                      QString::number(durationSeconds, 'f', 6) }
                } },
                { QLatin1String("streams"), QJsonArray {
                    QJsonObject {
                        { QLatin1String("codec_type"), QLatin1String("video") },
                        { QLatin1String("duration"),
                          QString::number(durationSeconds, 'f', 6) }
                    }
                } }
            }));
        }

        if (spec.program == ffmpegPath)
        {
            const QString muxer = lastArgumentAfter(spec.arguments,
                                                     QLatin1String("-f"));
            const QString timestamp = argumentAfter(spec.arguments,
                                                     QLatin1String("-ss"));
            const bool generic = spec.arguments.contains(QLatin1String("lavfi"));

            if ((generic && genericEncodeFails) ||
                (!generic && failedKeyframes.contains(timestamp)))
            {
                return failure(spec);
            }

            const QByteArray output = QByteArray("FAKEVIDEO|") + muxer.toLatin1() +
                                      '|' + (generic ? QByteArray("generic")
                                                     : QByteArray("blurred")) +
                                      '|' + timestamp.toLatin1();

            return success(spec, output);
        }

        return std::make_unique<FakeProcessHandle>(
            false, PrivacyProcessStatus::FailedToStart, -1, QByteArray(),
            spec.sensitiveOutput);
    }

    std::unique_ptr<PrivacyProcessHandle> success(
        const PrivacyProcessSpec& spec, const QByteArray& output)
    {
        return std::make_unique<FakeProcessHandle>(
            true, PrivacyProcessStatus::Exited, 0, output, spec.sensitiveOutput);
    }

    std::unique_ptr<PrivacyProcessHandle> failure(const PrivacyProcessSpec& spec)
    {
        return std::make_unique<FakeProcessHandle>(
            true, PrivacyProcessStatus::Exited, 1, QByteArray(),
            spec.sensitiveOutput);
    }

public:

    const QString ffmpegPath = QLatin1String("/synthetic/tools/ffmpeg");
    const QString ffprobePath = QLatin1String("/synthetic/tools/ffprobe");
    QList<PrivacyProcessSpec> specs;
    QList<QByteArray> inputs;
    QList<double> keyframes { 0.0, 4.0, 7.0, 12.0 };
    QSet<QString> failedKeyframes;
    QString sourceFormat = QLatin1String("mov,mp4,m4a,3gp,3g2,mj2");
    double durationSeconds = 12.0;
    bool sourceProbeFails = false;
    bool keyframeProbeFails = false;
    bool genericEncodeFails = false;
    bool validationFails = false;
};

PrivacyVideoToolPaths toolPaths(const FakeProcessRunner& runner)
{
    return { runner.ffmpegPath, runner.ffprobePath };
}

QString createSource(QTemporaryDir& directory,
                     const QString& name = QLatin1String("synthetic.mp4"))
{
    const QString path = directory.filePath(name);
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
        (file.write("synthetic-video-input") != 21) || !file.flush())
    {
        return QString();
    }

    return path;
}

PrivacyVideoProxyRequest requestFor(
    const QString& sourcePath, const QString& publicName,
    PrivacyVideoProxyPresentation presentation)
{
    PrivacyVideoProxyRequest request;
    request.sourcePath = sourcePath;
    request.publicFileName = publicName;
    request.presentation = presentation;

    return request;
}

} // namespace

class PrivacyVideoProxyGeneratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGenericUsesNoSourcePixelsAcrossSupportedContainers();
    void testBlurredChoosesNearestBoundedKeyframe();
    void testBlurredFallsBackToSecondKeyframe();
    void testBlurredFallsBackToGenericBlack();
    void testProbeFailuresFallBackWithoutSourcePixels();
    void testInvalidAndUnsupportedRequestsFailClosed();
    void testInvalidEncodedOutputFailsClosed();
    void testRealToolsGenerateValidSameContainerProxies();
};

void PrivacyVideoProxyGeneratorTest::
     testGenericUsesNoSourcePixelsAcrossSupportedContainers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());
    const QStringList names = {
        QLatin1String("proxy.mp4"), QLatin1String("proxy.mov"),
        QLatin1String("proxy.m4v"), QLatin1String("proxy.mkv"),
        QLatin1String("proxy.webm"), QLatin1String("proxy.avi"),
        QLatin1String("proxy.divx"), QLatin1String("proxy.ogv"),
        QLatin1String("proxy.mpeg"), QLatin1String("proxy.mpg"),
        QLatin1String("proxy.mpe"), QLatin1String("proxy.vob"),
        QLatin1String("proxy.m2v"), QLatin1String("proxy.mts"),
        QLatin1String("proxy.m2ts"), QLatin1String("proxy.wmv"),
        QLatin1String("proxy.asf"), QLatin1String("proxy.3gp"),
        QLatin1String("proxy.3g2")
    };

    for (const QString& publicName : names)
    {
        FakeProcessRunner runner;
        PrivacyVideoProxyGenerator generator(runner, toolPaths(runner));
        const PrivacyVideoProxyResult result = generator.generate(requestFor(
            sourcePath, publicName, PrivacyVideoProxyPresentation::Generic));

        QVERIFY2(result.isValid(), qPrintable(publicName));
        QCOMPARE(result.outcome,
                 PrivacyVideoProxyOutcome::GeneratedSameContainer);
        QCOMPARE(result.renderedPresentation,
                 PrivacyVideoProxyPresentation::Generic);
        QVERIFY(!result.sourcePixelsUsed);
        QCOMPARE(result.selectedKeyframeSeconds, -1.0);
        QCOMPARE(result.pixelSize, PrivacyVideoProxyGenerator::fixedPixelSize());
        QCOMPARE(result.sha256.size(), 32);
        QCOMPARE(runner.specs.size(), 2);
        QCOMPARE(runner.specs.at(0).program, runner.ffmpegPath);
        QVERIFY(runner.specs.at(0).arguments.contains(QLatin1String("lavfi")));
        QVERIFY(!runner.specs.at(0).arguments.contains(sourcePath));
        QCOMPARE(runner.specs.at(0).environment.value(QLatin1String("LC_ALL")),
                 QLatin1String("C"));
        QCOMPARE(runner.specs.at(0).environment.value(QLatin1String("PATH")),
                 QLatin1String("/usr/bin:/bin"));
        QVERIFY(!runner.specs.at(0).environment.contains(QLatin1String("FFREPORT")));
        QCOMPARE(runner.specs.at(1).program, runner.ffprobePath);
        QCOMPARE(runner.inputs.at(1), result.encodedBytes);
    }
}

void PrivacyVideoProxyGeneratorTest::testBlurredChoosesNearestBoundedKeyframe()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());
    FakeProcessRunner runner;
    PrivacyVideoProxyGenerator generator(runner, toolPaths(runner));
    const PrivacyVideoProxyResult result = generator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Blurred));

    QVERIFY(result.isValid());
    QCOMPARE(result.outcome, PrivacyVideoProxyOutcome::GeneratedSameContainer);
    QCOMPARE(result.renderedPresentation, PrivacyVideoProxyPresentation::Blurred);
    QVERIFY(result.sourcePixelsUsed);
    QCOMPARE(result.selectedKeyframeSeconds, 7.0);
    QCOMPARE(argumentAfter(runner.specs.at(2).arguments, QLatin1String("-ss")),
             QLatin1String("7.000000"));
    QCOMPARE(argumentAfter(runner.specs.at(1).arguments,
                           QLatin1String("-read_intervals")),
             QLatin1String("0%+21"));
}

void PrivacyVideoProxyGeneratorTest::testBlurredFallsBackToSecondKeyframe()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());
    FakeProcessRunner runner;
    runner.durationSeconds = 30.0;
    runner.keyframes = { 0.0, 3.0, 11.0 };
    runner.failedKeyframes.insert(QLatin1String("11.000000"));
    PrivacyVideoProxyGenerator generator(runner, toolPaths(runner));
    const PrivacyVideoProxyResult result = generator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Blurred));

    QVERIFY(result.isValid());
    QCOMPARE(result.renderedPresentation, PrivacyVideoProxyPresentation::Blurred);
    QCOMPARE(result.selectedKeyframeSeconds, 3.0);
    QCOMPARE(argumentAfter(runner.specs.at(2).arguments, QLatin1String("-ss")),
             QLatin1String("11.000000"));
    QCOMPARE(argumentAfter(runner.specs.at(3).arguments, QLatin1String("-ss")),
             QLatin1String("3.000000"));
}

void PrivacyVideoProxyGeneratorTest::testBlurredFallsBackToGenericBlack()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());
    FakeProcessRunner runner;
    runner.durationSeconds = 10.0;
    runner.keyframes = { 0.0, 5.0 };
    runner.failedKeyframes.insert(QLatin1String("5.000000"));
    PrivacyVideoProxyGenerator generator(runner, toolPaths(runner));
    const PrivacyVideoProxyResult result = generator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Blurred));

    QVERIFY(result.isValid());
    QCOMPARE(result.outcome,
             PrivacyVideoProxyOutcome::GeneratedGenericFallback);
    QCOMPARE(result.fallbackReason,
             PrivacyVideoProxyFallbackReason::KeyframeEncodeFailed);
    QCOMPARE(result.renderedPresentation, PrivacyVideoProxyPresentation::Generic);
    QVERIFY(!result.sourcePixelsUsed);
    QCOMPARE(result.selectedKeyframeSeconds, -1.0);
    QCOMPARE(runner.specs.size(), 5);
    QVERIFY(runner.specs.at(3).arguments.contains(QLatin1String("lavfi")));
}

void PrivacyVideoProxyGeneratorTest::testProbeFailuresFallBackWithoutSourcePixels()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());

    FakeProcessRunner failedRunner;
    failedRunner.sourceProbeFails = true;
    PrivacyVideoProxyGenerator failedGenerator(failedRunner,
                                                toolPaths(failedRunner));
    const auto failed = failedGenerator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Blurred));
    QVERIFY(failed.isValid());
    QCOMPARE(failed.fallbackReason,
             PrivacyVideoProxyFallbackReason::SourceProbeFailed);
    QVERIFY(!failed.sourcePixelsUsed);

    FakeProcessRunner mismatchRunner;
    mismatchRunner.sourceFormat = QLatin1String("avi");
    PrivacyVideoProxyGenerator mismatchGenerator(mismatchRunner,
                                                  toolPaths(mismatchRunner));
    const auto mismatch = mismatchGenerator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Blurred));
    QVERIFY(mismatch.isValid());
    QCOMPARE(mismatch.fallbackReason,
             PrivacyVideoProxyFallbackReason::SourceContainerMismatch);
    QVERIFY(!mismatch.sourcePixelsUsed);

    FakeProcessRunner noKeyframeRunner;
    noKeyframeRunner.keyframes.clear();
    PrivacyVideoProxyGenerator noKeyframeGenerator(noKeyframeRunner,
                                                    toolPaths(noKeyframeRunner));
    const auto noKeyframe = noKeyframeGenerator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Blurred));
    QVERIFY(noKeyframe.isValid());
    QCOMPARE(noKeyframe.fallbackReason,
             PrivacyVideoProxyFallbackReason::NoUsableKeyframe);
    QVERIFY(!noKeyframe.sourcePixelsUsed);
}

void PrivacyVideoProxyGeneratorTest::testInvalidAndUnsupportedRequestsFailClosed()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());
    FakeProcessRunner runner;
    PrivacyVideoProxyGenerator generator(runner, toolPaths(runner));

    const auto unsupported = generator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mxf"),
        PrivacyVideoProxyPresentation::Generic));
    QVERIFY(!unsupported.isValid());
    QCOMPARE(unsupported.error, PrivacyVideoProxyError::UnsupportedContainer);
    QVERIFY(runner.specs.isEmpty());

    const auto nested = generator.generate(requestFor(
        sourcePath, QLatin1String("nested/synthetic.mp4"),
        PrivacyVideoProxyPresentation::Generic));
    QVERIFY(!nested.isValid());
    QCOMPARE(nested.error, PrivacyVideoProxyError::InvalidRequest);

    const QString linkPath = directory.filePath(QLatin1String("link.mp4"));
    QVERIFY(QFile::link(sourcePath, linkPath));
    const auto symlink = generator.generate(requestFor(
        linkPath, QLatin1String("link.mp4"),
        PrivacyVideoProxyPresentation::Generic));
    QVERIFY(!symlink.isValid());
    QCOMPARE(symlink.error, PrivacyVideoProxyError::InvalidRequest);

    PrivacyVideoProxyGenerator badTools(runner, PrivacyVideoToolPaths());
    const auto unavailable = badTools.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Generic));
    QVERIFY(!unavailable.isValid());
    QCOMPARE(unavailable.error, PrivacyVideoProxyError::ToolUnavailable);
}

void PrivacyVideoProxyGeneratorTest::testInvalidEncodedOutputFailsClosed()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = createSource(directory);
    QVERIFY(!sourcePath.isEmpty());
    FakeProcessRunner runner;
    runner.validationFails = true;
    PrivacyVideoProxyGenerator generator(runner, toolPaths(runner));
    const auto result = generator.generate(requestFor(
        sourcePath, QLatin1String("synthetic.mp4"),
        PrivacyVideoProxyPresentation::Generic));

    QVERIFY(!result.isValid());
    QCOMPARE(result.error, PrivacyVideoProxyError::EncodedOutputInvalid);
    QVERIFY(result.encodedBytes.isEmpty());
}

void PrivacyVideoProxyGeneratorTest::
     testRealToolsGenerateValidSameContainerProxies()
{
    const PrivacyVideoToolPaths paths = PrivacyVideoToolPaths::discover();

    if (!paths.isValid())
    {
        QSKIP("ffmpeg and ffprobe are not installed in this test environment");
    }

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = directory.filePath(QLatin1String("source.mp4"));
    QProcessPrivacyProcessRunner runner;
    PrivacyProcessSpec sourceSpec;
    sourceSpec.program = paths.ffmpeg;
    sourceSpec.arguments = {
        QLatin1String("-nostdin"), QLatin1String("-hide_banner"),
        QLatin1String("-loglevel"), QLatin1String("error"),
        QLatin1String("-f"), QLatin1String("lavfi"),
        QLatin1String("-i"), QLatin1String("testsrc2=s=160x96:r=2"),
        QLatin1String("-t"), QLatin1String("12"),
        QLatin1String("-g"), QLatin1String("4"),
        QLatin1String("-keyint_min"), QLatin1String("4"),
        QLatin1String("-sc_threshold"), QLatin1String("0"),
        QLatin1String("-metadata"),
        QLatin1String("title=synthetic-private-metadata-5e8b"),
        QLatin1String("-c:v"), QLatin1String("libx264"),
        QLatin1String("-pix_fmt"), QLatin1String("yuv420p"),
        QLatin1String("-y"), sourcePath
    };
    sourceSpec.environment = QProcessEnvironment::systemEnvironment();
    sourceSpec.finishTimeoutMs = 30000;
    sourceSpec.maximumStdout = 1024;
    sourceSpec.maximumStderr = 16384;
    sourceSpec.sensitiveOutput = true;
    const PrivacyProcessResult sourceResult = runner.run(sourceSpec, {});
    QVERIFY(sourceResult.succeeded());

    PrivacyVideoProxyGenerator generator(runner, paths);
    const QStringList genericNames = {
        QLatin1String("proxy.mp4"), QLatin1String("proxy.mov"),
        QLatin1String("proxy.m4v"), QLatin1String("proxy.mkv"),
        QLatin1String("proxy.webm"), QLatin1String("proxy.avi"),
        QLatin1String("proxy.divx"), QLatin1String("proxy.ogv"),
        QLatin1String("proxy.mpeg"), QLatin1String("proxy.mpg"),
        QLatin1String("proxy.mpe"), QLatin1String("proxy.vob"),
        QLatin1String("proxy.m2v"), QLatin1String("proxy.mts"),
        QLatin1String("proxy.m2ts"), QLatin1String("proxy.wmv"),
        QLatin1String("proxy.asf"), QLatin1String("proxy.3gp"),
        QLatin1String("proxy.3g2")
    };

    for (const QString& publicName : genericNames)
    {
        const PrivacyVideoProxyResult result = generator.generate(requestFor(
            sourcePath, publicName, PrivacyVideoProxyPresentation::Generic));
        QVERIFY2(result.isValid(), qPrintable(publicName));
        QVERIFY(!result.sourcePixelsUsed);
        QVERIFY(!result.encodedBytes.contains("synthetic-private-metadata-5e8b"));
    }

    const PrivacyVideoProxyResult blurred = generator.generate(requestFor(
        sourcePath, QLatin1String("source.mp4"),
        PrivacyVideoProxyPresentation::Blurred));
    QVERIFY(blurred.isValid());
    QCOMPARE(blurred.renderedPresentation,
             PrivacyVideoProxyPresentation::Blurred);
    QVERIFY(blurred.sourcePixelsUsed);
    QCOMPARE(blurred.selectedKeyframeSeconds, 6.0);
    QVERIFY(!blurred.encodedBytes.contains("synthetic-private-metadata-5e8b"));
}

QTEST_GUILESS_MAIN(PrivacyVideoProxyGeneratorTest)

#include "privacyvideoproxygenerator_utest.moc"

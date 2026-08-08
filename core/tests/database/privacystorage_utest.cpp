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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "privacygocryptfsadapter.h"

using namespace Digikam;

namespace
{

class FakeMountProbe : public PrivacyMountStateProbe
{
public:

    State state(const QString&) const override
    {
        return current;
    }

    State current = State::NotMounted;
};

struct FakeProcessState
{
    bool running = false;
};

class FakeProcessHandle : public PrivacyProcessHandle
{
public:

    FakeProcessHandle(PrivacyProcessStatus status, int exitCode,
                      QByteArray output, bool sensitive,
                      std::shared_ptr<FakeProcessState> state = {})
        : m_status(status),
          m_exitCode(exitCode),
          m_output(std::move(output)),
          m_sensitive(sensitive),
          m_state(std::move(state))
    {
    }

    bool started() const override
    {
        return true;
    }

    bool isRunning() override
    {
        return (m_state && m_state->running);
    }

    PrivacyProcessResult waitForFinished(int) override
    {
        PrivacyProcessResult result;
        result.sensitiveOutput = m_sensitive;

        if (isRunning())
        {
            result.status = PrivacyProcessStatus::TimedOut;

            return result;
        }

        result.status         = m_status;
        result.exitCode       = m_exitCode;
        result.standardOutput = std::move(m_output);

        return result;
    }

    void terminate() override
    {
        if (m_state)
        {
            m_state->running = false;
        }
    }

private:

    PrivacyProcessStatus              m_status;
    int                               m_exitCode;
    QByteArray                        m_output;
    bool                              m_sensitive;
    std::shared_ptr<FakeProcessState> m_state;
};

class FakeProcessRunner : public PrivacyProcessRunner
{
public:

    explicit FakeProcessRunner(FakeMountProbe& mountProbe)
        : probe(mountProbe)
    {
    }

    std::unique_ptr<PrivacyProcessHandle> start(
        const PrivacyProcessSpec& spec, const QByteArray& standardInput) override
    {
        specs << spec;
        const QString secret = QString::fromUtf8(expectedPasswordLine).trimmed();
        secretLeaked = secretLeaked || spec.program.contains(secret);

        for (const QString& value : spec.arguments + spec.environment.toStringList())
        {
            secretLeaked = secretLeaked || value.contains(secret);
        }

        sawPasswordInput = sawPasswordInput || !standardInput.isEmpty();
        const bool passwordMatches = (standardInput == expectedPasswordLine);

        if (spec.program == gocryptfsPath)
        {
            if (spec.arguments.contains(QLatin1String("-version")))
            {
                return finished(QByteArray("gocryptfs ") + gocryptfsVersion + '\n', spec);
            }

            if (spec.arguments.contains(QLatin1String("-init")))
            {
                if (!passwordMatches || !hasPassfileStdin(spec))
                {
                    return failed(spec);
                }

                QFile config(spec.arguments.constLast() + QLatin1String("/gocryptfs.conf"));

                if (!config.open(QIODevice::WriteOnly) ||
                    (config.write(opaqueConfig) != opaqueConfig.size()))
                {
                    return failed(spec);
                }

                return finished({}, spec);
            }

            if (spec.arguments.contains(QLatin1String("-fg")))
            {
                if (!passwordMatches || !hasPassfileStdin(spec))
                {
                    return failed(spec);
                }

                probe.current            = mountStateOnStart;
                foreground               = std::make_shared<FakeProcessState>();
                foreground->running      = true;

                return std::make_unique<FakeProcessHandle>(
                    PrivacyProcessStatus::Exited, 0, QByteArray(), spec.sensitiveOutput,
                    foreground);
            }
        }

        if (spec.program == xrayPath)
        {
            if (spec.arguments.contains(QLatin1String("-version")))
            {
                return finished(QByteArray("gocryptfs-xray ") + xrayVersion + '\n', spec);
            }

            if (spec.arguments.contains(QLatin1String("-dumpmasterkey")))
            {
                QFile config(spec.arguments.constLast());

                if (!passwordMatches || !config.open(QIODevice::ReadOnly) ||
                    (config.readAll() != opaqueConfig))
                {
                    return failed(spec);
                }

                return finished(validMasterKeyOutput ? (QByteArray(64, 'a') + '\n')
                                                     : QByteArray("invalid\n"), spec);
            }
        }

        if (spec.program == fusermountPath)
        {
            if (spec.arguments.contains(QLatin1String("--version")))
            {
                return finished("fusermount3 version 3.14.0\n", spec);
            }

            if (spec.arguments.contains(QLatin1String("-u")))
            {
                ++unmountCalls;
                probe.current = PrivacyMountStateProbe::State::NotMounted;

                if (foreground)
                {
                    foreground->running = false;
                }

                return finished({}, spec);
            }
        }

        return failed(spec);
    }

    static bool hasPassfileStdin(const PrivacyProcessSpec& spec)
    {
        const int index = spec.arguments.indexOf(QLatin1String("-passfile"));

        return ((index >= 0) && ((index + 1) < spec.arguments.size()) &&
                (spec.arguments.at(index + 1) == QLatin1String("/dev/stdin")));
    }

    static std::unique_ptr<PrivacyProcessHandle> finished(
        const QByteArray& output, const PrivacyProcessSpec& spec)
    {
        return std::make_unique<FakeProcessHandle>(
            PrivacyProcessStatus::Exited, 0, output, spec.sensitiveOutput);
    }

    static std::unique_ptr<PrivacyProcessHandle> failed(const PrivacyProcessSpec& spec)
    {
        return std::make_unique<FakeProcessHandle>(
            PrivacyProcessStatus::Exited, 1, QByteArray(), spec.sensitiveOutput);
    }

public:

    FakeMountProbe&                   probe;
    QString                           gocryptfsPath;
    QString                           xrayPath;
    QString                           fusermountPath;
    QByteArray                        expectedPasswordLine = QByteArray("p\xC3\xA4ss\n");
    QByteArray                        opaqueConfig         = QByteArray("opaque config bytes\n");
    QByteArray                        gocryptfsVersion     = QByteArray("2.6.1");
    QByteArray                        xrayVersion          = QByteArray("2.6.1");
    QList<PrivacyProcessSpec>         specs;
    std::shared_ptr<FakeProcessState> foreground;
    bool                              validMasterKeyOutput = true;
    PrivacyMountStateProbe::State      mountStateOnStart =
        PrivacyMountStateProbe::State::Mounted;
    bool                              sawPasswordInput     = false;
    bool                              secretLeaked         = false;
    int                               unmountCalls         = 0;
};

bool makeExecutable(const QString& path)
{
    QFile file(path);

    return (file.open(QIODevice::WriteOnly) && (file.write("synthetic tool\n") > 0) &&
            file.setPermissions(QFileDevice::ReadOwner  | QFileDevice::WriteOwner |
                                QFileDevice::ExeOwner));
}

struct HarnessFixture
{
    HarnessFixture()
        : runner(probe)
    {
        if (!root.isValid())
        {
            return;
        }

        tools     = root.path() + QLatin1String("/tools");
        workspace = root.path() + QLatin1String("/workspace");
        valid     = QDir().mkdir(tools) && QDir().mkdir(workspace) &&
                    QFile::setPermissions(root.path(), QFileDevice::ReadOwner  |
                                                       QFileDevice::WriteOwner |
                                                       QFileDevice::ExeOwner) &&
                    QFile::setPermissions(tools, QFileDevice::ReadOwner  |
                                                 QFileDevice::WriteOwner |
                                                 QFileDevice::ExeOwner) &&
                    QFile::setPermissions(workspace, QFileDevice::ReadOwner  |
                                                     QFileDevice::WriteOwner |
                                                     QFileDevice::ExeOwner);

        runner.gocryptfsPath  = tools + QLatin1String("/gocryptfs");
        runner.xrayPath       = tools + QLatin1String("/gocryptfs-xray");
        runner.fusermountPath = tools + QLatin1String("/fusermount3");
        valid = valid && makeExecutable(runner.gocryptfsPath) &&
                makeExecutable(runner.xrayPath) && makeExecutable(runner.fusermountPath);
        paths = { runner.gocryptfsPath, runner.xrayPath, runner.fusermountPath };
    }

    std::unique_ptr<PrivacyGocryptfsStoreHarness> harness()
    {
        return std::make_unique<PrivacyGocryptfsStoreHarness>(
            runner, probe, paths, workspace);
    }

    QTemporaryDir             root;
    QString                   tools;
    QString                   workspace;
    FakeMountProbe            probe;
    FakeProcessRunner         runner;
    PrivacyGocryptfsToolPaths paths;
    bool                      valid = false;
};

PrivacyPassword testPassword()
{
    return PrivacyPassword::fromUnicode(QString::fromUtf8("p\xC3\xA4ss"));
}

} // namespace

class PrivacyStorageTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPasswordBoundary();
    void testOpaqueEnvelopeBoundary();
    void testCapabilitiesFailClosed();
    void testSyntheticLifecycle();
    void testMountFailuresFailClosed();
};

void PrivacyStorageTest::testPasswordBoundary()
{
    QCOMPARE(PrivacyPassword::encodingVersion(), QLatin1String("utf8-nfc-v1"));
    PrivacyPasswordError error = PrivacyPasswordError::None;
    PrivacyPassword password = PrivacyPassword::fromUnicode(QString::fromUtf8("e\xCC\x81"),
                                                             &error);
    QVERIFY(password.isValid());
    QCOMPARE(password.byteCount(), 2);
    QByteArray observed;
    QVERIFY(password.withStdinLine([&observed](const QByteArray& line)
    {
        observed = line;
        return true;
    }));
    QCOMPARE(observed, QByteArray("\xC3\xA9\n"));
    observed.fill('\0');

    QVERIFY(!PrivacyPassword::fromUnicode(QString(), &error).isValid());
    QVERIFY(error == PrivacyPasswordError::Empty);
    QString withNul = QLatin1String("a");
    withNul.append(QChar::Null);
    QVERIFY(!PrivacyPassword::fromUnicode(withNul, &error).isValid());
    QVERIFY(error == PrivacyPasswordError::ContainsNul);
    QVERIFY(!PrivacyPassword::fromUnicode(QLatin1String("a\rb"), &error).isValid());
    QVERIFY(error == PrivacyPasswordError::ContainsCarriageReturn);
    QVERIFY(!PrivacyPassword::fromUnicode(QLatin1String("a\nb"), &error).isValid());
    QVERIFY(error == PrivacyPasswordError::ContainsLineFeed);
    QVERIFY(!PrivacyPassword::fromUnicode(QString(1025, QLatin1Char('a')), &error).isValid());
    QVERIFY(error == PrivacyPasswordError::TooLong);
}

void PrivacyStorageTest::testOpaqueEnvelopeBoundary()
{
    PrivacyGocryptfsError error = PrivacyGocryptfsError::None;
    const QByteArray bytes("not JSON\0opaque", 15);
    const PrivacyGocryptfsEnvelope envelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
        QLatin1String("gocryptfs-config-v2"), bytes, &error);
    QVERIFY(envelope.isValid());
    QCOMPARE(envelope.size(), bytes.size());
    QVERIFY(!PrivacyGocryptfsEnvelope::fromOpaqueConfig(
        QLatin1String("unknown"), bytes, &error).isValid());
    QVERIFY(error == PrivacyGocryptfsError::InvalidEnvelope);
    QVERIFY(!PrivacyGocryptfsEnvelope::fromOpaqueConfig(
        QLatin1String("gocryptfs-config-v2"), QByteArray(), &error).isValid());
    QVERIFY(!PrivacyGocryptfsEnvelope::fromOpaqueConfig(
        QLatin1String("gocryptfs-config-v2"), QByteArray((1024 * 1024) + 1, 'x'),
        &error).isValid());
}

void PrivacyStorageTest::testCapabilitiesFailClosed()
{
    HarnessFixture fixture;
    QVERIFY(fixture.valid);
    auto harness = fixture.harness();
    PrivacyGocryptfsError error = PrivacyGocryptfsError::None;
    fixture.runner.xrayVersion = "2.6.2";
    QVERIFY(!harness->checkCapabilities(&error));
    QVERIFY(error == PrivacyGocryptfsError::UnsupportedToolVersion);

    fixture.paths.gocryptfs = QLatin1String("relative-gocryptfs");
    harness = fixture.harness();
    QVERIFY(!harness->checkCapabilities(&error));
    QVERIFY(error == PrivacyGocryptfsError::InvalidToolPath);
}

void PrivacyStorageTest::testSyntheticLifecycle()
{
    HarnessFixture fixture;
    QVERIFY(fixture.valid);
    auto harness = fixture.harness();
    PrivacyGocryptfsError error = PrivacyGocryptfsError::None;
    QVERIFY(harness->checkCapabilities(&error));
    PrivacyPassword password = testPassword();
    const QByteArray sentinel("category-sentinel-v1");
    PrivacyGocryptfsEnvelope envelope;
    QVERIFY(harness->createStore(password, sentinel, &envelope, &error));
    QVERIFY(envelope.isValid());
    QVERIFY(harness->validateEnvelope(envelope, password, &error));

    fixture.runner.validMasterKeyOutput = false;
    QVERIFY(!harness->validateEnvelope(envelope, password, &error));
    QVERIFY(error == PrivacyGocryptfsError::InvalidMasterKeyOutput);
    fixture.runner.validMasterKeyOutput = true;

    PrivacyPassword wrong = PrivacyPassword::fromUnicode(QLatin1String("wrong"));
    QVERIFY(!harness->validateEnvelope(envelope, wrong, &error));
    QVERIFY(error == PrivacyGocryptfsError::ProcessFailed);

    auto lease = harness->mountStore(password, sentinel, &error);
    QVERIFY(lease);
    QVERIFY(lease->isActive());
    QVERIFY(harness->unmountStore(*lease, &error));
    QVERIFY(!lease->isActive());
    QVERIFY(fixture.runner.sawPasswordInput);
    QVERIFY(!fixture.runner.secretLeaked);

    bool safeMount = false;
    for (const PrivacyProcessSpec& spec : fixture.runner.specs)
    {
        if (spec.arguments.contains(QLatin1String("-fg")))
        {
            safeMount = FakeProcessRunner::hasPassfileStdin(spec) &&
                        spec.arguments.contains(QLatin1String("-q")) &&
                        spec.arguments.contains(QLatin1String("-nosyslog")) &&
                        spec.arguments.contains(QLatin1String("-nodev")) &&
                        spec.arguments.contains(QLatin1String("-nosuid")) &&
                        spec.arguments.contains(QLatin1String("-noexec"));
        }
    }
    QVERIFY(safeMount);
}

void PrivacyStorageTest::testMountFailuresFailClosed()
{
    {
        HarnessFixture fixture;
        QVERIFY(fixture.valid);
        auto harness = fixture.harness();
        PrivacyGocryptfsError error = PrivacyGocryptfsError::None;
        QVERIFY(harness->checkCapabilities(&error));
        PrivacyPassword password = testPassword();
        PrivacyGocryptfsEnvelope envelope = PrivacyGocryptfsEnvelope::fromOpaqueConfig(
            QLatin1String("gocryptfs-config-v2"), QByteArray("old"));

        QVERIFY(!harness->createStore(password, QByteArray(), &envelope, &error));
        QVERIFY(error == PrivacyGocryptfsError::InvalidSentinel);
        QVERIFY(!envelope.isValid());
    }

    {
        HarnessFixture fixture;
        QVERIFY(fixture.valid);
        auto harness = fixture.harness();
        PrivacyGocryptfsError error = PrivacyGocryptfsError::None;
        QVERIFY(harness->checkCapabilities(&error));
        fixture.runner.mountStateOnStart = PrivacyMountStateProbe::State::Unknown;
        PrivacyPassword password = testPassword();
        PrivacyGocryptfsEnvelope envelope;

        QVERIFY(!harness->createStore(password, QByteArray("expected"), &envelope, &error));
        QVERIFY(error == PrivacyGocryptfsError::MountNotReady);
        QVERIFY(!envelope.isValid());
        QVERIFY(fixture.runner.unmountCalls == 1);
    }

    HarnessFixture fixture;
    QVERIFY(fixture.valid);
    auto harness = fixture.harness();
    PrivacyGocryptfsError error = PrivacyGocryptfsError::None;
    QVERIFY(harness->checkCapabilities(&error));
    PrivacyPassword password = testPassword();
    PrivacyGocryptfsEnvelope envelope;
    QVERIFY(harness->createStore(password, QByteArray("expected"), &envelope, &error));

    QVERIFY(!harness->mountStore(password, QByteArray("wrong"), &error));
    QVERIFY(error == PrivacyGocryptfsError::SentinelMismatch);
    QVERIFY(fixture.runner.unmountCalls >= 2);
    QVERIFY(fixture.probe.current == PrivacyMountStateProbe::State::NotMounted);

    fixture.probe.current = PrivacyMountStateProbe::State::Unknown;
    QVERIFY(!harness->mountStore(password, QByteArray("expected"), &error));
    QVERIFY(error == PrivacyGocryptfsError::UnsafeWorkspace);
}

QTEST_GUILESS_MAIN(PrivacyStorageTest)

#include "privacystorage_utest.moc"

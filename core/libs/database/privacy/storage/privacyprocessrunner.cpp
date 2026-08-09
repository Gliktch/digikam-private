/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyprocessrunner.h"

// C++ includes

#include <utility>

// Qt includes

#include <QElapsedTimer>
#include <QFileInfo>
#include <QProcess>

#if defined(Q_OS_LINUX)
#   include <signal.h>
#   include <sys/prctl.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

namespace Digikam
{

namespace
{

#if defined(Q_OS_LINUX)

void installParentDeathSignal(pid_t expectedParent)
{
    if ((::prctl(PR_SET_PDEATHSIG, SIGTERM) != 0) ||
        (::getppid() != expectedParent))
    {
        ::_exit(127);
    }
}

#endif

class PrivacyQProcess final : public QProcess
{
public:

#if defined(Q_OS_LINUX) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    bool terminateWithParent = false;
    pid_t expectedParent = 0;

protected:

    void setupChildProcess() override
    {
        QProcess::setupChildProcess();

        if (terminateWithParent)
        {
            installParentDeathSignal(expectedParent);
        }
    }

#endif
};

void overwriteByteArray(QByteArray& bytes)
{
    bytes.detach();
    volatile char* const data = bytes.data();

    for (qsizetype i = 0 ; i < bytes.size() ; ++i)
    {
        data[i] = 0;
    }

    bytes.clear();
    bytes.squeeze();
}

class QProcessPrivacyProcessHandle : public PrivacyProcessHandle
{
public:

    QProcessPrivacyProcessHandle(const PrivacyProcessSpec& spec,
                                 const QByteArray& standardInput)
        : m_spec(spec)
    {
        if (!QFileInfo(spec.program).isAbsolute()              ||
            (spec.startTimeoutMs <= 0)                         ||
            (spec.finishTimeoutMs <= 0)                        ||
            (spec.maximumStdout < 0)                           ||
            (spec.maximumStderr < 0))
        {
            return;
        }

        m_process.setProcessChannelMode(QProcess::SeparateChannels);
        m_process.setProcessEnvironment(spec.environment);
        m_process.setProgram(spec.program);
        m_process.setArguments(spec.arguments);

        if (spec.terminateWithParent)
        {
#if defined(Q_OS_LINUX)
            const pid_t expectedParent = ::getpid();

#   if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            m_process.setChildProcessModifier(
                [expectedParent]()
                {
                    installParentDeathSignal(expectedParent);
                });
#   else
            m_process.terminateWithParent = true;
            m_process.expectedParent = expectedParent;
#   endif
#else
            return;
#endif
        }

        m_process.start(QIODevice::ReadWrite);

        if (!m_process.waitForStarted(spec.startTimeoutMs))
        {
            drainOutput();

            return;
        }

        m_started = true;

        if (!standardInput.isEmpty() &&
            (m_process.write(standardInput) != standardInput.size()))
        {
            terminate();
            m_started = false;

            return;
        }

        m_process.closeWriteChannel();
        drainOutput();
    }

    ~QProcessPrivacyProcessHandle() override
    {
        terminate();

        if (m_spec.sensitiveOutput)
        {
            overwriteByteArray(m_stdout);
            overwriteByteArray(m_stderr);
        }
    }

    bool started() const override
    {
        return m_started;
    }

    bool isRunning() override
    {
        drainOutput();

        return (m_started && (m_process.state() != QProcess::NotRunning));
    }

    PrivacyProcessResult waitForFinished(int timeoutMs) override
    {
        PrivacyProcessResult result;
        result.sensitiveOutput = m_spec.sensitiveOutput;

        if (!m_started)
        {
            result.status = PrivacyProcessStatus::FailedToStart;
            moveOutputTo(result);

            return result;
        }

        const int effectiveTimeout = (timeoutMs < 0) ? m_spec.finishTimeoutMs : timeoutMs;
        QElapsedTimer timer;
        timer.start();

        while (m_process.state() != QProcess::NotRunning)
        {
            m_process.waitForReadyRead(25);
            drainOutput();

            if (m_outputLimitExceeded)
            {
                terminate();
                result.status = PrivacyProcessStatus::OutputLimitExceeded;
                moveOutputTo(result);

                return result;
            }

            if (timer.elapsed() >= effectiveTimeout)
            {
                terminate();
                result.status = PrivacyProcessStatus::TimedOut;
                moveOutputTo(result);

                return result;
            }
        }

        drainOutput();

        if (m_outputLimitExceeded)
        {
            result.status = PrivacyProcessStatus::OutputLimitExceeded;
            moveOutputTo(result);

            return result;
        }

        result.exitCode = m_process.exitCode();
        result.status   = (m_process.exitStatus() == QProcess::NormalExit)
                        ? PrivacyProcessStatus::Exited
                        : PrivacyProcessStatus::Crashed;
        moveOutputTo(result);

        return result;
    }

    void terminate() override
    {
        if (m_process.state() == QProcess::NotRunning)
        {
            drainOutput();

            return;
        }

        m_process.terminate();

        if (!m_process.waitForFinished(1000))
        {
            m_process.kill();
            m_process.waitForFinished(1000);
        }

        drainOutput();
    }

private:

    void appendBounded(QByteArray& destination, const QByteArray& source,
                       qsizetype maximumSize)
    {
        if (source.isEmpty() || m_outputLimitExceeded)
        {
            return;
        }

        if ((source.size() > maximumSize) ||
            (destination.size() > (maximumSize - source.size())))
        {
            m_outputLimitExceeded = true;

            return;
        }

        destination.append(source);
    }

    void drainOutput()
    {
        QByteArray standardOutput = m_process.readAllStandardOutput();
        QByteArray standardError  = m_process.readAllStandardError();
        appendBounded(m_stdout, standardOutput, m_spec.maximumStdout);
        appendBounded(m_stderr, standardError, m_spec.maximumStderr);

        if (m_spec.sensitiveOutput)
        {
            overwriteByteArray(standardOutput);
            overwriteByteArray(standardError);
        }
    }

    void moveOutputTo(PrivacyProcessResult& result)
    {
        result.standardOutput = std::move(m_stdout);
        result.standardError  = std::move(m_stderr);
    }

private:

    PrivacyProcessSpec m_spec;
    PrivacyQProcess    m_process;
    QByteArray         m_stdout;
    QByteArray         m_stderr;
    bool               m_started             = false;
    bool               m_outputLimitExceeded = false;
};

} // namespace

PrivacyProcessResult::PrivacyProcessResult(PrivacyProcessResult&& other) noexcept
    : status(other.status),
      exitCode(other.exitCode),
      standardOutput(std::move(other.standardOutput)),
      standardError(std::move(other.standardError)),
      sensitiveOutput(other.sensitiveOutput)
{
    other.sensitiveOutput = false;
}

PrivacyProcessResult& PrivacyProcessResult::operator=(PrivacyProcessResult&& other) noexcept
{
    if (this != &other)
    {
        clearOutput();
        status          = other.status;
        exitCode        = other.exitCode;
        standardOutput  = std::move(other.standardOutput);
        standardError   = std::move(other.standardError);
        sensitiveOutput = other.sensitiveOutput;
        other.sensitiveOutput = false;
    }

    return *this;
}

PrivacyProcessResult::~PrivacyProcessResult()
{
    clearOutput();
}

bool PrivacyProcessResult::succeeded() const
{
    return ((status == PrivacyProcessStatus::Exited) && (exitCode == 0));
}

void PrivacyProcessResult::clearOutput()
{
    if (sensitiveOutput)
    {
        overwriteByteArray(standardOutput);
        overwriteByteArray(standardError);
    }
    else
    {
        standardOutput.clear();
        standardError.clear();
    }
}

PrivacyProcessResult PrivacyProcessRunner::run(const PrivacyProcessSpec& spec,
                                               const QByteArray& standardInput)
{
    std::unique_ptr<PrivacyProcessHandle> handle = start(spec, standardInput);

    if (!handle || !handle->started())
    {
        PrivacyProcessResult result;
        result.status          = PrivacyProcessStatus::FailedToStart;
        result.sensitiveOutput = spec.sensitiveOutput;

        return result;
    }

    return handle->waitForFinished(spec.finishTimeoutMs);
}

std::unique_ptr<PrivacyProcessHandle> QProcessPrivacyProcessRunner::start(
    const PrivacyProcessSpec& spec, const QByteArray& standardInput)
{
    return std::make_unique<QProcessPrivacyProcessHandle>(spec, standardInput);
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <memory>

// Qt includes

#include <QByteArray>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyProcessStatus
{
    FailedToStart,
    Running,
    Exited,
    Crashed,
    TimedOut,
    OutputLimitExceeded
};

struct DIGIKAM_DATABASE_EXPORT PrivacyProcessSpec
{
    QString             program;
    QStringList         arguments;
    QProcessEnvironment environment;
    int                 startTimeoutMs  = 5000;
    int                 finishTimeoutMs = 30000;
    qsizetype           maximumStdout   = 8192;
    qsizetype           maximumStderr   = 8192;
    bool                sensitiveOutput = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProcessResult
{
public:

    PrivacyProcessResult() = default;
    PrivacyProcessResult(PrivacyProcessResult&& other) noexcept;
    PrivacyProcessResult& operator=(PrivacyProcessResult&& other) noexcept;
    ~PrivacyProcessResult();

    PrivacyProcessResult(const PrivacyProcessResult&)            = delete;
    PrivacyProcessResult& operator=(const PrivacyProcessResult&) = delete;

    bool succeeded() const;
    void clearOutput();

public:

    PrivacyProcessStatus status = PrivacyProcessStatus::FailedToStart;
    int                  exitCode = -1;
    QByteArray           standardOutput;
    QByteArray           standardError;
    bool                 sensitiveOutput = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProcessHandle
{
public:

    virtual ~PrivacyProcessHandle() = default;

    virtual bool started() const = 0;
    virtual bool isRunning() = 0;
    virtual PrivacyProcessResult waitForFinished(int timeoutMs = -1) = 0;
    virtual void terminate() = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyProcessRunner
{
public:

    virtual ~PrivacyProcessRunner() = default;

    /**
     * Starts one directly-executed process. Implementations must consume the
     * input synchronously and must not retain or log it after start returns.
     */
    virtual std::unique_ptr<PrivacyProcessHandle> start(const PrivacyProcessSpec& spec,
                                                         const QByteArray& standardInput) = 0;

    PrivacyProcessResult run(const PrivacyProcessSpec& spec,
                             const QByteArray& standardInput);
};

class DIGIKAM_DATABASE_EXPORT QProcessPrivacyProcessRunner : public PrivacyProcessRunner
{
public:

    std::unique_ptr<PrivacyProcessHandle> start(const PrivacyProcessSpec& spec,
                                                 const QByteArray& standardInput) override;
};

} // namespace Digikam

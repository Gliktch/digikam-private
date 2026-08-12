/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-04-13
 * Description : Generic process launcher with a capture of console output
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "processlauncher.h"

// Qt includes

#include <QByteArray>
#include <QProcess>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QProcessEnvironment>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN ProcessLauncher::Private
{
public:

    Private() = default;

public:

    QSharedPointer<QProcess> process       = nullptr;
    QStringList              args;
    QString                  prog;
    QString                  dir;
    bool                     consoleTraces = true;
    bool                     successFlag   = false;
    int                      exitCode      = 0;
    int                      timeOut       = 30000;           ///< in milli-seconds;
    qint64                   elapsed       = 0;
    QString                  output;
};

ProcessLauncher::ProcessLauncher(QObject* const parent)
    : QThread(parent),
      d      (new Private)
{
}

ProcessLauncher::~ProcessLauncher()
{
    if (!d->process.isNull())
    {
        d->process->kill();
    }

    delete d;
}

void ProcessLauncher::setProgram(const QString& prog)
{
    d->prog = prog;
}

void ProcessLauncher::setArguments(const QStringList& args)
{
    d->args = args;
}

void ProcessLauncher::setWorkingDirectory(const QString& dir)
{
    d->dir = dir;
}

void ProcessLauncher::setTimeOut(int msecs)
{
    d->timeOut = msecs;
}

int ProcessLauncher::exitCode() const
{
    return d->exitCode;
}

QString ProcessLauncher::output() const
{
    return d->output;
}

bool ProcessLauncher::success() const
{
    return d->successFlag;
}

qint64 ProcessLauncher::elapsedTime() const
{
    return d->elapsed;
}

void ProcessLauncher::setConsoleTraces(bool b)
{
    d->consoleTraces = b;
}

void ProcessLauncher::startProcess()
{
    start();
}

void ProcessLauncher::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("ProcessLauncher"));       // To customize thread name

    d->process.reset(new QProcess());
    d->process->setProcessChannelMode(QProcess::MergedChannels);
    d->process->setWorkingDirectory(d->dir);

    QProcessEnvironment env = adjustedEnvironmentForAppImage();
    d->process->setProcessEnvironment(env);

    connect(d->process.data(), SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));

    QElapsedTimer etimer;
    etimer.start();

    qCInfo(DIGIKAM_GENERAL_LOG) << "=== Starting process:" << d->prog << d->args;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->process->start(d->prog, d->args);

#else

    d->process->setProgram(d->prog);
    d->process->setArguments(d->args);
    d->process->start();

#endif

    if (d->process->waitForStarted(d->timeOut))
    {
        d->successFlag = d->process->waitForFinished(-1) && (d->process->exitStatus() == QProcess::NormalExit);
        d->elapsed     = etimer.elapsed();
        d->exitCode    = d->process->exitCode();

        msleep(500);  // Wait a little bit to flush the event loop.

        qCInfo(DIGIKAM_GENERAL_LOG) << "=== Process execution is complete!";
        qCInfo(DIGIKAM_GENERAL_LOG) << "> Process exit code        :" << d->exitCode;
        qCInfo(DIGIKAM_GENERAL_LOG) << "> Process elapsed time (ms):" << d->elapsed;
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "=== Process execution failed!";
    }

    Q_EMIT signalComplete(d->successFlag, d->exitCode);
}

void ProcessLauncher::slotReadyRead()
{
    QByteArray data = d->process->readAll();

    if (!data.isEmpty())
    {
        QString txt = QString::fromLocal8Bit(data.data(), data.size());
        d->output.append(txt);

        if (d->consoleTraces)
        {
            const auto list = txt.split(QLatin1Char('\n'));

            for (const QString& str : list)
            {
                if (!str.isEmpty())
                {
                    qCDebug(DIGIKAM_GENERAL_LOG)

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                       .noquote()

#endif

                       << str;
                }
            }
        }
    }
}

} // namespace Digikam

#include "moc_processlauncher.cpp"

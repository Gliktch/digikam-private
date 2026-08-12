/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-18
 * Description : Qt5 and Qt6 interface for exiftool - private container.
 *               Based on ZExifTool Qt interface published at 18 Feb 2021
 *               https://github.com/philvl/ZExifTool
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Philippe Vianney Liaud <philvl dot dev at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolprocess_p.h"

namespace Digikam
{

ExifToolProcess::Private::Private(ExifToolProcess* const q)
    : QObject(q),
      pp     (q)
{
    outAwait[0] = false;
    outAwait[1] = false;
    outReady[0] = false;
    outReady[1] = false;
}

void ExifToolProcess::Private::slotExecNextCmd()
{
    if (
        (pp->state() != QProcess::Running) ||
        writeChannelIsClosed
       )
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::execNextCmd(): ExifTool is not running";
        return;
    }

    QMutexLocker locker(&cmdMutex);

    if (cmdNumber || cmdQueue.isEmpty())
    {
        return;
    }

    // Clear QProcess buffers

    pp->readAllStandardOutput();
    pp->readAllStandardError();

    // Clear internal buffers

    outBuff[0]      = QByteArray();
    outBuff[1]      = QByteArray();
    outAwait[0]     = false;
    outAwait[1]     = false;
    outReady[0]     = false;
    outReady[1]     = false;

    // Exec Command

    execTimer.start();

    Command command = cmdQueue.takeFirst();
    cmdNumber       = command.id;
    cmdAction       = command.ac;

    pp->write(command.argsStr);
}

void ExifToolProcess::Private::readOutput(const QProcess::ProcessChannel channel)
{
    if (cmdNumber == 0)
    {
        return;
    }

    pp->setReadChannel(channel);

    while (pp->canReadLine() && !outReady[channel])
    {
        QByteArray line = pp->readLine();
/*
        qCDebug(DIGIKAM_METAENGINE_LOG) << channel << line;
*/
        if (!outAwait[channel])
        {
            if (line.startsWith(QByteArray("{await")))
            {
                if      (line.endsWith(QByteArray("}\n")))
                {
                    outAwait[channel] = line.mid(6, line.size() - 8).toInt();
                }
                else if (line.endsWith(QByteArray("}\r\n")))
                {
                    outAwait[channel] = line.mid(6, line.size() - 9).toInt();
                }
            }

            continue;
        }

        outBuff[channel] += line;

        if      (line.endsWith(QByteArray("{ready}\n")))
        {
            outBuff[channel].chop(8);
            outReady[channel] = true;

            break;
        }
        else if (line.endsWith(QByteArray("{ready}\r\n")))
        {
            outBuff[channel].chop(9);
            outReady[channel] = true;

            break;
        }
    }

    // Check if outputChannel and errorChannel are both ready

    if (
        !(outReady[QProcess::StandardOutput] &&
        outReady[QProcess::StandardError])
       )
    {
/*
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::readOutput(): ExifTool read channels are not ready";
*/
        return;
    }

    if (
        (cmdNumber != outAwait[QProcess::StandardOutput]) ||
        (cmdNumber != outAwait[QProcess::StandardError])
       )
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::readOutput: Sync error between CmdID("
                                           << cmdNumber
                                           << "), outChannel("
                                           << outAwait[0]
                                           << ") and errChannel("
                                           << outAwait[1]
                                           << ")";

        setProcessErrorAndEmit(QProcess::ReadError, i18n("Synchronization error between the channels"));
    }
    else
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::readOutput(): ExifTool command completed";

        setCommandResult(ExifToolProcess::COMMAND_RESULT);
    }

    Q_EMIT pp->signalExecNextCmd(); // Exec next command
}

void ExifToolProcess::Private::setProcessErrorAndEmit(QProcess::ProcessError error, const QString& description)
{
    processError = error;
    errorString  = description;

    setCommandResult(ExifToolProcess::ERROR_RESULT);
}

void ExifToolProcess::Private::setCommandResult(int cmdStatus)
{
    QMutexLocker locker(&mutex);

    ExifToolProcess::Result result;

    result.waitError = false;
    result.cmdStatus = cmdStatus;
    result.cmdAction = cmdAction;
    result.cmdNumber = cmdNumber;
    result.elapsed   = execTimer.elapsed();
    result.output    = outBuff[QProcess::StandardOutput];

    resultMap.insert(cmdNumber, result);

    Q_EMIT pp->signalExifToolResult(cmdNumber);

    cmdNumber        = 0;
    cmdAction        = ExifToolProcess::NO_ACTION;

    condVar.wakeAll();
}

} // namespace Digikam

#include "moc_exiftoolprocess_p.cpp"

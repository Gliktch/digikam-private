/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-18
 * Description : Qt5 and Qt6 interface for exiftool.
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

QPointer<ExifToolProcess> ExifToolProcess::internalPtr = QPointer<ExifToolProcess>();

ExifToolProcess::ExifToolProcess()
    : QProcess(nullptr),
      d       (new Private(this))
{
    setProcessEnvironment(adjustedEnvironmentForAppImage());
}

ExifToolProcess::~ExifToolProcess()
{
    internalPtr = nullptr;

    delete d;
}

bool ExifToolProcess::isCreated()
{
    return (!internalPtr.isNull());
}

ExifToolProcess* ExifToolProcess::instance()
{
    if (internalPtr.isNull())
    {
        internalPtr = new ExifToolProcess();
    }

    return internalPtr;
}

void ExifToolProcess::setExifToolProgram(const QString& etExePath)
{
    Q_EMIT signalChangeProgram(etExePath);
}

QString ExifToolProcess::getExifToolProgram() const
{
    return d->etExePath;
}

bool ExifToolProcess::startExifTool()
{
    // Check if ExifTool is starting or running

    if (state() != QProcess::NotRunning)
    {
        return true;
    }

    if (!checkExifToolProgram())
    {
        return false;
    }

    // Prepare command for ExifTool

    QString program = d->etExePath;
    QStringList args;

    if (!d->perlExePath.isEmpty())
    {
        program = d->perlExePath;
        args << d->etExePath;
    }

    //-- Advanced options

    args << QLatin1String("-stay_open");
    args << QLatin1String("true");

    //-- Other options

    args << QLatin1String("-@");
    args << QLatin1String("-");

    //-- Define common arguments

    args << QLatin1String("-common_args");

    //-- Use UTF-8 for file names

    args << QLatin1String("-charset");
    args << QLatin1String("filename=UTF8");

    args << QLatin1String("-charset");
    args << QLatin1String("iptc=UTF8");

    // Clear queue before start

    d->cmdQueue.clear();
    d->cmdNumber    = 0;
    d->cmdAction    = NO_ACTION;

    // Clear errors

    d->processError = QProcess::UnknownError;
    d->errorString.clear();

    // Start ExifTool process

    d->writeChannelIsClosed = false;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::start(): create new ExifTool instance:" << program << args;

    start(program, args, QProcess::ReadWrite);

    return waitForStarted(10000);
}

void ExifToolProcess::restartExifTool()
{
    shutDownExifTool();

    if (!startExifTool())
    {
        killExifTool();

        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool process cannot be started ("
                                          << getExifToolProgram() << ")";
    }
}

void ExifToolProcess::shutDownExifTool()
{
    if (state() == QProcess::Running)
    {
        // If process is in running state, close ExifTool normally

        qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::shutDown(): send ExifTool shutdown command...";

        d->cmdQueue.clear();
        write(QByteArray("-stay_open\nfalse\n"));
        d->writeChannelIsClosed = true;
        closeWriteChannel();

        if (!waitForFinished(5000))
        {
            // Otherwise, close ExifTool using OS system call
            // (WM_CLOSE [Windows] or SIGTERM [Unix])

            // Console applications on Windows that do not run an event loop,
            // or whose event loop does not handle the WM_CLOSE message,
            // can only be terminated by calling kill().

#ifdef Q_OS_WIN

            killExifTool();

#else

            qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::shutDown(): terminate ExifTool instance...";

            terminate();

#endif

        }
    }
}

void ExifToolProcess::killExifTool()
{
    if (state() == QProcess::Running)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::kill(): kill ExifTool instance...";

        kill();
    }
}

bool ExifToolProcess::exifToolAvailable() const
{
    return (state() == QProcess::Running);
}

bool ExifToolProcess::exifToolIsBusy() const
{
    return (d->cmdNumber ? true : false);
}

QProcess::ProcessError ExifToolProcess::exifToolError() const
{
    return d->processError;
}

QString ExifToolProcess::exifToolErrorString() const
{
    return d->errorString;
}

ExifToolProcess::Result ExifToolProcess::getExifToolResult(int cmdId) const
{
    QMutexLocker locker(&d->mutex);

    ExifToolProcess::Result result;
    result = d->resultMap.take(cmdId);

    return result;
}

ExifToolProcess::Result ExifToolProcess::waitForExifToolResult(int cmdId) const
{
    QMutexLocker locker(&d->mutex);

    bool ret         = d->condVar.wait(&d->mutex, 10000);

    ExifToolProcess::Result result;
    result           = d->resultMap.take(cmdId);
    result.waitError = !ret;

    return result;
}

int ExifToolProcess::command(const QByteArrayList& args, Action ac)
{
    if (
        (state() != QProcess::Running) ||
        d->writeChannelIsClosed        ||
        args.isEmpty()
       )
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifToolProcess::command(): cannot process command with ExifTool" << args;

        return 0;
    }

    // ThreadSafe incrementation of d->nextCmdId

    QMutexLocker locker(&d->cmdMutex);

    const int cmdId = d->nextCmdId;

    if (d->nextCmdId++ >= ExifToolProcess::Private::CMD_ID_MAX)
    {
        d->nextCmdId = ExifToolProcess::Private::CMD_ID_MIN;
    }

    // String representation of d->cmdId with leading zero -> constant size: 10 char

    const QByteArray cmdIdStr = QByteArray::number(cmdId).rightJustified(10, '0');

    // Build command string from args

    QByteArray cmdStr;

    for (const QByteArray& arg : EXIV2_AS_CONST(args))
    {
        cmdStr.append(arg + '\n');
    }

    //-- Advanced options

    cmdStr.append(QByteArray("-echo1\n{await") + cmdIdStr + QByteArray("}\n"));     // Echo text to stdout before processing is complete
    cmdStr.append(QByteArray("-echo2\n{await") + cmdIdStr + QByteArray("}\n"));     // Echo text to stderr before processing is complete

    if (
        cmdStr.contains(QByteArray("-q"))               ||
        cmdStr.toLower().contains(QByteArray("-quiet")) ||
        cmdStr.contains(QByteArray("-T"))               ||
        cmdStr.toLower().contains(QByteArray("-table"))
       )
    {
        cmdStr.append(QByteArray("-echo3\n{ready}\n"));                 // Echo text to stdout after processing is complete
    }

    cmdStr.append(QByteArray("-echo4\n{ready}\n"));                     // Echo text to stderr after processing is complete
    cmdStr.append(QByteArray("-execute\n"));                            // Execute command and echo {ready} to stdout after processing is complete

    // TODO: if -binary user, {ready} can not be present in the new line

    // Add command to queue

    Private::Command command;
    command.id      = cmdId;
    command.argsStr = cmdStr;
    command.ac      = ac;
    d->cmdQueue.append(command);

    // Exec cmd queue

    Q_EMIT signalExecNextCmd();

    return cmdId;
}

void ExifToolProcess::slotStarted()
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool process started";
}

void ExifToolProcess::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool process finished with code:"
                                    << exitCode << "and status" << exitStatus;

    QMutexLocker locker(&d->mutex);
    const auto keys = d->resultMap.keys();

    for (int cmdId : EXIV2_AS_CONST(keys))
    {
        d->resultMap[cmdId].cmdStatus = FINISH_RESULT;
    }

    d->condVar.wakeAll();
}

void ExifToolProcess::slotErrorOccurred(QProcess::ProcessError error)
{
    d->setProcessErrorAndEmit(error, errorString());
}

void ExifToolProcess::slotReadyReadStandardOutput()
{
    d->readOutput(QProcess::StandardOutput);
}

void ExifToolProcess::slotReadyReadStandardError()
{
    d->readOutput(QProcess::StandardError);
}

QString ExifToolProcess::exifToolBin() const
{

#ifdef Q_OS_WIN

    return QLatin1String("exiftool.exe");

#else

    return QLatin1String("exiftool");

#endif

}

bool ExifToolProcess::checkExifToolProgram() const
{
    // Check if Exiftool program exists and have execution permissions

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Path to ExifTool:" << d->etExePath;

    if (
        d->etExePath.isEmpty()                             ||
        (!QFile::exists(d->etExePath)                      ||
        !(QFile::permissions(d->etExePath) & QFile::ExeUser))
       )
    {
        d->processError = QProcess::FailedToStart;
        d->errorString  = i18n("ExifTool does not exists or exec permission is missing");

        return false;
    }

    // If perl path is defined, check if Perl program exists and have execution permissions

    if (
        !d->perlExePath.isEmpty()                            &&
        (!QFile::exists(d->perlExePath)                      ||
        !(QFile::permissions(d->perlExePath) & QFile::ExeUser))
       )
    {
        d->processError = QProcess::FailedToStart;
        d->errorString  = i18n("Perl does not exists or exec permission is missing");

        return false;
    }

    return true;
}

void ExifToolProcess::initExifTool()
{
    connect(this, &QProcess::started,
            this, &ExifToolProcess::slotStarted);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    connect(this, &QProcess::finished,
            this, &ExifToolProcess::slotFinished);

#else

    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ExifToolProcess::slotFinished);

#endif

    connect(this, &QProcess::errorOccurred,
            this, &ExifToolProcess::slotErrorOccurred);

    connect(this, &QProcess::readyReadStandardOutput,
            this, &ExifToolProcess::slotReadyReadStandardOutput);

    connect(this, &QProcess::readyReadStandardError,
            this, &ExifToolProcess::slotReadyReadStandardError);

    connect(this, &ExifToolProcess::signalExecNextCmd,
            d,    &ExifToolProcess::Private::slotExecNextCmd,
            Qt::QueuedConnection);

    connect(this, &ExifToolProcess::signalChangeProgram,
            this, &ExifToolProcess::slotChangeProgram,
            Qt::BlockingQueuedConnection);

    connect(MetaEngineSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotApplySettingsAndStart()),
            Qt::QueuedConnection);

    slotApplySettingsAndStart();
}

void ExifToolProcess::changeExifToolProgram(const QString& etExePath)
{
    d->etExePath = etExePath;

    if      (d->etExePath.isEmpty() || (d->etExePath == exifToolBin()))
    {

#ifdef Q_OS_WIN

        QStringList possiblePaths({QLatin1String("C:/Program Files/digiKam"),
                                   QLatin1String("C:/Program Files/digiKam/bin")});

        d->etExePath = QStandardPaths::findExecutable(exifToolBin(), possiblePaths);

#else

        d->etExePath = QStandardPaths::findExecutable(exifToolBin());

#endif

    }
    else if (QFileInfo(d->etExePath).isDir())
    {
        if (!d->etExePath.endsWith(QLatin1Char('/')))
        {
            d->etExePath.append(QLatin1Char('/'));
        }

        d->etExePath.append(exifToolBin());
    }
}

void ExifToolProcess::slotChangeProgram(const QString& etExePath)
{
    QString et(getExifToolProgram());

    changeExifToolProgram(etExePath);

    if (
        exifToolAvailable()        &&
        (et == getExifToolProgram())
       )
    {
        return;
    }

    restartExifTool();
}

void ExifToolProcess::slotApplySettingsAndStart()
{
    changeExifToolProgram(MetaEngineSettings::instance()->settings().exifToolPath);

    qCDebug(DIGIKAM_METAENGINE_LOG) << "ExifTool config path:" << getExifToolProgram();

    if (state() != QProcess::NotRunning)
    {
        return;
    }

    restartExifTool();
}

} // namespace Digikam

#include "moc_exiftoolprocess.cpp"

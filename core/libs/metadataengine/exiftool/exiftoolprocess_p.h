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

#pragma once

#include "exiftoolprocess.h"

// Qt includes

#include <QFile>
#include <QList>
#include <QMutex>
#include <QFileInfo>
#include <QByteArray>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QWaitCondition>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "metaenginesettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN ExifToolProcess::Private : public QObject
{
    Q_OBJECT

public:

    class Q_DECL_HIDDEN Command
    {
    public:

        Command() = default;

        int                     id      = 0;
        QByteArray              argsStr;
        ExifToolProcess::Action ac      = ExifToolProcess::NO_ACTION;
    };

public:

    explicit Private(ExifToolProcess* const q);

    void readOutput(const QProcess::ProcessChannel channel);
    void setProcessErrorAndEmit(QProcess::ProcessError error,
                                const QString& description);
    void setCommandResult(int cmdStatus);

public Q_SLOTS:

    void slotExecNextCmd();

public:

    static const int                   CMD_ID_MIN = 1;
    static const int                   CMD_ID_MAX = 2000000000;

    ExifToolProcess*                   pp                   = nullptr;
    QString                            etExePath;
    QString                            perlExePath;

    QElapsedTimer                      execTimer;
    QList<Command>                     cmdQueue;
    int                                cmdNumber            = 0;
    ExifToolProcess::Action            cmdAction            = ExifToolProcess::NO_ACTION;
    QMap<int, ExifToolProcess::Result> resultMap;

    int                                outAwait[2];         ///< [0] StandardOutput | [1] ErrorOutput
    bool                               outReady[2];         ///< [0] StandardOutput | [1] ErrorOutput
    QByteArray                         outBuff[2];          ///< [0] StandardOutput | [1] ErrorOutput

    bool                               writeChannelIsClosed = true;

    QProcess::ProcessError             processError         = QProcess::UnknownError;
    QString                            errorString;

    int                                nextCmdId            = CMD_ID_MIN;  ///< Unique identifier, even in a multi-instances or multi-thread environment

    QMutex                             cmdMutex;

    QMutex                             mutex;
    QWaitCondition                     condVar;
};

} // namespace Digikam

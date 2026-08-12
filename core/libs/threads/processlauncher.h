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

#pragma once

// Qt includes

#include <QThread>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ProcessLauncher : public QThread
{
    Q_OBJECT

public:

    explicit ProcessLauncher(QObject* const parent = nullptr);
    ~ProcessLauncher();

    void setProgram(const QString& prog);
    void setArguments(const QStringList& args);
    void setWorkingDirectory(const QString& dir);
    void setTimeOut(int msecs);

    /**
     * If turned on, all traces from the process are printed on the console.
     */
    void setConsoleTraces(bool b);

    /**
     * Start the process.
     */
    void startProcess();

    /**
     * Return the exit code from the process.
     */
    int exitCode()          const;

    /**
     * Return the elapsed time in ms to run the process.
     */
    qint64 elapsedTime()    const;

    /**
     * Return the process output as string.
     */
    QString output()        const;

    /**
     * Return true if the process is started and completed without error.
     */
    bool success()          const;

Q_SIGNALS:

    void signalComplete(bool success, int exitCode);

private:

    void run() override;

private Q_SLOTS:

    void slotReadyRead();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

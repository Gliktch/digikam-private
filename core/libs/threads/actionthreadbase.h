/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : Low level threads management for batch processing on multi-core
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2014 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2012 by Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QHash>
#include <QThread>

// Local includes

#include "digikam_export.h"
#include "actionjob.h"

namespace Digikam
{

class DIGIKAM_EXPORT ActionThreadBase : public QThread
{
    Q_OBJECT

public:

    explicit ActionThreadBase(QObject* const parent = nullptr);
    ~ActionThreadBase() override;

    /**
     * @brief Adjust the thread expiry timeout value in milliseconds.
     */
    void setExpiryTimeout(int timeout);

    /**
     * @return the thread expiry timeout value in milliseconds.
     */
    int expiryTimeout()             const;

    /**
     * @brief Adjust maximum number of threads used to parallelize collection of job processing.
     */
    void setMaximumNumberOfThreads(int n);

    /**
     * @return the maximum number of threads used to parallelize collection of job processing.
     */
    int  maximumNumberOfThreads()   const;

    /**
     * @brief Reset maximum number of threads used to parallelize collection of job processing to max core detected on computer.
     * This method is called in constructor.
     */
    void setDefaultMaximumNumberOfThreads();

    /**
     * @brief Cancel processing of current jobs under progress.
     */
    virtual void cancel();

    /**
     * @brief Call this method to customize a thread name visible in a process viewer.
     * This will replace the default "Thread (pooled)" string set by Qt for QRunable and QtConcurrent.
     * To show the thread names you can use:
     *    Linux:   htop --filter=PNAME --tree"
     *             sudo gdb -p PID -batch -ex "info threads" -ex "quit"
     *    macOS:   sudo lldb -p PID -o "thread list" -o "quit"
     *    Windows: Microsoft Process Explorer https://learn.microsoft.com/fr-fr/sysinternals/downloads/process-explorer
     * String limitation:
     *    Linux:   15 characters max (UTF8).
     *    macOS:   64 characters max (UTF8).
     *    Windows: 260 characters max (UTF16).
     * A C++ namespace prefix in the name (as when the name comes from a class
     * name) is stripped on all platforms, so that the significant part
     * survives these limits.
     */
    static void setCurrentThreadName(const QString& name);

protected:

    /**
     * @brief Main thread loop used to process jobs in todo list.
     */
    void run()                            override;

    /**
     * @brief Append a collection of jobs to process into QThreadPool.
     * Jobs are add to pending lists and will be deleted by ActionThreadBase, not QThreadPool.
     */
    void appendJobs(const ActionJobCollection& jobs);

    /**
     * @return true if list of pending jobs to process is empty.
     */
    bool isEmpty()                  const;

    /**
     * @return the number of pending jobs to process.
     */
    int pendingCount()              const;

protected Q_SLOTS:

    virtual void slotJobFinished();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

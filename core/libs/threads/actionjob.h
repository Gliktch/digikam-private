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

#include <QObject>
#include <QRunnable>
#include <QElapsedTimer>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ActionJob : public QObject,
                                 public QRunnable
{
    Q_OBJECT

public:

    /**
     * @brief Constructor which delegate deletion of QRunnable instance to ActionThreadBase, not QThreadPool.
     */
    explicit ActionJob(QObject* const parent = nullptr);

    /**
     * @brief Re-implement destructor in you implementation. Don't forget to cancel job.
     */
    ~ActionJob() override;

Q_SIGNALS:

    /**
     * @brief Use this signal in your implementation to inform ActionThreadBase manager that job is started
     */
    void signalStarted();

    /**
     * @brief Use this signal in your implementation to inform ActionThreadBase manager the job progress
     */
    void signalProgress(int);

    /**
     * @brief Use this signal in your implementation to inform ActionThreadBase manager the job is done.
     */
    void signalDone();

public Q_SLOTS:

    /**
     * @brief Call this method to cancel job.
     */
    void cancel();

public:

    /**
     * @brief Timer to determine the running time of the job.
     */
    QElapsedTimer m_timer;

protected:

    /**
     * @brief Reimplemented from QRunnable to customize the thread name based on the object name.
     * Call it on the top of your derived implementation to take effect.
     */
    virtual void run() override;

protected:

    /**
     * @brief You can use this boolean in your implementation to know if job must be canceled.
     */
    bool m_cancel = false;
};

/**
 * @brief Define a QHash of job/priority to process by ActionThreadBase manager.
 *        Priority value can be used to control the run queue's order of execution.
 *        Zero priority want mean to process job with higher priority.
 */
typedef QHash<ActionJob*, int> ActionJobCollection;

} // namespace Digikam

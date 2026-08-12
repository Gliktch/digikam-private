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

#include "actionthreadbase.h"

// C includes

#if defined(Q_OS_UNIX)
#   include <sys/time.h>
#   include <unistd.h>
#   if defined(Q_OS_LINUX)
#       include <sys/prctl.h>
#   endif
#elif defined(Q_OS_WIN)
#   include <windows.h>
#   include <processthreadsapi.h>
#elif defined(Q_OS_MACOS)
#   include <pthread.h>
#endif

// Qt includes

#include <QMutexLocker>
#include <QWaitCondition>
#include <QMutex>
#include <QThreadPool>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN ActionThreadBase::Private
{
public:

    Private() = default;

public:

    volatile bool       running   = false;
    volatile bool       cancelled = false;

    QWaitCondition      condVarJobs;
    QMutex              mutex;

    ActionJobCollection todo;
    ActionJobCollection pending;
    ActionJobCollection processed;

    QThreadPool*        pool    = nullptr;
};

ActionThreadBase::ActionThreadBase(QObject* const parent)
    : QThread(parent),
      d      (new Private)
{
    d->pool = new QThreadPool(this);

    // The pool size is set directly rather than through
    // setDefaultMaximumNumberOfThreads(), whose debug log reads the
    // objectName() that derived classes can only assign once this base
    // constructor has finished.

    d->pool->setMaxThreadCount(QThread::idealThreadCount());
}

ActionThreadBase::~ActionThreadBase()
{
    // Cancel the thread

    this->cancel();

    // Wait for the thread to finish

    wait();

    // Cleanup all jobs from memory

    const auto keys = d->processed.keys();

    for (ActionJob* const job : keys)
    {
        delete job;
    }

    delete d;
}

void ActionThreadBase::setExpiryTimeout(int timeout)
{
    d->pool->setExpiryTimeout(timeout);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Threads Pool" << objectName() << "will use expiry time-out of" << timeout << "ms";
}

int ActionThreadBase::expiryTimeout() const
{
    return d->pool->expiryTimeout();
}

void ActionThreadBase::setMaximumNumberOfThreads(int n)
{
    d->pool->setMaxThreadCount(n);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Threads Pool" << objectName() << "will use" << n << "threads";
}

int ActionThreadBase::maximumNumberOfThreads() const
{
    return d->pool->maxThreadCount();
}

void ActionThreadBase::setDefaultMaximumNumberOfThreads()
{
    setMaximumNumberOfThreads(QThread::idealThreadCount());
}

void ActionThreadBase::slotJobFinished()
{
    ActionJob* const job = dynamic_cast<ActionJob*>(sender());

    if (!job)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "One job is done" << job
                                 << "time:" << job->m_timer.elapsed();

    QMutexLocker lock(&d->mutex);

    d->processed.insert(job, 0);
    d->pending.remove(job);

    if (isEmpty())
    {
        d->running = false;
    }

    d->condVarJobs.wakeAll();
}

void ActionThreadBase::cancel()
{
    if (d->running)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cancel Main Thread";
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Finish Main Thread";
    }

    if (d->cancelled)
    {
        return;
    }

    d->cancelled = true;

    QMutexLocker lock(&d->mutex);

    const auto keys  = d->todo.keys();

    for (ActionJob* const job : keys)
    {
        delete job;
    }

    const auto keys2 = d->pending.keys();

    for (ActionJob* const job : keys2)
    {
        job->cancel();
        d->processed.insert(job, 0);
    }

    d->todo.clear();
    d->pending.clear();
    d->running = false;

    d->condVarJobs.wakeAll();
}

bool ActionThreadBase::isEmpty() const
{
    return (d->pending.isEmpty());
}

int ActionThreadBase::pendingCount() const
{
    return (d->pending.count());
}

void ActionThreadBase::appendJobs(const ActionJobCollection& jobs)
{
    QMutexLocker lock(&d->mutex);

    for (ActionJobCollection::const_iterator it = jobs.begin() ; it != jobs.end() ; ++it)
    {
        d->todo.insert(it.key(), it.value());
    }

    d->condVarJobs.wakeAll();
}

void ActionThreadBase::run()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Threads Pool" << objectName()
                                 << "will use" << maximumNumberOfThreads() << "threads";

    d->running   = true;
    d->cancelled = false;

    while (d->running)
    {
        QMutexLocker lock(&d->mutex);

        if (!d->todo.isEmpty())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Action Thread run" << d->todo.count() << "new jobs";

            for (ActionJobCollection::iterator it = d->todo.begin() ; it != d->todo.end() ; ++it)
            {
                ActionJob* const job = it.key();
                int priority         = it.value();

                connect(job, SIGNAL(signalDone()),
                        this, SLOT(slotJobFinished()));

                job->m_timer.start();
                d->pool->start(job, priority);
                d->pending.insert(job, priority);
            }

            d->todo.clear();
        }
        else
        {
            d->condVarJobs.wait(&d->mutex);
        }
    }

    // Wait for the jobs to finish

    d->pool->waitForDone();
}

void ActionThreadBase::setCurrentThreadName(const QString& name)
{
    // A name taken from a class name carries a C++ namespace prefix which
    // eats most of the platform length limits listed in the API docs (15
    // characters on Linux). Only the last, most significant part is applied.

    const QString threadName = name.section(QLatin1String("::"), -1);

#if defined(Q_OS_LINUX)

    prctl(PR_SET_NAME, (unsigned long)threadName.toLatin1().constData(), 0, 0, 0);

#elif defined(Q_OS_MACOS)

    pthread_setname_np(threadName.toLatin1().constData());

#elif defined(Q_OS_WIN)

    SetThreadDescription(GetCurrentThread(), reinterpret_cast<const wchar_t *>(threadName.utf16()));

#elif defined(Q_OS_NETBSD)

    pthread_setname_np(pthread_self(), "%s", (void*)threadName.toLatin1().constData());

#else

    qCWarning(DIGIKAM_GENERAL_LOG) << "Unsupported platform to customize the current thread name.";

#endif

}

} // namespace Digikam

#include "moc_actionthreadbase.cpp"

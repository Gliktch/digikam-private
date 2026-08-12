/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Thread object scheduling
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "threadmanager.h"

// Qt includes

#include <QEventLoop>
#include <QMutex>
#include <QMutexLocker>
#include <QPair>
#include <QThread>
#include <QThreadPool>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "dynamicthread.h"
#include "workerobject.h"
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN ParkingThread : public QThread
{
    Q_OBJECT

public:

    explicit ParkingThread(QObject* const parent = nullptr)
        : QThread(parent)
    {
        start();
    }

    ~ParkingThread() override
    {
        running = false;
        condVar.wakeAll();
        wait();
    }

    void parkObject(QObject* const object)
    {
        object->moveToThread(this);
        QMutexLocker locker(&mutex);
        condVar.wakeAll();
    }

    void moveToCurrentThread(QObject* const parkedObject)
    {
        if (parkedObject->thread() == QThread::currentThread())
        {
            return;
        }

        // We have 1. The current thread
        //         2. ParkingThread's thread
        //         3. The parkedObject's thread, subject to change.

        QMutexLocker locker(&mutex);

        // First, wait until the object has been parked in ParkingThread by its owning thread.

        while (parkedObject->thread() != this)
        {
            condVar.wait(&mutex);
        }

        QThread* const targetThread = QThread::currentThread();

        // Then, now that it's parked in ParkingThread, make ParkingThread move it to the current thread.

        todo << qMakePair(parkedObject, targetThread);
        condVar.wakeAll();

        // Wait until ParkingThread has pushed the object to current thread

        while (parkedObject->thread() != targetThread)
        {
            condVar.wait(&mutex);
        }
    }

    void run() override
    {
        ActionThreadBase::setCurrentThreadName(QLatin1String("ParkingThread"));       // To customize thread name

        /**
         * The quirk here is that this thread never runs an event loop.
         * That means events queued for parked object are only emitted when
         * these object have been moved to their own thread.
         */
        while (running)
        {
            QList<TodoPair> copyTodo;
            {
                QMutexLocker locker(&mutex);
                condVar.wakeAll();

                if (todo.isEmpty())
                {
                    condVar.wait(&mutex);
                    continue;
                }
                else
                {
                    copyTodo = todo;
                    todo.clear();
                }
            }

            for (const TodoPair& pair : std::as_const(copyTodo))
            {
                pair.first->moveToThread(pair.second);
            }
        }
    }

public:

    volatile bool                     running   = true;
    typedef QPair<QObject*, QThread*> TodoPair;
    QMutex                            mutex;
    QWaitCondition                    condVar;
    QList<TodoPair>                   todo;
};

// --------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN WorkerObjectRunnable : public QRunnable
{
public:

    WorkerObjectRunnable(WorkerObject* const object, ParkingThread* const parkingThread);

protected:

    WorkerObject*  object           = nullptr;
    ParkingThread* parkingThread    = nullptr;

protected:

    void run() override;

private:

    Q_DISABLE_COPY(WorkerObjectRunnable)
};

// --------------------------------------------------------------------------------------------------

WorkerObjectRunnable::WorkerObjectRunnable(WorkerObject* const object, ParkingThread* const parkingThread)
    : object       (object),
      parkingThread(parkingThread)
{
    setAutoDelete(true);
}

void WorkerObjectRunnable::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("WorkerObjectRunnable"));       // To customize thread name

    if (!object)
    {
        return;
    }

    // If another thread should still be running, wait until the object is parked in ParkingThread

    parkingThread->moveToCurrentThread(object);

    // The object is in state Scheduled or Deactivating now.
    // It won't be deleted until Inactive, and as long a runnable is set.

    object->addRunnable(this);

    Q_EMIT object->started();

    if (object->transitionToRunning())
    {
        QThread::Priority previousPriority = QThread::currentThread()->priority();

        if (object->priority() != QThread::InheritPriority)
        {
            QThread::currentThread()->setPriority(object->priority());
        }

        QEventLoop loop;
        object->setEventLoop(&loop);
        loop.exec();
        object->setEventLoop(nullptr);

        if (previousPriority != QThread::InheritPriority)
        {
            QThread::currentThread()->setPriority(previousPriority);
        }
    }

    object->transitionToInactive();

    Q_EMIT object->finished();

    // If this is rescheduled, it will wait in the other thread at moveToCurrentThread() above until we park

    parkingThread->parkObject(object);

    // Now, free the object - in case it wants to get deleted

    object->removeRunnable(this);
}

// -------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ThreadManager::Private
{
public:

    Private() = default;

    void changeMaxThreadCount(int diff)
    {
        pool->setMaxThreadCount(pool->maxThreadCount() + diff);
    }

public:

    ParkingThread* parkingThread    = nullptr;
    QThreadPool*   pool             = nullptr;
};

// -------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ThreadManagerCreator
{
public:

    ThreadManager object;
};

Q_GLOBAL_STATIC(ThreadManagerCreator, creator)

// -------------------------------------------------------------------------------------------------

ThreadManager::ThreadManager()
    : QObject(),
      d      (new Private)
{
    d->parkingThread = new ParkingThread(this);
    d->pool          = new QThreadPool(this);

    d->pool->setMaxThreadCount(QThread::idealThreadCount() + 1);
}

ThreadManager::~ThreadManager()
{
    d->pool->waitForDone();

    delete d;
}

ThreadManager* ThreadManager::instance()
{
    return (&creator->object);
}

void ThreadManager::initialize(WorkerObject* const object)
{
    connect(object, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotDestroyed(QObject*)));

    d->changeMaxThreadCount(+1);

    d->parkingThread->parkObject(object);
}

void ThreadManager::initialize(DynamicThread* const dynamicThread)
{
    connect(dynamicThread, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotDestroyed(QObject*)));

    d->changeMaxThreadCount(+1);
}

void ThreadManager::schedule(WorkerObject* object)
{
    d->pool->start(new WorkerObjectRunnable(object, d->parkingThread));
}

void ThreadManager::schedule(QRunnable* runnable)
{
    d->pool->start(runnable);
}

void ThreadManager::slotDestroyed(QObject*)
{
    d->changeMaxThreadCount(-1);
}

} // namespace Digikam

#include "threadmanager.moc"

#include "moc_threadmanager.cpp"

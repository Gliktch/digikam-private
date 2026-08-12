/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : Multithreaded worker objects, working in parallel
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "workerobject.h"

namespace Digikam
{

class DIGIKAM_EXPORT ParallelWorkers
{

public:

    /**
     * @brief ParallelWorkers is a helper class to distribute work over
     * several identical workers objects.
     * @see ParallelAdapter for guidance how to use it.
     */
    ParallelWorkers() = default;
    virtual ~ParallelWorkers();

    /**
     * @brief The corresponding methods of all added worker objects will be called
     */
    virtual void schedule();
    virtual void deactivate(WorkerObject::DeactivatingMode mode = WorkerObject::FlushSignals);
    virtual void wait();

    void setPriority(QThread::Priority priority);

    /**
     * @return true if the current number of added workers has reached the optimalWorkerCount()
     */
    bool optimalWorkerCountReached()                                    const;

    /**
     * @brief Regarding the number of logical CPUs on the current machine,
     * returns the optimal count of concurrent workers
     */
    static int optimalWorkerCount();

public:

    /**
     * @brief Connects signals outbound from all workers to a given receiver
     */
    virtual bool connect(const char* signal,
                         const QObject* receiver,
                         const char* method,
                         Qt::ConnectionType type = Qt::AutoConnection)  const;

protected:

    void add(WorkerObject* const worker);

    // Internal implementation

    /**
     * @brief Replaces slot call distribution of the target QObject
     */
    int replacementQtMetacall(QMetaObject::Call _c, int _id, void** _a);
    const QMetaObject* replacementMetaObject()                          const;

    /**
     * @return the target QObject (double inheritance)
     */
    virtual QObject* asQObject()                                                        = 0;

    /**
     * @brief The qt_metacall of WorkerObject, one level above the target QObject
     */
    virtual int WorkerObjectQtMetacall(QMetaObject::Call _c, int _id, void** _a)        = 0;

    /**
     * @brief The moc-generated metaObject of the target object
     */
    virtual const QMetaObject* mocMetaObject()                          const           = 0;

    int replacementStaticQtMetacall(QMetaObject::Call _c, int _id, void** _a);
    typedef void (*StaticMetacallFunction)(QObject*, QMetaObject::Call, int, void**);
    virtual StaticMetacallFunction staticMetacallPointer()                              = 0;

protected:

    QList<WorkerObject*>   m_workers;
    int                    m_currentIndex           = 0;
    QMetaObject*           m_replacementMetaObject  = nullptr;

    StaticMetacallFunction m_originalStaticMetacall = nullptr;

private:

    /// @note disabled
    ParallelWorkers(const ParallelWorkers&)            = delete;
    ParallelWorkers& operator=(const ParallelWorkers&) = delete;
};

// -------------------------------------------------------------------------------------------------

template <class A>

class ParallelAdapter : public A,
                        public ParallelWorkers
{
public:

    /**
     * @brief Instead of using a single WorkerObject, create a ParallelAdapter for
     * your worker object subclass, and add() individual WorkerObjects.
     * The load will be evenly distributed.
     * @note Unlike with WorkerObject directly, there is no need to call schedule().
     * For inbound connections (signals connected to a WorkerObject's slot, to be processed,
     * use a Qt::DirectConnection on the adapter.
     * For outbound connections (signals emitted from the WorkerObject),
     * use ParallelAdapter's connect to have a connection from all added WorkerObjects.
     */
    ParallelAdapter()                                                                            = default;
    ~ParallelAdapter()                                                                  override = default;

    void add(A* const worker)
    {
        ParallelWorkers::add(worker);
    }

    // Internal Implementation
    // I know this is a hack

    int WorkerObjectQtMetacall(QMetaObject::Call _c, int _id, void** _a)                override
    {
        return WorkerObject::qt_metacall(_c, _id, _a);
    }

    const QMetaObject* mocMetaObject()                                            const override
    {
        return A::metaObject();
    }

    static void qt_static_metacall(QObject* o, QMetaObject::Call _c, int _id, void** _a)
    {
        static_cast<ParallelAdapter*>(o)->replacementStaticQtMetacall(_c, _id, _a);
    }

    StaticMetacallFunction staticMetacallPointer()                                      override
    {
        return qt_static_metacall;
    }

    const QMetaObject* metaObject()                                               const override
    {
        return ParallelWorkers::replacementMetaObject();
    }

    int qt_metacall(QMetaObject::Call _c, int _id, void** _a)                           override
    {
        return ParallelWorkers::replacementQtMetacall(_c, _id, _a);
    }

    QObject* asQObject()                                                                override
    {
        return this;
    }

    void schedule()                                                                     override
    {
        ParallelWorkers::schedule();
    }

    void deactivate(WorkerObject::DeactivatingMode mode = WorkerObject::FlushSignals)   override
    {
        ParallelWorkers::deactivate(mode);
    }

    void wait()                                                                         override
    {
        ParallelWorkers::wait();
    }

    bool connect(const char* signal,
                 const QObject* receiver,
                 const char* method,
                 Qt::ConnectionType type = Qt::AutoConnection)                    const override
    {
        return ParallelWorkers::connect(signal, receiver, method, type);
    }

private:

    /// @note disabled
    ParallelAdapter(const ParallelAdapter&)            = delete;
    ParallelAdapter& operator=(const ParallelAdapter&) = delete;
};

} // namespace Digikam

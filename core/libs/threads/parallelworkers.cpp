/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : Multithreaded worker object
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "parallelworkers.h"

// Qt includes

#include <QCoreApplication>
#include <QEvent>
#include <QMetaMethod>
#include <QMutex>
#include <QThread>
#include <QMetaType>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "threadmanager.h"

namespace Digikam
{

ParallelWorkers::~ParallelWorkers()
{
    for (WorkerObject* const object : std::as_const(m_workers))
    {
        delete object;
    }

    delete m_replacementMetaObject;
}

int ParallelWorkers::optimalWorkerCount()
{
    return (qMax(1, QThread::idealThreadCount()));
}

bool ParallelWorkers::optimalWorkerCountReached() const
{
    return (m_workers.size() >= optimalWorkerCount());
}

void ParallelWorkers::schedule()
{
    for (WorkerObject* const object : std::as_const(m_workers))
    {
        object->schedule();
    }
}

void ParallelWorkers::deactivate(WorkerObject::DeactivatingMode mode)
{
    for (WorkerObject* const object : std::as_const(m_workers))
    {
        object->deactivate(mode);
    }
}

void ParallelWorkers::wait()
{
    for (WorkerObject* const object : std::as_const(m_workers))
    {
        object->wait();
    }
}

void ParallelWorkers::setPriority(QThread::Priority priority)
{
    for (WorkerObject* const object : std::as_const(m_workers))
    {
        object->setPriority(priority);
    }
}

// cppcheck-suppress constParameterPointer
void ParallelWorkers::add(WorkerObject* const worker)
{
/*
    if (!asQObject()->inherits(worker->metaObject()->className()))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "You need to derive the ParallelWorkers class from the WorkerObject you want to use";
        return;
    }

    QMetaObject* const meta = asQObject()->metaObject();

    for (int i = 0 ; i < meta->methodCount() ; ++i)
    {
        QMetaMethod method = meta->method(index);
        if (!method->methodType() == QMetaMethod::
    }
*/

    m_workers << worker;
}

/*
bool ParallelWorkers::connect(const QObject* sender, const char* signal,
                              const char* method,
                              Qt::ConnectionType type) const
{
    for (WorkerObject* const object : std::as_const(m_workers))
    {
        if (!WorkerObject::connect(sender, signal, object, method, type))
        {
            return false;
        }
    }

    return true;
}
*/

bool ParallelWorkers::connect(const char* const signal,
                              const QObject* const receiver,
                              const char* const method,
                              Qt::ConnectionType type) const
{
    for (const WorkerObject* const object : std::as_const(m_workers))
    {
        if (!QObject::connect(object, signal, receiver, method, type))
        {
            return false;
        }
    }

    return true;
}

int ParallelWorkers::replacementStaticQtMetacall(QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod)
    {
        // This is the common ancestor's meta object, below WorkerObject

        const QMetaObject* const mobj = mocMetaObject();
        const int properMethods       = mobj->methodCount() - mobj->methodOffset();

        if (_id >= properMethods)
        {
            return (_id - properMethods);
        }

        // Get the relevant meta method. I'm not quite sure if this is rock solid.

        QMetaMethod method            = mobj->method(_id + mobj->methodOffset());

        // Copy the argument data - _a is going to be deleted in our current thread

        QList<QByteArray> types       = method.parameterTypes();
        QVector<QGenericArgument> args(10);                         // clazy:exclude=missing-typeinfo

        for (int i = 0 ; i < types.size() ; ++i)
        {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            int typeId = QMetaType::fromName(QByteArrayView(types[i].constData())).id();

#else

            int typeId = QMetaType::type(types[i].constData());

#endif

            if (!typeId && _a[i+1])
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unable to handle unregistered datatype" << types[i] << "Dropping signal.";
                return (_id - properMethods);
            }

            // We use QMetaType to copy the data. _a[0] is reserved for a return parameter.

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

            void* const data = QMetaType(typeId).create(_a[i+1]);

#else

            void* const data = QMetaType::create(typeId, _a[i+1]);

#endif

            args[i]          = QGenericArgument(types[i].constData(), data);
        }

        // Find the object to be invoked

        WorkerObject* const obj = m_workers.at(m_currentIndex);

        if (++m_currentIndex == m_workers.size())
        {
            m_currentIndex = 0;
        }

        obj->schedule();

        // Invoke across-thread

        method.invoke(obj, Qt::QueuedConnection,
                      args[0],
                      args[1],
                      args[2],
                      args[3],
                      args[4],
                      args[5],
                      args[6],
                      args[7],
                      args[8],
                      args[9]);

        return (_id - properMethods); // This return is used by replacementQtMetacall
    }
    else
    {
        m_originalStaticMetacall(asQObject(), _c, _id, _a);
    }

    return _id; // This return will be ignored (qt_static_metacall is void)
}

int ParallelWorkers::replacementQtMetacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = WorkerObjectQtMetacall(_c, _id, _a);

    if (_id < 0)
    {
        return _id;
    }

    if (_c == QMetaObject::InvokeMetaMethod)
    {
        return replacementStaticQtMetacall(_c, _id, _a);
    }

    return _id;
}

const QMetaObject* ParallelWorkers::replacementMetaObject() const
{
    if (!m_replacementMetaObject)
    {
        QMetaObject* const rmo                 = new QMetaObject(*mocMetaObject());
        ParallelWorkers* const nonConstThis    = const_cast<ParallelWorkers*>(this);
        nonConstThis->m_originalStaticMetacall = rmo->d.static_metacall;
        rmo->d.static_metacall                 = nonConstThis->staticMetacallPointer();
        nonConstThis->m_replacementMetaObject  = rmo;
    }

    return m_replacementMetaObject;
}

} // namespace Digikam

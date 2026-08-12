/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Dynamically active thread
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QThread>
#include <QRunnable>
#include <QMutex>
#include <QMutexLocker>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DynamicThread : public QObject
{
    Q_OBJECT

public:

    enum State
    {
        Inactive,
        Scheduled,
        Running,
        Deactivating
    };

public:

    /**
     * @brief This class extends QRunnable, so you have to reimplement
     * virtual void run(). In all aspects the class will act similar to a QThread.
     */
    explicit DynamicThread(QObject* const parent = nullptr);

    /**
     * @brief The destructor calls stop() and wait(), but if you, in your destructor,
     * delete any data that is accessed by your run() method,
     * you must call stop() and wait() before yourself.
     */
    ~DynamicThread() override;

    /**
     * @brief Implement this pure virtual function in your subclass.
     */
    virtual void run() = 0;

    State state()               const;
    bool  isRunning()           const;
    bool  isFinished()          const;

    void setEmitSignals(bool emitThem);

    /**
     * @brief Sets the priority for this dynamic thread.
     * Can be set anytime. If the thread is currently not running,
     * the priority will be set when it is run next time.
     * When you set QThread::InheritPriority (default), the
     * priority is not changed but inherited from the thread pool.
     */
    void setPriority(QThread::Priority priority);
    QThread::Priority priority() const;

public Q_SLOTS:

    void start();

    /**
     * @brief Stop computation, sets the running flag to false.
     */
    void stop();

    /**
     * @brief Waits until the thread finishes. Typically, call stop() before.
     */
    void wait();

Q_SIGNALS:

    /**
     * @brief Emitted if emitSignals is enabled
     */
    void starting();
    void finished();

protected:

    /**
     * @brief If you are deleting data in your destructor which is accessed from the thread,
     * do one of the following from your destructor to guarantee a safe shutdown:
     * 1) Call this method
     * 2) Call stop() and wait(), knowing that nothing will
     *    call start() anymore after this
     * 3) Be sure the thread will never be running at destruction.
     * @note This irrevocably stops this object.
     * @note It is not sufficient that your parent class does this.
     * Calling this method, or providing one of the above mentioned
     * equivalent guarantees, must be done by every
     * single last class in the hierarchy with an implemented destructor deleting data.
     * (the base class destructor is always called after the derived class)
     */
    virtual void shutDown();

    /**
     * @brief In you run() method, you shall regularly check for runningFlag()
     * and cleanup and return if false.
     */
    bool runningFlag() const volatile;

    /**
     * @brief This is the non-recursive mutex used to protect state variables
     * and waiting in this class. You can use it if you want to protect
     * your memory in the same scope as calling start, stop or wait,
     * then using the QMutexLocker variants below. Note that when you have locked this mutex,
     * you must use these variants, as the mutex is non-recursive.
     */
    QMutex* threadMutex() const;

    /**
     * @brief Doing the same as start(), stop() and wait above, provide it
     * with a locked QMutexLocker on mutex().
     * Note the start() will unlock and relock for scheduling once, after state change.
     */

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void start(QMutexLocker<QMutex>& locker);
    void stop(const QMutexLocker<QMutex>& locker);
    void wait(QMutexLocker<QMutex>& locker);

#else

    void start(QMutexLocker& locker);
    void stop(const QMutexLocker& locker);
    void wait(QMutexLocker& locker);

#endif

private:

    /// @note disabled
    DynamicThread(const DynamicThread&)            = delete;
    DynamicThread& operator=(const DynamicThread&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

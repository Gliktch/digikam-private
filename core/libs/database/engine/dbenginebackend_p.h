 /* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Database engine abstract database backend
 *
 * SPDX-FileCopyrightText: 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dbenginebackend.h"

// Qt includes

#include <QHash>
#include <QSqlDatabase>
#include <QThread>
#include <QThreadStorage>
#include <QWaitCondition>

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN DbEngineThreadData
{
public:

    DbEngineThreadData() = default;
    ~DbEngineThreadData();

    void closeDatabase();

public:

    int       valid             = 0;
    int       transactionCount  = 0;
    QString   connectionName;
    QSqlError lastError;

private:

    /// @note disabled
    DbEngineThreadData(const DbEngineThreadData&)            = delete;
    DbEngineThreadData& operator=(const DbEngineThreadData&) = delete;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT BdEngineBackendPrivate : public DbEngineErrorAnswer
{
public:

    explicit BdEngineBackendPrivate(BdEngineBackend* const backend);
    ~BdEngineBackendPrivate()                                              override;

    void init(const QString& connectionName, DbEngineLocking* const locking);

    QSqlDatabase databaseForThread();
    QSqlError    databaseErrorForThread();
    bool         resetDatabaseForThread();
    void         setDatabaseErrorForThread(const QSqlError& lastError);

    QString      connectionName();
    QSqlDatabase createDatabaseConnection();

    void closeDatabaseForThread();
    bool incrementTransactionCount();
    bool decrementTransactionCount();

    bool isInMainThread()                                         const;
    bool isInUIThread()                                           const;

    bool reconnectOnError()                                       const;
    bool isSQLiteLockError(const QSqlQuery& query)                const;
    bool isSQLiteLockTransactionError(const QSqlError& lastError) const;
    bool isConnectionError(const QSqlQuery& query)                const;
    bool needToConsultUserForError(const QSqlQuery& query)        const;
    bool needToHandleWithErrorHandler(const QSqlQuery& query)     const;
    void debugOutputFailedQuery(const QSqlQuery& query)           const;
    void debugOutputFailedTransaction(const QSqlError& error)     const;

    bool checkRetrySQLiteLockError(int retries);
    bool checkOperationStatus();
    bool handleWithErrorHandler(const QSqlQuery* const query);
    void setQueryOperationFlag(BdEngineBackend::QueryOperationStatus _status);
    void queryOperationWakeAll(BdEngineBackend::QueryOperationStatus _status);

    /**
     * called by DbEngineErrorHandler, implementing DbEngineErrorAnswer.
     */
    void connectionErrorContinueQueries()                         override;
    void connectionErrorAbortQueries()                            override;

    virtual void transactionFinished();

public:

    QThreadStorage<DbEngineThreadData*>       threadDataStorage;

    /**
     * This compares to DbEngineThreadData's valid.
     * If currentValidity is increased and > valid, the db is marked as invalid
     */
    int                                       currentValidity           = 0;

    bool                                      isInTransaction           = false;

    QString                                   backendName;

    DbEngineParameters                        parameters;

    BdEngineBackend::Status                   status                    = BdEngineBackend::Unavailable;

    DbEngineLocking*                          lock                      = nullptr;

    BdEngineBackend::QueryOperationStatus     operationStatus           = BdEngineBackend::ExecuteNormal;

    QMutex                                    errorLockMutex;
    QWaitCondition                            errorLockCondVar;
    BdEngineBackend::QueryOperationStatus     errorLockOperationStatus  = BdEngineBackend::ExecuteNormal;

    QMutex                                    busyWaitMutex;
    QWaitCondition                            busyWaitCondVar;

    DbEngineErrorHandler*                     errorHandler              = nullptr;

public:

    class Q_DECL_HIDDEN AbstractUnlocker
    {
    public:

        explicit AbstractUnlocker(BdEngineBackendPrivate* const dd);
        ~AbstractUnlocker();

        void finishAcquire();

    protected:

        int                           count = 0;
        BdEngineBackendPrivate* const d     = nullptr;
    };

    friend class AbstractUnlocker;

    // ------------------------------------------------------------------

    class Q_DECL_HIDDEN AbstractWaitingUnlocker : public AbstractUnlocker
    {
    public:

        explicit AbstractWaitingUnlocker(BdEngineBackendPrivate* const d,
                                         QMutex* const mutex,
                                         QWaitCondition* const condVar);
        ~AbstractWaitingUnlocker();

        bool wait(unsigned long time = ULONG_MAX);

    protected:

        QMutex*         const mutex     = nullptr;
        QWaitCondition* const condVar   = nullptr;
    };

    // ------------------------------------------------------------------

    class Q_DECL_HIDDEN ErrorLocker : public AbstractWaitingUnlocker
    {
    public:

        explicit ErrorLocker(BdEngineBackendPrivate* const d);
        void wait();
    };

    // ------------------------------------------------------------------

    class Q_DECL_HIDDEN BusyWaiter : public AbstractWaitingUnlocker
    {
    public:

        explicit BusyWaiter(BdEngineBackendPrivate* const d);
    };

public:

    BdEngineBackend* const q = nullptr;
};

} // namespace Digikam

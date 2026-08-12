/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Database engine abstract database backend
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbenginebackend_p.h"

// C++ includes

#include <iterator>

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlRecord>
#include <QThread>
#include <QTime>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dbengineactiontype.h"

namespace Digikam
{

BdEngineBackendPrivate::BusyWaiter::BusyWaiter(BdEngineBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->busyWaitMutex, &d->busyWaitCondVar)
{
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::ErrorLocker::ErrorLocker(BdEngineBackendPrivate* const d)
    : AbstractWaitingUnlocker(d, &d->errorLockMutex, &d->errorLockCondVar)
{
}

// -----------------------------------------------------------------------------------------

DbEngineThreadData::~DbEngineThreadData()
{
    if (transactionCount)
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "WARNING !!! Transaction count is"
                                      << transactionCount << "when destroying database!!!";
    }

    closeDatabase();
}

void DbEngineThreadData::closeDatabase()
{
    if (!connectionName.isNull())
    {
        {
            QSqlDatabase database = QSqlDatabase::database(connectionName, false);

            if (database.isOpen())
            {
                database.close();
            }
        }

        // Remove connection

        QSqlDatabase::removeDatabase(connectionName);
    }

    // clear variables

    valid            = 0;
    transactionCount = 0;
    connectionName   = QString();
    lastError        = QSqlError();
}

BdEngineBackendPrivate::BdEngineBackendPrivate(BdEngineBackend* const backend)
    : q(backend)
{
}

BdEngineBackendPrivate::~BdEngineBackendPrivate()
{
    // Must be shut down from the main thread.
    // Clean up the QThreadStorage. It deletes any stored data.

    threadDataStorage.setLocalData(nullptr);
}

void BdEngineBackendPrivate::init(const QString& name, DbEngineLocking* const l)
{
    backendName = name;
    lock        = l;

    qRegisterMetaType<DbEngineErrorAnswer*>("DbEngineErrorAnswer*");
    qRegisterMetaType<QSqlError>();
}

/**
 * "A connection can only be used from within the thread that created it.
 *  Moving connections between threads or creating queries from a different thread is not supported."
 * => one QSqlDatabase object per thread.
 * The main class' open/close methods only interact with the "firstDatabase" object.
 * When another thread requests a DB, a new connection is opened and closed at
 * finishing of the thread.
 */
QSqlDatabase BdEngineBackendPrivate::databaseForThread()
{
    DbEngineThreadData* threadData = nullptr;

    if (!threadDataStorage.hasLocalData())
    {
        threadData = new DbEngineThreadData;
        threadDataStorage.setLocalData(threadData);
    }
    else
    {
        threadData = threadDataStorage.localData();
    }

    // do we need to reopen the database because parameter changed and validity was increased?

    if (threadData->valid && (threadData->valid < currentValidity))
    {
        threadData->closeDatabase();
    }

    QSqlDatabase database;

    if (!threadData->connectionName.isNull())
    {
       database = QSqlDatabase::database(threadData->connectionName, false);
    }

    if (!threadData->valid || !database.isOpen())
    {
        database                   = createDatabaseConnection();
        threadData->connectionName = database.connectionName();

        if (database.open())
        {
            threadData->valid = currentValidity;
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while opening the database. Error was"
                                          << database.lastError();
        }
    }

    return database;
}

QSqlDatabase BdEngineBackendPrivate::createDatabaseConnection()
{
    QSqlDatabase database  = QSqlDatabase::addDatabase(parameters.databaseType, connectionName());
    QString connectOptions = parameters.connectOptions;

    if (parameters.isSQLite())
    {
        QStringList toAdd;
/*
        if (!parameters.walMode)
        {
            // Enable shared cache, especially useful
            // with SQLite >= 3.5.0 and WAL mode is disabled.

            toAdd << QLatin1String("QSQLITE_ENABLE_SHARED_CACHE");
        }
*/
        // We do our own waiting.

        toAdd << QLatin1String("QSQLITE_BUSY_TIMEOUT=0");

        if (!connectOptions.isEmpty())
        {
            connectOptions += QLatin1Char(';');
        }

        connectOptions += toAdd.join(QLatin1Char(';'));
    }

    database.setDatabaseName(parameters.databaseNameCore);
    database.setConnectOptions(connectOptions);
    database.setHostName(parameters.hostName);
    database.setPort(parameters.port);
    database.setUserName(parameters.userName);
    database.setPassword(parameters.password);

    return database;
}

void BdEngineBackendPrivate::closeDatabaseForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->closeDatabase();
    }
}

QSqlError BdEngineBackendPrivate::databaseErrorForThread()
{
    if (threadDataStorage.hasLocalData())
    {
        return threadDataStorage.localData()->lastError;
    }

    return QSqlError();
}

bool BdEngineBackendPrivate::resetDatabaseForThread()
{
    int tc = 0;

    if (threadDataStorage.hasLocalData())
    {
        tc = threadDataStorage.localData()->transactionCount;

        closeDatabaseForThread();
        databaseForThread();
    }

    return ((bool)tc);
}

void BdEngineBackendPrivate::setDatabaseErrorForThread(const QSqlError& lastError)
{
    if (threadDataStorage.hasLocalData())
    {
        threadDataStorage.localData()->lastError = lastError;
    }
}

QString BdEngineBackendPrivate::connectionName()
{
    return (backendName + QString::number((quintptr)QThread::currentThread()));
}

bool BdEngineBackendPrivate::incrementTransactionCount()
{
    return (!threadDataStorage.localData()->transactionCount++);
}

bool BdEngineBackendPrivate::decrementTransactionCount()
{
    return (!--threadDataStorage.localData()->transactionCount);
}

bool BdEngineBackendPrivate::isInMainThread() const
{
    return (QThread::currentThread() == QCoreApplication::instance()->thread());
}

bool BdEngineBackendPrivate::isInUIThread() const
{
    QApplication* const app = qobject_cast<QApplication*>(QCoreApplication::instance());

    if (!app)
    {
        return false;
    }

    return (QThread::currentThread() == app->thread());
}

bool BdEngineBackendPrivate::reconnectOnError() const
{
    return parameters.isMySQL();
}

bool BdEngineBackendPrivate::isSQLiteLockError(const QSqlQuery& query) const
{
    return (
            parameters.isSQLite() &&
            (
             (query.lastError().nativeErrorCode() == QLatin1String("5"))   /*SQLITE_BUSY*/             ||
             (query.lastError().nativeErrorCode() == QLatin1String("6"))   /*SQLITE_LOCKED*/           ||
             (query.lastError().nativeErrorCode() == QLatin1String("517")) /*SQLITE_BUSY_SNAPSHOT*/    ||
             (query.lastError().nativeErrorCode() == QLatin1String("262")) /*SQLITE_LOCKED_SHAREDCACHE*/
            )
           );
}

bool BdEngineBackendPrivate::isSQLiteLockTransactionError(const QSqlError& lastError) const
{
    return (
            parameters.isSQLite()                                     &&
            (lastError.type()         == QSqlError::TransactionError) &&
            (lastError.databaseText() == QLatin1String("database is locked"))
           );

    // wouldn't it be great if they gave us the database error number...
}

bool BdEngineBackendPrivate::isConnectionError(const QSqlQuery& query) const
{
    // the backend returns connection error e.g. for Constraint Failed errors.

    if (parameters.isSQLite())
    {
        return false;
    }

    return (
            (query.lastError().type()            == QSqlError::ConnectionError) ||
            (query.lastError().nativeErrorCode() == QLatin1String("2006"))
           );
}

bool BdEngineBackendPrivate::needToConsultUserForError(const QSqlQuery&) const
{
    // no such conditions found and defined as yet

    return false;
}

bool BdEngineBackendPrivate::needToHandleWithErrorHandler(const QSqlQuery& query) const
{
    // cppcheck-suppress knownConditionTrueFalse
    return (isConnectionError(query) || needToConsultUserForError(query));
}

bool BdEngineBackendPrivate::checkRetrySQLiteLockError(int retries)
{
    if (!(retries % 25))
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Database is locked. Waited" << retries*10;
    }

    const int uiMaxRetries = 50;
    const int maxRetries   = 1000;

    if (retries > qMax(uiMaxRetries, maxRetries))
    {
        if (retries > (isInUIThread() ? uiMaxRetries : maxRetries))
        {
            qCWarning(DIGIKAM_DBENGINE_LOG) << "Detected locked database file. There is an active transaction. "
                                               "Waited but giving up now.";

            return false;
        }
    }

    BusyWaiter waiter(this);
    waiter.wait(10);

    return true;
}

void BdEngineBackendPrivate::debugOutputFailedQuery(const QSqlQuery& query) const
{
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Failure executing query:\n"
                                  << query.executedQuery()
                                  << "\nError messages:" << query.lastError().driverText() << query.lastError().databaseText()
                                  << query.lastError().nativeErrorCode() << query.lastError().type()
                                  << "\nBound values: " <<

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                    query.boundValues()

#else

                                    query.boundValues().values()

#endif

                                  ;
}

void BdEngineBackendPrivate::debugOutputFailedTransaction(const QSqlError& error) const
{
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Failure executing transaction. Error messages:\n"
                                  << error.driverText() << error.databaseText()
                                  << error.nativeErrorCode() << error.type();
}

void BdEngineBackendPrivate::transactionFinished()
{
    // wakes up any BusyWaiter waiting on the busyWaitCondVar.
    // Possibly called under d->lock->mutex lock, so we do not lock the busyWaitMutex

    busyWaitCondVar.wakeOne();
}

/**
 * Set the wait flag to queryStatus. Typically, call this with Wait.
 */
void BdEngineBackendPrivate::setQueryOperationFlag(BdEngineBackend::QueryOperationStatus _status)
{
    // Enforce lock order (first main mutex, second error lock mutex)

    QMutexLocker l(&errorLockMutex);

    // this change must be done under errorLockMutex lock

    errorLockOperationStatus = _status;
    operationStatus          = _status;
}

/**
 * Set the wait flag to queryStatus and wake all waiting threads.
 * Typically, call wakeAll with status ExecuteNormal or AbortQueries.
 */
void BdEngineBackendPrivate::queryOperationWakeAll(BdEngineBackend::QueryOperationStatus _status)
{
    QMutexLocker l(&errorLockMutex);
    operationStatus          = _status;
    errorLockOperationStatus = _status;
    errorLockCondVar.wakeAll();
}

bool BdEngineBackendPrivate::checkOperationStatus()
{
    while (operationStatus == BdEngineBackend::Wait)
    {
        ErrorLocker locker(this);
        locker.wait();
    }

    if      (operationStatus == BdEngineBackend::ExecuteNormal)
    {
        return true;
    }
    else if (operationStatus == BdEngineBackend::AbortQueries)
    {
        return false;
    }

    return false;
}

/**
 * Returns true if the query shall be retried.
 */
bool BdEngineBackendPrivate::handleWithErrorHandler(const QSqlQuery* const query)
{
    if (errorHandler)
    {
        setQueryOperationFlag(BdEngineBackend::Wait);

        ErrorLocker locker(this);
        bool called         = false;
        QSqlError lastError = query ? query->lastError() : databaseForThread().lastError();
        QString lastQuery   = query ? query->lastQuery() : QString();

        if      (!query || isConnectionError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "connectionError",
                                               Qt::AutoConnection,
                                               Q_ARG(DbEngineErrorAnswer*, this),
                                               Q_ARG(QSqlError, lastError),
                                               Q_ARG(QString, lastQuery));
        }
        // cppcheck-suppress knownConditionTrueFalse
        else if (needToConsultUserForError(*query))
        {
            called = QMetaObject::invokeMethod(errorHandler, "consultUserForError",
                                               Qt::AutoConnection,
                                               Q_ARG(DbEngineErrorAnswer*, this),
                                               Q_ARG(QSqlError, lastError),
                                               Q_ARG(QString, lastQuery));
        }
        else
        {
            // unclear what to do.

            errorLockOperationStatus = BdEngineBackend::ExecuteNormal;
            operationStatus          = BdEngineBackend::ExecuteNormal;

            return true;
        }

        if (called)
        {
            locker.wait();
        }
        else
        {
            qCWarning(DIGIKAM_DBENGINE_LOG) << "Failed to invoke DbEngineErrorHandler. Aborting all queries.";
            operationStatus = BdEngineBackend::AbortQueries;
        }

        switch (operationStatus)
        {
            case BdEngineBackend::ExecuteNormal:
            case BdEngineBackend::Wait:
            {
                return true;
            }
            case BdEngineBackend::AbortQueries:
            {
                return false;
            }
        }
    }
    else
    {
        // TODO check if it's better to use an own error handler for kio slaves.
        // But for now, close only the database in the hope, that the next
        // access will be successful.

        closeDatabaseForThread();
    }

    return false;
}

void BdEngineBackendPrivate::connectionErrorContinueQueries()
{
    // Attention: called from out of context, maybe without any lock

    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(BdEngineBackend::ExecuteNormal);
}

void BdEngineBackendPrivate::connectionErrorAbortQueries()
{
    // Attention: called from out of context, maybe without any lock

    QMutexLocker l(&lock->mutex);
    queryOperationWakeAll(BdEngineBackend::AbortQueries);
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::AbstractUnlocker::AbstractUnlocker(BdEngineBackendPrivate* const dd)
    : d(dd)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.

    // acquire lock

    d->lock->mutex.lock();

    // store lock count

    count = d->lock->lockCount;

    // set lock count to 0

    d->lock->lockCount = 0;

    // unlock

    for (int i = 0 ; i < count ; ++i)
    {
        d->lock->mutex.unlock();
    }
}

void BdEngineBackendPrivate::AbstractUnlocker::finishAcquire()
{
    // drop lock acquired in first line. Main mutex is now free.
    // We maintain lock order (first main mutex, second error lock mutex)
    // but we drop main mutex lock for waiting on the cond var.

    d->lock->mutex.unlock();
}

BdEngineBackendPrivate::AbstractUnlocker::~AbstractUnlocker()
{
    // lock main mutex as often as it was locked before

    for (int i = 0 ; i < count ; ++i)
    {
        d->lock->mutex.lock();
    }

    // update lock count

    d->lock->lockCount += count;
}

// -----------------------------------------------------------------------------------------

BdEngineBackendPrivate::AbstractWaitingUnlocker::AbstractWaitingUnlocker(BdEngineBackendPrivate* const d,
                                                                         QMutex* const mutex,
                                                                         QWaitCondition* const condVar)
    : AbstractUnlocker(d),
      mutex           (mutex),
      condVar         (condVar)
{
    // Why two mutexes? The main mutex is recursive and won't work with a condvar.
    // lock condvar mutex (lock only if main mutex is locked)

    mutex->lock();

    finishAcquire();
}

BdEngineBackendPrivate::AbstractWaitingUnlocker::~AbstractWaitingUnlocker()
{
    // unlock condvar mutex. Both mutexes are now free.

    mutex->unlock();

    // now base class destructor is executed, reallocating main mutex
}

bool BdEngineBackendPrivate::AbstractWaitingUnlocker::wait(unsigned long time)
{
    return condVar->wait(mutex, time);
}

// -----------------------------------------------------------------------------------------

/**
 * This suspends the current thread if the query status as
 * set by setFlag() is Wait and until the thread is woken with wakeAll().
 * The CoreDbAccess mutex will be unlocked while waiting.
 */
void BdEngineBackendPrivate::ErrorLocker::wait()
{
    // we use a copy of the flag under lock of the errorLockMutex to be able to check it here

    while (d->errorLockOperationStatus == BdEngineBackend::Wait)
    {
        wait();
    }
}

// -----------------------------------------------------------------------------------------

BdEngineBackend::BdEngineBackend(const QString& backendName,
                                 DbEngineLocking* const locking)
    : d_ptr(new BdEngineBackendPrivate(this))
{
    d_ptr->init(backendName, locking);
}

BdEngineBackend::BdEngineBackend(const QString& backendName,
                                 DbEngineLocking* const locking,
                                 BdEngineBackendPrivate& dd)
    : d_ptr(&dd)
{
    d_ptr->init(backendName, locking);
}

BdEngineBackend::~BdEngineBackend()
{
    Q_D(BdEngineBackend);

    close();

    delete d;
}

DbEngineConfigSettings BdEngineBackend::configElement() const
{
    Q_D(const BdEngineBackend);

    return DbEngineConfig::element(d->parameters.databaseType);
}

DbEngineAction BdEngineBackend::getDBAction(const QString& actionName) const
{
    Q_D(const BdEngineBackend);

    DbEngineAction action = configElement().sqlStatements.value(actionName);

    if (action.name.isNull())
    {
        qCWarning(DIGIKAM_DBENGINE_LOG) << "No DB action defined for" << actionName
                                        << "! Implementation missing for this database type ("
                                        << d->parameters.databaseType << ").";
    }

    return action;
}

BdEngineBackend::DbType BdEngineBackend::databaseType() const
{
    Q_D(const BdEngineBackend);

    return d->parameters.isSQLite() ? DbType::SQLite : DbType::MySQL;
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const DbEngineAction& action,
                                                          QList<QVariant>* const values,
                                                          QVariant* const lastInsertId)
{
    return execDBAction(action, QMap<QString, QVariant>(), values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const QString& action,
                                                          QList<QVariant>* const values,
                                                          QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), QMap<QString, QVariant>(), values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const QString& action,
                                                          const QMap<QString, QVariant>& bindingMap,
                                                          QList<QVariant>* const values, QVariant* const lastInsertId)
{
    return execDBAction(getDBAction(action), bindingMap, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execDBAction(const DbEngineAction& action,
                                                          const QMap<QString, QVariant>& bindingMap,
                                                          QList<QVariant>* const values, QVariant* const lastInsertId)
{
    Q_D(BdEngineBackend);

    BdEngineBackend::QueryState returnResult = BdEngineBackend::QueryState(BdEngineBackend::NoErrors);
    QSqlDatabase db                          = d->databaseForThread();

    if (action.name.isNull())
    {
        qCWarning(DIGIKAM_DBENGINE_LOG) << "Attempt to execute null action";

        return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
    }
/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Executing DBAction ["<<  action.name  <<"]";
*/
    bool wrapInTransaction = (action.mode == QLatin1String("transaction"));

    if (wrapInTransaction)
    {
        beginTransaction();
    }

    for (const DbEngineActionElement& actionElement : std::as_const(action.dbActionElements))
    {
        BdEngineBackend::QueryState result;

        if      (actionElement.mode == QLatin1String("query"))
        {
            result = execSql(actionElement.statement, bindingMap, values, lastInsertId);
        }
        else if (actionElement.mode == QLatin1String("unprepared"))
        {
            result = execDirectSqlWithResult(actionElement.statement, values, lastInsertId);
        }
        else
        {
            result = execDirectSql(actionElement.statement);
        }

        if (result != BdEngineBackend::NoErrors)
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while executing DBAction ["
                                          << action.name << "] Statement ["
                                          << actionElement.statement << "]";
            returnResult = result;

/*
            if (wrapInTransaction && !db.rollback())
            {
                qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while rollback changes of previous DBAction.";
            }
*/

            break;
        }
    }

    if (wrapInTransaction)
    {
        commitTransaction();
    }

/*
    if ((returnResult == BdEngineBackend::NoErrors && wrapInTransaction) && !db.commit())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while committing changes of previous DBAction.";
    }
*/

    return returnResult;
}

QSqlQuery BdEngineBackend::execDBActionQuery(const QString& action,
                                             const QMap<QString,
                                             QVariant>& bindingMap)
{
    return execDBActionQuery(getDBAction(action), bindingMap);
}

QSqlQuery BdEngineBackend::execDBActionQuery(const DbEngineAction& action, const QMap<QString, QVariant>& bindingMap)
{
    Q_D(BdEngineBackend);

    QSqlDatabase db = d->databaseForThread();
/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Executing DBAction ["<<  action.name  <<"]";
*/
    QSqlQuery result;

    for (const DbEngineActionElement& actionElement : std::as_const(action.dbActionElements))
    {
        if (actionElement.mode == QLatin1String("query"))
        {
            result = execQuery(actionElement.statement, bindingMap);
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error, only DBActions with mode 'query' are allowed at this call!";
        }

        if (result.lastError().isValid() && !result.lastError().nativeErrorCode().isEmpty())
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while executing DBAction [" <<  action.name
                                          << "] Statement [" << actionElement.statement
                                          << "] Errornr. [" << result.lastError() << "]";
            break;
        }
    }

    return result;
}

void BdEngineBackend::setDbEngineErrorHandler(DbEngineErrorHandler* const handler)
{
    Q_D(BdEngineBackend);

    delete d->errorHandler;

    d->errorHandler = handler;
}

bool BdEngineBackend::isCompatible(const DbEngineParameters& parameters)
{
    return QSqlDatabase::drivers().contains(parameters.databaseType);
}

bool BdEngineBackend::checkOrSetWALMode()
{
    Q_D(BdEngineBackend);

    if (!d->parameters.isSQLite())
    {
        return false;
    }

    QList<QVariant> values;

    execSql(QString::fromUtf8("PRAGMA journal_mode;"), &values);

    // cppcheck-suppress knownConditionTrueFalse
    if (values.isEmpty())
    {
        return false;
    }

    QSqlDatabase db = d->databaseForThread();
    QString walMode = values.constFirst().toString().toUpper();
    QString dbName  = QUrl::fromLocalFile(db.databaseName()).fileName();

    if       (walMode == QLatin1String("DELETE"))
    {
        if (d->parameters.walMode)
        {
            execSql(QString::fromUtf8("PRAGMA journal_mode=WAL;"), &values);
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "WAL mode is disabled for" << dbName;

            return false;
        }
    }
    else  if (walMode == QLatin1String("WAL"))
    {
        if (!d->parameters.walMode)
        {
            execSql(QString::fromUtf8("PRAGMA journal_mode=DELETE;"), &values);
        }
        else
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "WAL mode is enabled for" << dbName;

            return true;
        }
    }

    if (values.isEmpty())
    {
        return false;
    }

    walMode      = values.constFirst().toString().toUpper();
    bool newMode = (walMode == QLatin1String("WAL"));

    qCDebug(DIGIKAM_DBENGINE_LOG) << "WAL mode is set to" << newMode
                                  << "for" << dbName;

    return newMode;
}

bool BdEngineBackend::open(const DbEngineParameters& parameters)
{
    Q_D(BdEngineBackend);
    d->parameters = parameters;

    // This will make possibly opened thread dbs reload at next access

    d->currentValidity++;

    int retries = 0;

    Q_FOREVER
    {
        QSqlDatabase database = d->databaseForThread();

        if (!database.isOpen())
        {
/*
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Error while opening the database. Trying again.";
*/
            if (connectionErrorHandling(retries++))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        else
        {
            break;
        }
    }

    d->status = Open;

    return true;
}

void BdEngineBackend::close()
{
    Q_D(BdEngineBackend);

    d->closeDatabaseForThread();
    d->status = Unavailable;
}

BdEngineBackend::Status BdEngineBackend::status() const
{
    Q_D(const BdEngineBackend);

    return d->status;
}

/*
bool BdEngineBackend::execSql(const QString& sql, QStringList* const values)
{
    QSqlQuery query = execQuery(sql);

    if (!query.isActive())
    {
        return false;
    }

    if (!values)
    {
        return true;
    }

    int count = query.record().count();

    while (query.next())
    {
        for (int i = 0 ; i < count ; ++i)
        {
            (*values) << query.value(i).toString();
        }
    }

    return true;
}
*/

QList<QVariant> BdEngineBackend::readToList(QSqlQuery& query)
{
    QList<QVariant> list;

    QSqlRecord record = query.record();
    int count         = record.count();

    while (query.next())
    {
        for (int i = 0 ; i < count ; ++i)
        {
            list << query.value(i);
        }
    }
/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Setting result value list ["<< list <<"]";
*/
    return list;
}

BdEngineBackend::QueryState BdEngineBackend::handleQueryResult(QSqlQuery& query,
                                                               QList<QVariant>* const values,
                                                               QVariant* const lastInsertId)
{
    if (!query.isActive())
    {
        if (query.lastError().type() == QSqlError::ConnectionError)
        {
            return BdEngineBackend::QueryState(BdEngineBackend::ConnectionError);
        }
        else
        {
            return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
        }
    }

    if (lastInsertId)
    {
        (*lastInsertId) = query.lastInsertId();
    }

    if (values)
    {
        (*values) = readToList(query);
    }

    return BdEngineBackend::QueryState(BdEngineBackend::NoErrors);
}

// -------------------------------------------------------------------------------------

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql);

    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1);

    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1,
                                                     const QVariant& boundValue2,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1, boundValue2);

    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1,
                                                     const QVariant& boundValue2,
                                                     const QVariant& boundValue3,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3);

    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QVariant& boundValue1,
                                                     const QVariant& boundValue2,
                                                     const QVariant& boundValue3,
                                                     const QVariant& boundValue4,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValue1, boundValue2, boundValue3, boundValue4);

    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql,
                                                     const QList<QVariant>& boundValues,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql, boundValues);

    return handleQueryResult(query, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(const QString& sql, const QMap<QString, QVariant>& bindingMap,
                                                     QList<QVariant>* const values, QVariant* const lastInsertId)
{
    QSqlQuery query = execQuery(sql, bindingMap);

    return handleQueryResult(query, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

BdEngineBackend::QueryState BdEngineBackend::execSql(QSqlQuery& preparedQuery,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    exec(preparedQuery);

    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(QSqlQuery& preparedQuery,
                                                     const QVariant& boundValue1,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1);

    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(QSqlQuery& preparedQuery,
                                                     const QVariant& boundValue1,
                                                     const QVariant& boundValue2,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2);

    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(QSqlQuery& preparedQuery,
                                                     const QVariant& boundValue1,
                                                     const QVariant& boundValue2,
                                                     const QVariant& boundValue3,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3);

    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(QSqlQuery& preparedQuery,
                                                     const QVariant& boundValue1,
                                                     const QVariant& boundValue2,
                                                     const QVariant& boundValue3,
                                                     const QVariant& boundValue4,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValue1, boundValue2, boundValue3, boundValue4);

    return handleQueryResult(preparedQuery, values, lastInsertId);
}

BdEngineBackend::QueryState BdEngineBackend::execSql(QSqlQuery& preparedQuery,
                                                     const QList<QVariant>& boundValues,
                                                     QList<QVariant>* const values,
                                                     QVariant* const lastInsertId)
{
    execQuery(preparedQuery, boundValues);

    return handleQueryResult(preparedQuery, values, lastInsertId);
}

// -------------------------------------------------------------------------------------

QSqlQuery BdEngineBackend::execQuery(const QString& sql, const QVariant& boundValue1)
{
    QSqlQuery query = prepareQuery(sql);
/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Trying to sql ["<< sql <<"] query ["<<query.lastQuery()<<"]";
*/
    execQuery(query, boundValue1);

    return query;
}

QSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1,
                                     const QVariant& boundValue2)
{
    QSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2);

    return query;
}

QSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1,
                                     const QVariant& boundValue2,
                                     const QVariant& boundValue3)
{
    QSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3);

    return query;
}

QSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                     const QVariant& boundValue1,
                                     const QVariant& boundValue2,
                                     const QVariant& boundValue3,
                                     const QVariant& boundValue4)
{
    QSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValue1, boundValue2, boundValue3, boundValue4);

    return query;
}

QSqlQuery BdEngineBackend::execQuery(const QString& sql,
                                     const QList<QVariant>& boundValues)
{
    QSqlQuery query = prepareQuery(sql);
    execQuery(query, boundValues);

    return query;
}

QSqlQuery BdEngineBackend::execQuery(const QString& sql)
{
    QSqlQuery query = prepareQuery(sql);
/*
    qCDebug(DIGIKAM_DBENGINE_LOG)<<"execQuery: Using statement ["<< query.lastQuery() <<"]";
*/
    exec(query);

    return query;
}

// -------------------------------------------------------------------------------------

void BdEngineBackend::execQuery(QSqlQuery& query, const QVariant& boundValue1)
{
    query.bindValue(0, boundValue1);
    exec(query);
}

void BdEngineBackend::execQuery(QSqlQuery& query,
                                const QVariant& boundValue1,
                                const QVariant& boundValue2)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    exec(query);
}

void BdEngineBackend::execQuery(QSqlQuery& query,
                                const QVariant& boundValue1,
                                const QVariant& boundValue2,
                                const QVariant& boundValue3)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    exec(query);
}

void BdEngineBackend::execQuery(QSqlQuery& query,
                                const QVariant& boundValue1,
                                const QVariant& boundValue2,
                                const QVariant& boundValue3,
                                const QVariant& boundValue4)
{
    query.bindValue(0, boundValue1);
    query.bindValue(1, boundValue2);
    query.bindValue(2, boundValue3);
    query.bindValue(3, boundValue4);
    exec(query);
}

void BdEngineBackend::execQuery(QSqlQuery& query,
                                const QList<QVariant>& boundValues)
{
    for (int i = 0 ; i < boundValues.size() ; ++i)
    {
        query.bindValue(i, boundValues.at(i));
    }

    exec(query);
}

// -------------------------------------------------------------------------------------

QSqlQuery BdEngineBackend::execQuery(const QString& sql, const QMap<QString, QVariant>& bindingMap)
{
    QString preparedString = sql;
    QVariantList valuesToBind;

    if (!bindingMap.isEmpty())
    {
/*
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Prepare statement [" << preparedString << "] with binding map [" << bindingMap << "]";
*/
        QRegularExpression identifierRegExp(QLatin1String(":[A-Za-z0-9]+"));
        int pos = 0;

        while ((pos = preparedString.indexOf(identifierRegExp, pos)) != -1)
        {
            QRegularExpressionMatch regMatch = identifierRegExp.match(preparedString);
            QString namedPlaceholder         = regMatch.captured(0);

            if (!bindingMap.contains(namedPlaceholder))
            {
                qCWarning(DIGIKAM_DBENGINE_LOG) << "Missing place holder" << namedPlaceholder
                                                << "in binding map. The following values are defined for this action:"
                                                << bindingMap.keys() <<". This is a setup error!";

                // TODO: What should we do here? How can we cancel that action?
            }

            QVariant placeHolderValue = bindingMap.value(namedPlaceholder);
            QString replaceStr;

            if (placeHolderValue.userType() == qMetaTypeId<DbEngineActionType>())
            {
                DbEngineActionType actionType = placeHolderValue.value<DbEngineActionType>();
                bool isValue                  = actionType.isValue();
                QVariant value                = actionType.getActionValue();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                if      (value.typeId() == qMetaTypeId<QMap<QString, QVariant> >())

#else

                if      (value.type() == QVariant::Map)

#endif

                {
                    QMap<QString, QVariant> placeHolderMap = value.toMap();
                    QMap<QString, QVariant>::const_iterator iterator;

                    for (iterator = placeHolderMap.constBegin() ; iterator != placeHolderMap.constEnd() ; ++iterator)
                    {
                        const QString& key     = iterator.key();
                        const QVariant& value2 = iterator.value();
                        replaceStr.append(key);
                        replaceStr.append(QLatin1String("= ?"));
                        valuesToBind.append(value2);

                        // Add a semicolon to the statement, if we are not on the last entry

                        if (std::next(iterator, 1) != placeHolderMap.constEnd())
                        {
                            replaceStr.append(QLatin1String(", "));
                        }
                    }
                }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                else if (value.typeId() == qMetaTypeId<QList<QVariant> >())

#else

                else if (value.type() == QVariant::List)

#endif

                {
                    QList<QVariant> placeHolderList = value.toList();
                    QList<QVariant>::const_iterator iterator;

                    for (iterator = placeHolderList.constBegin() ; iterator != placeHolderList.constEnd() ; ++iterator)
                    {
                        const QVariant& entry = *iterator;

                        if (isValue)
                        {
                            replaceStr.append(QLatin1String("?"));
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry.value<QString>());
                        }

                        // Add a semicolon to the statement, if we are not on the last entry

                        if ((iterator + 1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(QLatin1String(", "));
                        }
                    }
                }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                else if (value.typeId() == QMetaType::QStringList)

#else

                else if (value.type() == QVariant::StringList)

#endif

                {
                    QStringList placeHolderList = value.toStringList();
                    QStringList::const_iterator iterator;

                    for (iterator = placeHolderList.constBegin() ; iterator != placeHolderList.constEnd() ; ++iterator)
                    {
                        const QString& entry = *iterator;

                        if (isValue)
                        {
                            replaceStr.append(QLatin1String("?"));
                            valuesToBind.append(entry);
                        }
                        else
                        {
                            replaceStr.append(entry);
                        }

                        // Add a semicolon to the statement, if we are not on the last entry

                        if ((iterator+1) != placeHolderList.constEnd())
                        {
                            replaceStr.append(QLatin1String(", "));
                        }
                    }
                }
                else
                {
                    if (isValue)
                    {
                        replaceStr = QLatin1Char('?');
                        valuesToBind.append(value);
                    }
                    else
                    {
                        replaceStr = value.toString();
                    }
                }
            }
            else
            {
/*
                qCDebug(DIGIKAM_DBENGINE_LOG) << "Bind key ["<< namedPlaceholder << "] to value [" << bindingMap[namedPlaceholder] << "]";
*/
                valuesToBind.append(placeHolderValue);
                replaceStr = QLatin1Char('?');
            }

            preparedString = preparedString.replace(pos, regMatch.capturedLength(), replaceStr);
            pos            = 0; // reset pos
        }
    }
/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Prepared statement [" << preparedString << "] values [" << valuesToBind << "]";
*/
    QSqlQuery query = prepareQuery(preparedString);

    for (int i = 0 ; i < valuesToBind.size() ; ++i)
    {
        query.bindValue(i, valuesToBind.at(i));
    }

    exec(query);

    return query;
}

BdEngineBackend::QueryState BdEngineBackend::execUpsertDBAction(const DbEngineAction& action,
                                                                const QVariant& id,
                                                                const QStringList& fieldNames,
                                                                const QList<QVariant>& values)
{
    QMap<QString, QVariant> parameters;
    QMap<QString, QVariant> fieldValueMap;

    for (int i = 0 ; i < fieldNames.size() ; ++i)
    {
        fieldValueMap.insert(fieldNames.at(i), values.at(i));
    }

    DbEngineActionType fieldValueList = DbEngineActionType::value(fieldValueMap);
    DbEngineActionType fieldList      = DbEngineActionType::fieldEntry(fieldNames);
    DbEngineActionType valueList      = DbEngineActionType::value(values);

    parameters.insert(QLatin1String(":id"),             id);
    parameters.insert(QLatin1String(":fieldValueList"), QVariant::fromValue(fieldValueList));
    parameters.insert(QLatin1String(":fieldList"),      QVariant::fromValue(fieldList));
    parameters.insert(QLatin1String(":valueList"),      QVariant::fromValue(valueList));

    return execDBAction(action, parameters);
}

BdEngineBackend::QueryState BdEngineBackend::execUpsertDBAction(const QString& action,
                                                                const QVariant& id,
                                                                const QStringList& fieldNames,
                                                                const QList<QVariant>& values)
{
    return execUpsertDBAction(getDBAction(action), id, fieldNames, values);
}

bool BdEngineBackend::connectionErrorHandling(int /*retries*/)
{
    Q_D(BdEngineBackend);

    if (d->reconnectOnError())
    {
        if (d->handleWithErrorHandler(nullptr))
        {
            d->closeDatabaseForThread();

            return true;
        }
    }

    return false;
}

bool BdEngineBackend::queryErrorHandling(QSqlQuery& query, int retries)
{
    Q_D(BdEngineBackend);

    if (d->isSQLiteLockError(query))
    {
        if (d->checkRetrySQLiteLockError(retries))
        {
            return true;
        }
    }

    d->debugOutputFailedQuery(query);

    /*
     * Check if the error is query or database related.
     * It seems, that insufficient privileges results only in query errors,
     * the database gives an invalid lastError() value.
     */
    if (query.lastError().isValid())
    {
        d->setDatabaseErrorForThread(query.lastError());
    }
    else
    {
        d->setDatabaseErrorForThread(d->databaseForThread().lastError());
    }

    if (d->isConnectionError(query) && d->reconnectOnError())
    {
        // after connection errors, it can be required
        // to start with a new connection and a fresh, copied query

        d->closeDatabaseForThread();
        query = copyQuery(query);
    }

    if (d->needToHandleWithErrorHandler(query))
    {
        if (d->handleWithErrorHandler(&query))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool BdEngineBackend::transactionErrorHandling(const QSqlError& lastError, int retries)
{
    Q_D(BdEngineBackend);

    if (d->isSQLiteLockTransactionError(lastError))
    {
        if (d->checkRetrySQLiteLockError(retries))
        {
            return true;
        }
    }

    d->debugOutputFailedTransaction(lastError);

    // no experience with other forms of failure

    return false;
}

BdEngineBackend::QueryState BdEngineBackend::execDirectSql(const QString& sql)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
    }

    QSqlQuery query = getQuery();
    int retries     = 0;

    Q_FOREVER
    {
        if (query.exec(sql))
        {
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                // Workaround for an endless locked

                if (d->isSQLiteLockError(query))
                {
                    if (d->resetDatabaseForThread())
                    {
                        beginTransaction();
                    }

                    query = copyQuery(query);
                }

                continue;
            }
            else
            {
                return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
            }
        }
    }

    return BdEngineBackend::QueryState(BdEngineBackend::NoErrors);
}

BdEngineBackend::QueryState BdEngineBackend::execDirectSqlWithResult(const QString& sql,
                                                                     QList<QVariant>* const values,
                                                                     QVariant* const lastInsertId)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
    }

    QSqlQuery query = getQuery();
    int retries     = 0;

    Q_FOREVER
    {
        if (query.exec(sql))
        {
            handleQueryResult(query, values, lastInsertId);
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                // Workaround for an endless locked

                if (d->isSQLiteLockError(query))
                {
                    if (d->resetDatabaseForThread())
                    {
                        beginTransaction();
                    }

                    query = copyQuery(query);
                }

                continue;
            }
            else
            {
                return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
            }
        }
    }

    return BdEngineBackend::QueryState(BdEngineBackend::NoErrors);
}

bool BdEngineBackend::exec(QSqlQuery& query)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return false;
    }

    int retries = 0;

    Q_FOREVER
    {
/*
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Trying to query ["
                                      << query.lastQuery()
                                      << "] values ["
                                      << query.boundValues()
                                      << "]";
*/

        if (query.exec())   // krazy:exclude=crashy
        {
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                // Workaround for an endless locked

                if (d->isSQLiteLockError(query))
                {
                    if (d->resetDatabaseForThread())
                    {
                        beginTransaction();
                    }

                    query = copyQuery(query);
                }

                continue;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

bool BdEngineBackend::execBatch(QSqlQuery& query)
{
    Q_D(BdEngineBackend);

    if (!d->checkOperationStatus())
    {
        return false;
    }

    int retries = 0;

    Q_FOREVER
    {
        if (query.execBatch())
        {
            break;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                // Workaround for an endless locked

                if (d->isSQLiteLockError(query))
                {
                    if (d->resetDatabaseForThread())
                    {
                        beginTransaction();
                    }

                    query = copyQuery(query);
                }

                continue;
            }
            else
            {
                return false;
            }
        }
    }

    return true;
}

QSqlQuery BdEngineBackend::prepareQuery(const QString& sql)
{
    int retries = 0;

    Q_FOREVER
    {
        QSqlQuery query = getQuery();

        if (query.prepare(sql))
        {
            return query;
        }
        else
        {
            if (queryErrorHandling(query, retries++))
            {
                qCDebug(DIGIKAM_DBENGINE_LOG) << "Retry preparing query";

                continue;
            }
            else
            {
                return query;
            }
        }
    }
}

QSqlQuery BdEngineBackend::copyQuery(const QSqlQuery& old)
{
/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Last query was [" << old.lastQuery() << "]";
*/
    QSqlQuery query = prepareQuery(old.lastQuery());
    query.setForwardOnly(old.isForwardOnly());

    // only for positional binding

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QVariantList boundValues = old.boundValues();

#else

    QList<QVariant> boundValues = old.boundValues().values();

#endif

    for (const QVariant& value : std::as_const(boundValues))
    {
/*
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Bind value to query ["<<value<<"]";
*/
        query.addBindValue(value);
    }

    return query;
}

QSqlQuery BdEngineBackend::getQuery()
{
    Q_D(BdEngineBackend);
    QSqlDatabase db = d->databaseForThread();

    QSqlQuery query(db);
    query.setForwardOnly(true);

    return query;
}

BdEngineBackend::QueryState BdEngineBackend::beginTransaction()
{
    Q_D(BdEngineBackend);

    // Call databaseForThread before touching transaction count - open() will reset the count!

    QSqlDatabase db = d->databaseForThread();

    if (d->incrementTransactionCount())
    {
        int retries = 0;

        Q_FOREVER
        {
            if (db.transaction())
            {
                break;
            }
            else
            {
                if (transactionErrorHandling(db.lastError(), retries++))
                {
                    continue;
                }
                else
                {
                    d->decrementTransactionCount();

                    if (db.lastError().type() == QSqlError::ConnectionError)
                    {
                        return BdEngineBackend::QueryState(BdEngineBackend::ConnectionError);
                    }
                    else
                    {
                        return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
                    }
                }
            }
        }

        d->isInTransaction = true;
    }

    return BdEngineBackend::QueryState(BdEngineBackend::NoErrors);
}

BdEngineBackend::QueryState BdEngineBackend::commitTransaction()
{
    Q_D(BdEngineBackend);

    if (d->decrementTransactionCount())
    {
        QSqlDatabase db = d->databaseForThread();
        int retries     = 0;

        Q_FOREVER
        {
            if (db.commit())
            {
                break;
            }
            else
            {
                QSqlError lastErr = db.lastError();

                if (transactionErrorHandling(lastErr, retries++))
                {
                    continue;
                }
                else
                {
                    qCDebug(DIGIKAM_DBENGINE_LOG) << "Failed to commit transaction. Starting rollback.";
                    bool ret = db.rollback();

                    // No need to check feedback for database as driver must support transactions.
                    // Also, in all cases the database error is checked outside this feedback.

                    Q_UNUSED(ret);

                    if (lastErr.type() == QSqlError::ConnectionError)
                    {
                        return BdEngineBackend::QueryState(BdEngineBackend::ConnectionError);
                    }
                    else
                    {
                        return BdEngineBackend::QueryState(BdEngineBackend::SQLError);
                    }
                }
            }
        }

        d->isInTransaction = false;
        d->transactionFinished();
    }

    return BdEngineBackend::QueryState(BdEngineBackend::NoErrors);
}

bool BdEngineBackend::isInTransaction() const
{
    Q_D(const BdEngineBackend);

    return d->isInTransaction;
}

void BdEngineBackend::rollbackTransaction()
{
    Q_D(BdEngineBackend);

    // we leave that out for transaction counting. It's an exceptional condition.

    if (!d->databaseForThread().rollback())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Failed to run database backend rollback transaction.";
    }
}

QStringList BdEngineBackend::tables()
{
    Q_D(BdEngineBackend);

    return d->databaseForThread().tables();
}

QSqlError BdEngineBackend::lastSQLError()
{
    Q_D(BdEngineBackend);

    return d->databaseErrorForThread();
}

QString BdEngineBackend::lastError()
{
    Q_D(BdEngineBackend);

    return d->databaseForThread().lastError().text();
}

int BdEngineBackend::maximumBoundValues() const
{
    Q_D(const BdEngineBackend);

    if (d->parameters.isSQLite())
    {
        return 999;   // SQLITE_MAX_VARIABLE_NUMBER
    }
    else
    {
        return 65535; // MySQL
    }
}

void BdEngineBackend::setForeignKeyChecks(bool check)
{
    Q_D(BdEngineBackend);

    if (d->parameters.isMySQL())
    {
        if (check)
        {
            execSql(QLatin1String("SET FOREIGN_KEY_CHECKS=1;"));
        }
        else
        {
            execSql(QLatin1String("SET FOREIGN_KEY_CHECKS=0;"));
        }
    }
}

QDateTime BdEngineBackend::asDBDateTime(const QDateTime& dateTime) const
{
    Q_D(const BdEngineBackend);

    if (d->parameters.isMySQL())
    {
        return asDateTimeUTC(dateTime);
    }
    else
    {
        return asDateTimeLocal(dateTime);
    }
}

} // namespace Digikam

#include "moc_dbenginebackend.cpp"

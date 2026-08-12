/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Face database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facedbaccess.h"

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "facedbbackend.h"
#include "facedb.h"
#include "facedbschemaupdater.h"
#include "dbengineaccess.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN FaceDbAccessStaticPriv
{
public:

    FaceDbAccessStaticPriv()  = default;
    ~FaceDbAccessStaticPriv() = default;

public:

    FaceDbBackend*     backend      = nullptr;
    FaceDb*            db           = nullptr;
    DbEngineParameters parameters;
    DbEngineLocking    lock;
    QString            lastError;
    bool               initializing = false;
};

FaceDbAccessStaticPriv* FaceDbAccess::d = nullptr;

// -----------------------------------------------------------------------------

class Q_DECL_HIDDEN FaceDbAccessMutexLocker

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    : public QMutexLocker<QRecursiveMutex>

#else

    : public QMutexLocker

#endif

{
public:

    explicit FaceDbAccessMutexLocker(FaceDbAccessStaticPriv* const dd)
        : QMutexLocker(&dd->lock.mutex),
          d           (dd)
    {
        d->lock.lockCount++;
    }

    ~FaceDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

public:

    FaceDbAccessStaticPriv* const d = nullptr;
};

// -----------------------------------------------------------------------------

FaceDbAccess::FaceDbAccess()
{
    // You will want to call setParameters before constructing FaceDbAccess.

    Q_ASSERT(d);

    d->lock.mutex.lock();
    d->lock.lockCount++;

    if (!d->backend->isOpen() && !d->initializing)
    {
        // Avoid endless loops.

        d->initializing = true;
        d->backend->open(d->parameters);
        d->initializing = false;
    }
}

FaceDbAccess::~FaceDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

FaceDbAccess::FaceDbAccess(bool)
{
    // Private constructor, when mutex is locked and
    // backend should not be checked.

    d->lock.mutex.lock();
    d->lock.lockCount++;
}

FaceDb* FaceDbAccess::db() const
{
    return d->db;
}

FaceDbBackend* FaceDbAccess::backend() const
{
    return d->backend;
}

DbEngineParameters FaceDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

bool FaceDbAccess::isInitialized()
{
    return d;
}

void FaceDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d)
    {
        d = new FaceDbAccessStaticPriv();
    }

    /*
        DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);
    */
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void FaceDbAccess::setParameters(const DbEngineParameters& parameters,
                                 FaceScanSettings::FaceRecognitionModel recognizeModel)
{
    if (!d)
    {
        d = new FaceDbAccessStaticPriv();
    }

    FaceDbAccessMutexLocker lock(d);

    if (d->parameters == parameters)
    {
        return;
    }

    if (d->backend && d->backend->isOpen())
    {
        d->backend->close();
    }

    // Kill the old database error handler.

    if (d->backend)
    {
        d->backend->setDbEngineErrorHandler(nullptr);
    }

    d->parameters = parameters;

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = new FaceDbBackend(&d->lock);
        d->db      = new FaceDb(d->backend, recognizeModel);
    }
}

bool FaceDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
    {
        return false;
    }

    // Create an object with private shortcut constructor

    FaceDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "Face database: no database backend available in checkReadyForUse. "
                                      "Did you call setParameters before?";
        return false;
    }

    if (d->backend->isReady())
    {
        return true;
    }

    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
        {
            access.setLastError(i18n("Error opening database backend.\n%1",
                                     d->backend->lastError()));
            return false;
        }
    }

    // Avoid endless loops (if called methods create new FaceDbAccess objects).

    d->initializing = true;

    // Update schema.

    FaceDbSchemaUpdater updater(&access);
    updater.setObserver(observer);

    // Check or set WAL mode for SQLite database from DbEngineParameters.

    d->backend->checkOrSetWALMode();

    if (!d->backend->initSchema(&updater))
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "Face database: cannot process schema initialization";

        d->initializing = false;
        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

const QString& FaceDbAccess::lastError() const
{
    return d->lastError;
}

void FaceDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void FaceDbAccess::cleanUpDatabase()
{
    if (d)
    {
        FaceDbAccessMutexLocker locker(d);

        if (d->backend)
        {
            d->backend->close();
            delete d->db;
            delete d->backend;
        }
    }

    delete d;
    d = nullptr;
}

// ---------------------------------------------------------------------------------

FaceDbAccessUnlock::FaceDbAccessUnlock()
{
    // Acquire lock.

    FaceDbAccess::d->lock.mutex.lock();

    // Store lock count.

    count = FaceDbAccess::d->lock.lockCount;

    // Set lock count to 0.

    FaceDbAccess::d->lock.lockCount = 0;

    // Unlock.

    for (int i = 0 ; i < count ; ++i)
    {
        FaceDbAccess::d->lock.mutex.unlock();
    }

    // Drop lock acquired in first line. Mutex is now free.

    FaceDbAccess::d->lock.mutex.unlock();
}

FaceDbAccessUnlock::FaceDbAccessUnlock(FaceDbAccess* const)
{
    // With the passed pointer, we have assured that the mutex is acquired.
    // Store lock count.

    count = FaceDbAccess::d->lock.lockCount;

    // Set lock count to 0.

    FaceDbAccess::d->lock.lockCount = 0;

    // Unlock.

    for (int i = 0 ; i < count ; ++i)
    {
        FaceDbAccess::d->lock.mutex.unlock();
    }

    // Mutex is now free.
}

FaceDbAccessUnlock::~FaceDbAccessUnlock()
{
    // Lock as often as it was locked before.

    for (int i = 0 ; i < count ; ++i)
    {
        FaceDbAccess::d->lock.mutex.lock();
    }

    // Update lock count.

    FaceDbAccess::d->lock.lockCount += count;
}

} // namespace Digikam

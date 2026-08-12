/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbsdbaccess.h"

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QFileInfo>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "thumbsdbbackend.h"
#include "thumbsdb.h"
#include "thumbsdbschemaupdater.h"
#include "dbengineaccess.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN ThumbsDbAccessStaticPriv
{
public:

    ThumbsDbAccessStaticPriv() = default;

    ThumbsDbBackend*   backend      = nullptr;
    ThumbsDb*          db           = nullptr;
    DbEngineParameters parameters;
    DbEngineLocking    lock;
    QString            lastError;

    bool               initializing = false;
};

ThumbsDbAccessStaticPriv* ThumbsDbAccess::d = nullptr;

// -----------------------------------------------------------------------------

class Q_DECL_HIDDEN ThumbsDbAccessMutexLocker

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    : public QMutexLocker<QRecursiveMutex>

#else

    : public QMutexLocker

#endif

{
public:

    explicit ThumbsDbAccessMutexLocker(ThumbsDbAccessStaticPriv* const dd)
        : QMutexLocker(&dd->lock.mutex),
          d           (dd)
    {
        d->lock.lockCount++;
    }

    ~ThumbsDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

public:

    ThumbsDbAccessStaticPriv* const d = nullptr;
};

// -----------------------------------------------------------------------------

ThumbsDbAccess::ThumbsDbAccess()
{
    // You will want to call setParameters before constructing ThumbsDbAccess.

    Q_ASSERT(d);

    d->lock.mutex.lock();
    d->lock.lockCount++;

    if (!d->backend->isOpen() && !d->initializing)
    {
        // avoid endless loops

        d->initializing = true;

        d->backend->open(d->parameters);

        d->initializing = false;
    }
}

ThumbsDbAccess::~ThumbsDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

ThumbsDbAccess::ThumbsDbAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked

    d->lock.mutex.lock();
    d->lock.lockCount++;
}

ThumbsDb* ThumbsDbAccess::db() const
{
    return d->db;
}

ThumbsDbBackend* ThumbsDbAccess::backend() const
{
    return d->backend;
}

DbEngineParameters ThumbsDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

bool ThumbsDbAccess::isInitialized()
{
    return d;
}

void ThumbsDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d)
    {
        d = new ThumbsDbAccessStaticPriv();
    }
/*
    DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);
*/
    d->backend->setDbEngineErrorHandler(errorhandler);
}

void ThumbsDbAccess::setParameters(const DbEngineParameters& parameters)
{
    if (!d)
    {
        d = new ThumbsDbAccessStaticPriv();
    }

    ThumbsDbAccessMutexLocker lock(d);

    if (d->parameters == parameters)
    {
        return;
    }

    if (d->backend && d->backend->isOpen())
    {
        d->backend->close();
    }

    // Kill the old database error handler

    if (d->backend)
    {
        d->backend->setDbEngineErrorHandler(nullptr);
    }

    d->parameters = parameters;

    if (d->parameters.isMySQL() && !d->parameters.internalServer)
    {
        QFileInfo thumbDB(d->parameters.databaseNameCore);

        if (thumbDB.exists() && thumbDB.isDir() && thumbDB.isAbsolute())
        {
            d->parameters.databaseType     = QLatin1String("QSQLITE");
            d->parameters.databaseNameCore = DbEngineParameters::thumbnailDatabaseFileSQLite(thumbDB.filePath());
        }
    }

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;

        d->backend = new ThumbsDbBackend(&d->lock);
        d->db      = new ThumbsDb(d->backend);
    }
}

bool ThumbsDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
    {
        return false;
    }

    // Create an object with private shortcut constructor

    ThumbsDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: no database backend available in checkReadyForUse. "
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

    // Avoid endless loops (if called methods create new ThumbsDbAccess objects)

    d->initializing = true;

    // Update schema

    ThumbsDbSchemaUpdater updater(&access);
    updater.setObserver(observer);

    // Check or set WAL mode for SQLite database from DbEngineParameters

    d->backend->checkOrSetWALMode();

    if (!d->backend->initSchema(&updater))
    {
        qCWarning(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: cannot process schema initialization";

        d->initializing = false;

        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString ThumbsDbAccess::lastError() const
{
    return d->lastError;
}

void ThumbsDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void ThumbsDbAccess::cleanUpDatabase()
{
    if (d)
    {
        ThumbsDbAccessMutexLocker locker(d);

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

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-28
 * Description : Similarity Database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * SPDX-FileCopyrightText:      2018 by Mario  Frank   <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "similaritydbaccess.h"

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "similaritydbbackend.h"
#include "similaritydb.h"
#include "similaritydbschemaupdater.h"
#include "dbengineaccess.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN SimilarityDbAccessStaticPriv
{
public:

    SimilarityDbAccessStaticPriv() = default;

    SimilarityDbBackend* backend        = nullptr;
    SimilarityDb*        db             = nullptr;
    DbEngineParameters   parameters;
    DbEngineLocking      lock;
    QString              lastError;

    bool                 initializing   = false;
};

SimilarityDbAccessStaticPriv* SimilarityDbAccess::d = nullptr;

// -----------------------------------------------------------------------------

class Q_DECL_HIDDEN SimilarityDbAccessMutexLocker

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    : public QMutexLocker<QRecursiveMutex>

#else

    : public QMutexLocker

#endif

{
public:

    explicit SimilarityDbAccessMutexLocker(SimilarityDbAccessStaticPriv* const dd)
        : QMutexLocker(&dd->lock.mutex),
          d           (dd)
    {
        d->lock.lockCount++;
    }

    ~SimilarityDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

public:

    SimilarityDbAccessStaticPriv* const d = nullptr;
};

// -----------------------------------------------------------------------------

SimilarityDbAccess::SimilarityDbAccess()
{
    // You will want to call setParameters before constructing SimilarityDbAccess.

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

SimilarityDbAccess::~SimilarityDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

SimilarityDbAccess::SimilarityDbAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked

    d->lock.mutex.lock();
    d->lock.lockCount++;
}

SimilarityDb* SimilarityDbAccess::db() const
{
    return d->db;
}

SimilarityDbBackend* SimilarityDbAccess::backend() const
{
    return d->backend;
}

DbEngineParameters SimilarityDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

bool SimilarityDbAccess::isInitialized()
{
    return d;
}

void SimilarityDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d)
    {
        d = new SimilarityDbAccessStaticPriv();
    }

    //DbEngineErrorHandler* const errorhandler = new DbEngineGuiErrorHandler(d->parameters);

    d->backend->setDbEngineErrorHandler(errorhandler);
}

void SimilarityDbAccess::setParameters(const DbEngineParameters& parameters)
{
    if (!d)
    {
        d = new SimilarityDbAccessStaticPriv();
    }

    SimilarityDbAccessMutexLocker lock(d);

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

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;

        d->backend = new SimilarityDbBackend(&d->lock);
        d->db      = new SimilarityDb(d->backend);
    }
}

bool SimilarityDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
    {
        return false;
    }

    // Create an object with private shortcut constructor

    SimilarityDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: no database backend available in checkReadyForUse. "
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

    // Avoid endless loops (if called methods create new SimilarityDbAccess objects)

    d->initializing = true;

    // Update schema

    SimilarityDbSchemaUpdater updater(&access);
    updater.setObserver(observer);

    // Check or set WAL mode for SQLite database from DbEngineParameters

    d->backend->checkOrSetWALMode();

    if (!d->backend->initSchema(&updater))
    {
        qCWarning(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: cannot process schema initialization";

        d->initializing = false;

        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString SimilarityDbAccess::lastError() const
{
    return d->lastError;
}

void SimilarityDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void SimilarityDbAccess::cleanUpDatabase()
{
    if (d)
    {
        SimilarityDbAccessMutexLocker locker(d);

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

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-02
 * Description : Database engine low level error handler
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QMetaType>
#include <QSqlError>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineErrorAnswer
{

public:

    DbEngineErrorAnswer()                         = default;
    virtual ~DbEngineErrorAnswer()                = default;

    virtual void connectionErrorContinueQueries() = 0;
    virtual void connectionErrorAbortQueries()    = 0;

private:

    Q_DISABLE_COPY(DbEngineErrorAnswer)
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT DbEngineErrorHandler : public QObject
{
    Q_OBJECT

public:

    DbEngineErrorHandler();
    ~DbEngineErrorHandler() override;

public Q_SLOTS:

    // NOTE: These all need to be slots, possibly called by queued connection

    /**
     *  In the situation of a connection error,
     *  all threads will be waiting with their queries
     *  and this method is called.
     *  This method can display an error dialog and try to repair
     *  the connection.
     *  It must then call either connectionErrorContinueQueries()
     *  or connectionErrorAbortQueries().
     *  The method is guaranteed to be invoked in the UI thread.
     */
    virtual void connectionError(DbEngineErrorAnswer* answer,
                                 const QSqlError& error,
                                 const QString& query)          = 0;

    /**
     *  In the situation of an error requiring user intervention or information,
     *  all threads will be waiting with their queries
     *  and this method is called.
     *  This method can display an error dialog.
     *  It must then call either connectionErrorContinueQueries()
     *  or connectionErrorAbortQueries().
     *  The method is guaranteed to be invoked in the UI thread.
     */
    virtual void consultUserForError(DbEngineErrorAnswer* answer,
                                     const QSqlError& error,
                                     const QString& query)      = 0;

private:

    /// @note disabled
    DbEngineErrorHandler(QObject*) = delete;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DbEngineErrorAnswer*)

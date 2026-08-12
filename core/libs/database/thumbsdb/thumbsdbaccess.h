/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnail database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DbEngineErrorHandler;
class InitializationObserver;
class ThumbsDb;
class ThumbsDbAccessStaticPriv;
class ThumbsDbBackend;

class DIGIKAM_EXPORT ThumbsDbAccess
{
public:

    /**
     * This class is written in analogy to CoreDbAccess
     * (some features stripped off).
     * For documentation, see coredbaccess.h
     */
    ThumbsDbAccess();
    ~ThumbsDbAccess();

    ThumbsDb*        db()        const;
    ThumbsDbBackend* backend()   const;
    QString          lastError() const;

    /**
     * Set the "last error" message. This method is not for public use.
     */
    void setLastError(const QString& error);

    static DbEngineParameters parameters();

public:

    static void initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler);
    static void setParameters(const DbEngineParameters& parameters);
    static bool checkReadyForUse(InitializationObserver* const observer);
    static bool isInitialized();
    static void cleanUpDatabase();

private:

    explicit ThumbsDbAccess(bool);

    /// @note disabled
    ThumbsDbAccess(const ThumbsDbAccess&)            = delete;
    ThumbsDbAccess& operator=(const ThumbsDbAccess&) = delete;

    static ThumbsDbAccessStaticPriv* d;
};

} // namespace Digikam

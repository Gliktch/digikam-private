/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Face database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "collectionscannerobserver.h"
#include "facescansettings.h"

namespace Digikam
{

class FaceDb;
class FaceDbAccessStaticPriv;
class FaceDbBackend;
class DbEngineErrorHandler;

class DIGIKAM_GUI_EXPORT FaceDbAccess
{
public:

    /**
     * @brief This class is written in analogy to CoreDbAccess
     * (some features stripped off).
     * For advanced documentation, see at coredbaccess.h
     */
    FaceDbAccess();
    ~FaceDbAccess();

    FaceDb*        db()         const;
    FaceDbBackend* backend()    const;
    const QString& lastError()  const;

    /**
     * @brief Set the "last error" message. This method is not for public use.
     */
    void setLastError(const QString& error);

    static DbEngineParameters parameters();

public:

    static void initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler);
    static void setParameters(const DbEngineParameters& parameters, FaceScanSettings::FaceRecognitionModel recognizeModel);
    static bool checkReadyForUse(InitializationObserver* const observer);
    static bool isInitialized();
    static void cleanUpDatabase();

private:

    explicit FaceDbAccess(bool);

    /// @note disabled
    FaceDbAccess(const FaceDbAccess&)            = delete;
    FaceDbAccess& operator=(const FaceDbAccess&) = delete;

private:

    friend class FaceDbAccessUnlock;
    static FaceDbAccessStaticPriv* d;
};

// ------------------------------------------------------------------------------------------

class FaceDbAccessUnlock
{
public:

    /**
     * @brief Acquire an object of this class if you want to assure
     * that the FaceDbAccess is _not_ held during the lifetime of the object.
     * At creation, the lock is obtained shortly, then all locks are released.
     * At destruction, all locks are acquired again.
     * If you need to access any locked structures during lifetime, acquire a new
     * FaceDbAccess.
     */
    FaceDbAccessUnlock();
    explicit FaceDbAccessUnlock(FaceDbAccess* const access);
    ~FaceDbAccessUnlock();

private:

    int count = 0;

private:

    /// @note disabled
    FaceDbAccessUnlock(const FaceDbAccessUnlock&)            = delete;
    FaceDbAccessUnlock& operator=(const FaceDbAccessUnlock&) = delete;
};

} // namespace Digikam

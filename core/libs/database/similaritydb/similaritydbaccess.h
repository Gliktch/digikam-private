/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-28
 * Description : Similarity database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * SPDX-FileCopyrightText:      2018 by Mario  Frank   <mario dot frank at uni minus potsdam dot de>
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
class SimilarityDb;
class SimilarityDbAccessStaticPriv;
class SimilarityDbBackend;

class DIGIKAM_DATABASE_EXPORT SimilarityDbAccess
{
public:

    /**
     * This class is written in analogy to CoreDbAccess
     * (some features stripped off).
     * For documentation, see coredbaccess.h
     */
    SimilarityDbAccess();
    ~SimilarityDbAccess();

    SimilarityDb*        db()        const;
    SimilarityDbBackend* backend()   const;
    QString              lastError() const;

    /**
     * Set the "last error" message. This method is not for public use.
     */
    void setLastError(const QString& error);

    /**
     * This static method returns the current db parameters.
     * @return the current db parameters.
     */
    static DbEngineParameters parameters();

public:

    /**
     * This static method initialises the error handler for the similarity db.
     * @param errorhandler The error handler.
     */
    static void initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler);

    /**
     * This static method sets the database parameters that are needed to initialise the db connection.
     * @param parameters The db parameters.
     */
    static void setParameters(const DbEngineParameters& parameters);

    /**
     * This static method checks if the similarity db is ready for use.
     * @param observer the observer.
     * @return true, if the database is ready for use.
     */
    static bool checkReadyForUse(InitializationObserver* const observer);

    /**
     * This static method returns if the similarity db is initialised.
     * @return true, if the similarityDb is initialised.
     */
    static bool isInitialized();

    /**
     * This static method removes the connection to the similarity database.
     */
    static void cleanUpDatabase();

private:

    /**
     * Constructs the db access without checking the connectivity to the database itself.
     */
    explicit SimilarityDbAccess(bool);

    /// @note disabled
    SimilarityDbAccess(const SimilarityDbAccess&)            = delete;
    SimilarityDbAccess& operator=(const SimilarityDbAccess&) = delete;

    static SimilarityDbAccessStaticPriv* d;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CollectionScanner;

class DIGIKAM_DATABASE_EXPORT CollectionScannerObserver
{
public:

    CollectionScannerObserver()          = default;
    virtual ~CollectionScannerObserver() = default;

    virtual bool continueQuery()         = 0;

private:

    Q_DISABLE_COPY(CollectionScannerObserver)
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT InitializationObserver : public CollectionScannerObserver
{
public:

    enum UpdateResult
    {
        UpdateSuccess,
        UpdateError,
        UpdateErrorMustAbort
    };

public:

    InitializationObserver()                                                         = default;
    ~InitializationObserver() override                                               = default;

    virtual void moreSchemaUpdateSteps(int numberOfSteps)                            = 0;
    virtual void schemaUpdateProgress(const QString& message, int numberOfSteps = 1) = 0;
    virtual void finishedSchemaUpdate(UpdateResult result)                           = 0;
    virtual void connectCollectionScanner(CollectionScanner* const scanner)          = 0;
    virtual void error(const QString& errorMessage)                                  = 0;

private:

    Q_DISABLE_COPY(InitializationObserver)
};

} // namespace Digikam

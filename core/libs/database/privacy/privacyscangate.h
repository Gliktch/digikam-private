/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSharedPointer>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyScanDisposition
{
    Unprotected                 = 1,
    ProtectedProxyExpected      = 2,
    PrivacyInspectionRequired   = 3,
    RootRecovering              = 4,
    RootOffline                 = 5,
    RootIdentityMismatch        = 6,
    CompatibilityOriginalExposed = 7
};

class DIGIKAM_DATABASE_EXPORT PrivacyScanRequest
{
public:

    int       albumRootId = -1;
    qlonglong imageId = -1;
    QString   absolutePath;
    bool      pathExists = false;
    qlonglong byteSize = -1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyScanGateProvider
{
public:

    PrivacyScanGateProvider()          = default;
    virtual ~PrivacyScanGateProvider() = default;

    virtual PrivacyScanDisposition evaluate(const PrivacyScanRequest& request) const = 0;
    virtual bool hasDeferredRoots() const = 0;
    virtual bool rootContainsProtectedItems(int albumRootId) const = 0;

private:

    Q_DISABLE_COPY(PrivacyScanGateProvider)
};

class DIGIKAM_DATABASE_EXPORT PrivacyScanGate
{
public:

    static void setProvider(const QSharedPointer<const PrivacyScanGateProvider>& provider);
    static void resetProvider();

    static PrivacyScanDisposition evaluate(const PrivacyScanRequest& request);
    static bool hasDeferredRoots();
    static bool rootContainsProtectedItems(int albumRootId);
};

} // namespace Digikam

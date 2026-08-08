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

#include <QList>
#include <QSharedPointer>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

enum class PrivacyAnalysisDisposition
{
    Allowed           = 1,
    ProtectedExcluded = 2,
    Unavailable       = 3
};

enum class PrivacyAnalysisNotice
{
    None                            = 1,
    ProtectedItemsExcluded          = 2,
    AnalysisUnavailable             = 3,
    ProtectedItemsAndUnavailable    = 4
};

class DIGIKAM_DATABASE_EXPORT PrivacyAnalysisSelectionResult
{
public:

    int excludedCount() const;
    PrivacyAnalysisNotice notice() const;

public:

    QList<qlonglong> allowedImageIds;
    int              requestedCount = 0;
    int              protectedExcludedCount = 0;
    int              unavailableExcludedCount = 0;
};

class DIGIKAM_DATABASE_EXPORT PrivacyAnalysisGateProvider
{
public:

    PrivacyAnalysisGateProvider()          = default;
    virtual ~PrivacyAnalysisGateProvider() = default;

    virtual PrivacyAnalysisDisposition analysisDisposition(qlonglong imageId) const = 0;

private:

    Q_DISABLE_COPY(PrivacyAnalysisGateProvider)
};

/**
 * Process-wide, thread-safe facade for automated face, autotag and similarity
 * work. Missing/replaced providers and invalid decisions fail closed. UI code
 * maps PrivacyAnalysisNotice plus the result counts to localized text.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyAnalysisGate
{
public:

    static void setProvider(const QSharedPointer<const PrivacyAnalysisGateProvider>& provider);
    static void resetProvider();

    static PrivacyAnalysisDisposition evaluate(qlonglong imageId);
    static bool mayAnalyze(qlonglong imageId);
    static PrivacyAnalysisSelectionResult filter(const QList<qlonglong>& imageIds);
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyanalysisgate.h"

// Qt includes

#include <QGlobalStatic>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QWriteLocker>

namespace Digikam
{

namespace
{

class PrivacyAnalysisGateData
{
public:

    QReadWriteLock                                lock;
    QSharedPointer<const PrivacyAnalysisGateProvider> provider;
    quint64                                       generation = 0;
};

Q_GLOBAL_STATIC(PrivacyAnalysisGateData, analysisGateData)

bool isKnownDisposition(PrivacyAnalysisDisposition disposition)
{
    return ((disposition == PrivacyAnalysisDisposition::Allowed)           ||
            (disposition == PrivacyAnalysisDisposition::ProtectedExcluded) ||
            (disposition == PrivacyAnalysisDisposition::Unavailable));
}

PrivacyAnalysisDisposition evaluateWithProvider(
    qlonglong imageId,
    const QSharedPointer<const PrivacyAnalysisGateProvider>& provider)
{
    if ((imageId <= 0) || !provider)
    {
        return PrivacyAnalysisDisposition::Unavailable;
    }

    const PrivacyAnalysisDisposition disposition = provider->analysisDisposition(imageId);

    return isKnownDisposition(disposition)
         ? disposition
         : PrivacyAnalysisDisposition::Unavailable;
}

} // namespace

int PrivacyAnalysisSelectionResult::excludedCount() const
{
    return (protectedExcludedCount + unavailableExcludedCount);
}

PrivacyAnalysisNotice PrivacyAnalysisSelectionResult::notice() const
{
    if ((protectedExcludedCount > 0) && (unavailableExcludedCount > 0))
    {
        return PrivacyAnalysisNotice::ProtectedItemsAndUnavailable;
    }

    if (protectedExcludedCount > 0)
    {
        return PrivacyAnalysisNotice::ProtectedItemsExcluded;
    }

    if (unavailableExcludedCount > 0)
    {
        return PrivacyAnalysisNotice::AnalysisUnavailable;
    }

    return PrivacyAnalysisNotice::None;
}

void PrivacyAnalysisGate::setProvider(
    const QSharedPointer<const PrivacyAnalysisGateProvider>& provider)
{
    QWriteLocker locker(&analysisGateData->lock);
    analysisGateData->provider = provider;

    if (++analysisGateData->generation == 0)
    {
        ++analysisGateData->generation;
    }
}

void PrivacyAnalysisGate::resetProvider()
{
    setProvider(QSharedPointer<const PrivacyAnalysisGateProvider>());
}

PrivacyAnalysisDisposition PrivacyAnalysisGate::evaluate(qlonglong imageId)
{
    QSharedPointer<const PrivacyAnalysisGateProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&analysisGateData->lock);
        provider   = analysisGateData->provider;
        generation = analysisGateData->generation;
    }

    const PrivacyAnalysisDisposition disposition = evaluateWithProvider(imageId, provider);

    {
        QReadLocker locker(&analysisGateData->lock);

        if ((generation != analysisGateData->generation) ||
            (provider != analysisGateData->provider))
        {
            return PrivacyAnalysisDisposition::Unavailable;
        }
    }

    return disposition;
}

bool PrivacyAnalysisGate::mayAnalyze(qlonglong imageId)
{
    return (evaluate(imageId) == PrivacyAnalysisDisposition::Allowed);
}

PrivacyAnalysisSelectionResult PrivacyAnalysisGate::filter(
    const QList<qlonglong>& imageIds)
{
    PrivacyAnalysisSelectionResult result;
    result.requestedCount = imageIds.size();

    QSharedPointer<const PrivacyAnalysisGateProvider> provider;
    quint64 generation = 0;

    {
        QReadLocker locker(&analysisGateData->lock);
        provider   = analysisGateData->provider;
        generation = analysisGateData->generation;
    }

    for (qlonglong imageId : imageIds)
    {
        switch (evaluateWithProvider(imageId, provider))
        {
            case PrivacyAnalysisDisposition::Allowed:
            {
                result.allowedImageIds << imageId;
                break;
            }

            case PrivacyAnalysisDisposition::ProtectedExcluded:
            {
                ++result.protectedExcludedCount;
                break;
            }

            case PrivacyAnalysisDisposition::Unavailable:
            {
                ++result.unavailableExcludedCount;
                break;
            }
        }
    }

    {
        QReadLocker locker(&analysisGateData->lock);

        if ((generation != analysisGateData->generation) ||
            (provider != analysisGateData->provider))
        {
            result.allowedImageIds.clear();
            result.protectedExcludedCount = 0;
            result.unavailableExcludedCount = result.requestedCount;
        }
    }

    return result;
}

} // namespace Digikam

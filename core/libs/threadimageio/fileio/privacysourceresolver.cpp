/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-08-08
 * Description : privacy-aware image source resolution interface
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacysourceresolver.h"

// Qt includes

#include <QCryptographicHash>
#include <QDir>
#include <QGlobalStatic>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QWriteLocker>

// Local includes

#include "privacycachetransition.h"

namespace Digikam
{

namespace
{

class PrivacySourceResolverData
{
public:

    QReadWriteLock                             lock;
    QSharedPointer<const PrivacySourceProvider> provider;
    quint64                                    generation = 0;
};

Q_GLOBAL_STATIC(PrivacySourceResolverData, resolverData)

const QLatin1String invalidProviderNamespace("invalid-provider-result");
const QLatin1String changedProviderNamespace("provider-changed-during-resolution");

PrivacySourceResult stampedResult(PrivacySourceResult result, quint64 generation)
{
    result.resolverGeneration = generation;

    return result;
}

} // namespace

PrivacySourceResult PrivacySourceResult::notHandled()
{
    return PrivacySourceResult();
}

PrivacySourceResult PrivacySourceResult::resolved(const QString& physicalFilePath,
                                                  const QString& cacheNamespace,
                                                  CachePolicy cachePolicy)
{
    PrivacySourceResult result;
    result.disposition      = Resolved;
    result.physicalFilePath = physicalFilePath;
    result.cacheNamespace   = cacheNamespace;
    result.cachePolicy      = cachePolicy;

    return result;
}

PrivacySourceResult PrivacySourceResult::denied(const QString& cacheNamespace)
{
    PrivacySourceResult result;
    result.disposition    = Denied;
    result.cacheNamespace = cacheNamespace;
    result.cachePolicy    = MemoryOnly;

    return result;
}

void PrivacySourceResolver::setProvider(const QSharedPointer<const PrivacySourceProvider>& provider)
{
    QSharedPointer<const PrivacySourceProvider> previous;

    {
        QWriteLocker locker(&resolverData->lock);
        previous.swap(resolverData->provider);
        resolverData->provider = provider;

        if (++resolverData->generation == 0)
        {
            ++resolverData->generation;
        }
    }
}

void PrivacySourceResolver::resetProvider()
{
    setProvider(QSharedPointer<const PrivacySourceProvider>());
}

PrivacySourceResult PrivacySourceResolver::resolve(const PrivacySourceRequest& request)
{
    const QString initiallyBlockedNamespace =
        PrivacyCacheTransition::blockedCacheNamespace(request.logicalFilePath);

    if (!initiallyBlockedNamespace.isEmpty())
    {
        return stampedResult(PrivacySourceResult::denied(initiallyBlockedNamespace),
                             currentGeneration());
    }

    QSharedPointer<const PrivacySourceProvider> provider;
    quint64 providerGeneration = 0;

    {
        QReadLocker locker(&resolverData->lock);
        provider           = resolverData->provider;
        providerGeneration = resolverData->generation;
    }

    if (!provider)
    {
        quint64 currentResolverGeneration = 0;
        bool providerChanged = false;

        {
            QReadLocker locker(&resolverData->lock);
            currentResolverGeneration = resolverData->generation;
            providerChanged = ((providerGeneration != resolverData->generation) ||
                               resolverData->provider);
        }

        if (providerChanged)
        {
            return stampedResult(PrivacySourceResult::denied(changedProviderNamespace),
                                 currentResolverGeneration);
        }

        const QString finallyBlockedNamespace =
            PrivacyCacheTransition::blockedCacheNamespace(request.logicalFilePath);

        if (!finallyBlockedNamespace.isEmpty())
        {
            return stampedResult(PrivacySourceResult::denied(finallyBlockedNamespace),
                                 currentGeneration());
        }

        return stampedResult(PrivacySourceResult::notHandled(), providerGeneration);
    }

    PrivacySourceResult result = provider->resolve(request);

    {
        QReadLocker locker(&resolverData->lock);

        if ((providerGeneration != resolverData->generation) ||
            (provider != resolverData->provider))
        {
            return stampedResult(PrivacySourceResult::denied(changedProviderNamespace),
                                 resolverData->generation);
        }
    }

    const QString finallyBlockedNamespace =
        PrivacyCacheTransition::blockedCacheNamespace(request.logicalFilePath);

    if (!finallyBlockedNamespace.isEmpty())
    {
        return stampedResult(PrivacySourceResult::denied(finallyBlockedNamespace),
                             currentGeneration());
    }

    if (result.disposition == PrivacySourceResult::NotHandled)
    {
        return stampedResult(PrivacySourceResult::notHandled(), providerGeneration);
    }

    if (result.cacheNamespace.isEmpty() ||
        ((result.cachePolicy != PrivacySourceResult::MemoryOnly) &&
         (result.cachePolicy != PrivacySourceResult::Persistent)))
    {
        return stampedResult(PrivacySourceResult::denied(invalidProviderNamespace),
                             providerGeneration);
    }

    if ((result.disposition == PrivacySourceResult::Resolved) &&
        !result.physicalFilePath.isEmpty()                    &&
        QDir::isAbsolutePath(result.physicalFilePath)         &&
        !result.physicalFilePath.contains(QChar::Null))
    {
        return stampedResult(result, providerGeneration);
    }

    if (result.disposition == PrivacySourceResult::Denied)
    {
        return stampedResult(PrivacySourceResult::denied(result.cacheNamespace),
                             providerGeneration);
    }

    return stampedResult(PrivacySourceResult::denied(invalidProviderNamespace),
                         providerGeneration);
}

quint64 PrivacySourceResolver::currentGeneration()
{
    QReadLocker locker(&resolverData->lock);

    return resolverData->generation;
}

bool PrivacySourceResolver::advanceGenerationIfCurrent(quint64 expectedGeneration)
{
    QWriteLocker locker(&resolverData->lock);

    if (resolverData->generation != expectedGeneration)
    {
        return false;
    }

    if (++resolverData->generation == 0)
    {
        ++resolverData->generation;
    }

    return true;
}

QString PrivacySourceResolver::cacheNamespaceDigest(const QString& cacheNamespace)
{
    if (cacheNamespace.isEmpty())
    {
        return QString();
    }

    const QByteArray digest = QCryptographicHash::hash(cacheNamespace.toUtf8(),
                                                       QCryptographicHash::Sha256);

    return QString::fromLatin1(digest.toHex());
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-08-08
 * Description : privacy cache transition lifecycle
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacycachetransition.h"

// Qt includes

#include <QCryptographicHash>
#include <QGlobalStatic>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// Local includes

#include "loadingcacheinterface.h"
#include "loadingdescription.h"
#include "privacysourceresolver.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace
{

class TransitionState
{
public:

    ThumbnailIdentifier priorIdentifier;
    quint64             serial          = 0;
    bool                barrierReady    = false;
    bool                purgeInProgress = false;
    bool                purgeComplete   = false;
};

class TransitionData
{
public:

    QMutex                          mutex;
    QHash<QString, TransitionState> active;
    QHash<QString, quint64>         completed;
    QHash<QString, quint64>         rolledBack;
    QHash<QString, QHash<QPair<QString, quint64>, int> > sourceUsers;
    QHash<QString, int>             persistentWriters;
    QWaitCondition                  stateChanged;
    quint64                         nextSerial = 0;
};

Q_GLOBAL_STATIC(TransitionData, transitionData)

bool samePersistentIdentity(const ThumbnailIdentifier& first,
                            const ThumbnailIdentifier& second)
{
    return ((first.filePath                 == second.filePath)                 &&
            (first.id                       == second.id)                       &&
            (first.cacheNamespace           == second.cacheNamespace)           &&
            (first.sourceResolverGeneration == second.sourceResolverGeneration));
}

ThumbnailIdentifier persistentIdentityOnly(const ThumbnailIdentifier& identifier)
{
    ThumbnailIdentifier sanitized = identifier;
    sanitized.sourceFilePath.clear();
    sanitized.sourceAccessDenied = false;

    return sanitized;
}

} // namespace

PrivacySourceUseGuard::PrivacySourceUseGuard(
    const LoadingDescription& description)
    : m_logicalFilePath    (description.filePath),
      m_cacheNamespace     (description.privacyCacheNamespace()),
      m_resolverGeneration(description.sourceResolverGeneration())
{
    if (m_logicalFilePath.isEmpty()             ||
        !description.sourceResolutionApplied() ||
        description.isSourceDenied())
    {
        return;
    }

    QMutexLocker locker(&transitionData->mutex);

    if (transitionData->active.contains(m_logicalFilePath))
    {
        return;
    }

    const QPair<QString, quint64> snapshot(m_cacheNamespace,
                                           m_resolverGeneration);
    transitionData->sourceUsers[m_logicalFilePath][snapshot] += 1;
    m_acquired  = true;
    m_registered = true;
}

PrivacySourceUseGuard::PrivacySourceUseGuard(
    const QString& logicalFilePath,
    const PrivacySourceResult& resolvedSource)
    : m_logicalFilePath    (logicalFilePath),
      m_cacheNamespace     (resolvedSource.cacheNamespace),
      m_resolverGeneration(resolvedSource.resolverGeneration)
{
    if (m_logicalFilePath.isEmpty() ||
        (resolvedSource.disposition == PrivacySourceResult::Denied))
    {
        return;
    }

    QMutexLocker locker(&transitionData->mutex);

    if (transitionData->active.contains(m_logicalFilePath))
    {
        return;
    }

    const QPair<QString, quint64> snapshot(m_cacheNamespace,
                                           m_resolverGeneration);
    transitionData->sourceUsers[m_logicalFilePath][snapshot] += 1;
    m_acquired   = true;
    m_registered = true;
}

PrivacySourceUseGuard::~PrivacySourceUseGuard()
{
    release();
}

bool PrivacySourceUseGuard::isAcquired() const
{
    return m_acquired;
}

void PrivacySourceUseGuard::release()
{
    if (!m_registered)
    {
        return;
    }

    QMutexLocker locker(&transitionData->mutex);
    auto users = transitionData->sourceUsers.find(m_logicalFilePath);

    if (users != transitionData->sourceUsers.end())
    {
        const QPair<QString, quint64> snapshot(m_cacheNamespace,
                                               m_resolverGeneration);
        auto count = users->find(snapshot);

        if (count != users->end())
        {
            const int remaining = count.value() - 1;

            if (remaining <= 0)
            {
                users->erase(count);
            }
            else
            {
                count.value() = remaining;
            }
        }

        if (users->isEmpty())
        {
            transitionData->sourceUsers.erase(users);
        }

        transitionData->stateChanged.wakeAll();
    }

    m_registered = false;
}

PrivacyPersistentCacheWriteGuard::PrivacyPersistentCacheWriteGuard(
    const QString& logicalFilePath,
    bool enabled)
    : m_logicalFilePath(logicalFilePath)
{
    if (!enabled)
    {
        m_acquired = true;

        return;
    }

    if (logicalFilePath.isEmpty())
    {
        return;
    }

    QMutexLocker locker(&transitionData->mutex);

    if (transitionData->active.contains(logicalFilePath))
    {
        return;
    }

    transitionData->persistentWriters[logicalFilePath] += 1;
    m_acquired  = true;
    m_registered = true;
}

PrivacyPersistentCacheWriteGuard::~PrivacyPersistentCacheWriteGuard()
{
    release();
}

bool PrivacyPersistentCacheWriteGuard::isAcquired() const
{
    return m_acquired;
}

void PrivacyPersistentCacheWriteGuard::release()
{
    if (!m_registered)
    {
        return;
    }

    QMutexLocker locker(&transitionData->mutex);
    auto writers = transitionData->persistentWriters.find(m_logicalFilePath);

    if (writers != transitionData->persistentWriters.end())
    {
        const int remaining = writers.value() - 1;

        if (remaining <= 0)
        {
            transitionData->persistentWriters.erase(writers);
        }
        else
        {
            writers.value() = remaining;
        }

        transitionData->stateChanged.wakeAll();
    }

    m_registered = false;
}

bool PrivacyCacheTransitionToken::isValid() const
{
    return (!m_logicalFilePath.isEmpty() && (m_serial != 0));
}

QString PrivacyCacheTransitionToken::logicalFilePath() const
{
    return m_logicalFilePath;
}

ThumbnailIdentifier PrivacyCacheTransitionToken::priorThumbnailIdentifier() const
{
    return m_priorIdentifier;
}

void ThreadImageIOPrivacyCacheTransitionBackend::evictRamCaches(const QString& logicalFilePath)
{
    LoadingCacheInterface::cleanFileCache(logicalFilePath);
}

bool ThreadImageIOPrivacyCacheTransitionBackend::removePersistentThumbnail(
    const ThumbnailIdentifier& identifier,
    const QRect& detailRect)
{
    return ThumbnailLoadThread::deleteThumbnailFromPersistentCache(identifier, detailRect);
}

PrivacyCacheTransitionToken PrivacyCacheTransition::begin(
    const ThumbnailIdentifier& priorIdentifier)
{
    PrivacyCacheTransitionToken token;

    if (priorIdentifier.filePath.isEmpty() ||
        !priorIdentifier.sourceResolutionApplied)
    {
        return token;
    }

    QMutexLocker locker(&transitionData->mutex);
    auto existing = transitionData->active.constFind(priorIdentifier.filePath);

    if ((existing == transitionData->active.constEnd()) &&
        !transitionData->active.isEmpty())
    {
        return token;
    }

    if (existing != transitionData->active.constEnd())
    {
        while ((existing != transitionData->active.constEnd()) &&
               !existing->barrierReady)
        {
            transitionData->stateChanged.wait(locker.mutex());
            existing = transitionData->active.constFind(priorIdentifier.filePath);
        }

        if (existing == transitionData->active.constEnd())
        {
            return token;
        }

        if (!samePersistentIdentity(existing->priorIdentifier, priorIdentifier))
        {
            return token;
        }

        token.m_logicalFilePath = priorIdentifier.filePath;
        token.m_priorIdentifier = existing->priorIdentifier;
        token.m_serial          = existing->serial;

        return token;
    }

    if (PrivacySourceResolver::currentGeneration() !=
        priorIdentifier.sourceResolverGeneration)
    {
        return token;
    }

    TransitionState state;
    state.priorIdentifier = persistentIdentityOnly(priorIdentifier);

    if (++transitionData->nextSerial == 0)
    {
        ++transitionData->nextSerial;
    }

    state.serial = transitionData->nextSerial;
    transitionData->active.insert(priorIdentifier.filePath, state);

    while ((transitionData->persistentWriters.value(priorIdentifier.filePath) > 0) ||
           !transitionData->sourceUsers.value(priorIdentifier.filePath).isEmpty())
    {
        transitionData->stateChanged.wait(locker.mutex());
    }

    auto publishedState = transitionData->active.find(priorIdentifier.filePath);

    if ((publishedState == transitionData->active.end()) ||
        (publishedState->serial != state.serial))
    {
        return token;
    }

    // Recheck after publishing the barrier. If the provider changed in the
    // narrow validation-to-insert window, do not pretend this token captured
    // the old generation before the transition. A change after this check sees
    // the already-published barrier and is a valid lifecycle ordering.

    if (PrivacySourceResolver::currentGeneration() !=
        priorIdentifier.sourceResolverGeneration)
    {
        transitionData->active.remove(priorIdentifier.filePath);
        transitionData->stateChanged.wakeAll();

        return token;
    }

    publishedState->barrierReady = true;
    transitionData->stateChanged.wakeAll();

    token.m_logicalFilePath = priorIdentifier.filePath;
    token.m_priorIdentifier = state.priorIdentifier;
    token.m_serial          = state.serial;

    return token;
}

PrivacyCacheTransition::Result PrivacyCacheTransition::purge(
    const PrivacyCacheTransitionToken& token,
    const PrivacyCacheTransitionInventory& inventory,
    PrivacyCacheTransitionBackend* const backend)
{
    Result result;

    if (!backend || !token.isValid())
    {
        return result;
    }

    {
        QMutexLocker locker(&transitionData->mutex);
        auto state = transitionData->active.find(token.m_logicalFilePath);

        if ((state == transitionData->active.end()) ||
            (state->serial != token.m_serial))
        {
            return result;
        }

        if (state->purgeInProgress)
        {
            result.status = TransitionInProgress;

            return result;
        }

        state->purgeInProgress = true;
    }

    const auto finishPurgeAttempt = [&token](bool complete)
    {
        QMutexLocker locker(&transitionData->mutex);
        auto state = transitionData->active.find(token.m_logicalFilePath);

        if ((state != transitionData->active.end()) &&
            (state->serial == token.m_serial))
        {
            state->purgeInProgress = false;
            state->purgeComplete   = state->purgeComplete || complete;
        }
    };

    const ThumbnailIdentifier prior =
        persistentIdentityOnly(token.priorThumbnailIdentifier());
    const bool priorIsNamespaced = !prior.cacheNamespace.isEmpty();

    if (((inventory.direction != PrivacyCacheTransitionInventory::Protect) &&
         (inventory.direction != PrivacyCacheTransitionInventory::Unprotect)) ||
        ((inventory.direction == PrivacyCacheTransitionInventory::Protect) &&
         priorIsNamespaced) ||
        ((inventory.direction == PrivacyCacheTransitionInventory::Unprotect) &&
         !priorIsNamespaced) ||
        ((inventory.direction == PrivacyCacheTransitionInventory::Unprotect) &&
         (!inventory.detailAndFaceRectangles.isEmpty() ||
          inventory.detailAndFaceInventoryComplete ||
          inventory.legacyPrimaryAliasInventoryComplete)))
    {
        result.status = InvalidInventory;
        finishPurgeAttempt(false);

        return result;
    }

    for (const QRect& rect : inventory.detailAndFaceRectangles)
    {
        if (!rect.isValid() || rect.isEmpty())
        {
            result.status = InvalidInventory;
            finishPurgeAttempt(false);

            return result;
        }
    }

    backend->evictRamCaches(token.logicalFilePath());
    result.ramCachesEvicted = true;

    bool purgeSucceeded = true;

    if (inventory.direction == PrivacyCacheTransitionInventory::Protect)
    {
        if (inventory.legacyPrimaryAliasInventoryComplete)
        {
            purgeSucceeded = backend->removePersistentThumbnail(prior, QRect()) &&
                             purgeSucceeded;
            ++result.primaryEntriesAddressed;
        }

        for (const QRect& rect : inventory.detailAndFaceRectangles)
        {
            purgeSucceeded = backend->removePersistentThumbnail(prior, rect) &&
                             purgeSucceeded;
            ++result.detailEntriesAddressed;
        }
    }
    else
    {
        purgeSucceeded = backend->removePersistentThumbnail(prior, QRect()) &&
                         purgeSucceeded;
        ++result.primaryEntriesAddressed;
    }

    if (!purgeSucceeded)
    {
        result.status = PersistentPurgeFailed;
        finishPurgeAttempt(false);

        return result;
    }

    if ((inventory.direction == PrivacyCacheTransitionInventory::Protect) &&
        (!inventory.detailAndFaceInventoryComplete ||
         !inventory.legacyPrimaryAliasInventoryComplete))
    {
        result.status = IncompleteOwnershipInventory;
        finishPurgeAttempt(false);

        return result;
    }

    finishPurgeAttempt(true);

    result.status = Complete;

    return result;
}

bool PrivacyCacheTransition::finish(const PrivacyCacheTransitionToken& token)
{
    if (!token.isValid())
    {
        return false;
    }

    QMutexLocker locker(&transitionData->mutex);
    auto state = transitionData->active.find(token.m_logicalFilePath);

    if (state == transitionData->active.end())
    {
        return (transitionData->completed.value(token.m_logicalFilePath) == token.m_serial);
    }

    if ((state->serial != token.m_serial) ||
        state->purgeInProgress            ||
        !state->purgeComplete             ||
        !PrivacySourceResolver::advanceGenerationIfCurrent(
            token.m_priorIdentifier.sourceResolverGeneration))
    {
        return false;
    }

    transitionData->completed.insert(token.m_logicalFilePath, token.m_serial);
    transitionData->active.erase(state);

    return true;
}

bool PrivacyCacheTransition::rollback(const PrivacyCacheTransitionToken& token)
{
    if (!token.isValid())
    {
        return false;
    }

    QMutexLocker locker(&transitionData->mutex);
    auto state = transitionData->active.find(token.m_logicalFilePath);

    if (state == transitionData->active.end())
    {
        return (transitionData->rolledBack.value(token.m_logicalFilePath) == token.m_serial);
    }

    if ((state->serial != token.m_serial) ||
        state->purgeInProgress            ||
        (PrivacySourceResolver::currentGeneration() !=
         token.m_priorIdentifier.sourceResolverGeneration))
    {
        return false;
    }

    transitionData->rolledBack.insert(token.m_logicalFilePath, token.m_serial);
    transitionData->active.erase(state);

    return true;
}

bool PrivacyCacheTransition::isActive(const PrivacyCacheTransitionToken& token)
{
    if (!token.isValid())
    {
        return false;
    }

    QMutexLocker locker(&transitionData->mutex);
    const auto state = transitionData->active.constFind(token.m_logicalFilePath);

    return ((state != transitionData->active.constEnd()) &&
            (state->serial == token.m_serial));
}

QString PrivacyCacheTransition::blockedCacheNamespace(const QString& logicalFilePath)
{
    QMutexLocker locker(&transitionData->mutex);
    const auto state = transitionData->active.constFind(logicalFilePath);

    if (state == transitionData->active.constEnd())
    {
        return QString();
    }

    QByteArray input = logicalFilePath.toUtf8();
    input.append('\0');
    input.append(QByteArray::number(state->serial));
    const QByteArray digest = QCryptographicHash::hash(input,
                                                       QCryptographicHash::Sha256).toHex();

    return (QLatin1String("privacy-cache-transition:/v1/") +
            QString::fromLatin1(digest));
}

} // namespace Digikam

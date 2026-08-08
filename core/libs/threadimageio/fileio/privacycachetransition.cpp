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

// C++ includes

#include <utility>

// Qt includes

#include <QCryptographicHash>
#include <QGlobalStatic>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// Local includes

#include "loadingcacheinterface.h"
#include "loadsavethread.h"
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

bool validInventoryIdentifier(const ThumbnailIdentifier& identifier,
                              const PrivacyCacheTransitionToken& token)
{
    const ThumbnailIdentifier prior = token.priorThumbnailIdentifier();

    return (!identifier.filePath.isEmpty()                   &&
            (identifier.filePath == token.logicalFilePath()) &&
            (identifier.id       == prior.id));
}

ThumbnailIdentifier persistentIdentityOnly(const ThumbnailIdentifier& identifier)
{
    ThumbnailIdentifier sanitized = identifier;
    sanitized.sourceFilePath.clear();
    sanitized.sourceAccessDenied = false;

    return sanitized;
}

} // namespace

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

ThreadImageIOPrivacyCacheTransitionBackend::ThreadImageIOPrivacyCacheTransitionBackend(
    const QList<LoadSaveThread*>& loadThreads)
    : m_loadThreads(loadThreads)
{
}

bool ThreadImageIOPrivacyCacheTransitionBackend::cancelAndDrain(
    const QList<ThumbnailIdentifier>& priorIdentifiers)
{
    for (LoadSaveThread* const thread : std::as_const(m_loadThreads))
    {
        if (!thread)
        {
            return false;
        }

        for (const ThumbnailIdentifier& priorIdentifier : priorIdentifiers)
        {
            if (!thread->stopLoadingAndWaitForSource(
                    priorIdentifier.filePath,
                    priorIdentifier.cacheNamespace,
                    priorIdentifier.sourceResolverGeneration))
            {
                return false;
            }
        }
    }

    return true;
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

    while (transitionData->persistentWriters.value(priorIdentifier.filePath) > 0)
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

    QList<ThumbnailIdentifier> identities;
    identities << persistentIdentityOnly(token.priorThumbnailIdentifier());

    ThumbnailIdentifier legacy(token.logicalFilePath());
    legacy.id = token.priorThumbnailIdentifier().id;
    identities << legacy;

    for (const ThumbnailIdentifier& supplied : inventory.priorPersistentIdentifiers)
    {
        if (!validInventoryIdentifier(supplied, token))
        {
            result.status = InvalidInventory;
            finishPurgeAttempt(false);

            return result;
        }

        const ThumbnailIdentifier sanitized = persistentIdentityOnly(supplied);
        bool duplicate = false;

        for (const ThumbnailIdentifier& identity : std::as_const(identities))
        {
            if (samePersistentIdentity(identity, sanitized))
            {
                duplicate = true;
                break;
            }
        }

        if (!duplicate)
        {
            identities << sanitized;
        }
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

    if (!backend->cancelAndDrain(identities))
    {
        result.status = CancellationFailed;
        finishPurgeAttempt(false);

        return result;
    }

    backend->evictRamCaches(token.logicalFilePath());
    result.ramCachesEvicted = true;

    bool purgeSucceeded = true;

    for (const ThumbnailIdentifier& identity : std::as_const(identities))
    {
        if (!identity.cacheNamespace.isEmpty() ||
            inventory.legacyPrimaryAliasInventoryComplete)
        {
            purgeSucceeded = backend->removePersistentThumbnail(identity, QRect()) &&
                             purgeSucceeded;
            ++result.primaryEntriesAddressed;
        }

        for (const QRect& rect : inventory.detailAndFaceRectangles)
        {
            purgeSucceeded = backend->removePersistentThumbnail(identity, rect) &&
                             purgeSucceeded;
            ++result.detailEntriesAddressed;
        }
    }

    if (!purgeSucceeded)
    {
        result.status = PersistentPurgeFailed;
        finishPurgeAttempt(false);

        return result;
    }

    if (!inventory.loadThreadInventoryComplete                    ||
        !inventory.priorPersistentIdentifierInventoryComplete     ||
        !inventory.detailAndFaceInventoryComplete                 ||
        !inventory.legacyPrimaryAliasInventoryComplete)
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
        (PrivacySourceResolver::currentGeneration() ==
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

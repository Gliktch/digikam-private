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

#pragma once

// Qt includes

#include <QList>
#include <QRect>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "thumbnailinfo.h"

namespace Digikam
{

class LoadingDescription;
class PrivacySourceResult;

class DIGIKAM_EXPORT PrivacySourceUseGuard
{
public:

    /**
     * Registers one resolved, non-denied source snapshot before a loading task
     * rechecks freshness or touches caches/files. Ordinary NotHandled sources
     * are registered with an empty namespace: they are the source snapshots
     * which Protect must drain most critically.
     */
    explicit PrivacySourceUseGuard(const LoadingDescription& description);
    PrivacySourceUseGuard(const QString& logicalFilePath,
                          const PrivacySourceResult& resolvedSource);
    ~PrivacySourceUseGuard();

    bool isAcquired() const;
    void release();

private:

    QString m_logicalFilePath;
    QString m_cacheNamespace;
    quint64 m_resolverGeneration = 0;
    bool    m_acquired           = false;
    bool    m_registered         = false;

private:

    Q_DISABLE_COPY(PrivacySourceUseGuard)
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacyPersistentCacheWriteGuard
{
public:

    /**
     * Registers an in-flight persistent operation before it reads or writes a
     * cache backend. A transition publishes its resolver barrier first, denies
     * later guards, and waits for previously registered guards to leave before
     * begin() returns.
     */
    explicit PrivacyPersistentCacheWriteGuard(const QString& logicalFilePath,
                                              bool enabled = true);
    ~PrivacyPersistentCacheWriteGuard();

    bool isAcquired() const;
    void release();

private:

    QString m_logicalFilePath;
    bool    m_acquired  = false;
    bool    m_registered = false;

private:

    Q_DISABLE_COPY(PrivacyPersistentCacheWriteGuard)
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacyCacheTransitionToken
{
public:

    PrivacyCacheTransitionToken() = default;

    bool                isValid()                   const;
    QString             logicalFilePath()           const;
    ThumbnailIdentifier priorThumbnailIdentifier() const;

private:

    friend class PrivacyCacheTransition;

    QString             m_logicalFilePath;
    ThumbnailIdentifier m_priorIdentifier;
    quint64             m_serial = 0;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacyCacheTransitionInventory
{
public:

    enum Direction
    {
        Protect,
        Unprotect
    };

    /**
     * Complete set of actual ordinary detail/crop/face CustomIdentifier rows
     * for this logical item. Used only by Protect.
     */
    QList<QRect> detailAndFaceRectangles;

    /**
     * This assertion is made only after query-safe enumeration of the actual
     * CustomIdentifiers table while the resolver barrier is active.
     */
    bool detailAndFaceInventoryComplete = false;

    /**
     * True only after the higher item-identity layer has inventoried every
     * exact-duplicate/hardlink alias which may share an ordinary ThumbsDB
     * primary row. Without this proof, legacy primary deletion is skipped;
     * namespaced and rectangle-custom entries remain exactly addressable.
     */
    bool legacyPrimaryAliasInventoryComplete = false;

    Direction direction = Protect;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacyCacheTransitionBackend
{
public:

    PrivacyCacheTransitionBackend()          = default;
    virtual ~PrivacyCacheTransitionBackend() = default;

    virtual void evictRamCaches(const QString& logicalFilePath) = 0;
    virtual bool removePersistentThumbnail(const ThumbnailIdentifier& identifier,
                                           const QRect& detailRect) = 0;

private:

    Q_DISABLE_COPY(PrivacyCacheTransitionBackend)
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT ThreadImageIOPrivacyCacheTransitionBackend
    : public PrivacyCacheTransitionBackend
{
public:

    ThreadImageIOPrivacyCacheTransitionBackend() = default;

    void evictRamCaches(const QString& logicalFilePath) override;
    bool removePersistentThumbnail(const ThumbnailIdentifier& identifier,
                                   const QRect& detailRect) override;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacyCacheTransition
{
public:

    enum Status
    {
        Complete,
        InvalidToken,
        InvalidInventory,
        IncompleteOwnershipInventory,
        TransitionInProgress,
        PersistentPurgeFailed
    };

    class Result
    {
    public:

        Status status = InvalidToken;
        int    primaryEntriesAddressed = 0;
        int    detailEntriesAddressed  = 0;
        bool   ramCachesEvicted        = false;
    };

public:

    /**
     * Atomically blocks new source resolutions/use guards for the logical path
     * if the supplied old snapshot still belongs to the current resolver
     * generation, then waits for prior source users and persistent writers.
     * Version 1 admits one live path transition process-wide. Repeating begin()
     * for the same active snapshot returns the same token; another path is
     * rejected until that token finishes or rolls back.
     */
    static PrivacyCacheTransitionToken begin(const ThumbnailIdentifier& priorIdentifier);

    /**
     * After begin() has drained old-source work, evict all logical RAM
     * namespaces and remove only exact persistent addresses derived from the
     * token/inventory. Safe exact cleanup is performed even for incomplete
     * inventories, but the path remains blocked and the result is
     * IncompleteOwnershipInventory.
     */
    static Result purge(const PrivacyCacheTransitionToken& token,
                        const PrivacyCacheTransitionInventory& inventory,
                        PrivacyCacheTransitionBackend* const backend);

    /**
     * After the higher owner has published its runtime state, atomically
     * advances the unchanged resolver provider's expected generation and
     * releases the path barrier. Repeating completion for the same token is
     * idempotent; a stale token can never release a newer transition.
     */
    static bool finish(const PrivacyCacheTransitionToken& token);

    /**
     * Abandons a transition only while the captured provider generation is
     * still current. Once the provider changed, rollback cannot prove that the
     * old presentation is authoritative and the barrier remains fail-closed.
     */
    static bool rollback(const PrivacyCacheTransitionToken& token);

    static bool isActive(const PrivacyCacheTransitionToken& token);

    /** Internal resolver hook: returns empty when the path is not blocked. */
    static QString blockedCacheNamespace(const QString& logicalFilePath);
};

} // namespace Digikam

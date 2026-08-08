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

class LoadSaveThread;

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

    /**
     * Older exact source snapshots retained by the higher lifecycle journal.
     * The transition token already contributes its immediately prior snapshot.
     */
    QList<ThumbnailIdentifier> priorPersistentIdentifiers;

    /**
     * Complete set of rectangles used for detail, crop and face thumbnails for
     * this logical item. ThreadImageIO cannot derive this set from ThumbsDB.
     */
    QList<QRect> detailAndFaceRectangles;

    /**
     * These assertions must be made by the higher coordinator after consulting
     * its consumer/thread registry, transition journal/history and item/face
     * inventory. False keeps the resolver barrier active and prevents
     * transition completion.
     */
    bool loadThreadInventoryComplete = false;
    bool priorPersistentIdentifierInventoryComplete = false;
    bool detailAndFaceInventoryComplete = false;

    /**
     * True only after the higher item-identity layer has inventoried every
     * exact-duplicate/hardlink alias which may share an ordinary ThumbsDB
     * primary row. Without this proof, legacy primary deletion is skipped;
     * namespaced and rectangle-custom entries remain exactly addressable.
     */
    bool legacyPrimaryAliasInventoryComplete = false;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacyCacheTransitionBackend
{
public:

    PrivacyCacheTransitionBackend()          = default;
    virtual ~PrivacyCacheTransitionBackend() = default;

    virtual bool cancelAndDrain(const QList<ThumbnailIdentifier>& priorIdentifiers) = 0;
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

    /**
     * The caller retains every thread for the synchronous purge call and must
     * provide the complete set of LoadSaveThread consumers which could
     * hold this logical source. The inventory completion flag records that
     * higher-level proof; this backend deliberately has no global raw-pointer
     * registry.
     */
    explicit ThreadImageIOPrivacyCacheTransitionBackend(
        const QList<LoadSaveThread*>& loadThreads);

    bool cancelAndDrain(const QList<ThumbnailIdentifier>& priorIdentifiers) override;
    void evictRamCaches(const QString& logicalFilePath) override;
    bool removePersistentThumbnail(const ThumbnailIdentifier& identifier,
                                   const QRect& detailRect) override;

private:

    QList<LoadSaveThread*> m_loadThreads;
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
        CancellationFailed,
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
     * Atomically blocks new source resolutions for the logical path if the
     * supplied old snapshot still belongs to the current resolver generation.
     * Repeating begin() for the same active snapshot returns the same token.
     */
    static PrivacyCacheTransitionToken begin(const ThumbnailIdentifier& priorIdentifier);

    /**
     * Cancel/drain old-source work, evict all logical RAM namespaces and remove
     * only exact persistent addresses derived from the token/inventory. Safe
     * exact cleanup is performed even for incomplete inventories, but the path
     * remains blocked and the result is IncompleteOwnershipInventory.
     */
    static Result purge(const PrivacyCacheTransitionToken& token,
                        const PrivacyCacheTransitionInventory& inventory,
                        PrivacyCacheTransitionBackend* const backend);

    /**
     * Releases the path barrier only after a complete purge and a resolver
     * provider-generation change. Repeating completion for the same token is
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

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

#pragma once

// C++ includes

#include <functional>

// Qt includes

#include <QByteArray>
#include <QSharedPointer>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PrivacySourceRequest
{
public:

    enum Consumer
    {
        Image,
        Preview,
        Thumbnail,
        PreparedAccess
    };

public:

    QString  logicalFilePath;
    QVariant itemReference;
    Consumer consumer = Image;
    bool     detailThumbnail = false;
    int      assetRole = 1;
    int      assetOrdinal = 0;
    std::function<bool()> isCancelled;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacySourceLifetime
{
public:

    PrivacySourceLifetime()          = default;
    virtual ~PrivacySourceLifetime() = default;

private:

    Q_DISABLE_COPY(PrivacySourceLifetime)
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacySourceResult
{
public:

    enum Disposition
    {
        NotHandled,
        Resolved,
        Denied
    };

    enum CachePolicy
    {
        MemoryOnly,
        Persistent
    };

public:

    static PrivacySourceResult notHandled();
    static PrivacySourceResult resolved(const QString& physicalFilePath,
                                        const QString& cacheNamespace,
                                        CachePolicy cachePolicy = MemoryOnly);
    static PrivacySourceResult resolvedMemory(const QByteArray& encodedBytes,
                                              const QString& cacheNamespace);
    static PrivacySourceResult denied(const QString& cacheNamespace);

public:

    Disposition disposition = NotHandled;
    QString     physicalFilePath;
    QByteArray  encodedBytes;
    QString     cacheNamespace;
    CachePolicy cachePolicy = Persistent;
    QSharedPointer<PrivacySourceLifetime> lifetimeOwner;

    /**
     * Resolver-owned provider generation. Providers must not set or interpret
     * this value; PrivacySourceResolver overwrites it on every result.
     */
    quint64     resolverGeneration = 0;
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacySourceProvider
{
public:

    PrivacySourceProvider()          = default;
    virtual ~PrivacySourceProvider() = default;

    /**
     * Implementations are called without a resolver lock and must be safe for
     * concurrent calls. NotHandled is valid only for requests the provider
     * knows are unprotected. A handled result must carry a non-empty,
     * deterministic cache namespace which changes whenever authorization,
     * presentation state, or source content generation could change the
     * returned pixels. The namespace must not contain credentials or key
     * material; hashing it for cache identity is not secret storage. Returning
     * malformed handled data is normalized to a denied result. Handled
     * Results may instead carry small immutable encoded bytes for a thumbnail
     * decoder; providers must never use that form for Image or Preview
     * consumers. Handled results default to memory-only caching. Persistent is appropriate only
     * when the provider knows the selected pixels are safe to retain outside
     * the protected derivative store (for example, a public locked proxy).
     *
     * A result is an authorization snapshot, not a filesystem lease. Before a
     * relock unmounts or deletes a physical source, the owner must first stop
     * new resolutions, cancel/drain tasks using the old namespace, clear the
     * RAM caches, and only then revoke the runtime plaintext.
     */
    virtual PrivacySourceResult resolve(const PrivacySourceRequest& request) const = 0;

private:

    Q_DISABLE_COPY(PrivacySourceProvider)
};

// -----------------------------------------------------------------------------

class DIGIKAM_EXPORT PrivacySourceResolver
{
public:

    static void setProvider(const QSharedPointer<const PrivacySourceProvider>& provider);
    static void resetProvider();

    static PrivacySourceResult resolve(const PrivacySourceRequest& request);

    /**
     * Publishes the logical paths intentionally focused by one thumbnail view.
     * Multiple views are unioned. The returned paths are the exact global
     * reveal-state delta and should have their RAM caches evicted/repainted.
     */
    static QSet<QString> setThumbnailRevealPaths(
        quintptr requester, const QSet<QString>& logicalFilePaths);
    static QSet<QString> clearThumbnailRevealPaths(quintptr requester);
    static bool thumbnailRevealRequested(const QString& logicalFilePath);

    /** Resolver generation used by cache-transition snapshot validation. */
    static quint64 currentGeneration();

    /**
     * Stable, non-sensitive representation suitable for composing cache keys.
     * An empty namespace returns an empty string.
     */
    static QString cacheNamespaceDigest(const QString& cacheNamespace);

private:

    /** Advances only the still-installed provider generation captured by a
     * cache transition. Kept private so ordinary callers cannot publish a
     * generation independently of the transition lifecycle. */
    static bool advanceGenerationIfCurrent(quint64 expectedGeneration);

    friend class PrivacyCacheTransition;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacythreadimagestillitemcachegate.h"

// Qt includes

#include <QDir>
#include <QMutex>
#include <QMutexLocker>

// Local includes

#include "iteminfo.h"
#include "loadingdescription.h"
#include "privacycachetransition.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN PrivacyThreadImageIOStillItemCacheGate::Private
{
public:

    bool matches(qlonglong candidateImageId, const QString& candidatePath,
                 bool candidateProtecting) const
    {
        return (imageId == candidateImageId) &&
               (logicalPath == candidatePath) &&
               (protecting == candidateProtecting);
    }

    bool hasTransition() const
    {
        return token.isValid();
    }

    void clear()
    {
        imageId = -1;
        logicalPath.clear();
        protecting = false;
        aliasInventoryComplete = false;
        token = PrivacyCacheTransitionToken();
    }

public:

    qlonglong                           imageId = -1;
    QString                             logicalPath;
    bool                                protecting = false;
    bool                                aliasInventoryComplete = false;
    QMutex                              mutex;
    PrivacyCacheTransitionToken         token;
    ThreadImageIOPrivacyCacheTransitionBackend backend;
};

namespace
{

QString verifiedLogicalPath(qlonglong imageId, const QString& logicalPath)
{
    const QString cleanPath = QDir::cleanPath(logicalPath);

    if ((imageId <= 0) || !QDir::isAbsolutePath(cleanPath))
    {
        return QString();
    }

    const ItemInfo info(imageId);

    if (info.isNull() ||
        (QDir::cleanPath(info.filePath()) != cleanPath))
    {
        return QString();
    }

    return cleanPath;
}

} // namespace

PrivacyThreadImageIOStillItemCacheGate::PrivacyThreadImageIOStillItemCacheGate()
    : d(new Private)
{
}

PrivacyThreadImageIOStillItemCacheGate::~PrivacyThreadImageIOStillItemCacheGate() = default;

bool PrivacyThreadImageIOStillItemCacheGate::begin(
    qlonglong imageId, const QString& logicalPath, bool protecting,
    bool legacyPrimaryAliasInventoryComplete)
{
    QMutexLocker locker(&d->mutex);
    const QString verifiedPath = verifiedLogicalPath(imageId, logicalPath);

    if (verifiedPath.isEmpty() ||
        (protecting && !legacyPrimaryAliasInventoryComplete))
    {
        return false;
    }

    if (d->hasTransition())
    {
        if (!d->matches(imageId, verifiedPath, protecting) ||
            !PrivacyCacheTransition::isActive(d->token))
        {
            return false;
        }

        d->aliasInventoryComplete = d->aliasInventoryComplete ||
                                    legacyPrimaryAliasInventoryComplete;
    }
    else
    {
        LoadingDescription description(
            verifiedPath, PreviewSettings(), 0,
            LoadingDescription::NoColorConversion,
            LoadingDescription::PreviewParameters::Thumbnail);
        description.previewParameters.storageReference = imageId;
        description.resolveSource();
        const ThumbnailIdentifier prior = description.thumbnailIdentifier();

        if (!description.sourceResolutionApplied() ||
            !description.sourceResolutionIsCurrent() ||
            (protecting != prior.cacheNamespace.isEmpty()))
        {
            return false;
        }

        const PrivacyCacheTransitionToken token =
            PrivacyCacheTransition::begin(prior);

        if (!token.isValid())
        {
            return false;
        }

        d->imageId = imageId;
        d->logicalPath = verifiedPath;
        d->protecting = protecting;
        d->aliasInventoryComplete = legacyPrimaryAliasInventoryComplete;
        d->token = token;
    }

    PrivacyCacheTransitionInventory inventory;
    inventory.direction = protecting
        ? PrivacyCacheTransitionInventory::Protect
        : PrivacyCacheTransitionInventory::Unprotect;

    if (protecting)
    {
        inventory.legacyPrimaryAliasInventoryComplete =
            d->aliasInventoryComplete;
        inventory.detailAndFaceInventoryComplete =
            ThumbnailLoadThread::privacyLegacyDetailRectangles(
                d->token, &inventory.detailAndFaceRectangles);
    }

    const PrivacyCacheTransition::Result result =
        PrivacyCacheTransition::purge(d->token, inventory, &d->backend);

    return (result.status == PrivacyCacheTransition::Complete);
}

bool PrivacyThreadImageIOStillItemCacheGate::finish(
    qlonglong imageId, const QString& logicalPath, bool protecting,
    bool publicStateVerifiedOrLater)
{
    QMutexLocker locker(&d->mutex);
    const QString verifiedPath = verifiedLogicalPath(imageId, logicalPath);

    if (verifiedPath.isEmpty() || !publicStateVerifiedOrLater)
    {
        return false;
    }

    if (!d->hasTransition())
    {
        // A process restart discards all ThreadImageIO RAM state and transition
        // tokens. Never mistake a token owned by another live gate for that
        // cold-replay case.
        return PrivacyCacheTransition::blockedCacheNamespace(verifiedPath).isEmpty();
    }

    if (!d->matches(imageId, verifiedPath, protecting) ||
        !PrivacyCacheTransition::isActive(d->token) ||
        !PrivacyCacheTransition::finish(d->token))
    {
        return false;
    }

    d->clear();
    return true;
}

} // namespace Digikam

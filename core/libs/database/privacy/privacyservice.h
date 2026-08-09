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

#include <QHash>
#include <QReadWriteLock>
#include <QSet>

// Local includes

#include "digikam_export.h"
#include "privacytypes.h"

namespace Digikam
{

enum class PrivacyItemAccess
{
    Unprotected,
    Locked,
    Unlocked
};

class DIGIKAM_DATABASE_EXPORT PrivacyManualTagVisibilityProvider
{
public:

    PrivacyManualTagVisibilityProvider()          = default;
    virtual ~PrivacyManualTagVisibilityProvider() = default;

    /// Answers whether manual catalogue tags, including People associations,
    /// may be shown, searched or edited for this item. Failure is false.
    virtual bool mayAccessManualTags(qlonglong imageId) const = 0;

    /// Returns category UUIDs whose associations may participate in current
    /// SQL-backed tag queries. An empty set fails protected associations closed.
    virtual QSet<QString> visibleManualTagCategoryUuids() const = 0;

private:

    Q_DISABLE_COPY(PrivacyManualTagVisibilityProvider)
};

class DIGIKAM_DATABASE_EXPORT PrivacyServiceItemState
{
public:

    bool              protectedItem = false;
    QString           categoryUuid;
    PrivacyItemAccess access = PrivacyItemAccess::Unprotected;
    quint64           categoryEpoch = 0;
    qlonglong         itemGeneration = -1;
};

class DIGIKAM_DATABASE_EXPORT PrivacyService
{
public:

    PrivacyService() = default;
    PrivacyService(const QList<PrivacyCategory>& categories,
                   const QList<PrivacyItem>& items);

    void reset(const QList<PrivacyCategory>& categories,
               const QList<PrivacyItem>& items);

    bool setCategoryUnlocked(const QString& categoryUuid, bool unlocked);
    bool addCategory(const PrivacyCategory& category);
    bool addItem(const PrivacyItem& item);
    bool removeItem(const PrivacyItem& item);
    bool setCategoryUnlockedThumbnailMode(
        const QString& categoryUuid,
        PrivacyUnlockedThumbnailMode mode,
        bool categoryAuthenticationVerified);
    bool setCategoryTagVisibilityMode(const QString& categoryUuid,
                                      PrivacyTagVisibilityMode mode,
                                      bool categoryAuthenticationVerified);
    bool isCategoryUnlocked(const QString& categoryUuid) const;
    quint64 categoryEpoch(const QString& categoryUuid) const;
    quint64 itemCategoryEpoch(qlonglong imageId) const;
    qlonglong itemGeneration(qlonglong imageId) const;
    bool sessionStateForItem(qlonglong imageId,
                             PrivacyServiceItemState* state) const;
    bool compareAndSetItemGeneration(qlonglong imageId,
                                     qlonglong expectedGeneration,
                                     qlonglong newGeneration);
    bool isInitialized() const;
    void lockAll();

    bool isProtected(qlonglong imageId) const;
    QString categoryUuidForItem(qlonglong imageId) const;
    PrivacyItemAccess itemAccess(qlonglong imageId) const;
    bool mayAccessOriginal(qlonglong imageId) const;
    bool mayAnalyze(qlonglong imageId) const;
    bool mayAccessManualTags(qlonglong imageId) const;
    QSet<QString> visibleManualTagCategoryUuids() const;

private:

    quint64 advanceEpoch();

private:

    mutable QReadWriteLock      m_lock;
    QHash<QString, bool>        m_categoryUnlockState;
    QHash<QString, PrivacyUnlockedThumbnailMode> m_categoryUnlockedThumbnailModes;
    QHash<QString, PrivacyTagVisibilityMode> m_categoryTagVisibilityModes;
    QHash<QString, quint64>     m_categoryEpochs;
    QHash<qlonglong, QString>   m_itemCategories;
    QHash<qlonglong, QString>   m_itemUuids;
    QHash<QString, qlonglong>   m_imageIdsByItemUuid;
    QHash<qlonglong, qlonglong> m_itemGenerations;
    quint64                     m_epochCounter = 0;
    bool                        m_initialized = false;
};

} // namespace Digikam

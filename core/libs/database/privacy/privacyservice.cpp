/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyservice.h"

// Qt includes

#include <QReadLocker>
#include <QUuid>
#include <QWriteLocker>

namespace Digikam
{

namespace
{

QString normalizedUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return parsed.isNull() ? QString() : parsed.toString(QUuid::WithoutBraces);
}

} // namespace

PrivacyService::PrivacyService(const QList<PrivacyCategory>& categories,
                               const QList<PrivacyItem>& items)
{
    reset(categories, items);
}

void PrivacyService::reset(const QList<PrivacyCategory>& categories,
                           const QList<PrivacyItem>& items)
{
    QWriteLocker locker(&m_lock);

    m_categoryUnlockState.clear();
    m_categoryTagVisibilityModes.clear();
    m_categoryEpochs.clear();
    m_itemCategories.clear();
    m_itemUuids.clear();
    m_imageIdsByItemUuid.clear();
    m_itemGenerations.clear();
    m_initialized = false;

    for (const PrivacyCategory& category : categories)
    {
        if (category.isValid() &&
            (category.lifecycleState == PrivacyCategoryLifecycleState::Active))
        {
            const QString categoryUuid = normalizedUuid(category.uuid);
            m_categoryUnlockState.insert(categoryUuid, false);
            m_categoryTagVisibilityModes.insert(categoryUuid, category.tagVisibilityMode);
            m_categoryEpochs.insert(categoryUuid, advanceEpoch());
        }
    }

    QHash<qlonglong, int> imageIdCounts;
    QHash<QString, int> itemUuidCounts;

    for (const PrivacyItem& item : items)
    {
        if (item.imageId > 0)
        {
            ++imageIdCounts[item.imageId];
            ++itemUuidCounts[normalizedUuid(item.uuid)];
        }
    }

    for (const PrivacyItem& item : items)
    {
        if (item.imageId > 0)
        {
            const QString categoryUuid = normalizedUuid(item.categoryUuid);
            const QString itemUuid = normalizedUuid(item.uuid);

            if (!item.isValid() || (imageIdCounts.value(item.imageId) != 1) ||
                itemUuid.isEmpty() || (itemUuidCounts.value(itemUuid) != 1))
            {
                // Malformed or duplicate mappings remain protected but cannot
                // inherit an unlock state until the durable facts are repaired.

                m_itemCategories.insert(item.imageId, QString());
                m_itemUuids.insert(item.imageId, QString());
                m_itemGenerations.insert(item.imageId, -1);

                if (!itemUuid.isEmpty())
                {
                    m_imageIdsByItemUuid.insert(itemUuid, -1);
                }
            }
            else
            {
                m_itemCategories.insert(item.imageId, categoryUuid);
                m_itemUuids.insert(item.imageId, itemUuid);
                m_imageIdsByItemUuid.insert(itemUuid, item.imageId);
                m_itemGenerations.insert(item.imageId,
                                         item.generation);
            }
        }
    }

    m_initialized = true;
}

bool PrivacyService::setCategoryUnlocked(const QString& categoryUuid, bool unlocked)
{
    QWriteLocker locker(&m_lock);
    const QString uuid = normalizedUuid(categoryUuid);
    auto it            = m_categoryUnlockState.find(uuid);

    if (it == m_categoryUnlockState.end())
    {
        return false;
    }

    if (it.value() != unlocked)
    {
        it.value() = unlocked;
        m_categoryEpochs.insert(uuid, advanceEpoch());
    }

    return true;
}

bool PrivacyService::addCategory(const PrivacyCategory& category)
{
    if (!category.isValid() ||
        (category.lifecycleState != PrivacyCategoryLifecycleState::Active))
    {
        return false;
    }

    QWriteLocker locker(&m_lock);
    const QString uuid = normalizedUuid(category.uuid);

    if (!m_initialized || uuid.isEmpty() || m_categoryUnlockState.contains(uuid))
    {
        return false;
    }

    m_categoryUnlockState.insert(uuid, false);
    m_categoryTagVisibilityModes.insert(uuid, category.tagVisibilityMode);
    m_categoryEpochs.insert(uuid, advanceEpoch());

    return true;
}

bool PrivacyService::addItem(const PrivacyItem& item)
{
    if (!item.isValid())
    {
        return false;
    }

    QWriteLocker locker(&m_lock);
    const QString categoryUuid = normalizedUuid(item.categoryUuid);
    const QString itemUuid = normalizedUuid(item.uuid);

    if (!m_initialized || categoryUuid.isEmpty() || itemUuid.isEmpty() ||
        !m_categoryUnlockState.contains(categoryUuid) ||
        m_itemCategories.contains(item.imageId) ||
        m_imageIdsByItemUuid.contains(itemUuid))
    {
        return false;
    }

    m_itemCategories.insert(item.imageId, categoryUuid);
    m_itemUuids.insert(item.imageId, itemUuid);
    m_imageIdsByItemUuid.insert(itemUuid, item.imageId);
    m_itemGenerations.insert(item.imageId, item.generation);
    m_categoryEpochs.insert(categoryUuid, advanceEpoch());

    return true;
}

bool PrivacyService::removeItem(const PrivacyItem& item)
{
    if (!item.isValid())
    {
        return false;
    }

    QWriteLocker locker(&m_lock);
    const QString categoryUuid = normalizedUuid(item.categoryUuid);
    const QString itemUuid = normalizedUuid(item.uuid);

    if (!m_initialized || categoryUuid.isEmpty() || itemUuid.isEmpty() ||
        !m_categoryUnlockState.contains(categoryUuid) ||
        (m_itemCategories.value(item.imageId) != categoryUuid) ||
        (m_itemUuids.value(item.imageId) != itemUuid) ||
        (m_imageIdsByItemUuid.value(itemUuid, -1) != item.imageId) ||
        (m_itemGenerations.value(item.imageId, -1) != item.generation))
    {
        return false;
    }

    m_itemCategories.remove(item.imageId);
    m_itemUuids.remove(item.imageId);
    m_imageIdsByItemUuid.remove(itemUuid);
    m_itemGenerations.remove(item.imageId);
    m_categoryEpochs.insert(categoryUuid, advanceEpoch());

    return true;
}

bool PrivacyService::setCategoryTagVisibilityMode(
    const QString& categoryUuid,
    PrivacyTagVisibilityMode mode,
    bool categoryAuthenticationVerified)
{
    if (((mode != PrivacyTagVisibilityMode::UnlockedOnly) &&
         (mode != PrivacyTagVisibilityMode::AlwaysVisible)) ||
        ((mode == PrivacyTagVisibilityMode::AlwaysVisible) &&
         !categoryAuthenticationVerified))
    {
        return false;
    }

    QWriteLocker locker(&m_lock);
    const QString uuid = normalizedUuid(categoryUuid);
    auto it = m_categoryTagVisibilityModes.find(uuid);

    if (it == m_categoryTagVisibilityModes.end())
    {
        return false;
    }

    if (it.value() != mode)
    {
        it.value() = mode;
        m_categoryEpochs.insert(uuid, advanceEpoch());
    }

    return true;
}

bool PrivacyService::isCategoryUnlocked(const QString& categoryUuid) const
{
    QReadLocker locker(&m_lock);

    return m_categoryUnlockState.value(normalizedUuid(categoryUuid), false);
}

quint64 PrivacyService::categoryEpoch(const QString& categoryUuid) const
{
    QReadLocker locker(&m_lock);

    return m_categoryEpochs.value(normalizedUuid(categoryUuid), 0);
}

quint64 PrivacyService::itemCategoryEpoch(qlonglong imageId) const
{
    QReadLocker locker(&m_lock);
    const QString categoryUuid = m_itemCategories.value(imageId);

    return categoryUuid.isEmpty() ? 0 : m_categoryEpochs.value(categoryUuid, 0);
}

qlonglong PrivacyService::itemGeneration(qlonglong imageId) const
{
    QReadLocker locker(&m_lock);

    return m_itemGenerations.value(imageId, -1);
}

bool PrivacyService::sessionStateForItem(qlonglong imageId,
                                         PrivacyServiceItemState* state) const
{
    if (!state || (imageId <= 0))
    {
        return false;
    }

    *state = PrivacyServiceItemState();
    QReadLocker locker(&m_lock);

    if (!m_initialized)
    {
        return false;
    }

    const auto categoryIt = m_itemCategories.constFind(imageId);

    if (categoryIt == m_itemCategories.constEnd())
    {
        return true;
    }

    const QString categoryUuid = categoryIt.value();
    const quint64 categoryEpoch = m_categoryEpochs.value(categoryUuid, 0);
    const qlonglong generation = m_itemGenerations.value(imageId, -1);

    if (categoryUuid.isEmpty() || (categoryEpoch == 0) || (generation < 0) ||
        !m_categoryUnlockState.contains(categoryUuid))
    {
        return false;
    }

    state->protectedItem = true;
    state->categoryUuid = categoryUuid;
    state->access = m_categoryUnlockState.value(categoryUuid)
                  ? PrivacyItemAccess::Unlocked
                  : PrivacyItemAccess::Locked;
    state->categoryEpoch = categoryEpoch;
    state->itemGeneration = generation;

    return true;
}

bool PrivacyService::compareAndSetItemGeneration(qlonglong imageId,
                                                 qlonglong expectedGeneration,
                                                 qlonglong newGeneration)
{
    if ((imageId <= 0) || (expectedGeneration < 0) ||
        (newGeneration <= expectedGeneration))
    {
        return false;
    }

    QWriteLocker locker(&m_lock);
    auto it = m_itemGenerations.find(imageId);

    if ((it == m_itemGenerations.end()) || (it.value() != expectedGeneration))
    {
        return false;
    }

    it.value() = newGeneration;

    return true;
}

bool PrivacyService::isInitialized() const
{
    QReadLocker locker(&m_lock);

    return m_initialized;
}

void PrivacyService::lockAll()
{
    QWriteLocker locker(&m_lock);

    for (auto it = m_categoryUnlockState.begin() ; it != m_categoryUnlockState.end() ; ++it)
    {
        it.value() = false;
        m_categoryEpochs.insert(it.key(), advanceEpoch());
    }
}

bool PrivacyService::isProtected(qlonglong imageId) const
{
    QReadLocker locker(&m_lock);

    return (!m_initialized || m_itemCategories.contains(imageId));
}

QString PrivacyService::categoryUuidForItem(qlonglong imageId) const
{
    QReadLocker locker(&m_lock);

    return m_itemCategories.value(imageId);
}

PrivacyItemAccess PrivacyService::itemAccess(qlonglong imageId) const
{
    QReadLocker locker(&m_lock);

    if (!m_initialized)
    {
        return PrivacyItemAccess::Locked;
    }

    const auto itemIt = m_itemCategories.constFind(imageId);

    if (itemIt == m_itemCategories.constEnd())
    {
        return PrivacyItemAccess::Unprotected;
    }

    if (m_categoryUnlockState.value(itemIt.value(), false))
    {
        return PrivacyItemAccess::Unlocked;
    }

    return PrivacyItemAccess::Locked;
}

bool PrivacyService::mayAccessOriginal(qlonglong imageId) const
{
    return (itemAccess(imageId) != PrivacyItemAccess::Locked);
}

bool PrivacyService::mayAnalyze(qlonglong imageId) const
{
    return !isProtected(imageId);
}

bool PrivacyService::mayAccessManualTags(qlonglong imageId) const
{
    if (imageId <= 0)
    {
        return false;
    }

    QReadLocker locker(&m_lock);

    if (!m_initialized)
    {
        return false;
    }

    const auto itemIt = m_itemCategories.constFind(imageId);

    if (itemIt == m_itemCategories.constEnd())
    {
        return true;
    }

    const QString& categoryUuid = itemIt.value();
    const auto modeIt = m_categoryTagVisibilityModes.constFind(categoryUuid);

    if (categoryUuid.isEmpty() ||
        (modeIt == m_categoryTagVisibilityModes.constEnd()) ||
        (m_itemGenerations.value(imageId, -1) < 0) ||
        !m_categoryUnlockState.contains(categoryUuid))
    {
        return false;
    }

    return ((modeIt.value() == PrivacyTagVisibilityMode::AlwaysVisible) ||
            m_categoryUnlockState.value(categoryUuid, false));
}

quint64 PrivacyService::advanceEpoch()
{
    if (++m_epochCounter == 0)
    {
        ++m_epochCounter;
    }

    return m_epochCounter;
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacyleaseregistry.h"

// C++ includes

#include <utility>

// Qt includes

#include <QReadLocker>
#include <QUuid>
#include <QWriteLocker>

namespace Digikam
{

namespace
{

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

} // namespace

bool PrivacyLeaseCurrentState::isValid() const
{
    return (isCanonicalUuid(itemUuid) && (itemGeneration >= 0) &&
            (categoryEpoch > 0) && (publicRootEpoch > 0) &&
            (!storeRootAvailable || (storeRootEpoch > 0)));
}

PrivacyLeaseRegistry::PrivacyLeaseRegistry(
    QSharedPointer<const PrivacyLeaseStateProvider> stateProvider)
    : m_stateProvider(std::move(stateProvider))
{
}

PrivacyLeaseToken PrivacyLeaseRegistry::issue(const QString& itemUuid,
                                              bool dependsOnStore)
{
    PrivacyLeaseToken lease;

    if (!m_stateProvider || !isCanonicalUuid(itemUuid))
    {
        return lease;
    }

    PrivacyLeaseCurrentState state;

    if (!m_stateProvider->currentState(itemUuid, &state) || !state.isValid() ||
        (state.itemUuid != itemUuid) ||
        state.unresolvedTransaction ||
        !state.publicRootAvailable ||
        (dependsOnStore && (!state.categoryUnlocked ||
                            !state.storeRootAvailable ||
                            (state.storeRootEpoch == 0))))
    {
        return lease;
    }

    lease.uuid            = QUuid::createUuid().toString(QUuid::WithoutBraces);
    lease.itemUuid        = state.itemUuid;
    lease.itemGeneration  = state.itemGeneration;
    lease.categoryEpoch   = state.categoryEpoch;
    lease.publicRootEpoch = state.publicRootEpoch;
    lease.storeRootEpoch  = dependsOnStore ? state.storeRootEpoch : 0;

    if (!lease.isValid())
    {
        return PrivacyLeaseToken();
    }

    QWriteLocker locker(&m_lock);
    m_activeLeases.insert(lease.uuid, lease);

    locker.unlock();

    // Close the issuance race: a relock, root transition, or item commit may
    // happen after the first sample but before registry insertion.

    PrivacyLeaseCurrentState confirmed;

    if (!m_stateProvider->currentState(itemUuid, &confirmed) ||
        !confirmed.isValid() || confirmed.unresolvedTransaction ||
        !stateMatchesLease(confirmed, lease) ||
        (dependsOnStore && !confirmed.categoryUnlocked))
    {
        revoke(lease.uuid);

        return PrivacyLeaseToken();
    }

    {
        QReadLocker confirmationLocker(&m_lock);
        const auto it = m_activeLeases.constFind(lease.uuid);

        if ((it == m_activeLeases.constEnd()) || !tokensMatch(it.value(), lease))
        {
            return PrivacyLeaseToken();
        }
    }

    return lease;
}

PrivacyLeaseValidation PrivacyLeaseRegistry::validate(
    const PrivacyLeaseToken& lease) const
{
    if (!lease.isValid())
    {
        return PrivacyLeaseValidation::Revoked;
    }

    PrivacyLeaseToken registered;

    {
        QReadLocker locker(&m_lock);
        const auto it = m_activeLeases.constFind(lease.uuid);

        if ((it == m_activeLeases.constEnd()) || !tokensMatch(it.value(), lease))
        {
            return PrivacyLeaseValidation::Revoked;
        }

        registered = it.value();
    }

    if (!m_stateProvider)
    {
        return PrivacyLeaseValidation::RootUnavailable;
    }

    PrivacyLeaseCurrentState state;

    if (!m_stateProvider->currentState(registered.itemUuid, &state) ||
        !state.isValid() || (state.itemUuid != registered.itemUuid))
    {
        return PrivacyLeaseValidation::StateChanged;
    }

    if (!state.publicRootAvailable ||
        ((registered.storeRootEpoch > 0) && !state.storeRootAvailable))
    {
        return PrivacyLeaseValidation::RootUnavailable;
    }

    if (state.unresolvedTransaction)
    {
        return PrivacyLeaseValidation::StateChanged;
    }

    if ((registered.storeRootEpoch > 0) && !state.categoryUnlocked)
    {
        return PrivacyLeaseValidation::StateChanged;
    }

    if (!stateMatchesLease(state, registered))
    {
        return PrivacyLeaseValidation::StateChanged;
    }

    // A relock may revoke the capability while the provider state is being
    // sampled. Recheck membership before reporting success. Filesystem users
    // must still revalidate root identity at descriptor/open time because a
    // process-local lease cannot make path lookup itself atomic.

    QReadLocker locker(&m_lock);
    const auto it = m_activeLeases.constFind(registered.uuid);

    if ((it == m_activeLeases.constEnd()) || !tokensMatch(it.value(), registered))
    {
        return PrivacyLeaseValidation::Revoked;
    }

    return PrivacyLeaseValidation::Valid;
}

bool PrivacyLeaseRegistry::revoke(const QString& leaseUuid)
{
    QWriteLocker locker(&m_lock);

    return (m_activeLeases.remove(leaseUuid) == 1);
}

int PrivacyLeaseRegistry::revokeItem(const QString& itemUuid)
{
    if (!isCanonicalUuid(itemUuid))
    {
        return 0;
    }

    QWriteLocker locker(&m_lock);
    int removed = 0;

    for (auto it = m_activeLeases.begin() ; it != m_activeLeases.end() ; )
    {
        if (it->itemUuid == itemUuid)
        {
            it = m_activeLeases.erase(it);
            ++removed;
        }
        else
        {
            ++it;
        }
    }

    return removed;
}

void PrivacyLeaseRegistry::revokeAll()
{
    QWriteLocker locker(&m_lock);
    m_activeLeases.clear();
}

int PrivacyLeaseRegistry::activeLeaseCount() const
{
    QReadLocker locker(&m_lock);

    return m_activeLeases.size();
}

bool PrivacyLeaseRegistry::tokensMatch(const PrivacyLeaseToken& left,
                                       const PrivacyLeaseToken& right)
{
    return ((left.uuid == right.uuid) &&
            (left.itemUuid == right.itemUuid) &&
            (left.itemGeneration == right.itemGeneration) &&
            (left.categoryEpoch == right.categoryEpoch) &&
            (left.publicRootEpoch == right.publicRootEpoch) &&
            (left.storeRootEpoch == right.storeRootEpoch));
}

bool PrivacyLeaseRegistry::stateMatchesLease(const PrivacyLeaseCurrentState& state,
                                             const PrivacyLeaseToken& lease)
{
    return ((state.itemUuid == lease.itemUuid) &&
            (state.itemGeneration == lease.itemGeneration) &&
            (state.categoryEpoch == lease.categoryEpoch) &&
            (state.publicRootEpoch == lease.publicRootEpoch) &&
            ((lease.storeRootEpoch == 0) ||
             (state.storeRootEpoch == lease.storeRootEpoch)) &&
            state.publicRootAvailable &&
            ((lease.storeRootEpoch == 0) || state.storeRootAvailable));
}

} // namespace Digikam

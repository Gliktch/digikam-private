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
#include <QSharedPointer>

// Local includes

#include "digikam_export.h"
#include "privacycontracts.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT PrivacyLeaseCurrentState
{
public:

    bool isValid() const;

public:

    QString   itemUuid;
    qlonglong itemGeneration = -1;
    quint64   categoryEpoch = 0;
    quint64   publicRootEpoch = 0;
    quint64   storeRootEpoch = 0;
    bool      categoryUnlocked = false;
    bool      publicRootAvailable = false;
    bool      storeRootAvailable = false;
    bool      unresolvedTransaction = false;
};

class DIGIKAM_DATABASE_EXPORT PrivacyLeaseStateProvider
{
public:

    PrivacyLeaseStateProvider()          = default;
    virtual ~PrivacyLeaseStateProvider() = default;

    virtual bool currentState(const QString& itemUuid,
                              PrivacyLeaseCurrentState* state) const = 0;

private:

    Q_DISABLE_COPY(PrivacyLeaseStateProvider)
};

/**
 * Required execution-time seam for filesystem consumers. After ordinary
 * lease validation and after opening root descriptors without following
 * symlinks, the consumer must pass those native descriptors here before any
 * media I/O. No production implementation is installed until album and
 * managed-store identity can both be verified against already-open handles.
 * storeRootDescriptor is mandatory exactly when lease.storeRootEpoch is
 * non-zero; it may be the same descriptor as the public root for an adjacent
 * casual archive.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyLeaseOpenDescriptorVerifier
{
public:

    PrivacyLeaseOpenDescriptorVerifier()          = default;
    virtual ~PrivacyLeaseOpenDescriptorVerifier() = default;

    virtual PrivacyLeaseValidation validateOpenDescriptors(
        const PrivacyLeaseToken& lease,
        qintptr publicRootDescriptor,
        qintptr storeRootDescriptor = -1) const = 0;

private:

    Q_DISABLE_COPY(PrivacyLeaseOpenDescriptorVerifier)
};

/**
 * Process-local capability registry. A lease token contains no password and
 * never authorizes I/O by itself: it must still be present in this registry
 * and match the provider's current item/category/root generations.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyLeaseRegistry final : public PrivacyLeaseAuthority
{
public:

    explicit PrivacyLeaseRegistry(
        QSharedPointer<const PrivacyLeaseStateProvider> stateProvider);

    PrivacyLeaseToken issue(const QString& itemUuid, bool dependsOnStore);
    PrivacyLeaseValidation validate(const PrivacyLeaseToken& lease) const override;

    bool revoke(const QString& leaseUuid);
    int revokeItem(const QString& itemUuid);
    void revokeAll();
    int activeLeaseCount() const;

private:

    static bool tokensMatch(const PrivacyLeaseToken& left,
                            const PrivacyLeaseToken& right);
    static bool stateMatchesLease(const PrivacyLeaseCurrentState& state,
                                  const PrivacyLeaseToken& lease);

private:

    QSharedPointer<const PrivacyLeaseStateProvider> m_stateProvider;
    mutable QReadWriteLock                          m_lock;
    QHash<QString, PrivacyLeaseToken>               m_activeLeases;
};

} // namespace Digikam

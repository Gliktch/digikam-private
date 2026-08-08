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

#include <QScopedPointer>
#include <QSharedPointer>

// Local includes

#include "privacyruntime.h"

namespace Digikam
{

class PrivacyThreadImageIOStillItemTransactionOwner final
    : public PrivacyTransactionRecovery
{
public:

    static QSharedPointer<const PrivacyTransactionRecovery> create(
        PrivacyRuntimeCoordinator& runtime);

    ~PrivacyThreadImageIOStillItemTransactionOwner() override;

    PrivacyRecoveryDisposition recoverRoot(
        const PrivacyStorageRoot& root,
        const PrivacyTransaction& transaction,
        const QList<PrivacyTransactionJournal>& journals) const override;

private:

    explicit PrivacyThreadImageIOStillItemTransactionOwner(
        PrivacyRuntimeCoordinator& runtime);

    class Private;
    QScopedPointer<Private> d;

    Q_DISABLE_COPY(PrivacyThreadImageIOStillItemTransactionOwner)
};

} // namespace Digikam

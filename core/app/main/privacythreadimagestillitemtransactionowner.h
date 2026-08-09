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

// C++ includes

#include <functional>

// Qt includes

#include <QList>
#include <QScopedPointer>
#include <QSharedPointer>

// Local includes

#include "digikam_export.h"
#include "privacyprotectpreflight.h"
#include "privacyruntime.h"
#include "privacystillitemtransaction.h"

namespace Digikam
{

class ItemInfo;

enum class PrivacyStillItemActionAvailability
{
    Unavailable,
    Protectable,
    Unprotectable,
    ResumeProtectable,
    ResumeUnprotectable,
    ProtectedUnavailable
};

class PrivacyStillItemActionContext
{
public:

    PrivacyStillItemActionAvailability availability =
        PrivacyStillItemActionAvailability::Unavailable;
    PrivacyCategory protectedCategory;
    QList<PrivacyCategory> protectCategories;
    PrivacyRootRuntimeState publicRootState = PrivacyRootRuntimeState::Unknown;
    QString recoveryTransactionUuid;
};

class PrivacyThreadImageIOStillItemTransactionOwner final
    : public PrivacyTransactionRecovery
{
public:

    using ProtectAcknowledgement =
        std::function<bool(const PrivacyProtectPreflightResult&)>;

    static DIGIKAM_GUI_EXPORT QSharedPointer<const PrivacyTransactionRecovery> create(
        PrivacyRuntimeCoordinator& runtime);
    static QSharedPointer<PrivacyThreadImageIOStillItemTransactionOwner> current();

    ~PrivacyThreadImageIOStillItemTransactionOwner() override;

    PrivacyStillItemActionContext actionContextForImage(qlonglong imageId) const;
    bool categoryIsUnlocked(const QString& categoryUuid) const;

    PrivacyStillItemTransactionResult protect(
        const ItemInfo& info,
        const QString& categoryUuid,
        const QString& passwordText,
        const ProtectAcknowledgement& acknowledgeAssetSet = {});
    PrivacyStillItemTransactionResult unprotect(
        const ItemInfo& info,
        const QString& passwordText);
    PrivacyStillItemTransactionResult resume(
        qlonglong imageId,
        const QString& transactionUuid,
        const QString& passwordText);

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

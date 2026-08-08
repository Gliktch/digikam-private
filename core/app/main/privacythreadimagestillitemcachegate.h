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

// Local includes

#include "privacystillitemtransaction.h"

namespace Digikam
{

/** Application bridge between the database transaction owner and ThreadImageIO. */
class PrivacyThreadImageIOStillItemCacheGate final
    : public PrivacyStillItemCacheGate
{
public:

    PrivacyThreadImageIOStillItemCacheGate();
    ~PrivacyThreadImageIOStillItemCacheGate() override;

    bool begin(qlonglong imageId, const QString& logicalPath,
               bool protecting,
               bool legacyPrimaryAliasInventoryComplete) override;
    bool finish(qlonglong imageId, const QString& logicalPath,
                bool protecting,
                bool publicStateVerifiedOrLater) override;

private:

    class Private;
    QScopedPointer<Private> d;

    Q_DISABLE_COPY(PrivacyThreadImageIOStillItemCacheGate)
};

} // namespace Digikam

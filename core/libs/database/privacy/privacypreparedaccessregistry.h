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

#include <QString>
#include <QStringList>
#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT PrivacyPreparedAccessToken
{
public:

    bool isValid() const;

private:

    friend class PrivacyPreparedAccessRegistry;

    QString     uuid;
    QStringList categoryUuids;
};

class DIGIKAM_DATABASE_EXPORT PrivacyPreparedAccessQuiesceGuard
{
public:

    PrivacyPreparedAccessQuiesceGuard();
    ~PrivacyPreparedAccessQuiesceGuard();

    bool isAcquired() const;

private:

    bool m_acquired = false;

private:

    Q_DISABLE_COPY(PrivacyPreparedAccessQuiesceGuard)
};

/**
 * Process-local ownership barrier for prepared protected sources whose caller
 * can outlive one synchronous source-resolution call. A category lock checks
 * this registry before and after draining ordinary image-source users, so it
 * fails quickly instead of invalidating a source still owned by a worker or
 * tool. Runtime replacement additionally holds a quiesce guard, which refuses
 * new prepared consumers and can be acquired only after existing ones drain.
 */
class DIGIKAM_DATABASE_EXPORT PrivacyPreparedAccessRegistry
{
public:

    static PrivacyPreparedAccessToken acquire(
        const QStringList& categoryUuids);
    static bool release(const PrivacyPreparedAccessToken& token);
    static bool hasActiveAccess(const QString& categoryUuid = QString());
    static int activeAccessCount();
};

} // namespace Digikam

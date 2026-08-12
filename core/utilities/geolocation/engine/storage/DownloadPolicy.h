/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QStringList>

// Local includes

#include "MarbleGlobal.h"

namespace Marble
{

class DownloadPolicyKey
{
    friend bool operator==(DownloadPolicyKey const& lhs, DownloadPolicyKey const& rhs);

public:

    DownloadPolicyKey();
    DownloadPolicyKey(const QStringList& hostNames, const DownloadUsage usage);
    DownloadPolicyKey(const QString& hostName, const DownloadUsage usage);

    QStringList hostNames() const;
    void setHostNames(const QStringList& hostNames);

    DownloadUsage usage() const;
    void setUsage(DownloadUsage const usage);

    bool matches(const QString& hostName, const DownloadUsage usage) const;

private:

    QStringList     m_hostNames;
    DownloadUsage   m_usage;
};

inline bool operator==(const DownloadPolicyKey& lhs, const DownloadPolicyKey& rhs)
{
    return lhs.m_hostNames == rhs.m_hostNames && lhs.m_usage == rhs.m_usage;
}

class DownloadPolicy
{
    friend bool operator==(const DownloadPolicy& lhs, const DownloadPolicy& rhs);

public:

    DownloadPolicy();
    explicit DownloadPolicy(const DownloadPolicyKey& key);

    int maximumConnections() const;
    void setMaximumConnections(const int);

    DownloadPolicyKey key() const;

private:

    DownloadPolicyKey m_key;
    int m_maximumConnections;
};

inline bool operator==(const DownloadPolicy& lhs, const DownloadPolicy& rhs)
{
    return ((lhs.m_key == rhs.m_key) && (lhs.m_maximumConnections == rhs.m_maximumConnections));
}

} // namespace Marble

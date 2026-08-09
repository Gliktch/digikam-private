/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "privacypreparedaccessregistry.h"

// C++ includes

#include <utility>

// Qt includes

#include <QGlobalStatic>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QThread>
#include <QUuid>

namespace Digikam
{

namespace
{

class PreparedAccessData
{
public:

    QMutex                    lock;
    QHash<QString, QSet<QString> > categoriesByToken;
    bool                      quiescing = false;
    Qt::HANDLE                quiesceOwner = nullptr;
    int                       quiesceDepth = 0;
};

Q_GLOBAL_STATIC(PreparedAccessData, preparedAccessData)

bool isCanonicalUuid(const QString& uuid)
{
    const QUuid parsed(uuid);

    return (!parsed.isNull() &&
            (uuid == parsed.toString(QUuid::WithoutBraces)));
}

} // namespace

PrivacyPreparedAccessQuiesceGuard::PrivacyPreparedAccessQuiesceGuard()
{
    QMutexLocker locker(&preparedAccessData->lock);
    const Qt::HANDLE current = QThread::currentThreadId();

    if (preparedAccessData->quiescing &&
        (preparedAccessData->quiesceOwner == current))
    {
        ++preparedAccessData->quiesceDepth;
        m_acquired = true;
    }
    else if (!preparedAccessData->quiescing &&
        preparedAccessData->categoriesByToken.isEmpty())
    {
        preparedAccessData->quiescing = true;
        preparedAccessData->quiesceOwner = current;
        preparedAccessData->quiesceDepth = 1;
        m_acquired = true;
    }
}

PrivacyPreparedAccessQuiesceGuard::~PrivacyPreparedAccessQuiesceGuard()
{
    if (!m_acquired)
    {
        return;
    }

    QMutexLocker locker(&preparedAccessData->lock);

    if (preparedAccessData->quiescing &&
        (preparedAccessData->quiesceOwner == QThread::currentThreadId()) &&
        (--preparedAccessData->quiesceDepth == 0))
    {
        preparedAccessData->quiescing = false;
        preparedAccessData->quiesceOwner = nullptr;
    }
}

bool PrivacyPreparedAccessQuiesceGuard::isAcquired() const
{
    return m_acquired;
}

bool PrivacyPreparedAccessToken::isValid() const
{
    if (!isCanonicalUuid(uuid) || categoryUuids.isEmpty())
    {
        return false;
    }

    QSet<QString> categories;

    for (const QString& categoryUuid : categoryUuids)
    {
        if (!isCanonicalUuid(categoryUuid) || categories.contains(categoryUuid))
        {
            return false;
        }

        categories.insert(categoryUuid);
    }

    return true;
}

PrivacyPreparedAccessToken PrivacyPreparedAccessRegistry::acquire(
    const QStringList& categoryUuids)
{
    PrivacyPreparedAccessToken token;
    QSet<QString> categories;

    for (const QString& categoryUuid : categoryUuids)
    {
        if (!isCanonicalUuid(categoryUuid))
        {
            return token;
        }

        categories.insert(categoryUuid);
    }

    if (categories.isEmpty())
    {
        return token;
    }

    token.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    token.categoryUuids = categories.values();
    token.categoryUuids.sort();

    if (!token.isValid())
    {
        return PrivacyPreparedAccessToken();
    }

    QMutexLocker locker(&preparedAccessData->lock);

    if (preparedAccessData->quiescing)
    {
        return PrivacyPreparedAccessToken();
    }

    preparedAccessData->categoriesByToken.insert(token.uuid, categories);
    return token;
}

bool PrivacyPreparedAccessRegistry::release(
    const PrivacyPreparedAccessToken& token)
{
    if (!token.isValid())
    {
        return false;
    }

    QMutexLocker locker(&preparedAccessData->lock);
    const auto it = preparedAccessData->categoriesByToken.constFind(token.uuid);

    if ((it == preparedAccessData->categoriesByToken.constEnd()) ||
        (it.value() != QSet<QString>(token.categoryUuids.cbegin(),
                                    token.categoryUuids.cend())))
    {
        return false;
    }

    preparedAccessData->categoriesByToken.erase(it);
    return true;
}

bool PrivacyPreparedAccessRegistry::hasActiveAccess(
    const QString& categoryUuid)
{
    if (!categoryUuid.isEmpty() && !isCanonicalUuid(categoryUuid))
    {
        return true;
    }

    QMutexLocker locker(&preparedAccessData->lock);

    if (categoryUuid.isEmpty())
    {
        return !preparedAccessData->categoriesByToken.isEmpty();
    }

    for (const QSet<QString>& categories :
         std::as_const(preparedAccessData->categoriesByToken))
    {
        if (categories.contains(categoryUuid))
        {
            return true;
        }
    }

    return false;
}

int PrivacyPreparedAccessRegistry::activeAccessCount()
{
    QMutexLocker locker(&preparedAccessData->lock);
    return preparedAccessData->categoriesByToken.size();
}

} // namespace Digikam

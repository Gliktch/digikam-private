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

#include "CacheStoragePolicy.h"

// Qt includes

#include <QDir>

// KDE includes

#include <klocalizedstring.h>

namespace Marble
{

CacheStoragePolicy::CacheStoragePolicy(const QString& cacheDirectory)
    : m_cache(cacheDirectory)
{
    if (! QDir(cacheDirectory).exists())
    {
        QDir::root().mkpath(cacheDirectory);
    }
}

CacheStoragePolicy::~CacheStoragePolicy()
{
}

bool CacheStoragePolicy::fileExists(const QString& fileName) const
{
    return m_cache.exists(fileName);
}

bool CacheStoragePolicy::updateFile(const QString& fileName, const QByteArray& data)
{
    if (!m_cache.insert(fileName, data))
    {
        m_errorMsg = i18n("Unable to insert data into cache");
        return false;
    }

    return true;
}

void CacheStoragePolicy::clearCache()
{
    m_cache.clear();
}

QString CacheStoragePolicy::lastErrorMessage() const
{
    return m_errorMsg;
}

QByteArray CacheStoragePolicy::data(const QString& fileName)
{
    QByteArray data;
    m_cache.find(fileName, data);

    return data;
}

void CacheStoragePolicy::setCacheLimit(quint64 bytes)
{
    m_cache.setCacheLimit(bytes);
}

quint64 CacheStoragePolicy::cacheLimit() const
{
    return m_cache.cacheLimit();
}

} // namespace Marble

#include "moc_CacheStoragePolicy.cpp"

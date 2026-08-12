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

#include <QDateTime>
#include <QMap>
#include <QPair>
#include <QString>

class QByteArray;

namespace Marble
{

class DiscCache
{
public:

    explicit DiscCache(const QString& cacheDirectory);
    ~DiscCache();

    quint64 cacheLimit() const;
    void clear();
    bool exists(const QString& key) const;
    bool find(const QString& key, QByteArray& data);
    bool insert(const QString& key, const QByteArray& data);
    void remove(const QString& key);
    void setCacheLimit(quint64 n);

private:

    QString keyToFileName(const QString&) const;
    void cleanup();

private:

    typedef QPair<QDateTime, quint64> Entry;

    QString                 m_CacheDirectory;
    quint64                 m_CacheLimit;
    quint64                 m_CurrentCacheSize;
    QMap<QString, Entry>    m_Entries;
};

} // namespace Marble

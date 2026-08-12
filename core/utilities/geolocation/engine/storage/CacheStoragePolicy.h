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

// Local includes

#include "DiscCache.h"
#include "StoragePolicy.h"
#include "digikam_export.h"

class QByteArray;

namespace Marble
{

class DIGIKAM_EXPORT CacheStoragePolicy : public StoragePolicy
{
    Q_OBJECT

public:

    /**
     * Creates a new cache storage policy.
     *
     * @param cacheDirectory The directory which shall be used for the cache.
     */
    explicit CacheStoragePolicy(const QString& cacheDirectory);

    /**
     * Destroys the cache storage policy.
     */
    ~CacheStoragePolicy() override;

    /**
     * Returns whether the @p fileName exists already.
     */
    bool fileExists(const QString& fileName) const override;

    /**
     * Updates the @p fileName with the given @p data.
     */
    bool updateFile(const QString& fileName, const QByteArray& data) override;

    /**
     * Clears the cache.
     */
    void clearCache() override;

    /**
     * Returns the last error message.
     */
    QString lastErrorMessage() const override;

    /**
     * Returns the data of a file.
     */
    QByteArray data(const QString& fileName);

    /**
     * Sets the limit of the cache in @p bytes.
     */
    void setCacheLimit(quint64 bytes);

    /**
     * Returns the limit of the cache in bytes.
     */
    quint64 cacheLimit() const;

private:

    DiscCache   m_cache;
    QString     m_errorMsg;
};

} // namespace Marble

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

// Local includes

#include "StoragePolicy.h"

namespace Marble
{

class FileStoragePolicy : public StoragePolicy
{
    Q_OBJECT

public:

    /**
     * Creates a new file storage policy.
     *
     * @param dataDirectory The directory where the data should go to.
     * @param parent The parent object.
     */
    explicit FileStoragePolicy(const QString& dataDirectory = QString(), QObject* parent = nullptr);

    /**
     * Destroys the cache storage policy.
     */
    ~FileStoragePolicy() override;

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

private:

    Q_DISABLE_COPY(FileStoragePolicy)

private:

    QString m_dataDirectory;
    QString m_errorMsg;
};

} // namespace Marble

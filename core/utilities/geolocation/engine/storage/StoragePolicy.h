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

#include <QObject>

class QByteArray;
class QString;

namespace Marble
{

class StoragePolicy : public QObject
{
    Q_OBJECT

public:

    explicit StoragePolicy(QObject* parent = nullptr);

    ~StoragePolicy() override {}

    virtual bool fileExists(const QString& fileName) const = 0;

    /**
     * Return true if file was written successfully.
     */
    virtual bool updateFile(const QString& fileName, const QByteArray& data) = 0;

    virtual void clearCache() = 0;

    virtual QString lastErrorMessage() const = 0;

Q_SIGNALS:

    void cleared();
    void sizeChanged(qint64);

private:

    Q_DISABLE_COPY(StoragePolicy)
};

} // namespace Marble

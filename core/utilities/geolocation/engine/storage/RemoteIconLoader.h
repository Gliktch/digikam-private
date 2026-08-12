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

class QString;
class QByteArray;
class QImage;
class QUrl;

namespace Marble
{

class RemoteIconLoaderPrivate;

class RemoteIconLoader : public QObject
{
    Q_OBJECT

public:

    explicit RemoteIconLoader(QObject* parent = nullptr);
    ~RemoteIconLoader() override;

    /**
     * Handles the icon request for passed url( whether icon is available
     * in cache or on disk or it should gor for a download
     */
    QImage load(const QUrl& url);

Q_SIGNALS:

    /**
     * Signal to indicate that image has been downloaded
     * and is ready to be display now
     */
    void iconReady();

private Q_SLOTS:

    void storeIcon(const QByteArray&, const QString&);

private:

    RemoteIconLoaderPrivate* d = nullptr;
};

} // namespace Marble

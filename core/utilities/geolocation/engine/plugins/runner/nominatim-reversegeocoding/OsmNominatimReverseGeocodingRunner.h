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
#include <QNetworkAccessManager>
#include <QNetworkRequest>

// Local includes

#include "ReverseGeocodingRunner.h"
#include "GeoDataCoordinates.h"

class QNetworkReply;
class QDomNode;

namespace Marble
{

class GeoDataExtendedData;

class OsmNominatimRunner : public ReverseGeocodingRunner
{
    Q_OBJECT

public:

    explicit OsmNominatimRunner(QObject* parent = nullptr);

    ~OsmNominatimRunner() override;

    // Overriding MarbleAbstractRunner
    void reverseGeocoding(const GeoDataCoordinates& coordinates) override;

private Q_SLOTS:

    // Forward a result to the search or reverse geocoding handler
    void handleResult(QNetworkReply*);

    void returnNoReverseGeocodingResult();

    void startReverseGeocoding();

private:

    static void extractChildren(const QDomNode& node, GeoDataPlacemark& placemark);

private:

    QNetworkAccessManager m_manager;

    QNetworkRequest       m_request;

    GeoDataCoordinates    m_coordinates;
};

} // namespace Marble

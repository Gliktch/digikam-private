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

#include "OsmNominatimReverseGeocodingRunner.h"

// Qt includes

#include <QUrl>
#include <QTimer>
#include <QNetworkReply>
#include <QDomDocument>

// Local includes

#include "MarbleLocale.h"
#include "GeoDataPlacemark.h"
#include "GeoDataExtendedData.h"
#include "GeoDataData.h"
#include "HttpDownloadManager.h"
#include "OsmPlacemarkData.h"
#include "digikam_debug.h"

namespace Marble
{

OsmNominatimRunner::OsmNominatimRunner(QObject* parent)
    : ReverseGeocodingRunner(parent),
      m_manager(this)
{
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleResult(QNetworkReply*)));
}

OsmNominatimRunner::~OsmNominatimRunner()
{
    // nothing to do
}

void OsmNominatimRunner::returnNoReverseGeocodingResult()
{
    Q_EMIT reverseGeocodingFinished(m_coordinates, GeoDataPlacemark());
}

void OsmNominatimRunner::reverseGeocoding(const GeoDataCoordinates& coordinates)
{
    m_coordinates = coordinates;
    QString base  = QString::fromUtf8("https://nominatim.openstreetmap.org/reverse?format=xml&addressdetails=1");

    // @todo: Alternative URI with addressdetails=1 could be used for shorter placemark name

    QString query = QString::fromUtf8("&lon=%1&lat=%2&accept-language=%3");
    double lon    = coordinates.longitude(GeoDataCoordinates::Degree);
    double lat    = coordinates.latitude(GeoDataCoordinates::Degree);
    QString url   = QString(base + query).arg(lon).arg(lat).arg(MarbleLocale::languageCode());

    m_request.setUrl(QUrl(url));
    m_request.setRawHeader(QByteArray("User-Agent"),
                           HttpDownloadManager::userAgent(QLatin1String("Browser"),
                                                          QLatin1String("OsmNominatimRunner")));

    QEventLoop eventLoop;

    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(15000);

    connect(&timer, SIGNAL(timeout()),
            &eventLoop, SLOT(quit()));

    connect(this, SIGNAL(reverseGeocodingFinished(GeoDataCoordinates,GeoDataPlacemark)),
            &eventLoop, SLOT(quit()));

    // @todo FIXME Must currently be done in the main thread, see bug 257376

    QTimer::singleShot(0, this, SLOT(startReverseGeocoding()));
    timer.start();

    eventLoop.exec();
}

void OsmNominatimRunner::startReverseGeocoding()
{
    QNetworkReply* reply = m_manager.get(m_request);

    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)),
            this, SLOT(returnNoReverseGeocodingResult()));
}

void OsmNominatimRunner::handleResult(QNetworkReply* reply)
{
    if (!reply->bytesAvailable())
    {
        returnNoReverseGeocodingResult();
        return;
    }

    QDomDocument xml;

    if (!xml.setContent(reply->readAll()))
    {
        qCDebug(DIGIKAM_GEOENGINE_LOG) << "Cannot parse osm nominatim result " << xml.toString();
        returnNoReverseGeocodingResult();

        return;
    }

    QDomElement root    = xml.documentElement();
    QDomNodeList places = root.elementsByTagName(QStringLiteral("result"));

    if (places.size() == 1)
    {
        QString address = places.item(0).toElement().text();
        GeoDataPlacemark placemark;
        placemark.setVisualCategory(GeoDataPlacemark::Coordinate);
        placemark.setAddress(address);
        placemark.setCoordinate(m_coordinates);

        QDomNode details = root.firstChildElement(QStringLiteral("addressparts"));
        extractChildren(details, placemark);

        Q_EMIT reverseGeocodingFinished(m_coordinates, placemark);
    }
    else
    {
        returnNoReverseGeocodingResult();
    }
}

void OsmNominatimRunner::extractChildren(const QDomNode& node, GeoDataPlacemark& placemark)
{
    QMap<QString, QString> tagTranslator;
    tagTranslator[QLatin1String("house_number")]   = QLatin1String("addr:housenumber");
    tagTranslator[QLatin1String("road")]           = QLatin1String("addr:street");
    tagTranslator[QLatin1String("suburb")]         = QLatin1String("addr:suburb");
    tagTranslator[QLatin1String("city")]           = QLatin1String("addr:city");
    tagTranslator[QLatin1String("state_district")] = QLatin1String("addr:district");
    tagTranslator[QLatin1String("state")]          = QLatin1String("addr:state");
    tagTranslator[QLatin1String("postcode")]       = QLatin1String("addr:postcode");
    tagTranslator[QLatin1String("country_code")]   = QLatin1String("addr:country"); // correct mapping

    // @todo Find a proper mapping for those

    //tagTranslator["village"] = "";
    //tagTranslator["town"] = "";

    GeoDataExtendedData data;
    OsmPlacemarkData osmData;
    QDomNodeList nodes = node.childNodes();

    for (int i = 0, n = nodes.length() ; i < n ; ++i)
    {
        QDomNode child = nodes.item(i);
        data.addValue(GeoDataData(child.nodeName(), child.toElement().text()));

        if (tagTranslator.contains(child.nodeName()))
        {
            QString const key = tagTranslator[child.nodeName()];
            osmData.addTag(key, child.toElement().text());
        }
    }

    placemark.setExtendedData(data);
    placemark.setOsmData(osmData);
}

} // namespace Marble

#include "moc_OsmNominatimReverseGeocodingRunner.cpp"

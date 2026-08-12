/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A simple backend to search OSM and Geonames.org.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchresultbackend.h"

// Qt includes

#include <QDomDocument>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "gpscommon.h"
#include "digikam_debug.h"
#include "networkmanager.h"

namespace DigikamGenericGeolocationEditPlugin
{

class Q_DECL_HIDDEN SearchResultBackend::Private
{
public:

    Private() = default;

public:

    SearchResultBackend::SearchResult::List results;
    QString                                 runningBackend;
    QByteArray                              searchData;
    QString                                 errorMessage;

    QNetworkReply*                          netReply    = nullptr;
    QNetworkAccessManager*                  mngr        = nullptr;
};

SearchResultBackend::SearchResultBackend(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->mngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->mngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

SearchResultBackend::~SearchResultBackend()
{
    delete d;
}

bool SearchResultBackend::search(const QString& backendName, const QString& searchTerm)
{
    d->searchData.clear();
    d->errorMessage.clear();
    d->results.clear();

    if (backendName == QLatin1String("osm"))
    {
        d->runningBackend = backendName;

        QUrl netUrl(QLatin1String("https://nominatim.openstreetmap.org/search"));

        QUrlQuery q(netUrl);
        q.addQueryItem(QLatin1String("format"), QLatin1String("xml"));
        q.addQueryItem(QLatin1String("q"), searchTerm);
        netUrl.setQuery(q);

        QNetworkRequest netRequest(netUrl);
        netRequest.setRawHeader("User-Agent", getUserAgentName().toLatin1());

        d->netReply = d->mngr->get(netRequest);

        return true;
    }

    if (backendName == QLatin1String("geonames.org"))
    {
        d->runningBackend = backendName;

        // documentation: https://www.geonames.org/export/geonames-search.html

        QUrl netUrl(QLatin1String("http://api.geonames.org/search"));       // krazy:exclude=insecurenet

        QUrlQuery q(netUrl);
        q.addQueryItem(QLatin1String("type"), QLatin1String("xml"));
        q.addQueryItem(QLatin1String("q"), searchTerm);
        q.addQueryItem(QLatin1String("username"), QLatin1String("digikam"));
        netUrl.setQuery(q);

        QNetworkRequest netRequest(netUrl);
        netRequest.setRawHeader("User-Agent", getUserAgentName().toLatin1());

        d->netReply = d->mngr->get(netRequest);

        return true;
    }

    return false;
}

void SearchResultBackend::slotFinished(QNetworkReply* reply)
{
    if (reply != d->netReply)
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        d->errorMessage = reply->errorString();
        Q_EMIT signalSearchCompleted();
        reply->deleteLater();
        return;
    }

    d->searchData.append(reply->readAll());

    const QString resultString = QString::fromUtf8(d->searchData.constData(), d->searchData.size());

    if      (d->runningBackend == QLatin1String("osm"))
    {
        QDomDocument doc;
        doc.setContent(resultString); // error-handling
        QDomElement docElement = doc.documentElement(); // error-handling

        for (QDomNode resultNode = docElement.firstChild(); !resultNode.isNull(); resultNode = resultNode.nextSibling())
        {
            QDomElement resultElement = resultNode.toElement();

            if (resultElement.isNull())
            {
                continue;
            }

            if (resultElement.tagName() != QLatin1String("place"))
            {
                continue;
            }

            const QString boundingBoxString = resultElement.attribute(QLatin1String("boundingbox"));
            const QString latString         = resultElement.attribute(QLatin1String("lat"));
            const QString lonString         = resultElement.attribute(QLatin1String("lon"));
            const QString displayName       = resultElement.attribute(QLatin1String("display_name"));
            const QString placeId           = resultElement.attribute(QLatin1String("place_id"));

            if (latString.isEmpty() || lonString.isEmpty() || displayName.isEmpty())
            {
                continue;
            }

            // now parse the strings:

            qreal lat;
            qreal lon;
            bool okay = false;
            lat       = latString.toDouble(&okay);

            if (okay)
            {
                lon = lonString.toDouble(&okay);
            }

            if (!okay)
            {
                continue;
            }

            SearchResult result;
            result.coordinates = GeoCoordinates(lat, lon);
            result.name        = displayName;

            if (!placeId.isEmpty())
            {
                result.internalId = QLatin1String("osm-") + placeId;
            }

            // TODO: parse bounding box
            Q_UNUSED(boundingBoxString);

            d->results << result;
        }
    }
    else if (d->runningBackend == QLatin1String("geonames.org"))
    {
        QDomDocument doc;
        doc.setContent(resultString);                       // error-handling
        QDomElement docElement = doc.documentElement();     // error-handling
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG)<<docElement.toElement().tagName();

        for (QDomNode resultNode = docElement.firstChild() ; !resultNode.isNull() ; resultNode = resultNode.nextSibling())
        {
            QDomElement resultElement = resultNode.toElement();
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << resultElement.tagName();

            if (resultElement.isNull())
            {
                continue;
            }

            if (resultElement.tagName() != QLatin1String("geoname"))
            {
                continue;
            }

            QString latString;
            QString lonString;
            QString displayName;
            QString geoNameId;

            for (QDomNode resultSubNode = resultElement.firstChild(); !resultSubNode.isNull(); resultSubNode = resultSubNode.nextSibling())
            {
                QDomElement resultSubElement = resultSubNode.toElement();

                if (resultSubElement.isNull())
                {
                    continue;
                }

                if      (resultSubElement.tagName() == QLatin1String("lat"))
                {
                    latString = resultSubElement.text();
                }
                else if (resultSubElement.tagName() == QLatin1String("lng"))
                {
                    lonString = resultSubElement.text();
                }
                else if (resultSubElement.tagName() == QLatin1String("name"))
                {
                    displayName = resultSubElement.text();
                }
                else if (resultSubElement.tagName() == QLatin1String("geonameId"))
                {
                    geoNameId = resultSubElement.text();
                }
            }

            if (latString.isEmpty() || lonString.isEmpty() || displayName.isEmpty())
            {
                continue;
            }

            // now parse the strings:

            qreal lat;
            qreal lon;
            bool okay = false;
            lat       = latString.toDouble(&okay);

            if (okay)
            {
                lon = lonString.toDouble(&okay);
            }

            if (!okay)
            {
                continue;
            }

            SearchResult result;
            result.coordinates = GeoCoordinates(lat, lon);
            result.name        = displayName;

            if (!geoNameId.isEmpty())
            {
                result.internalId = QLatin1String("geonames.org-") + geoNameId;
            }

            d->results << result;
        }
    }

    Q_EMIT signalSearchCompleted();

    reply->deleteLater();
}

SearchResultBackend::SearchResult::List SearchResultBackend::getResults() const
{
    return d->results;
}

QString SearchResultBackend::getErrorMessage() const
{
    return d->errorMessage;
}

QList<QPair<QString, QString> > SearchResultBackend::getBackends() const
{
    QList<QPair<QString, QString> > resultList;
    resultList << QPair<QString, QString>(i18n("GeoNames"), QLatin1String("geonames.org"));
    resultList << QPair<QString, QString>(i18n("OSM"), QLatin1String("osm"));

    return resultList;
}

} // namespace DigikamGenericGeolocationEditPlugin

#include "moc_searchresultbackend.cpp"

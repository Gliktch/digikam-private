/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-30
 * Description : Class for geonames.org based altitude lookup
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lookupaltitudegeonames.h"

// Qt includes


#include <QUrlQuery>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "geoifacetypes.h"
#include "networkmanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN MergedRequests
{
public:

    QList<QPair<GeoCoordinates, QIntList> > groupedRequestIndices;

    typedef QList<MergedRequests> List;

    bool addRequestIfCoordinatesAreThere(const LookupAltitude::Request& request, const int requestIndex)
    {
        for (int i = 0 ; i < groupedRequestIndices.size() ; ++i)
        {
            if (groupedRequestIndices.at(i).first.sameLonLatAs(request.coordinates))
            {
                groupedRequestIndices[i].second << requestIndex;

                return true;
            }
        }

        return false;
    }
};

// ------------------------------------------------------------

class Q_DECL_HIDDEN LookupAltitudeGeonames::Private
{
public:

    Private() = default;

public:

    Request::List          requests;
    MergedRequests::List   mergedRequests;
    StatusAltitude         status                       = StatusSuccess;
    QString                errorMessage;

    int                    currentMergedRequestIndex    = 0;

    QNetworkReply*         netReply                     = nullptr;
    QNetworkAccessManager* mngr                         = nullptr;
};

// ------------------------------------------------------------

LookupAltitudeGeonames::LookupAltitudeGeonames(QObject* const parent)
    : LookupAltitude(parent),
      d             (new Private)
{
    d->mngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->mngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

LookupAltitudeGeonames::~LookupAltitudeGeonames()
{
}

QString LookupAltitudeGeonames::backendName() const
{
    return QLatin1String("geonames");
}

QString LookupAltitudeGeonames::backendHumanName() const
{
    return i18n("geonames.org");
}

void LookupAltitudeGeonames::addRequests(const Request::List& requests)
{
    d->requests << requests;
}

LookupAltitude::Request::List LookupAltitudeGeonames::getRequests() const
{
    return d->requests;
}

LookupAltitude::Request LookupAltitudeGeonames::getRequest(const int index) const
{
    return d->requests.at(index);
}

void LookupAltitudeGeonames::startLookup()
{
    MergedRequests currentMergedRequest;

    for (int i = 0 ; i < d->requests.size() ; ++i)
    {
        const Request& currentRequest = d->requests.at(i);

        // is there another request with the same coordinates?

        bool requestAdded = currentMergedRequest.addRequestIfCoordinatesAreThere(currentRequest, i);

        for (int j = 0 ; (!requestAdded) && j < d->mergedRequests.size() ; ++j)
        {
            requestAdded = d->mergedRequests[j].addRequestIfCoordinatesAreThere(currentRequest, i);
        }

        if (!requestAdded)
        {
            currentMergedRequest.groupedRequestIndices.append(QPair<GeoCoordinates, QIntList>(currentRequest.coordinates, QIntList()<<i));

            if (currentMergedRequest.groupedRequestIndices.size() >= (20-1))
            {
                d->mergedRequests << currentMergedRequest;
                currentMergedRequest = MergedRequests();
            }
        }
    }

    if (!currentMergedRequest.groupedRequestIndices.isEmpty())
    {
        d->mergedRequests << currentMergedRequest;
    }

    // all requests have been grouped into batches of 20, now start the first one

    d->currentMergedRequestIndex = -1;
    startNextRequest();
}

void LookupAltitudeGeonames::startNextRequest()
{
    ++(d->currentMergedRequestIndex);

    if (d->currentMergedRequestIndex >= d->mergedRequests.count())
    {
        d->status = StatusSuccess;

        Q_EMIT signalDone();

        return;
    }

    const MergedRequests& currentMergedRequest = d->mergedRequests.at(d->currentMergedRequestIndex);

    QString latString;
    QString lonString;

    for (int i = 0 ; i < currentMergedRequest.groupedRequestIndices.count() ; ++i)
    {
        const QPair<GeoCoordinates, QIntList>& currentPair = currentMergedRequest.groupedRequestIndices.at(i);
        const GeoCoordinates requestCoordinates            = currentPair.first;

        if (!latString.isEmpty())
        {
            latString += QLatin1Char(',');
            lonString += QLatin1Char(',');
        }

        latString += requestCoordinates.latString();
        lonString += requestCoordinates.lonString();
    }

    QUrl netUrl(QLatin1String("http://api.geonames.org/srtm3"));            // krazy:exclude=insecurenet

    QUrlQuery q(netUrl);
    q.addQueryItem(QLatin1String("lats"), latString);
    q.addQueryItem(QLatin1String("lngs"), lonString);
    q.addQueryItem(QLatin1String("username"), QLatin1String("digikam"));
    netUrl.setQuery(q);

    d->netReply = d->mngr->get(QNetworkRequest(netUrl));
}

void LookupAltitudeGeonames::slotFinished(QNetworkReply* reply)
{
    if (reply != d->netReply)
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        d->errorMessage = reply->errorString();
        d->status       = StatusError;

        // after an error, we abort:

        reply->deleteLater();

        Q_EMIT signalDone();

        return;
    }

    QByteArray data                            = reply->readAll();
    const QStringList altitudes                = QString::fromLatin1(data).split(QRegularExpression(QLatin1String("\\s+")));
    const MergedRequests& currentMergedRequest = d->mergedRequests.at(d->currentMergedRequestIndex);
    QIntList readyRequests;

    for (int i = 0 ; i < qMin(altitudes.count(), currentMergedRequest.groupedRequestIndices.count()) ; ++i)
    {
        const QString& altitudeString = altitudes.at(i);
        bool haveAltitude             = false;
        const qreal altitude          = altitudeString.toFloat(&haveAltitude);

        // -32786 means that geonames.org has no data for these coordinates

        if (altitude == -32768)
        {
            haveAltitude = false;
        }

        const QIntList& currentRequestIndexes = currentMergedRequest.groupedRequestIndices.at(i).second;

        for (const int requestIndex : std::as_const(currentRequestIndexes))
        {
            if (haveAltitude)
            {
                d->requests[requestIndex].coordinates.setAlt(altitude);
            }
            else
            {
                d->requests[requestIndex].coordinates.clearAlt();
            }

            // The request has been carried out. Even if no altitude was
            // found, we return success.

            d->requests[requestIndex].success = true;
        }

        readyRequests << currentRequestIndexes;
    }

    Q_EMIT signalRequestsReady(readyRequests);

    reply->deleteLater();

    startNextRequest();
}

LookupAltitude::StatusAltitude LookupAltitudeGeonames::getStatus() const
{
    return d->status;
}

QString LookupAltitudeGeonames::errorMessage() const
{
    return d->errorMessage;
}

void LookupAltitudeGeonames::cancel()
{
    if (d->netReply && !d->netReply->isFinished())
    {
        d->netReply->abort();
    }

    d->status = StatusCanceled;

    Q_EMIT signalDone();
}

} // namespace Digikam

#include "moc_lookupaltitudegeonames.cpp"

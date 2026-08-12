/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : Backend for reverse geocoding using geonames.org (non-US)
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "backend-geonames-rg.h"

// Qt includes

#include <QDomDocument>
#include <QUrlQuery>
#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "networkmanager.h"
#include "gpscommon.h"

namespace Digikam
{

/**
 * @class BackendGeonamesRG
 *
 * @brief This class calls Geonames' reverse geocoding service.
 */

class Q_DECL_HIDDEN GeonamesInternalJobs
{
public:

    GeonamesInternalJobs()                                       = default;

    GeonamesInternalJobs(const GeonamesInternalJobs& other)
        : language(other.language),
          request (other.request),
          data    (other.data),
          netReply(other.netReply)
    {
    }

    GeonamesInternalJobs& operator=(const GeonamesInternalJobs& other)
    {
        language = other.language;
        request  = other.request;
        data     = other.data;
        netReply = other.netReply;

        return *this;
    }

    ~GeonamesInternalJobs()
    {
        if (netReply)
        {
            netReply->deleteLater();
        }
    }

    QString            language;
    QList<RGInfo>      request;
    QByteArray         data;

    QNetworkReply*     netReply = nullptr;
};

class Q_DECL_HIDDEN BackendGeonamesRG::Private
{
public:

    Private() = default;

    int                         itemCounter     = 0;
    int                         itemCount       = 0;
    QList<GeonamesInternalJobs> jobs;
    QString                     errorMessage;

    QNetworkAccessManager*      mngr            = nullptr;
};

/**
 * Constructor
 * @param parent the parent object.
 */
BackendGeonamesRG::BackendGeonamesRG(QObject* const parent)
    : RGBackend(parent),
      d        (new Private)
{
    d->mngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->mngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

/**
 * Destructor
 */
BackendGeonamesRG::~BackendGeonamesRG()
{
    delete d;
}

/**
 * This function calls Geonames's reverse geocoding service for each image.
 */
void BackendGeonamesRG::nextPhoto()
{
    if (d->jobs.isEmpty())
    {
        return;
    }

    QUrl netUrl(QLatin1String("http://api.geonames.org/findNearbyPlaceName"));      // krazy:exclude=insecurenet

    QUrlQuery q(netUrl);
    q.addQueryItem(QLatin1String("lat"),      d->jobs.first().request.first().coordinates.latString());
    q.addQueryItem(QLatin1String("lng"),      d->jobs.first().request.first().coordinates.lonString());
    q.addQueryItem(QLatin1String("lang"),     d->jobs.first().language);
    q.addQueryItem(QLatin1String("username"), QLatin1String("digikam"));
    netUrl.setQuery(q);

    QNetworkRequest netRequest(netUrl);
    netRequest.setRawHeader("User-Agent", getUserAgentName().toLatin1());

    d->jobs.first().netReply = d->mngr->get(netRequest);
}

/**
 * Takes coordinates from each image and then connects to Open Street Map's reverse geocoding service.
 * @param rgList A list containing information needed in reverse geocoding process. At this point, it contains only coordinates.
 * @param language The language in which the data will be returned.
 */
void BackendGeonamesRG::callRGBackend(const QList<RGInfo>& rgList, const QString& language)
{
    d->errorMessage.clear();

    for (int i = 0 ; i < rgList.count() ; ++i)
    {
            bool foundIt = false;

            for (int j = 0 ; j < d->jobs.count() ; ++j)
            {
                if (d->jobs[j].request.first().coordinates.sameLonLatAs(rgList[i].coordinates))
                {
                    d->jobs[j].request << rgList[i];
                    d->jobs[j].language = language;
                    foundIt             = true;
                    break;
                }
            }

            if (!foundIt)
            {
                GeonamesInternalJobs newJob;
                newJob.request << rgList.at(i);
                newJob.language = language;
                d->jobs << newJob;
            }
    }

    nextPhoto();
}

/**
 * The data is returned from Open Street Map in a XML. This function translates the XML into a QMap.
 * @param xmlData The returned XML.
 */
QMap<QString, QString> BackendGeonamesRG::makeQMapFromXML(const QString& xmlData)
{
    QMap<QString, QString> mappedData;
    QString resultString;
    QDomDocument doc;

    doc.setContent(xmlData);

    QDomElement docElem = doc.documentElement();
    QDomNode n          = docElem.firstChild().firstChild();

    while (!n.isNull())
    {
        const QDomElement e = n.toElement();

        if (!e.isNull())
        {
            if ((e.tagName() == QLatin1String("countryName")) ||
                (e.tagName() == QLatin1String("countryCode")) ||
                (e.tagName() == QLatin1String("name")))
            {
                mappedData.insert(e.tagName(), e.text());
                resultString.append(e.tagName() + QLatin1Char(':') + e.text() + QLatin1Char('\n'));
            }
        }

        n = n.nextSibling();
    }

    return mappedData;
}

/**
 * @return Error message, if any.
 */
QString BackendGeonamesRG::getErrorMessage()
{
    return d->errorMessage;
}

/**
 * @return Backend name.
 */
QString BackendGeonamesRG::backendName()
{
    return QLatin1String("Geonames");
}

void BackendGeonamesRG::slotFinished(QNetworkReply* reply)
{
    for (int i = 0 ; i < d->jobs.count() ; ++i)
    {
        if (d->jobs.at(i).netReply == reply)
        {
            if (reply->error() != QNetworkReply::NoError)
            {
                d->errorMessage = reply->errorString();

                Q_EMIT signalRGReady(d->jobs.first().request);

                reply->deleteLater();
                d->jobs.clear();

                return;
            }

            d->jobs[i].data.append(reply->readAll());
            break;
        }
    }

    for (int i = 0 ; i < d->jobs.count() ; ++i)
    {
        if (d->jobs.at(i).netReply == reply)
        {
            QString dataString;
            dataString = QString::fromUtf8(d->jobs[i].data.constData(),qstrlen(d->jobs[i].data.constData()));
            int pos    = dataString.indexOf(QLatin1String("<geonames"));
            dataString.remove(0,pos);
            dataString.chop(1);

            QMap<QString, QString> resultMap = makeQMapFromXML(dataString);

            for (int j = 0 ; j < d->jobs[i].request.count() ; ++j)
            {
                d->jobs[i].request[j].rgData =  resultMap;
            }

            Q_EMIT signalRGReady(d->jobs[i].request);

            d->jobs.removeAt(i);

            if (!d->jobs.isEmpty())
            {
                QTimer::singleShot(500, this, SLOT(nextPhoto()));
            }

            reply->deleteLater();
            break;
        }
    }
}

void BackendGeonamesRG::cancelRequests()
{
    d->jobs.clear();
    d->errorMessage.clear();
}

} // namespace Digikam

#include "moc_backend-geonames-rg.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021-2022 by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inattalker.h"

// Qt includes

#include <QByteArray>
#include <QMap>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QStringList>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wstoolutils.h"
#include "inatutils.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "digikam_version.h"
#include "previewloadthread.h"
#include "inatbrowserdlg.h"
#include "networkmanager.h"

// OAuth2 library includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wextra-semi"
#endif

#include "o0globals.h"
#include "o0settingsstore.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

namespace DigikamGenericINatPlugin
{

static const char COOKIE_SEPARATOR          = '\n';

enum
{
    INAT_API_TOKEN_EXPIRATION               = 86000, ///< api tokens are valid for 24 hours
    GEOLOCATION_PRECISION                   =     8, ///< # digits after decimal point
    RADIUS_PRECISION                        =     6, ///< # digits after decimal point
    EARTH_RADIUS_KM                         =  6371, ///< Earth radius in kilometers
    TIMEOUT_TIMER_RESOLUTION_SECS           =    30, ///< timeout check every 30 seconds
    RESPONSE_TIMEOUT_SECS                   =   300, ///< network timeout after this many seconds
    MAX_RETRIES                             =     5  ///< retry network requests this many times
};

/**
 * Fields in server responses received from iNaturalist.
 */
static const QString API_TOKEN              = QLatin1String("api_token");
static const QString TOTAL_RESULTS          = QLatin1String("total_results");
static const QString PAGE                   = QLatin1String("page");
static const QString PER_PAGE               = QLatin1String("per_page");
static const QString LOCALE                 = QLatin1String("locale");
static const QString RESULTS                = QLatin1String("results");
static const QString NAME                   = QLatin1String("name");
static const QString TAXON                  = QLatin1String("taxon");
static const QString TAXON_ID               = QLatin1String("taxon_id");
static const QString ID                     = QLatin1String("id");
static const QString PARENT_ID              = QLatin1String("parent_id");
static const QString RANK                   = QLatin1String("rank");
static const QString RANK_LEVEL             = QLatin1String("rank_level");
static const QString PREFERRED_COMMON_NAME  = QLatin1String("preferred_common_name");
static const QString ENGLISH_COMMON_NAME    = QLatin1String("english_common_name");
static const QString MATCHED_TERM           = QLatin1String("matched_term");
static const QString DEFAULT_PHOTO          = QLatin1String("default_photo");
static const QString SQUARE_URL             = QLatin1String("square_url");
static const QString ANCESTORS              = QLatin1String("taxon_photos");
static const QString OBSCURED               = QLatin1String("obscured");
static const QString GEOJSON                = QLatin1String("geojson");
static const QString COORDINATES            = QLatin1String("coordinates");
static const QString LOGIN                  = QLatin1String("login");
static const QString ICON                   = QLatin1String("icon");
static const QString OBSERVATION            = QLatin1String("observation");
static const QString OBSERVATIONS           = QLatin1String("observations");
static const QString OBSERVED_ON            = QLatin1String("observed_on");
static const QString OBSERVED_ON_STRING     = QLatin1String("observed_on_string");
static const QString OBSERVATION_PHOTOS     = QLatin1String("observation_photos");
static const QString PHOTO                  = QLatin1String("photo");

static QJsonObject parseJsonResponse(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        qCWarning(DIGIKAM_WEBSERVICES_LOG) << "parseJsonResponse: Failed to parse json response:"
                                           << err.errorString();

        return QJsonObject();
    }

    if (!doc.isObject())
    {
        qCWarning(DIGIKAM_WEBSERVICES_LOG) << "parseJsonResponse: Json response is not an object!";

        return QJsonObject();
    }

    return doc.object();
}

static Taxon parseTaxon(const QJsonObject& taxon)
{
    int id           = -1;
    int parentId     = -1;
    double rankLevel = -1.0;
    QString name;
    QString rank;
    QString commonName;
    QString matchedTerm;
    QUrl squareUrl;
    QList<Taxon> ancestors;

    if (taxon.contains(NAME))
    {
        name = taxon[NAME].toString();
    }

    if (taxon.contains(ID))
    {
        id = taxon[ID].toInt();
    }

    if (taxon.contains(PARENT_ID))
    {
        parentId = taxon[PARENT_ID].toInt();
    }

    if (taxon.contains(RANK))
    {
        rank = taxon[RANK].toString();
    }

    if (taxon.contains(RANK_LEVEL))
    {
        rankLevel = taxon[RANK_LEVEL].toDouble();
    }

    if      (taxon.contains(PREFERRED_COMMON_NAME))
    {
        commonName = taxon[PREFERRED_COMMON_NAME].toString();
    }
    else if (isEnglish && taxon.contains(ENGLISH_COMMON_NAME))
    {
        commonName = taxon[ENGLISH_COMMON_NAME].toString();
    }

    if (taxon.contains(MATCHED_TERM))
    {
        matchedTerm = taxon[MATCHED_TERM].toString();
    }

    if (taxon.contains(DEFAULT_PHOTO) &&
        taxon[DEFAULT_PHOTO].toObject().contains(SQUARE_URL))
    {
        squareUrl = QUrl(taxon[DEFAULT_PHOTO].toObject()
                         .value(SQUARE_URL).toString());
    }

    if (taxon.contains(ANCESTORS))
    {
        const auto array = taxon[ANCESTORS].toArray();

        for (const auto& ancestorTaxon : array)
        {
            ancestors << parseTaxon(ancestorTaxon.toObject());
        }
    }

    return Taxon(id, parentId, name, rank, rankLevel, commonName,
                 matchedTerm, squareUrl, ancestors);
}

// ------------------------------------------------------------------------------------------

/**
 * A request consists in a state and a function that is called with the response.
 */
class Q_DECL_HIDDEN Request
{
public:

    Request()          = default;
    virtual ~Request() = default;

    static bool networkErrorRetry(QNetworkReply::NetworkError code)
    {
        return (
                (code == QNetworkReply::ConnectionRefusedError)       ||
                (code == QNetworkReply::RemoteHostClosedError)        ||
                (code == QNetworkReply::HostNotFoundError)            ||
                (code == QNetworkReply::TimeoutError)                 ||
                (code == QNetworkReply::TemporaryNetworkFailureError) ||
                (code == QNetworkReply::NetworkSessionFailedError)    ||
                (code == QNetworkReply::InternalServerError)          ||
                (code == QNetworkReply::ServiceUnavailableError)      ||
                (code == QNetworkReply::UnknownServerError)
               );
    }

    virtual void reportError(INatTalker&, QNetworkReply::NetworkError,
                             const QString& errorString) const = 0;

    virtual void parseResponse(INatTalker& talker,
                               const QByteArray& data) const = 0;

    // Has operation timed out?

    bool isTimeout() const
    {
        return (durationMilliSecs() > (1000 * RESPONSE_TIMEOUT_SECS));
    }

    // How long did it take?

    qint64 durationMilliSecs() const
    {
        return (QDateTime::currentMSecsSinceEpoch() - m_startTime);
    }

private:

    qint64 m_startTime = QDateTime::currentMSecsSinceEpoch();

private:

    Q_DISABLE_COPY(Request)
};

// --------------------------------------------------------------------------------

class Q_DECL_HIDDEN INatTalker::Private
{
public:

    Private() = default;

    void clear()
    {
        apiTokenExpires = 0;
        apiToken        = QString();
    }

public:

    QWidget*                          parent            = nullptr;
    QNetworkAccessManager*            netMngr           = nullptr;
    QTimer*                           timer             = nullptr;
    QSettings*                        settings          = nullptr;
    DInfoInterface*                   iface             = nullptr;
    O0SettingsStore*                  store             = nullptr;

    QString                           serviceName;
    const QString                     cryptId           = QLatin1String("119b0b8a57644341fe03eca486a341");
    const QString                     apiUrl            = QLatin1String("https://api.inaturalist.org/v1/");

    /// keys used in O0SettingsStore
    const QString                     keyToken          = QString(QLatin1String(O2_KEY_TOKEN)).arg(cryptId);
    const QString                     keyExpires        = QString(QLatin1String(O2_KEY_EXPIRES)).arg(cryptId);
    const QString                     keyCookies        = QString(QLatin1String("cookies.%1")).arg(cryptId);

    /// the api token and its expiration time in seconds since January 1st, 1970
    QString                           apiToken;
    uint                              apiTokenExpires   = 0;

    /// this hash table allows us to serve multiple requests concurrently
    QHash<QNetworkReply*, Request*>   pendingRequests;

    /// cached api call results
    QHash<QString, AutoCompletions>   cachedAutoCompletions;
    QHash<QUrl, QByteArray>           cachedLoadUrls;
    QHash<QString, ImageScores>       cachedImageScores;
    QHash<QString, QStringList>       cachedNearbyPlaces;
    QHash<QString, NearbyObservation> cachedNearbyObservations;
};

INatTalker::INatTalker(QWidget* const parent, const QString& serviceName,
                       DInfoInterface* const iface)
    : d(new Private)
{
    d->parent          = parent;
    d->serviceName     = serviceName;
    d->iface           = iface;
    m_authProgressDlg  = nullptr;

    d->netMngr         = NetworkManager::instance()->getNetworkManager(this);
    d->timer           = new QTimer(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));

    d->settings = WSToolUtils::getOauthSettings(this);
    d->store    = new O0SettingsStore(d->settings,
                                      QLatin1String(O2_ENCRYPTION_KEY), this);
    d->store->setGroupKey(d->serviceName);
    d->timer->start(TIMEOUT_TIMER_RESOLUTION_SECS * 1000);
}

INatTalker::~INatTalker()
{
    d->timer->stop();
    d->clear();
    WSToolUtils::removeTemporaryDir(d->serviceName.toLatin1().constData());

    delete d;
}

/**
 * Try to restore a valid API token; they are good for 24 hours.
 */
bool INatTalker::restoreApiToken(const QString& username,
                                 QList<QNetworkCookie>& cookies,
                                 bool emitSignal)
{
    cookies.clear();

    if (username.isEmpty())
    {
        return false;
    }

    d->store->setGroupKey(d->serviceName + username);
    d->apiToken        = d->store->value(d->keyToken);
    d->apiTokenExpires = d->store->value(d->keyExpires,
                                         QString::number(0)).toInt();
    QString cookiesStr = d->store->value(d->keyCookies);

    if (!cookiesStr.isEmpty())
    {
        QDateTime now(QDateTime::currentDateTime());
        const auto strs = cookiesStr.split(QLatin1Char(COOKIE_SEPARATOR));

        for (const auto& str : strs)
        {
            QList<QNetworkCookie> lst(QNetworkCookie::parseCookies(str.toUtf8()));

            Q_ASSERT(lst.count() == 1);

            for (const auto& cookie : std::as_const(lst))
            {
                if (INatBrowserDlg::filterCookie(cookie, true, now))
                {
                    cookies << cookie;
                }
            }
        }
    }

    bool valid = (emitSignal && (apiTokenExpiresIn() > 0));

    if (valid)
    {
        userInfo(cookies);
    }

    return valid;
}

bool INatTalker::stillUploading() const
{
    return d->pendingRequests.count();
}

void INatTalker::unLink()
{
    d->clear();
}

void INatTalker::removeUserName(const QString& userName)
{
    if (userName.startsWith(d->serviceName))
    {
        d->settings->beginGroup(userName);
        d->settings->remove(QString());
        d->settings->endGroup();
    }
}

int INatTalker::apiTokenExpiresIn() const
{
    if (d->apiToken.isEmpty())
    {
        return -1;
    }

    uint time = (uint)(QDateTime::currentMSecsSinceEpoch() / 1000);

    return (
            (d->apiTokenExpires <= time) ? -1
                                         : (d->apiTokenExpires - time)
           );
}

/**
 * We received an api token from a web browser login.
 */
void INatTalker::slotApiToken(const QString& apiToken,
                              const QList<QNetworkCookie>& cookies)
{
    d->apiToken = apiToken;

    if (apiToken.isEmpty())
    {
        Q_EMIT signalLinkingFailed(QLatin1String("no api token"));

        return;
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "API token received; "
                                            "querying user info.";

        d->apiTokenExpires = (uint)(QDateTime::currentMSecsSinceEpoch() / 1000 +
                                    INAT_API_TOKEN_EXPIRATION);
        userInfo(cookies);
    }
}

// ------------------------------------------------------------------------------------------

/**
 * Get login, name, and icon of authorized user.
 */
class Q_DECL_HIDDEN UserRequest : public Request
{
public:

    explicit UserRequest(const QList<QNetworkCookie>& cookies)
        : m_cookies(cookies)
    {
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "users/me error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        Q_EMIT talker.signalLinkingFailed(QLatin1String("user-info request "
                                                        "failed"));
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        QJsonObject json = parseJsonResponse(data);

        if (json.contains(RESULTS) && (json[RESULTS].toArray().count() == 1))
        {
            QJsonObject result = json[RESULTS].toArray()[0].toObject();
            QString username(result[LOGIN].toString());

            Q_EMIT talker.signalLinkingSucceeded(username,
                                               result[NAME].toString(),
                                               QUrl(result[ICON].toString()));

            talker.d->store->setGroupKey(talker.d->serviceName + username);

            // save api token

            talker.d->store->setValue(talker.d->keyToken, talker.d->apiToken);

            // save api token expiration

            talker.d->store->setValue(talker.d->keyExpires, QString::number(talker.d->apiTokenExpires));

            // save cookies

            QDateTime now(QDateTime::currentDateTime());
            QByteArray saveCookies;

            for (const auto& cookie : m_cookies)
            {
                if (!INatBrowserDlg::filterCookie(cookie, true, now))
                {
                    continue;
                }

                if (!saveCookies.isEmpty())
                {
                    saveCookies += COOKIE_SEPARATOR;
                }

                saveCookies += cookie.toRawForm();
            }

            talker.d->store->setValue(talker.d->keyCookies,
                                      QString::fromUtf8(saveCookies));
        }
        else
        {
            Q_EMIT talker.signalLinkingFailed(QLatin1String("user-info request "
                                                            "failed"));
        }

        if (talker.m_authProgressDlg)
        {
            talker.m_authProgressDlg->setValue(2);
            talker.m_authProgressDlg->hide();
        }

        Q_EMIT talker.signalBusy(false);
    }

private:

    QList<QNetworkCookie> m_cookies;
};

void INatTalker::userInfo(const QList<QNetworkCookie>& cookies)
{
    if (d->apiToken.isEmpty())
    {
        return;
    }

    Q_EMIT signalBusy(true);

    if (m_authProgressDlg)
    {
        m_authProgressDlg->setLabelText(QLatin1String("<font color=\"#74ac00\">") +
                                        i18n("iNaturalist")                       +
                                        QLatin1String("</font> ")                 +
                                        i18n("Login"));
        m_authProgressDlg->setMaximum(2);
        m_authProgressDlg->setValue(1);
        m_authProgressDlg->show();
    }

    QUrl url(d->apiUrl + QLatin1String("users/me"));
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", d->apiToken.toLatin1());
    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new UserRequest(cookies));
}

// ------------------------------------------------------------------------------------------

/**
 * Load a URL and return result as QByteArry; used for images (icons).
 */
class Q_DECL_HIDDEN LoadUrlRequest : public Request
{
public:

    LoadUrlRequest(const QUrl& url, int retries)
        : m_url    (url),
          m_retries(retries)
    {
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError code,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url" << m_url << "error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        if      (networkErrorRetry(code) && (m_retries < MAX_RETRIES))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Attempting to load" << m_url << "again, retry"
                                             << m_retries + 1 << "of" << MAX_RETRIES;

            talker.loadUrl(m_url, m_retries + 1);
        }
        else if (talker.d->cachedLoadUrls.contains(m_url))
        {
            talker.d->cachedLoadUrls.remove(m_url);
        }
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url" << m_url << "loaded in"
                                         << durationMilliSecs() << "msecs.";

        talker.d->cachedLoadUrls.insert(m_url, data);

        Q_EMIT talker.signalLoadUrlSucceeded(m_url, data);
    }

private:

    QUrl m_url;
    int  m_retries = 0;

private:

    Q_DISABLE_COPY(LoadUrlRequest)
};

void INatTalker::loadUrl(const QUrl& imgUrl, int retries)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting url" << imgUrl.url();

    if (imgUrl.isEmpty() || imgUrl.isLocalFile() || imgUrl.isRelative())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Ignoring; NOT loading url"
                                         << imgUrl;
        return;
    }

    if (d->cachedLoadUrls.contains(imgUrl))
    {
        const QByteArray& result = d->cachedLoadUrls.value(imgUrl);

        if (!result.isEmpty())
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url" << imgUrl
                                             << "found in cache.";

            Q_EMIT signalLoadUrlSucceeded(imgUrl, result);
        }
        else
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url load of" << imgUrl
                                             << "already in progress; ignoring request.";
        }

        return;
    }

    // empty cache entry means load in progress

    d->cachedLoadUrls.insert(imgUrl, QByteArray());

    QNetworkRequest netRequest(imgUrl);
    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new LoadUrlRequest(imgUrl, retries));
}

// ------------------------------------------------------------------------------------------

/**
 *  taxon auto-completion
 */
class Q_DECL_HIDDEN AutoCompletionRequest : public Request
{
public:

    explicit AutoCompletionRequest(const QString& name)
        : m_partialName(name)
    {
    }

    void reportError(INatTalker&, QNetworkReply::NetworkError,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Taxon auto-completion" << m_partialName
                                         << "error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        QJsonObject json = parseJsonResponse(data);

        if (json.contains(RESULTS))
        {
            QJsonArray results = json[RESULTS].toArray();
            QList<Taxon> taxa;

            for (const auto& result : std::as_const(results))
            {
                taxa << parseTaxon(result.toObject());
            }

            QPair<QString, QList<Taxon> > result(m_partialName, taxa);
            talker.d->cachedAutoCompletions.insert(m_partialName, result);

            Q_EMIT talker.signalTaxonAutoCompletions(result);
        }
    }

private:

    QString m_partialName;

private:

    Q_DISABLE_COPY(AutoCompletionRequest)
};

void INatTalker::taxonAutoCompletions(const QString& partialName)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting taxon auto-completions for"
                                     <<  partialName;

    if (d->cachedAutoCompletions.contains(partialName))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Taxon auto-completions for"
                                         <<  partialName << "found in cache.";

        Q_EMIT signalTaxonAutoCompletions(d->cachedAutoCompletions.
                                        value(partialName));
        return;
    }

    QUrl url(d->apiUrl + QLatin1String("taxa/autocomplete"));
    QUrlQuery query;
    query.addQueryItem(QLatin1String("q"),         partialName);
    query.addQueryItem(QLatin1String("is_active"), QLatin1String("true"));
    query.addQueryItem(PER_PAGE,                   QString::number(12));
    query.addQueryItem(LOCALE,                     locale.name());
    url.setQuery(query.query());

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));

    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new AutoCompletionRequest(partialName));
}

// ------------------------------------------------------------------------------------------

/**
 * get nearby places
 */
class Q_DECL_HIDDEN NearbyPlacesRequest : public Request
{

public:

    NearbyPlacesRequest(double latitude, double longitude, const QString& query)
        : m_latitude (latitude),
          m_longitude(longitude),
          m_query    (query)
    {
        Q_UNUSED(m_latitude);
        Q_UNUSED(m_longitude);
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        QJsonObject json = parseJsonResponse(data);

        if (json.contains(RESULTS))
        {
            static const QString BBOX_AREA    = QLatin1String("bbox_area");
            static const QString DISPLAY_NAME = QLatin1String("display_name");
            QJsonObject results               = json[RESULTS].toObject();
            QList<Place> places;
            const auto keys                   = results.keys();

            for (const auto& key : keys)
            {
                const auto array = results.value(key).toArray();

                for (const auto& placeValue : array)
                {
                    QJsonObject place = placeValue.toObject();
                    places.push_front(Place(place[DISPLAY_NAME].toString(),
                                            place[BBOX_AREA].toDouble()));
                }
            }

            std::sort(places.begin(), places.end());
            QStringList placesStrList;

            for (const auto& place : places)
            {
                placesStrList << place.m_name;
            }

            talker.d->cachedNearbyPlaces.insert(m_query, placesStrList);

            Q_EMIT talker.signalNearbyPlaces(placesStrList);
        }
    }

    void reportError(INatTalker&, QNetworkReply::NetworkError,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Nearby places error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";
    }

private:

    struct Q_DECL_HIDDEN Place
    {
        Place() = default;

        Place(const QString& n, double ba)
            : m_name    (n),
              m_bboxArea(ba)
        {
        }

        bool operator <(const Place& other) const
        {
            return (m_bboxArea < other.m_bboxArea);
        }

        QString m_name;
        double  m_bboxArea = 0.0;
    };

private:

    double  m_latitude  = 0.0;
    double  m_longitude = 0.0;
    QString m_query;

private:

    Q_DISABLE_COPY(NearbyPlacesRequest)
};

void INatTalker::nearbyPlaces(double latitude, double longitude)
{
    QUrl url(d->apiUrl + QLatin1String("places/nearby"));

    QString lat = QString::number(latitude,  'f', GEOLOCATION_PRECISION);
    QString lng = QString::number(longitude, 'f', GEOLOCATION_PRECISION);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting nearby places for lat"
                                     << lat << "lon" << lng;
    QUrlQuery query;
    query.addQueryItem(QLatin1String("nelat"), lat);
    query.addQueryItem(QLatin1String("nelng"), lng);
    query.addQueryItem(QLatin1String("swlat"), lat);
    query.addQueryItem(QLatin1String("swlng"), lng);
    query.addQueryItem(PER_PAGE, QString::number(100));
    url.setQuery(query.query());

    if (d->cachedNearbyPlaces.contains(query.query()))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Nearby places for lat" << lat
                                         << "lon" << lng << "found in cache.";

        Q_EMIT signalNearbyPlaces(d->cachedNearbyPlaces.value(query.query()));

        return;
    }

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));

    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new NearbyPlacesRequest(latitude, longitude,
                                                      query.query()));
}

// ------------------------------------------------------------------------------------------

/**
 * Find the closest observation; used as a sanity check for identifications:
 * when the closest known observation is hundreds of kilometers away, we
 * have likely misidentified the organism in our photo.
 */
class Q_DECL_HIDDEN NearbyObservationRequest : public Request
{

public:

    NearbyObservationRequest(uint taxon, double latitude, double longitude,
                             double km, const QString& query)
        : m_taxon    (taxon),
          m_latitude (latitude),
          m_longitude(longitude),
          m_radiusKm (km),
          m_query    (query)
    {
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        QJsonObject json = parseJsonResponse(data);

        if (json.contains(TOTAL_RESULTS))
        {
            static const double MAX_DISTANGE_KM = (int)EARTH_RADIUS_KM * M_PI;
            int total_results                   = json[TOTAL_RESULTS].toInt();

            if (total_results == 0)
            {
                if (m_radiusKm < MAX_DISTANGE_KM)
                {
                    // Try again, double the radius.

                    talker.closestObservation(m_taxon, m_latitude, m_longitude,
                                              2 * m_radiusKm, m_query);
                }
                else
                {
                    // The entire Earth searched, no observation found.

                    talker.d->cachedNearbyObservations.insert(m_query,
                            INatTalker::NearbyObservation());

                    Q_EMIT talker.signalNearbyObservation(INatTalker::
                                                        NearbyObservation());
                }
            }
            else
            {
                INatTalker::NearbyObservation closestObscured
                (
                    -1,
                    0.0,
                    0.0,
                    MAX_DISTANGE_KM * 1000.0,
                    true,
                    m_taxon,
                    m_latitude,
                    m_longitude
                );

                INatTalker::NearbyObservation closestOpen
                (
                    -1,
                    0.0,
                    0.0,
                    MAX_DISTANGE_KM * 1000.0,
                    false,
                    m_taxon,
                    m_latitude,
                    m_longitude
                );

                const auto results = json[RESULTS].toArray();

                for (const auto& resultValue : results)
                {
                    QJsonObject result = resultValue.toObject();

                    if (!result.contains(GEOJSON))
                    {
                        continue;
                    }

                    int observationId     = result[ID].toInt();
                    QJsonArray coordinates(result[GEOJSON].toObject()
                                           .value(COORDINATES).toArray());
                    double latitude       = coordinates[1].toDouble();
                    double longitude      = coordinates[0].toDouble();
                    bool obscured         = result[OBSCURED].toBool();
                    double distanceMeters = distanceBetween(m_latitude,
                                                            m_longitude,
                                                            latitude,
                                                            longitude);

                    if      (obscured)
                    {
                        if (distanceMeters < closestObscured.m_distanceMeters)
                        {
                            closestObscured.updateObservation(observationId,
                                                              latitude,
                                                              longitude,
                                                              distanceMeters);
                        }
                    }
                    else if (distanceMeters < closestOpen.m_distanceMeters)
                    {
                        closestOpen.updateObservation(observationId,
                                                      latitude, longitude,
                                                      distanceMeters);
                    }
                }

                double newDistanceMeters = closestOpen.isValid() ? closestOpen.m_distanceMeters
                                                                 : closestObscured.m_distanceMeters;

                if ((results.count() < total_results) && (newDistanceMeters > 10.0))
                {
                    // There are additional observations that we have not
                    // downloaded; request closer ones.

                    talker.closestObservation(m_taxon, m_latitude, m_longitude,
                                              newDistanceMeters / 999.0,
                                              m_query);
                }
                else
                {
                    talker.d->cachedNearbyObservations.insert(m_query,
                            closestOpen.isValid() ? closestOpen
                                                  : closestObscured);

                    Q_EMIT talker.signalNearbyObservation(closestOpen.isValid()
                                                        ? closestOpen
                                                        : closestObscured);
                }
            }
        }
    }

    void reportError(INatTalker&, QNetworkReply::NetworkError,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Closest observation error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";
    }

private:

    uint    m_taxon     = 0;
    double  m_latitude  = 0.0;
    double  m_longitude = 0.0;
    double  m_radiusKm  = 0.0;
    QString m_query;

private:

    Q_DISABLE_COPY(NearbyObservationRequest)
};

void INatTalker::closestObservation(uint taxon, double latitude,
                                    double longitude, double radiusKm,
                                    const QString& origQuery)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting closest observation of"
                                     << taxon << "to" << latitude << longitude
                                     << "with radius" << radiusKm << "km.";

    QUrl url(d->apiUrl + OBSERVATIONS);

    QUrlQuery query;
    query.addQueryItem(QLatin1String("geo"), QLatin1String("true"));
    query.addQueryItem(TAXON_ID, QString::number(taxon));
    query.addQueryItem(QLatin1String("lat"), QString::number(latitude, 'f',
                       GEOLOCATION_PRECISION));
    query.addQueryItem(QLatin1String("lng"), QString::number(longitude, 'f',
                       GEOLOCATION_PRECISION));
    query.addQueryItem(QLatin1String("radius"), QString::number(radiusKm, 'f',
                       RADIUS_PRECISION));
    query.addQueryItem(QLatin1String("quality_grade"),
                       QLatin1String("research"));
    query.addQueryItem(LOCALE, locale.name());
    query.addQueryItem(PER_PAGE, QString::number(100));
    url.setQuery(query.query());

    if (d->cachedNearbyObservations.contains(query.query()))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Closest observation of"
                                         << taxon << "at" << latitude
                                         << longitude << "with radius"
                                         << radiusKm << "km found in cache.";

        Q_EMIT signalNearbyObservation(d->cachedNearbyObservations.value(
                                         query.query()));
        return;
    }

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));

    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new NearbyObservationRequest(taxon, latitude, longitude, radiusKm,
                                                           origQuery.isEmpty() ? query.query()
                                                                               : origQuery));
}

// ------------------------------------------------------------------------------------------

/**
 *  get taxon suggestions for an image
 */
class Q_DECL_HIDDEN ComputerVisionRequest : public Request
{

public:

    ComputerVisionRequest(const QString& imgPath, const QString& tmpFile)
        : m_imagePath  (imgPath),
          m_tmpFilePath(tmpFile)
    {
    }

    ~ComputerVisionRequest() override
    {
        if (!m_tmpFilePath.isEmpty() && QFile::exists(m_tmpFilePath))
        {
            QFile::remove(m_tmpFilePath);
        }
    }

    void parseScore(const QJsonObject& json, QList<ComputerVisionScore>& scores) const
    {
        static const QString FREQUENCY_SCORE = QLatin1String("frequency_score");
        static const QString VISION_SCORE    = QLatin1String("vision_score");
        static const QString COMBINED_SCORE  = QLatin1String("combined_score");

        double frequency_score               = 0.0;
        double vision_score                  = 0.0;
        double combined_score                = 0.0;
        Taxon  taxon;

        if (json.contains(FREQUENCY_SCORE))
        {
            frequency_score = json[FREQUENCY_SCORE].toDouble();
        }

        if (json.contains(VISION_SCORE))
        {
            vision_score = json[VISION_SCORE].toDouble();
        }

        if (json.contains(COMBINED_SCORE))
        {
            combined_score = json[COMBINED_SCORE].toDouble();
        }

        if (json.contains(TAXON))
        {
            taxon = parseTaxon(json[TAXON].toObject());
        }

        scores << ComputerVisionScore(frequency_score, vision_score,
                                      combined_score, taxon);
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        static const QString COMMON_ANCESTOR = QLatin1String("common_ancestor");

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Computer vision for" << m_imagePath
                                         << "took" << durationMilliSecs() << "msecs.";

        QJsonObject json = parseJsonResponse(data);
        QList<ComputerVisionScore> scores;

        if (json.contains(COMMON_ANCESTOR))
        {
            parseScore(json[COMMON_ANCESTOR].toObject(), scores);
        }

        if (json.contains(RESULTS))
        {
            const auto array = json[RESULTS].toArray();

            for (const auto& result : array)
            {
                parseScore(result.toObject(), scores);
            }
        }

        ImageScores result(m_imagePath, scores);
        talker.d->cachedImageScores.insert(m_imagePath, result);
        Q_EMIT talker.signalComputerVisionResults(result);
    }

    void reportError(INatTalker&, QNetworkReply::NetworkError,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Computer vision error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";
    }

private:

    QString m_imagePath;
    QString m_tmpFilePath;

private:

    Q_DISABLE_COPY(ComputerVisionRequest)
};

void INatTalker::computerVision(const QUrl& localImage)
{
    if (localImage.isEmpty() || (apiTokenExpiresIn() <= 0))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Computer-vision API not called:"
                                         << (localImage.isEmpty() ? "No image."
                                                                  : "Not logged in.");
        return;
    }

    enum
    {
        HEIGHT = 299,
        WIDTH  = 299
    };

    QString path = localImage.toLocalFile();

    if (d->cachedImageScores.contains(path))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Vision identification for"
                                         << localImage.toLocalFile()
                                         << "found in cache.";

        Q_EMIT signalComputerVisionResults(d->cachedImageScores.value(path));

        return;
    }

    QImage image = PreviewLoadThread::loadHighQualitySynchronously(path).
                   copyQImage();

    if (image.isNull())
    {
        image.load(path);
    }

    path  = tmpFileName(path);
    image = image.scaled(WIDTH, HEIGHT);
    image.save(path, "JPEG");

    QList<Parameter> parameters;

    DItemInfo info(d->iface->itemInfo(localImage));

    if (info.hasGeolocationInfo())
    {
        static const QString lat = QLatin1String("lat");
        parameters << Parameter(lat, QString::number(info.latitude(), 'f',
                                GEOLOCATION_PRECISION));
        static const QString lng = QLatin1String("lng");
        parameters << Parameter(lng, QString::number(info.longitude(), 'f',
                                GEOLOCATION_PRECISION));
    }

    QDateTime dateTime = asDateTimeLocal(info.dateTime());

    if (dateTime.isValid())
    {
        parameters << Parameter(OBSERVED_ON,
                                dateTime.date().toString(Qt::ISODate));
    }

    parameters << Parameter(LOCALE, locale.name());

    QHttpMultiPart* const multiPart = getMultiPart(parameters,
                                                   QLatin1String("image"),
                                                   QFileInfo(path).fileName(),
                                                   path);

    QUrl url(d->apiUrl + QLatin1String("computervision/score_image"));
    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", d->apiToken.toLatin1());

    QNetworkReply* const reply = d->netMngr->post(netRequest, multiPart);
    multiPart->setParent(reply);
    d->pendingRequests.insert(reply, new ComputerVisionRequest(localImage.toLocalFile(), path));
}

QString INatTalker::tmpFileName(const QString& path)
{
    QString suffix;

    for ( ; ; )
    {
        QString tmpFn = WSToolUtils::makeTemporaryDir(d->serviceName.toLatin1().constData()).
            filePath(QFileInfo(path).baseName().trimmed() + suffix + QLatin1String(".jpg"));

        if (!QFile::exists(tmpFn))
        {
            return tmpFn;
        }

        suffix += QLatin1String("_");
    }
}

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN VerifyCreateObservationRequest : public Request
{
public:

    VerifyCreateObservationRequest(const QByteArray& params,
                                   const INatTalker::PhotoUploadRequest& req,
                                   const QString& observed_on,
                                   int taxon_id, int retries)
      : m_parameters(params),
        m_uploadRequest(req),
        m_observed_on(observed_on),
        m_taxon_id(taxon_id),
        m_retries(retries)
    {
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError code,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "VerifyCreateObservation: " << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        if (networkErrorRetry(code) && (m_retries < MAX_RETRIES))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Attempting to call "
                                                "VerifyCreateObservation again, retry"
                                             << m_retries+1 << "of" << MAX_RETRIES;

            talker.verifyCreateObservation(m_parameters, m_uploadRequest, 1,
                                           m_retries+1);
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "ERROR While Creating Observation"),
                                  errorString);
        }
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        QJsonObject json          = parseJsonResponse(data);
        int         observationId = -1;

        if (
            json.contains(TOTAL_RESULTS) &&
            json.contains(PER_PAGE)      &&
            json.contains(RESULTS)       &&
            json.contains(PAGE)
           )
        {
            int        totalResults = json[TOTAL_RESULTS].toInt();
            int        perPage      = json[PER_PAGE].toInt();
            QJsonArray results      = json[RESULTS].toArray();

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Observation check:" << results.count()
                                             << "results of" << totalResults
                                             << "received in" << durationMilliSecs() << "msecs.";

            for (int i = 0 ; i < results.count() ; ++i)
            {
                QJsonObject result = results[i].toObject();

                if (
                    result.contains(OBSERVED_ON_STRING)                      &&
                    result.contains(TAXON)                                   &&
                    (result[OBSERVED_ON_STRING].toString() == m_observed_on) &&
                    (result[TAXON].toObject().value(ID).toInt()  == m_taxon_id)
                   )
                {
                    observationId = result[ID].toInt();
                    break;
                }
            }

            // Not found. If the server has more results request them.

            if ((observationId < 0) && (results.count() >= perPage))
            {
                talker.verifyCreateObservation(m_parameters, m_uploadRequest,
                                               json[PAGE].toInt()+1, m_retries);
                return;
            }
        }

        if (observationId > 0)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "VerifyCreateObservation: "
                                                "observation" << observationId
                                             << "of taxon_id" << m_taxon_id
                                             << "of" << m_observed_on
                                             << "found; uploading photos.";

            INatTalker::PhotoUploadRequest request(m_uploadRequest);
            request.m_observationId = observationId;

            Q_EMIT talker.signalObservationCreated(request);
        }
        else
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "VerifyCreateObservation: "
                                                "observation of taxon_id"
                                             << m_taxon_id << "at" << m_observed_on
                                             << "not found on server; uploading again.";

            talker.createObservation(m_parameters, m_uploadRequest);
        }
    }

private:

    QByteArray                     m_parameters;
    INatTalker::PhotoUploadRequest m_uploadRequest;
    QString                        m_observed_on;
    int                            m_taxon_id       = 0;
    int                            m_retries        = 0;

private:

    Q_DISABLE_COPY(VerifyCreateObservationRequest)
};

void INatTalker::verifyCreateObservation(const QByteArray& parameters,
                                         const PhotoUploadRequest& photoUpload,
                                         int page, int retries)
{
    QJsonObject json = parseJsonResponse(parameters);

    QUrl url(d->apiUrl + OBSERVATIONS);
    QUrlQuery query;
    query.addQueryItem(QLatin1String("user_login"), photoUpload.m_user);
    query.addQueryItem(QLatin1String("photos"), QLatin1String("false"));
    query.addQueryItem(PER_PAGE, QString::number(200));
    query.addQueryItem(PAGE, QString::number(page));

    int     taxon_id = 0;
    QString observed_on;

    if (json.contains(OBSERVATION))
    {
        QJsonObject params = json[OBSERVATION].toObject();

        if (params.contains(OBSERVED_ON_STRING))
        {
            observed_on = params[OBSERVED_ON_STRING].toString();
            query.addQueryItem(OBSERVED_ON, observed_on.left(10));
        }

        if (params.contains(TAXON_ID))
        {
            taxon_id = params[TAXON_ID].toInt();
            query.addQueryItem(TAXON_ID, QString::number(taxon_id));
        }
    }

    url.setQuery(query.query());

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", d->apiToken.toLatin1());

    PhotoUploadRequest upload(photoUpload);
    upload.m_apiKey = d->apiToken;
    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new VerifyCreateObservationRequest(parameters,
                                                                 upload,
                                                                 observed_on,
                                                                 taxon_id,
                                                                 retries));
}

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN CreateObservationRequest : public Request
{
public:

    CreateObservationRequest(const QByteArray& params,
                             const INatTalker::PhotoUploadRequest& req)
        : m_parameters(params),
          m_uploadRequest(req)
    {
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError code,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Observation not created due to "
                                            "network error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        if (networkErrorRetry(code))
        {
            talker.verifyCreateObservation(m_parameters, m_uploadRequest, 1, 0);
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "ERROR While Creating Observation"),
                                  errorString);
        }
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Observation created in"
                                         << durationMilliSecs() << "msecs.";

        QJsonObject json = parseJsonResponse(data);

        if (json.contains(ID))
        {
            INatTalker::PhotoUploadRequest request(m_uploadRequest);
            request.m_observationId = json[ID].toInt();

            Q_EMIT talker.signalObservationCreated(request);
        }
    }

private:

    QByteArray                     m_parameters;
    INatTalker::PhotoUploadRequest m_uploadRequest;

private:

    Q_DISABLE_COPY(CreateObservationRequest)
};

void INatTalker::createObservation(const QByteArray& parameters,
                                   const PhotoUploadRequest& photoUpload)
{
    QUrl url(d->apiUrl + OBSERVATIONS);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", d->apiToken.toLatin1());

    PhotoUploadRequest upload(photoUpload);
    upload.m_apiKey = d->apiToken;
    d->pendingRequests.insert(d->netMngr->post(netRequest, parameters),
                              new CreateObservationRequest(parameters, upload));
}

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN VerifyUploadPhotoRequest : public Request
{
public:

    VerifyUploadPhotoRequest(const INatTalker::PhotoUploadRequest& req,
                             int retries)
        : m_request(req),
          m_retries(retries)
    {
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError code,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "VerifyPhotoUploadNextPhoto: " << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        if (networkErrorRetry(code) && m_retries < MAX_RETRIES)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Attempting to call "
                                                "VerifyPhotoUploadNextPhoto again, retry"
                                             << m_retries + 1 << "of" << MAX_RETRIES;

            talker.verifyUploadNextPhoto(m_request, m_retries+1);
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "ERROR While Uploading Photo"),
                                  errorString);
        }
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        QJsonObject json = parseJsonResponse(data);

        int noPhotos               =  0;
        int lastObservationPhotoId = -1;
        int lastPhotoId            = -1;

        if (
            json.contains(TOTAL_RESULTS)     &&
            json.contains(RESULTS)           &&
            (json[TOTAL_RESULTS].toInt() == 1)
           )
        {
            QJsonObject result = json[RESULTS].toArray()[0].toObject();

            if (result.contains(OBSERVATION_PHOTOS))
            {
                noPhotos = result[OBSERVATION_PHOTOS].toArray().count();

                if (noPhotos >= 1)
                {
                    QJsonObject obsPhoto   = result[OBSERVATION_PHOTOS].
                                             toArray()[noPhotos-1].toObject();
                    lastObservationPhotoId = obsPhoto[ID].toInt();
                    lastPhotoId            = obsPhoto[PHOTO].toObject().value(ID).toInt();
                }
            }

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "VerifyUploadNextPhoto:" << noPhotos
                                             << "photos on server," << m_request.m_images.count()
                                             << "photos to upload," << m_request.m_totalImages
                                             << "total photos, checked in" << durationMilliSecs() << "msecs.";

            if      ((noPhotos + m_request.m_images.count()) == m_request.m_totalImages)
            {
                talker.uploadNextPhoto(m_request);
            }
            else if ((noPhotos + m_request.m_images.count()) == (m_request.m_totalImages + 1))
            {
                INatTalker::PhotoUploadResult uploadResult(m_request,
                                                           lastObservationPhotoId,
                                                           lastPhotoId);
                Q_EMIT talker.signalPhotoUploaded(uploadResult);
            }
        }
        else
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "VerifyPhotoUploadNextPhoto: "
                                                "observation" << m_request.m_observationId
                                             << "NOT FOUND in" << durationMilliSecs() << "msecs.";
        }
    }

private:

    INatTalker::PhotoUploadRequest m_request;
    int                            m_retries = 0;

private:

    Q_DISABLE_COPY(VerifyUploadPhotoRequest)
};

void INatTalker::verifyUploadNextPhoto(const PhotoUploadRequest& request,
                                       int retries)
{
    QUrl url(d->apiUrl + QLatin1String("observations/") +
             QString::number(request.m_observationId));
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", request.m_apiKey.toLatin1());
    d->pendingRequests.insert(d->netMngr->get(netRequest),
                              new VerifyUploadPhotoRequest(request, retries));
}

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN UploadPhotoRequest : public Request
{
public:

    UploadPhotoRequest(const INatTalker::PhotoUploadRequest& req,
                       const QString& tmpImg)
        : m_request (req),
          m_tmpImage(tmpImg)
    {
    }

    ~UploadPhotoRequest() override
    {
        if (!m_tmpImage.isEmpty() && QFile::exists(m_tmpImage))
        {
            QFile::remove(m_tmpImage);
        }
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError code,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photo not uploaded due to "
                                            "network error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        if (networkErrorRetry(code))
        {
            talker.verifyUploadNextPhoto(m_request, 0);
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "ERROR While Uploading Photo"),
                                  errorString);
        }
    }

    void parseResponse(INatTalker& talker,
                       const QByteArray& data) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photo" << m_request.m_images.front().toLocalFile()
                                         << "to observation" << m_request.m_observationId
                                         << "uploaded in" << durationMilliSecs() << "msecs.";

        static const QString PHOTO_ID = QLatin1String("photo_id");
        QJsonObject json              = parseJsonResponse(data);

        if (json.contains(PHOTO_ID))
        {
            INatTalker::PhotoUploadResult result(m_request, json[ID].toInt(),
                                                 json[PHOTO_ID].toInt());
            Q_EMIT talker.signalPhotoUploaded(result);
        }
    }

private:

    INatTalker::PhotoUploadRequest m_request;
    QString                        m_tmpImage;

private:

    Q_DISABLE_COPY(UploadPhotoRequest)
};

void INatTalker::uploadNextPhoto(const PhotoUploadRequest& request)
{
    QList<Parameter> parameters;
    parameters << Parameter(QLatin1String("observation_photo[observation_id]"),
                            QString::number(request.m_observationId));
    QString tmpImage;
    QString path = request.m_images.front().toLocalFile();

    bool isJpeg = (
                   path.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive) ||
                   path.endsWith(QLatin1String(".jpeg"), Qt::CaseInsensitive)
                  );

    if (request.m_rescale || !isJpeg)
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(path).
                       copyQImage();

        if (image.isNull())
        {
            image.load(path);
        }

        if (!image.isNull())
        {
            tmpImage = tmpFileName(path);

            if (
                (image.width()  > request.m_maxDim) ||
                (image.height() > request.m_maxDim)
               )
            {
                image = image.scaled(request.m_maxDim, request.m_maxDim,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
            }

            image.save(tmpImage, "JPEG", request.m_quality);

            if (!isJpeg)
            {
                path += QLatin1String(".jpeg");
            }
        }
    }

    QHttpMultiPart* const multiPart  = getMultiPart(parameters,
                                                    QLatin1String("file"),
                                                    QFileInfo(path).fileName(),
                                                    tmpImage.isEmpty() ? path
                                                                       : tmpImage);
    QUrl url(d->apiUrl + OBSERVATION_PHOTOS);
    QNetworkRequest netRequest(url);
    netRequest.setRawHeader("Authorization", request.m_apiKey.toLatin1());
    QNetworkReply* const reply = d->netMngr->post(netRequest, multiPart);
    multiPart->setParent(reply);
    d->pendingRequests.insert(reply, new UploadPhotoRequest(request, tmpImage));
}

// ------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DeleteObservationRequest : public Request
{
public:

    DeleteObservationRequest(const QString& apiKey, int id, int retries)
        : m_apiKey       (apiKey),
          m_observationId(id),
          m_retries      (retries)
    {
    }

    void reportError(INatTalker& talker, QNetworkReply::NetworkError code,
                     const QString& errorString) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Delete observation failed with "
                                            "error" << errorString
                                         << "after" << durationMilliSecs() << "msecs.";

        if (networkErrorRetry(code) && m_retries < MAX_RETRIES)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Attempting to delete "
                                                "observation" << m_observationId
                                             << "again, retry" << m_retries + 1
                                             << "of" << MAX_RETRIES;

            talker.deleteObservation(m_observationId, m_apiKey, m_retries + 1);
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "ERROR While Deleting Observation"),
                                  errorString);
        }
    }

    void parseResponse(INatTalker& talker, const QByteArray&) const override
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Observation" << m_observationId
                                         << "deleted in" << durationMilliSecs() << "msecs.";

        Q_EMIT talker.signalObservationDeleted(m_observationId);
    }

private:

    QString m_apiKey;
    int     m_observationId = 0;
    int     m_retries       = 0;

private:

    Q_DISABLE_COPY(DeleteObservationRequest)
};

/**
 * Delete an observation; called when canceling uploads.
 */
void INatTalker::deleteObservation(int id, const QString& apiKey, int retries)
{
    QUrl url(d->apiUrl + QLatin1String("observations/") + QString::number(id));
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String(O2_MIME_TYPE_JSON));
    netRequest.setRawHeader("Authorization", apiKey.toLatin1());
    d->pendingRequests.insert(d->netMngr->deleteResource(netRequest),
                              new DeleteObservationRequest(apiKey, id,
                                                           retries));
}

void INatTalker::cancel()
{
    if (m_authProgressDlg && !m_authProgressDlg->isHidden())
    {
        m_authProgressDlg->hide();
    }

    d->clear();

    Q_EMIT signalBusy(false);
}

void INatTalker::slotFinished(QNetworkReply* reply)
{
    // ignore unexpected response

    if (!d->pendingRequests.contains(reply))
    {
        return;
    }

    Request* const request = d->pendingRequests.take(reply);

    if (reply->error() == QNetworkReply::NoError)
    {
        request->parseResponse(*this, reply->readAll());
    }
    else
    {
        request->reportError(*this, reply->error(), reply->errorString());
    }

    delete request;
    reply->deleteLater();
}

void INatTalker::slotTimeout()
{
    QList<QPair<QNetworkReply*, Request*> > timeoutList;
    QHash<QNetworkReply*, Request*>::const_iterator it;

    for (it = d->pendingRequests.constBegin() ;
         it != d->pendingRequests.constEnd() ; ++it)
    {
        Request* const request = it.value();

        if (request->isTimeout())
        {
            timeoutList.append(qMakePair(it.key(), request));
        }
    }

    for (const auto& pair : timeoutList)
    {
        QNetworkReply* const reply            = pair.first;
        d->pendingRequests.remove(reply);

        QNetworkReply::NetworkError errorCode = reply->error();
        QString errorString                   = reply->errorString();

        reply->abort();
        delete reply;

        if (errorCode == QNetworkReply::NoError)
        {
            errorCode   = QNetworkReply::TimeoutError;
            errorString = i18n("Timeout after exceeding %1 seconds",
                               RESPONSE_TIMEOUT_SECS);
        }

        Request* const request = pair.second;
        request->reportError(*this, errorCode, errorString);
        delete request;
    }
}

} // namespace DigikamGenericINatPlugin

#include "moc_inattalker.cpp"

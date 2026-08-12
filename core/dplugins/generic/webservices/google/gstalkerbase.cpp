/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-21
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gstalkerbase.h"

// Qt includes

#include <QMap>
#include <QMultiMap>
#include <QDateTime>
#include <QDesktopServices>
#include <QOAuthHttpServerReplyHandler>

// Local includes

#include "digikam_debug.h"
#include "networkmanager.h"
#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

static QString s_refreshToken;

class Q_DECL_HIDDEN GSTalkerBase::Private
{
public:

    Private() = default;

public:

    bool                   linked       = false;

    QString                authUrl      = QLatin1String("https://accounts.google.com/o/oauth2/auth");
    QString                tokenUrl     = QLatin1String("https://accounts.google.com/o/oauth2/token");
    QString                identity     = QLatin1String("c3d7cXF2c3xxeXh6YCYoNDQ1Izs9PzU7MzsSFAhVFVNeXQ8a"
                                                        "HQYCHF5FARAQWhQGBwtXHV9eVV9RQEVSSlouLDchKzJpKyYn");
    QString                sharedKey    = QLatin1String("Bg0AFxUeai05JyYVAwU9NzkhJSUdbhUpbzhUTzsqNl80PVA=");

    QNetworkAccessManager* netMngr      = nullptr;
};

GSTalkerBase::GSTalkerBase(QObject* const parent, const QStringList& scope, const QString& serviceName)
    : QObject      (parent),
      m_scope      (scope),
      m_serviceName(serviceName),
      d            (new Private)
{
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);
    m_service  = new QOAuth2AuthorizationCodeFlow(d->netMngr, this);

    m_service->setClientIdentifierSharedKey(WSToolUtils::decodeKey(d->sharedKey));
    m_service->setClientIdentifier(WSToolUtils::decodeKey(d->identity));
    m_service->setContentType(QAbstractOAuth::ContentType::Json);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

    QSet<QByteArray> scopeba;

    for (const QString& scp : std::as_const(m_scope))
    {
        scopeba.insert(scp.toUtf8());
    }

    m_service->setRequestedScopeTokens(scopeba);
    m_service->setTokenUrl(QUrl(d->tokenUrl));

#else

    m_service->setScope(m_scope.join(QLatin1String(" ")));
    m_service->setAccessTokenUrl(QUrl(d->tokenUrl));

#endif

    m_service->setAuthorizationUrl(QUrl(d->authUrl));

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)

    m_service->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QVariantMap* parameters)
        {
            if (stage == QAbstractOAuth::Stage::RequestingAccessToken)
            {
                QByteArray code = parameters->value(QLatin1String("code")).toByteArray();
                (*parameters)[QLatin1String("code")] = QUrl::fromPercentEncoding(code);
            }
        }
    );

#else

    m_service->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters)
        {
            if (stage == QAbstractOAuth::Stage::RequestingAccessToken)
            {
                QByteArray code = parameters->value(QLatin1String("code")).toByteArray();
                (*parameters).replace(QLatin1String("code"), QUrl::fromPercentEncoding(code));
            }
        }
    );

#endif

    QOAuthHttpServerReplyHandler* const replyHandler = new QOAuthHttpServerReplyHandler(8000, this);
    m_service->setReplyHandler(replyHandler);

    // OAuth configuration saved to between dk sessions
/*
    m_service->setRefreshToken(WSToolUtils::readToken(m_serviceName));
*/

    m_service->setRefreshToken(s_refreshToken);

    connect(m_service, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, &GSTalkerBase::slotOpenBrowser);

    connect(m_service, &QOAuth2AuthorizationCodeFlow::tokenChanged,
            this, &GSTalkerBase::slotTokenChanged);

    connect(m_service, &QOAuth2AuthorizationCodeFlow::granted,
            this, &GSTalkerBase::slotLinkingSucceeded);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

    connect(m_service, &QOAuth2AuthorizationCodeFlow::serverReportedErrorOccurred,
            this, &GSTalkerBase::slotLinkingFailed);

#else

    connect(m_service, &QOAuth2AuthorizationCodeFlow::error,
            this, &GSTalkerBase::slotLinkingFailed);

#endif

}

GSTalkerBase::~GSTalkerBase()
{
    if (m_reply)
    {
        m_reply->abort();
    }

    delete d;
}

void GSTalkerBase::link()
{
    Q_EMIT signalBusy(true);
    m_service->grant();
}

void GSTalkerBase::unlink()
{
    Q_EMIT signalBusy(true);

    d->linked = false;

    m_service->setToken(QString());
    m_service->setRefreshToken(QString());

    WSToolUtils::clearToken(m_serviceName);

    m_bearerAccessToken.clear();
    m_accessToken.clear();
}

void GSTalkerBase::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to" << m_serviceName << "fail";

    d->linked = false;

    Q_EMIT signalBusy(false);
    Q_EMIT signalAuthenticationRefused();
}

void GSTalkerBase::slotLinkingSucceeded()
{
    if (m_service->status() == QAbstractOAuth::Status::Granted)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to" << m_serviceName << "ok";

        d->linked = true;

        Q_EMIT signalAccessTokenObtained();
    }
}

void GSTalkerBase::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser... (" << url << ")";

    QDesktopServices::openUrl(url);
}

bool GSTalkerBase::authenticated() const
{
    return d->linked;
}

void GSTalkerBase::slotTokenChanged(const QString& token)
{
    m_accessToken       = token;
    m_bearerAccessToken = QLatin1String("Bearer ") + m_accessToken;
/*
    WSToolUtils::saveToken(m_serviceName, m_service->refreshToken());
*/
    s_refreshToken      = m_service->refreshToken();
}

void GSTalkerBase::doOAuth()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "current time" << QDateTime::currentDateTime();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "expires at: " << m_service->expirationAt();

    if (!m_service->refreshToken().isEmpty())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))

        m_service->refreshTokens();

#else

        m_service->refreshAccessToken();

#endif

    }
    else
    {
        link();
    }
}

} // namespace DigikamGenericGoogleServicesPlugin

#include "moc_gstalkerbase.cpp"

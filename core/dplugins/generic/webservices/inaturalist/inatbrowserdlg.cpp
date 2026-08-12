/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-21
 * Description : a simple web browser dialog based on Qt WebEngine.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inatbrowserdlg.h"
#include "digikam_config.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QToolBar>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QHash>

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "statusprogressbar.h"
#include "searchtextbar.h"
#include "dxmlguiwindow.h"
#include "digikam_debug.h"

using namespace Digikam;

namespace DigikamGenericINatPlugin
{

class Q_DECL_HIDDEN INatBrowserDlg::Private
{
public:

    Private() = default;

public:

    const QUrl                        home          = QUrl(QLatin1String("https://www.inaturalist.org/users/api_token"));
    QWebEngineView*                   browser       = nullptr;
    QString                           username;
    QToolBar*                         toolbar       = nullptr;
    bool                              apiKeyFound   = false;

    QHash<QByteArray, QNetworkCookie> cookies;
};

INatBrowserDlg::INatBrowserDlg(const QString& username,
                               const QList<QNetworkCookie>& cookies,
                               QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(false);
    d->username = username;

    d->browser                             = new QWebEngineView(this);
    d->browser->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    d->browser->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    QWebEngineCookieStore* const cookieJar = d->browser->page()->profile()->cookieStore();
    cookieJar->deleteAllCookies();

    connect(cookieJar, SIGNAL(cookieAdded(QNetworkCookie)),
            this, SLOT(slotCookieAdded(QNetworkCookie)));

    connect(cookieJar, SIGNAL(cookieRemoved(QNetworkCookie)),
            this, SLOT(slotCookieRemoved(QNetworkCookie)));

    QDateTime now(QDateTime::currentDateTime());

    for (const auto& cookie : cookies)
    {
        if (filterCookie(cookie, false, now))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Setting cookie" << cookie;

            cookieJar->setCookie(cookie);
        }
    }

    // --------------------------

    d->toolbar = new QToolBar(this);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Back));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Forward));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Reload));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Stop));

    QAction* const gohome  = new QAction(QIcon::fromTheme(QLatin1String("go-home")),
                                         i18n("Home"), this);
    gohome->setToolTip(i18n("Go back to Home page"));
    d->toolbar->addAction(gohome);

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(layoutSpacing());
    grid->addWidget(d->toolbar, 0, 0, 1, 1);
    grid->addWidget(d->browser, 1, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    setLayout(grid);

    // ----------------------

    connect(d->browser, SIGNAL(titleChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));

    connect(d->browser, SIGNAL(loadFinished(bool)),
            this, SLOT(slotLoadingFinished(bool)));

    connect(gohome, SIGNAL(triggered()),
            this, SLOT(slotGoHome()));

    connect(this, SIGNAL(signalWebText(QString)),
            this, SLOT(slotWebText(QString)));

    // ----------------------

    resize(800, 800);

    slotGoHome();
}

INatBrowserDlg::~INatBrowserDlg()
{
    delete d;
}

void INatBrowserDlg::closeEvent(QCloseEvent* e)
{
    if (!d->apiKeyFound)
    {
        Q_EMIT signalApiToken(QString(), QList<QNetworkCookie>());
    }

    e->accept();
}

void INatBrowserDlg::slotTitleChanged(const QString& title)
{
    setWindowTitle(title);
}

void INatBrowserDlg::slotLoadingFinished(bool status)
{
    QString url = d->browser->url().toString();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Url" << url << "loaded.";

    if      (status && (url == d->home.toString()))
    {
        d->browser->page()->toPlainText(
            [this](const QString& text)
                {
                    Q_EMIT signalWebText(text);
                }
        );
    }
    else if (
             !d->username.isEmpty() &&
             (url == QLatin1String("https://www.inaturalist.org/users/sign_in"))
            )
    {
        QString script = QString(QLatin1String("document.getElementById(\"user_"
                                               "email\").value=\"%1\";")).
                         arg(d->username);

        d->browser->page()->runJavaScript(script);
    }
}

void INatBrowserDlg::slotWebText(const QString& text)
{
    const QString key = QLatin1String("api_token");

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);

    if (
        (err.error == QJsonParseError::NoError) && doc.isObject() &&
        doc.object().contains(key)
       )
    {
        Q_EMIT signalApiToken(doc.object().value(key).toString(),
                              filterCookies(d->cookies.values(), false));

        d->apiKeyFound = true;
        close();
    }
}

void INatBrowserDlg::slotGoHome()
{
    d->browser->setUrl(d->home);
}

static QByteArray cookieKey(const QNetworkCookie& cookie)
{
    static const char sep('\n');

    return (
            cookie.name()            +
            sep                      +
            cookie.domain().toUtf8() +
            sep                      +
            cookie.path().toUtf8()
           );
}

bool INatBrowserDlg::filterCookie(const QNetworkCookie& cookie,
                                  bool keepSessionCookies,
                                  const QDateTime& now)
{
    return cookie.isSessionCookie() ? keepSessionCookies
                                    : (cookie.expirationDate() > now);
}

QList<QNetworkCookie> INatBrowserDlg::filterCookies(const QList<QNetworkCookie>& cookies,
                                                    bool keepSessionCookies)
{
    QList<QNetworkCookie> result;
    QDateTime now(QDateTime::currentDateTime());

    for (const auto& cookie : cookies)
    {
        if (filterCookie(cookie, keepSessionCookies, now))
        {
            result << cookie;
        }
    }

    return result;
}

void INatBrowserDlg::slotCookieAdded(const QNetworkCookie& cookie)
{
    d->cookies.insert(cookieKey(cookie), cookie);
}

void INatBrowserDlg::slotCookieRemoved(const QNetworkCookie& cookie)
{
    d->cookies.remove(cookieKey(cookie));
}

} // namespace DigikamGenericINatPlugin

#include "moc_inatbrowserdlg.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-02
 * Description : embedded web browser for web service authentication
 *
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "digikam_config.h"

// Qt includes

#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QMap>

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>

// Local includes

#include "dwizardpage.h"
#include "dinfointerface.h"
#include "wsauthentication.h"

using namespace Digikam;

namespace DigikamGenericUnifiedPlugin
{

class WSAuthenticationPage : public QWebEnginePage
{
    Q_OBJECT

public:

    explicit WSAuthenticationPage(QObject* const parent, QWebEngineProfile* profile, const QString& callbackUrl);

    virtual ~WSAuthenticationPage() = default;

    void setCallbackUrl(const QString& url);

    bool acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType type, bool isMainFrame);

Q_SIGNALS:

    void callbackCatched(const QString&);

private:

    QString m_callbackUrl;
};

// -------------------------------------------------------------------

class WSAuthenticationPageView : public QWebEngineView
{
    Q_OBJECT

public:

    explicit WSAuthenticationPageView(QWidget* const parent,
                                      WSAuthentication* const wsAuth,
                                      const QString& callbackUrl);
    ~WSAuthenticationPageView() = default;

    bool authenticationComplete() const;

private:

    /*
     * Parse url into a hash map with key and value for each parameter
     */
    QMap<QString, QString> parseUrlFragment(const QString& urlFragment);

Q_SIGNALS:

    void signalAuthenticationComplete(bool);

private Q_SLOTS:

    void slotCallbackCatched(const QString& callbackUrl);
    void slotOpenBrowser(const QUrl& url);
    void slotCloseBrowser();

private:

    WSAuthentication* m_WSAuthentication = nullptr;
};

// -------------------------------------------------------------------

class WSAuthenticationWizard : public DWizardPage
{
    Q_OBJECT

public:

    explicit WSAuthenticationWizard(QWizard* const dialog, const QString& title,
                                    const QString& callback = QLatin1String("http://127.1.1.0:8000/"));     // krazy:exclude=insecurenet
    ~WSAuthenticationWizard();

    bool isComplete() const;

    void initializePage();
    bool validatePage();
    void cleanupPage();

public Q_SLOTS:

    void slotAuthenticationComplete(bool isLinked);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericUnifiedPlugin

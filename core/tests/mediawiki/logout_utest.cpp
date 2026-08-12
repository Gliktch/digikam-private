/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QObject>
#include <QTest>

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_logout.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::Logout;

class Q_DECL_HIDDEN LogoutTest : public QObject
{
    Q_OBJECT

public:

    explicit LogoutTest(QObject* const parent = nullptr)
    {
        Q_UNUSED(parent);
        logoutCount = 0;
        m_mediaWiki = nullptr;
        m_server    = nullptr;
    }

public Q_SLOTS:

    void logoutHandle(KJob* job)
    {
        Q_UNUSED(job)
        logoutCount++;
    }

private Q_SLOTS:

    void initTestCase()
    {
        logoutCount       = 0;
        this->m_mediaWiki = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));      // krazy:exclude=insecurenet
        this->m_server    = new FakeServer;
        this->request     = QStringLiteral("/?format=xml&action=logout");
    }

    void logoutTestConnectTrue()
    {
        QString scenario(QStringLiteral("<api />") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" expires=\"Sat, 12-Feb-2011 21:39:30 GMT\""));
        m_server->setScenario(scenario, cookie);
        m_server->startAndWait();

        logoutCount = 0;
        Logout logout(*m_mediaWiki);

        connect(&logout, SIGNAL(result(KJob*)),
                this, SLOT(logoutHandle(KJob*)));

        logout.exec();   // krazy:exclude=crashy
        QCOMPARE(this->logoutCount, 1);
        QCOMPARE(logout.error(), (int)Logout::NoError);

        QList<FakeServer::Request> requests = m_server->getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request2        = requests[0];
        QCOMPARE(request2.agent, m_mediaWiki->userAgent());
        QCOMPARE(request2.type,  QStringLiteral("GET"));
        QCOMPARE(request2.value, QStringLiteral("/?format=xml&action=logout"));
    }

    void cleanupTestCase()
    {
        delete this->m_mediaWiki;
        delete this->m_server;
    }

private:

    int         logoutCount;
    QString     request;
    Iface*      m_mediaWiki;
    FakeServer* m_server;
};

QTEST_MAIN(LogoutTest)

#include "logout_utest.moc"

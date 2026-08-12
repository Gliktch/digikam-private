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
#include "mediawiki_login.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::Login;

class Q_DECL_HIDDEN LoginTest : public QObject
{
    Q_OBJECT

public:

    explicit LoginTest(QObject* const parent = nullptr)
    {
        Q_UNUSED(parent);
        loginCount = 0;
        m_server   = 0;
    }

public Q_SLOTS:

    void loginHandle(KJob* job)
    {
        Q_UNUSED(job)
        loginCount++;
    }

private Q_SLOTS:

    void initTestCase()
    {
        loginCount     = 0;
        this->m_server = new FakeServer;
    }

    // Failing due to https://bugreports.qt.io/browse/QTBUG-44860
    void loginTestConnectTrue()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Success\" lguserid=\"12345\" lgusername=\"MyUsername\" lgtoken=\"b5780b6e2f27e20b450921d9461010b4\" cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>");
        m_server->addScenario(scenario, cookie);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));
        QCOMPARE(login.error(), (int)Login::NoError);
    }

    void loginTestAttribute()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Success\" lguserid=\"12345\" lgusername=\"MyUsername\" lgtoken=\"b5780b6e2f27e20b450921d9461010b4\" cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /></api>");
        m_server->addScenario(scenario, cookie);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));         // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy

        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));
        QCOMPARE(login.error(), (int)Login::NoError);
    }

    void loginTestConnectTrueWithoutCookie()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario);
        scenario = QStringLiteral("<api><login result=\"Success\" lguserid=\"12345\" lgusername=\"MyUsername\" lgtoken=\"b5780b6e2f27e20b450921d9461010b4\" cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /></api>");
        m_server->addScenario(scenario, cookie);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(login.error(), (int)Login::NoError);
    }

    void loginTestFalseXMLLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Success\" lguserid=\"12345\" lgusername=\"MyUsername\" lgtoken=\"b5780b6e2f27e20b450921d9461010b4\" cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /></api>");
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));
        QCOMPARE(login.error(), (int)Login::XmlError);
    }

    void loginTestNoNameLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NoName\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::LoginMissing);
    }

    void loginTestIllegalLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"Illegal\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::IllegalUsername);
    }

    void loginTestNotExistsLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NotExists\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::UsernameNotExists);
    }

    void loginTestEmptyPassLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"EmptyPass\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QString());

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword="));

        QCOMPARE(login.error(), (int)Login::PasswordMissing);
    }

    void loginTestWrongPassLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"WrongPass\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::WrongPassword);
    }

    void loginTestWrongPluginPassLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"WrongPluginPass\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::WrongPluginPassword);
    }

    void loginTestCreateBlockedLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"CreateBlocked\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::IPAddressBlocked);
    }

    void loginTestThrottledLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"Throttled\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::TooManyConnections);
    }

    void loginTestBlockedLogin()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"Blocked\" info=\"\" /> </api>") );
        m_server->setScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::UserBlocked);
    }

    void loginTestFalseXMLToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Success\" lguserid=\"12345\" lgusername=\"MyUsername\" lgtoken=\"b5780b6e2f27e20b450921d9461010b4\" cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\"</api>");
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));
        QCOMPARE(login.error(), (int)Login::XmlError);
    }

    void loginTestNoNameToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"NoName\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::LoginMissing);
    }

    void loginTestIllegalToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Illegal\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::IllegalUsername);
    }

    void loginTestNotExistsToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"NotExists\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::UsernameNotExists);
    }

    void loginTestEmptyPassToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"EmptyPass\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QString());

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword="));

        QCOMPARE(login.error(), (int)Login::PasswordMissing);
    }

    void loginTestWrongPassToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"WrongPass\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::WrongPassword);
    }

    void loginTestWrongPluginPassToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"WrongPluginPass\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::WrongPluginPassword);
    }

    void loginTestCreateBlockedToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"CreateBlocked\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::IPAddressBlocked);
    }

    void loginTestThrottledToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Throttled\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::TooManyConnections);
    }

    void loginTestBlockedToken()
    {
        loginCount = 0;
        QString scenario(QStringLiteral("<api><login result=\"NeedToken\" token=\"b5780b6e2f27e20b450921d9461010b4\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" /> </api>") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\""));
        m_server->setScenario(scenario, cookie);
        scenario = QStringLiteral("<api><login result=\"Blocked\" info=\"\" /> </api>") ;
        m_server->addScenario(scenario);
        m_server->startAndWait();

        Iface site(QUrl(QStringLiteral("http://127.0.0.1:12566")));     // krazy:exclude=insecurenet
        Login login(site, QStringLiteral("MyUsername"), QStringLiteral("test"));

        connect(&login, SIGNAL(result(KJob*)),
                this, SLOT(loginHandle(KJob*)));

        login.exec();   // krazy:exclude=crashy
        FakeServer::Request serverrequest = m_server->getRequest()[0];
        QCOMPARE(this->loginCount, 1);
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, QStringLiteral("/?format=xml&action=login&lgname=MyUsername&lgpassword=test"));

        QCOMPARE(login.error(), (int)Login::UserBlocked);
    }

    void cleanupTestCase()
    {
        delete this->m_server;
    }

private:

    int         loginCount;
    FakeServer* m_server;
};

QTEST_MAIN(LoginTest)

#include "login_utest.moc"

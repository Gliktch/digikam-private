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
#include "mediawiki_queryinfo.h"
#include "mediawiki_page.h"
#include "mediawiki_protection.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::QueryInfo;
using MediaWiki::Page;
using MediaWiki::Protection;

Q_DECLARE_METATYPE(Page)
Q_DECLARE_METATYPE(Protection)
Q_DECLARE_METATYPE(QueryInfo*)
Q_DECLARE_METATYPE(QVector <Protection>)

class Q_DECL_HIDDEN QueryInfoTest : public QObject
{
    Q_OBJECT

public:

    explicit QueryInfoTest(QObject* const parent = nullptr)
    {
        Q_UNUSED(parent);
        m_queryInfoCount = 0;
        m_mediaWiki      = nullptr;
    }

public Q_SLOTS:

    void queryInfoHandlePages(const Page& page)
    {
        ++m_queryInfoCount;
        m_queryInfoResultsPage = page;
    }

    void queryInfoHandleProtection(const QVector<Protection>& protection)
    {
        ++m_queryInfoCount;
        m_queryInfoResultsProtections = protection;
    }

private Q_SLOTS:

    void initTestCase()
    {
        m_queryInfoCount = 0;
        m_mediaWiki      = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));       // krazy:exclude=insecurenet
    }

    void constructQuery()
    {
        QFETCH(QString, request);
        QFETCH(QueryInfo*, job);

        m_queryInfoCount = 0;
        FakeServer fakeserver;
        fakeserver.startAndWait();

        job->exec();

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request requestServeur  = requests[0];
        QCOMPARE(requestServeur.agent, m_mediaWiki->userAgent());
        QCOMPARE(requestServeur.type,  QStringLiteral("GET"));
        QCOMPARE(requestServeur.value, request);
    }

    void constructQuery_data()
    {
        QTest::addColumn<QString>("request");
        QTest::addColumn<QueryInfo*>("job");

        QueryInfo* const j1 = new QueryInfo(*m_mediaWiki);
        j1->setPageName(QStringLiteral("API"));

        QTest::newRow("Name pages")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7"
                                  "Cwatched%7Csubjectid%7Curl%7Cpreload&intestactions=read&titles=API")
                << j1;

        QueryInfo* const j2 = new QueryInfo(*m_mediaWiki);
        j2->setToken( QStringLiteral("cecded1f35005d22904a35cc7b736e18+\\") );

        QTest::newRow("Token")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7Cwatched%7"
                                  "Csubjectid%7Curl%7Cpreload&intestactions=read&meta=cecded1f35005d22904a35cc7b736e18+%5C")
                << j2;

        QueryInfo* const j3 = new QueryInfo(*m_mediaWiki);
        j3->setPageId(25255);

        QTest::newRow("Page Id")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7"
                                  "Cwatched%7Csubjectid%7Curl%7Cpreload&intestactions=read&pageids=25255")
                << j3;

        QueryInfo* const j4 = new QueryInfo(*m_mediaWiki);
        j4->setRevisionId(44545);

        QTest::newRow("Revision Id")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7"
                                  "Cwatched%7Csubjectid%7Curl%7Cpreload&intestactions=read&revids=44545")
                << j4;
    }

    void parseData()
    {
        QFETCH(QString,scenario);
        QFETCH(Page ,page);
        QFETCH(QVector<Protection> ,protections);

        QueryInfo job(*m_mediaWiki);
        m_queryInfoCount = 0;
        FakeServer fakeserver;
        fakeserver.addScenario(scenario);
        fakeserver.startAndWait();

        connect(&job, SIGNAL(page(Page)),
                this, SLOT(queryInfoHandlePages(Page)));

        connect(&job, SIGNAL(protection(QVector<Protection>)),
                this, SLOT(queryInfoHandleProtection(QVector<Protection>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        QCOMPARE(m_queryInfoCount, 2);
        QCOMPARE(m_queryInfoResultsPage, page);
        QCOMPARE(m_queryInfoResultsProtections, protections);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void parseData_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn< Page >("page");
        QTest::addColumn< QVector<Protection> >("protections");

        Protection pr1;
        pr1.setType(QStringLiteral("edit"));
        pr1.setLevel(QStringLiteral("sysop"));
        pr1.setExpiry(QStringLiteral("infinity"));
        pr1.setSource(QString());

        Protection pr2;
        pr2.setType(QStringLiteral("move"));
        pr2.setLevel(QStringLiteral("sysop"));
        pr2.setExpiry(QStringLiteral("infinity"));
        pr2.setSource(QString());

        Page page;
        page.setPageId(27697087);
        page.setTitle(QStringLiteral("API"));
        page.setNs(0);
        page.setTouched( QDateTime::fromString(QStringLiteral("2010-11-25T13:59:03Z"),
                                               QStringLiteral("yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'")) );
        page.setLastRevId(367741756);
        page.setCounter(0);
        page.setLength(70);
        page.setStarttimestamp(QDateTime::fromString(QStringLiteral("2010-11-25T16:14:51Z"),
                                                     QStringLiteral("yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'")));
        page.setEditToken(QStringLiteral("+\\"));
        page.setTalkid(5477418);
        page.setFullurl(QUrl(QStringLiteral("https://en.wikipedia.org/wiki/API")));
        page.setEditurl(QUrl(QStringLiteral("https://en.wikipedia.org/w/index.php?title=API&action=edit")));
        page.setReadable(QString());
        page.setPreload(QString());

        QTest::newRow("No protection")
                << QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" "
                                  "touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" "
                                  "length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" "
                                  "talkid=\"5477418\" fullurl=\"https://en.wikipedia.org/wiki/API\" "
                                  "editurl=\"https://en.wikipedia.org/w/index.php?title=API&action=edit\" >"
                                  "<protection /></page></pages><tokens csrftoken=\"+\\\" /></query></api>")
                << page
                << QVector<Protection>();

        QTest::newRow("One pages and one protection")
                << QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" "
                                  "touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" "
                                  "length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" "
                                  "talkid=\"5477418\" fullurl=\"https://en.wikipedia.org/wiki/API\" "
                                  "editurl=\"https://en.wikipedia.org/w/index.php?title=API&action=edit\" >"
                                  "<protection><pr type=\"edit\" level=\"sysop\" expiry=\"infinity\" />"
                                  "</protection></page></pages><tokens csrftoken=\"+\\\" /></query></api>")
                << page
                << (QVector<Protection>() << pr1);

        QTest::newRow("One pages and two protection")
                << QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" "
                                  "touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" "
                                  "length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" "
                                  "talkid=\"5477418\" fullurl=\"https://en.wikipedia.org/wiki/API\" "
                                  "editurl=\"https://en.wikipedia.org/w/index.php?title=API&action=edit\" >"
                                  "<protection><pr type=\"edit\" level=\"sysop\" expiry=\"infinity\" />"
                                  "<pr type=\"move\" level=\"sysop\" expiry=\"infinity\" /></protection>"
                                  "</page></pages><tokens csrftoken=\"+\\\" /></query></api>")
                << page
                << (QVector<Protection>() << pr1 << pr2);
    }

    void cleanupTestCase()
    {
        delete m_mediaWiki;
    }

private:

    int                  m_queryInfoCount;
    Page                 m_queryInfoResultsPage;
    QVector <Protection> m_queryInfoResultsProtections;
    Iface*               m_mediaWiki;
};

QTEST_MAIN(QueryInfoTest)

#include "queryinfo_utest.moc"

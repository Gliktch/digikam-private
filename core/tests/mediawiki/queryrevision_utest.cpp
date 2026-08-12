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

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QTest>
#include <QFile>
#include <QTextStream>

// KDE includes

#include <kjob.h>

// Local includes

#include "fakeserver/fakeserver.h"
#include "mediawiki_iface.h"
#include "mediawiki_queryrevision.h"
#include "mediawiki_revision.h"
#include "dtestdatadir.h"
#include "digikam_debug.h"

using MediaWiki::Iface;
using MediaWiki::QueryRevision;
using MediaWiki::Revision;

Q_DECLARE_METATYPE(QList<Revision>)
Q_DECLARE_METATYPE(FakeServer::Request)
Q_DECLARE_METATYPE(QueryRevision*)
Q_DECLARE_METATYPE(QueryRevision::Properties)

Revision constructRevision(int i, int p, int s, bool m,
                           const QString& u, const QDateTime& t,
                           const QString& cm, const QString& ct,
                           const QString& pt, const QString& r)
{
    Revision rev;
    rev.setRevisionId(i);
    rev.setParentId(p);
    rev.setSize(s);
    rev.setMinorRevision(m);
    rev.setUser(u);
    rev.setTimestamp(t);
    rev.setComment(cm);
    rev.setContent(ct);
    rev.setParseTree(pt);
    rev.setRollback(r);

    return rev;
}

QString QStringFromFile(const QString& fileName)
{
    QFile file( fileName );

    if (!file.open( QFile::ReadOnly ))
    {
        qWarning() << "file" << fileName << "cannot be open";

        return QString();
    }

    QTextStream in(&file);
    QString scenario;

    // When loading from files we never have the authentication phase
    // force jumping directly to authenticated state.

    while ( !in.atEnd() )
    {
        scenario.append( in.readLine() );
    }

    file.close();

    return scenario;
}

// ---------------------------------------------------------------------------------

class Q_DECL_HIDDEN QueryRevisionTest : public QObject
{
    Q_OBJECT

public:

    explicit QueryRevisionTest(QObject* const parent = nullptr)
    {
        Q_UNUSED(parent);
        m_revisionCount = 0;
    }

public Q_SLOTS:

    void revisionHandle(const QList<Revision>& revision)
    {
        ++m_revisionCount;
        m_revisionResults = revision;
    }

private Q_SLOTS:

    void init()
    {
        m_revisionCount = 0;
        m_revisionResults.clear();
    }

    void testrvpropall()
    {
        QFETCH(QString, scenario);
        QFETCH(FakeServer::Request, requestTrue);
        QFETCH(QString, title);
        QFETCH(int, error);
        QFETCH(QueryRevision::Properties, rvprop);
        QFETCH(int, size);
        QFETCH(QList<Revision>, results);

        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer fakeserver;
        fakeserver.setScenario(scenario);
        fakeserver.startAndWait();

        QueryRevision* const job = new QueryRevision(MediaWiki);
        job->setProperties(rvprop);
        job->setPageName(title);

        connect(job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job->exec();   // krazy:exclude=crashy;

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request = requests[0];
        QCOMPARE( requestTrue.type, request.type);
        QCOMPARE(job->error(), error);
        QCOMPARE(m_revisionCount, 1);
        QCOMPARE(requestTrue.value, request.value);
        QCOMPARE(m_revisionResults.size(), size);

        QCOMPARE(m_revisionResults, results);

        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testrvpropall_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn<FakeServer::Request>("requestTrue");
        QTest::addColumn<QString>("title");
        QTest::addColumn<int>("error");
        QTest::addColumn<QueryRevision::Properties>("rvprop");
        QTest::addColumn<int>("size");
        QTest::addColumn< QList<Revision> >("results");

        QTest::newRow("All rvprop enable")
            << QStringFromFile(queryRevisionTestBasePath() + QStringLiteral(".rc"))
                << FakeServer::Request(QStringLiteral("GET"),
                                       QString(),
                                       QStringLiteral("/?format=xml&action=query&prop=revisions&rvprop=ids%7Cflags%7Ctimestamp%7Cuser%7Ccomment%7Csize%7Ccontent&titles=API%7CMain%20Page"))
                << QStringLiteral("API|Main%20Page")
                << int(KJob::NoError)
                << QueryRevision::Properties(QueryRevision::Ids | QueryRevision::Flags | QueryRevision::Timestamp | QueryRevision::User | QueryRevision::Comment | QueryRevision::Size | QueryRevision::Content)
                << 2
                << (QList<Revision>()
                        << constructRevision(367741756, 367741564, 70, false, QStringLiteral("Graham87"),
                                                           QDateTime::fromString(QStringLiteral("2010-06-13T08:41:17Z"),
                                                                                 QStringLiteral("yyyy-MM-ddThh:mm:ssZ")),
                                                           QStringLiteral("Protected API: restore protection ([edit=sysop] (indefinite) [move=sysop] (indefinite))"),
                                                           QStringLiteral("#REDIRECT [[Application programming interface]]{{R from abbreviation}}"),QString(),QString())
                        << constructRevision(387545037, 387542946, 5074, false, QStringLiteral("Rich Farmbrough"),
                                                 QDateTime::fromString(QStringLiteral("2010-09-28T15:21:07Z"),
                                                                       QStringLiteral("yyyy-MM-ddThh:mm:ssZ")),
                                                 QStringLiteral("[[Help:Reverting|Reverted]] edits by [[Special:Contributions/Rich Farmbrough|Rich Farmbrough]] ([[User talk:Rich Farmbrough|talk]]) to last version by David Levy"),
                                                 QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_content.rc")),QString(),QString()));

        QTest::newRow("One title")
                << QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_onetitle.rc"))
                << FakeServer::Request(QStringLiteral("GET"),
                                       QString(),
                                       QStringLiteral("/?format=xml&action=query&prop=revisions&rvprop=ids%7Cflags%7Ctimestamp%7Cuser%7Ccomment%7Csize%7Ccontent&titles=API"))
                << QStringLiteral("API")
                << int(KJob::NoError)
                << QueryRevision::Properties(QueryRevision::Ids | QueryRevision::Flags | QueryRevision::Timestamp | QueryRevision::User | QueryRevision::Comment | QueryRevision::Size | QueryRevision::Content)
                << 1
                << (QList<Revision>()
                        << constructRevision(367741756, 367741564, 70, false, QStringLiteral("Graham87"),
                                                 QDateTime::fromString(QStringLiteral("2010-06-13T08:41:17Z"),
                                                                       QStringLiteral("yyyy-MM-ddThh:mm:ssZ")),
                                                 QStringLiteral("Protected API: restore protection ([edit=sysop] (indefinite) [move=sysop] (indefinite))"),
                                                 QStringLiteral("#REDIRECT [[Application programming interface]]{{R from abbreviation}}"),QString(),QString()));

        QTest::newRow("Timestamp only")
                << QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_timestamponly.rc"))
                << FakeServer::Request(QStringLiteral("GET"),
                                       QString(),
                                       QStringLiteral("/?format=xml&action=query&prop=revisions&rvprop=timestamp&titles=API%7CMain%20Page"))
                << QStringLiteral("API|Main%20Page")
                << int(KJob::NoError)
                << QueryRevision::Properties(QueryRevision::Timestamp)
                << 2
                << (QList<Revision>()
                    << constructRevision(-1, -1, -1, false, QString(),
                                             QDateTime::fromString(QStringLiteral("2010-06-13T08:41:17Z"),
                                                                   QStringLiteral("yyyy-MM-ddThh:mm:ssZ")),
                                             QString(),
                                             QString(),QString(),QString())
                    << constructRevision(-1, -1, -1, false, QString(),
                                             QDateTime::fromString(QStringLiteral("2010-09-28T15:21:07Z"),
                                                                   QStringLiteral("yyyy-MM-ddThh:mm:ssZ")),
                                             QString(),
                                             QString(),QString(),QString()));
        QTest::newRow("User only")
                << QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_useronly.rc"))
                << FakeServer::Request(QStringLiteral("GET"),
                                       QString(),
                                       QStringLiteral("/?format=xml&action=query&prop=revisions&rvprop=user&titles=API%7CMain%20Page"))
                << QStringLiteral("API|Main%20Page")
                << int(KJob::NoError)
                << QueryRevision::Properties(QueryRevision::User)
                << 2
                << (QList<Revision>()
                    << constructRevision(-1, -1, -1, false, QStringLiteral("Graham87"),
                                             QDateTime(),
                                             QString(),
                                             QString(),QString(),QString())
                    << constructRevision(-1, -1, -1, false, QStringLiteral("Rich Farmbrough"),
                                             QDateTime(),
                                             QString(),
                                             QString(),QString(),QString()));
    }

    void testerror()
    {
        QFETCH(QString, scenario);
        QFETCH(int, error);

        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer fakeserver;

        if (scenario != QStringLiteral("error serveur"))
        {
            fakeserver.setScenario(scenario);
            fakeserver.startAndWait();
        }

        QueryRevision* const job = new QueryRevision(MediaWiki);
        job->setProperties( QueryRevision::Size | QueryRevision::Content );
        job->setPageName(QStringLiteral("title"));

        connect(job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job->exec();   // krazy:exclude=crashy;

        if (scenario != QStringLiteral("error serveur"))
        {
            QList<FakeServer::Request> requests = fakeserver.getRequest();
            QCOMPARE(requests.size(), 1);
        }

        QCOMPARE(job->error(), error);
        QCOMPARE(m_revisionCount, 0);
        QCOMPARE(m_revisionResults.size(), 0);

        if (scenario != QStringLiteral("error serveur"))
        {
            QVERIFY(fakeserver.isAllScenarioDone());
        }
    }

    void testerror_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn<int>("error");

        QTest::newRow("XML")
                << QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_cuted.rc"))
                << int(QueryRevision::XmlError);

        QTest::newRow("Network")
                << QStringLiteral("error serveur")
                << int(QueryRevision::NetworkError);

        QTest::newRow("Revision Ids")
                << QStringLiteral("<api><error code=\"rvrevids\" info=\"\"/></api>")
                << int(QueryRevision::WrongRevisionId);

        QTest::newRow("Multiple pages")
                << QStringLiteral("<api><error code=\"rvmultpages\" info=\"\"/></api>")
                << int(QueryRevision::MultiPagesNotAllowed);

        QTest::newRow("Access Denied")
                << QStringLiteral("<api><error code=\"rvaccessdenied\" info=\"\"/></api>")
                << int(QueryRevision::TitleAccessDenied);

        QTest::newRow("Add Parameters")
                << QStringLiteral("<api><error code=\"rvbadparams\" info=\"\"/></api>")
                << int(QueryRevision::TooManyParams);

        QTest::newRow("No Such Section")
                << QStringLiteral("<api><error code=\"rvnosuchsection\" info=\"\"/></api>")
                << int(QueryRevision::SectionNotFound);
    }

    void testRvLimit()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));            // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvlimit=2&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setLimit(2);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvStartId()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvstartid=2&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setStartId(2);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvEndId()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvendid=2&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setEndId(2);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvStart()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvstart=2010-09-28T15:21:07Z&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setStartTimestamp(QDateTime::fromString(QStringLiteral("2010-09-28T15:21:07Z"),
                                                    QStringLiteral("yyyy-MM-ddThh:mm:ssZ")));

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvEnd()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvend=2010-09-28T15:21:07Z&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setEndTimestamp(QDateTime::fromString(QStringLiteral("2010-09-28T15:21:07Z"),
                                                  QStringLiteral("yyyy-MM-ddThh:mm:ssZ")));

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvUser()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvuser=martine&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setUser(QStringLiteral("martine"));

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvExcludeUser()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvexcludeuser=martine&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setExcludeUser(QStringLiteral("martine"));

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvDirOlder()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvdir=older&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setDirection(QueryRevision::Older);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvDirNewer()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvdir=newer&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setDirection(QueryRevision::Newer);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testGenerateXML()
    {
        QString scenario = QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_parsetree.rc"));
        FakeServer::Request requestTrue(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvgeneratexml=on&rvprop=timestamp%7Cuser%7Ccomment%7Ccontent&titles=API"));
        QString title = QStringLiteral("API");
        int error     = 0;
        QueryRevision::Properties rvprop = QueryRevision::Timestamp | QueryRevision::User | QueryRevision::Comment | QueryRevision::Content;
        int size      = 2;
        QList<Revision> results;

        Revision rev;
        rev.setUser(QStringLiteral("martine"));
        rev.setTimestamp(QDateTime::fromString(QStringLiteral("2010-06-13T08:41:17Z"),
                         QStringLiteral("yyyy-MM-ddThh:mm:ssZ")));
        rev.setComment(QStringLiteral("Protected API: restore protection ([edit=sysop] (indefinite) [move=sysop] (indefinite))"));
        rev.setContent(QStringLiteral("#REDIRECT [[Application programming interface]]{{R from abbreviation}}"));
        rev.setParseTree(QStringLiteral("<root>#REDIRECT [[Application programming interface]]<template><title>R from abbreviation</title></template></root>"));
        results << rev;
        Revision rev2;
        rev2.setUser(QStringLiteral("Graham87"));
        rev2.setTimestamp(QDateTime::fromString(QStringLiteral("2010-06-13T08:41:17Z"),
                                                QStringLiteral("yyyy-MM-ddThh:mm:ssZ")));
        rev2.setComment(QStringLiteral("Protected API: restore protection ([edit=sysop] (indefinite) [move=sysop] (indefinite))"));
        rev2.setContent(QStringLiteral("#REDIRECT [[Application programming interface]]{{R from abbreviation}}"));
        rev2.setParseTree(QStringLiteral("<root>#REDIRECT [[Application programming interface]]<template><title>R from abbreviation</title></template></root>"));
        results << rev2;
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet

        FakeServer fakeserver;
        fakeserver.setScenario(scenario);
        fakeserver.startAndWait();

        QueryRevision* const job = new QueryRevision(MediaWiki);
        job->setProperties( rvprop );
        job->setPageName(title);
        job->setGenerateXML(true);

        connect(job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job->exec();   // krazy:exclude=crashy;

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request = requests[0];
        QCOMPARE(request.type, requestTrue.type);
        QCOMPARE(m_revisionCount, 1);
        QCOMPARE(request.value, requestTrue.value);

        QCOMPARE(job->error(), error);
        QCOMPARE(m_revisionResults.size(), size);
        QCOMPARE(m_revisionResults, results);

        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvSection()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvsection=1&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setSection(1);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvToken()
    {
        QString scenario = QStringFromFile(queryRevisionTestBasePath() + QStringLiteral("_rollback.rc"));
        FakeServer::Request requestTrue(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvtoken=rollback&titles=API"));
        QString title = QStringLiteral("API");
        int error     = 0;
        int size      = 1;
        QList<Revision> results;
        results << constructRevision(-1, -1, -1, false, QString(),
                                           QDateTime(),
                                           QString(),
                                           QString(),
                                           QString(),
                                           QStringLiteral("094a45ddbbd5e90d55d79d2a23a8c921+\\"));


        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet

        FakeServer fakeserver;
        fakeserver.setScenario(scenario);
        fakeserver.startAndWait();

        QueryRevision* const job = new QueryRevision(MediaWiki);
        job->setPageName(title);

        job->setToken(QueryRevision::Rollback);

        connect(job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job->exec();   // krazy:exclude=crashy;

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request = requests[0];
        QCOMPARE( requestTrue.type, request.type);
        QCOMPARE(m_revisionCount, 1);
        QCOMPARE(requestTrue.value, request.value);

        QCOMPARE(job->error(), error);
        QCOMPARE(m_revisionResults.size(), size);
        QCOMPARE(m_revisionResults, results);

        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvExpandTemplates()
    {
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet
        FakeServer::Request requestSend(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&rvexpandtemplates=on&titles=API"));
        QueryRevision job(MediaWiki);
        job.setPageName(QStringLiteral("API"));
        job.setExpandTemplates(true);

        FakeServer fakeserver;
        fakeserver.startAndWait();

        connect(&job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job.exec();   // krazy:exclude=crashy

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0].value, requestSend.value);
        QCOMPARE(requests[0].type, requestSend.type);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvPageId()
    {
        FakeServer::Request requestTrue(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&pageids=2993&rvprop=timestamp%7Cuser%7Ccomment%7Ccontent"));
        QueryRevision::Properties rvprop = QueryRevision::Timestamp | QueryRevision::User | QueryRevision::Comment | QueryRevision::Content;
        int id                           = 2993;

        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet

        FakeServer fakeserver;
        fakeserver.startAndWait();

        QueryRevision* const job = new QueryRevision(MediaWiki);
        job->setProperties( rvprop );
        job->setPageId(id);

        connect(job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job->exec();   // krazy:exclude=crashy;

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request         = requests[0];
        QCOMPARE( requestTrue.type, request.type);
        QCOMPARE(m_revisionCount, 0);
        QCOMPARE(requestTrue.value, request.value);

        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testRvRevisionId()
    {
        FakeServer::Request requestTrue(QStringLiteral("GET"),
                                        QString(),
                                        QStringLiteral("/?format=xml&action=query&prop=revisions&revids=2993&rvprop=timestamp%7Cuser%7Ccomment%7Ccontent"));
        QueryRevision::Properties rvprop = QueryRevision::Timestamp |QueryRevision::User | QueryRevision::Comment | QueryRevision::Content;
        int id                           = 2993;

        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));        // krazy:exclude=insecurenet

        FakeServer fakeserver;
        fakeserver.startAndWait();

        QueryRevision* const job = new QueryRevision(MediaWiki);
        job->setProperties( rvprop );
        job->setRevisionId(id);

        connect(job, SIGNAL(revision(QList<Revision>)),
                this, SLOT(revisionHandle(QList<Revision>)));

        job->exec();   // krazy:exclude=crashy;

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request = requests[0];
        QCOMPARE(requestTrue.type, request.type);
        QCOMPARE(m_revisionCount, 0);
        QCOMPARE(requestTrue.value, request.value);

        QVERIFY(fakeserver.isAllScenarioDone());
    }

private:

    QString queryRevisionTestBasePath()
    {
        const QString filePath = DTestDataDir::TestData(QString::fromUtf8("core/tests/mediawiki"))
                                   .root().path() + QLatin1Char('/') + QLatin1String("queryrevisiontest");
        qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << filePath;

        return filePath;
    }

private:

    int             m_revisionCount;
    QList<Revision> m_revisionResults;
};

QTEST_MAIN(QueryRevisionTest)

#include "queryrevision_utest.moc"

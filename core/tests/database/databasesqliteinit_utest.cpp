/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : unit test program for digiKam sqlite database init
 *
 * SPDX-FileCopyrightText: 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databasesqliteinit_utest.h"

// Qt includes

#include <QApplication>
#include <QSqlDatabase>
#include <QTimer>
#include <QCommandLineParser>

// KDE includes

#include <kaboutdata.h>

// Local includes

#include "digikam_debug.h"
#include "daboutdata.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "thumbsdbaccess.h"
#include "facedbaccess.h"
#include "dbengineparameters.h"
#include "scancontroller.h"
#include "digikam_version.h"
#include "dtestdatadir.h"
#include "wstoolutils.h"

using namespace Digikam;

QTEST_MAIN(DatabaseSqliteInitTest)

void DatabaseSqliteInitTest::cleanupTestCase()
{
    WSToolUtils::removeTemporaryDir(m_tempPath.toLatin1().data());
}

void DatabaseSqliteInitTest::initTestCase()
{
    m_tempPath  = QString::fromLatin1(QTest::currentAppName());
    m_tempPath.replace(QLatin1String("./"), QString());
    m_tempDir   = WSToolUtils::makeTemporaryDir(m_tempPath.toLatin1().data());
    qCDebug(DIGIKAM_TESTS_LOG) << "Database Dir:" << m_tempDir.path();

    m_filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/database/testimages"))
                           .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << m_filesPath;

    KAboutData aboutData(QLatin1String("digikam"),
                         QLatin1String("digiKam"), // No need i18n here.
                         digiKamVersion());

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(*QCoreApplication::instance());
    aboutData.processCommandLine(&parser);
}

void DatabaseSqliteInitTest::testSqliteInit()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    DbEngineParameters params;
    params.databaseType = DbEngineParameters::SQLiteDatabaseType();
    params.setCoreDatabasePath(m_tempDir.path() + QLatin1String("/digikam-core-test.db"));
    params.setThumbsDatabasePath(m_tempDir.path() + QLatin1String("/digikam-thumbs-test.db"));
    params.setFaceDatabasePath(m_tempDir.path() + QLatin1String("/digikam-faces-test.db"));
    params.setSimilarityDatabasePath(m_tempDir.path() + QLatin1String("/digikam-similarity-test.db"));
    params.legacyAndDefaultChecks();

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    QVERIFY2(AlbumManager::instance()->setDatabase(params, false, m_filesPath),
             "Cannot initialize Sqlite database");

    QTest::qWait(3000);

    qCDebug(DIGIKAM_TESTS_LOG) << "Shutting down SQlite database";
    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();

    qCDebug(DIGIKAM_TESTS_LOG) << "Cleaning Sqlite database";
    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();
}

#include "moc_databasesqliteinit_utest.cpp"

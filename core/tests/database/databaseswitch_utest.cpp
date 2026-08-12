/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : unit test to switch digiKam database from sqlite to mysql
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databaseswitch_utest.h"

// Qt includes

#include <QApplication>
#include <QFileInfo>
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
#include "similaritydbaccess.h"
#include "dbengineparameters.h"
#include "scancontroller.h"
#include "digikam_version.h"
#include "dtestdatadir.h"
#include "wstoolutils.h"
#include "mysqlupgradebinary.h"
#include "mysqlserverbinary.h"
#include "mysqladminbinary.h"
#include "mysqlinitbinary.h"
#include "databaseserverstarter.h"

using namespace Digikam;

QTEST_MAIN(DatabaseSwitchTest)

void DatabaseSwitchTest::cleanupTestCase()
{
    WSToolUtils::removeTemporaryDir(m_db1Path.toLatin1().data());
    WSToolUtils::removeTemporaryDir(m_db2Path.toLatin1().data());
}

void DatabaseSwitchTest::initTestCase()
{
    m_db1Path   = QString::fromLatin1(QTest::currentAppName());
    m_db1Path.replace(QLatin1String("./"), QString());
    m_db1Dir    = WSToolUtils::makeTemporaryDir(m_db1Path.toLatin1().data());
    qCDebug(DIGIKAM_TESTS_LOG) << "Database Dir Source:" << m_db1Dir.path();

    m_db2Path   = QString::fromLatin1(QTest::currentAppName());
    m_db2Path.replace(QLatin1String("./"), QString());
    m_db2Dir    = WSToolUtils::makeTemporaryDir(m_db2Path.toLatin1().data());
    qCDebug(DIGIKAM_TESTS_LOG) << "Database Dir Target:" << m_db2Dir.path();

    m_filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/database/testimages"))
                           .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Images Dir:" << m_filesPath;

    m_sqlitePath = DTestDataDir::TestData(QString::fromUtf8("core/tests/database/testimages-sqlite"))
                           .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Sqlite Dir:" << m_sqlitePath;

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

void DatabaseSwitchTest::startStopSqlite(const QDir& dbDir)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    DbEngineParameters params;
    params.databaseType = DbEngineParameters::SQLiteDatabaseType();
    params.setCoreDatabasePath(dbDir.path() + QLatin1String("/digikam-core-test.db"));
    params.setThumbsDatabasePath(dbDir.path() + QLatin1String("/digikam-thumbs-test.db"));
    params.setFaceDatabasePath(dbDir.path() + QLatin1String("/digikam-faces-test.db"));
    params.setSimilarityDatabasePath(dbDir.path() + QLatin1String("/digikam-similarity-test.db"));
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

void DatabaseSwitchTest::startStopMysql(const QDir& dbDir)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Mysql Database...";

    MysqlUpgradeBinary mysqlUpgradeBin;

    if (!mysqlUpgradeBin.recheckDirectories())
    {
        qWarning() << QLatin1String("Not able to found the Mysql Upgrade binary program. Test is aborted...");
        return;
    }

    MysqlServerBinary  mysqlServerBin;
    mysqlServerBin.slotAddPossibleSearchDirectory(QLatin1String("/usr/sbin"));

    if (!mysqlServerBin.recheckDirectories())
    {
        qWarning() << QLatin1String("Not able to found the Mysql Server binary program. Test is aborted...");
        return;
    }

    MysqlAdminBinary   mysqlAdminBin;

    if (!mysqlAdminBin.recheckDirectories())
    {
        qWarning() << QLatin1String("Not able to found the Mysql Admin binary program. Test is aborted...");
        return;
    }

    MysqlInitBinary    mysqlInitBin;

    if (!mysqlInitBin.recheckDirectories())
    {
        qWarning() << QLatin1String("Not able to found the Mysql Init binary program. Test is aborted...");
        return;
    }

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::MySQLDatabaseType()))
    {
        qWarning() << QLatin1String("Qt MySQL plugin is missing.");
        return;
    }

    DbEngineParameters params;
    QString defaultAkDir                 = DbEngineParameters::serverPrivatePath();
    QString miscDir                      = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_misc"));
    params.databaseType                  = DbEngineParameters::MySQLDatabaseType();
    params.databaseNameCore              = QLatin1String("digikam");
    params.databaseNameThumbnails        = QLatin1String("digikam");
    params.databaseNameFace              = QLatin1String("digikam");
    params.databaseNameSimilarity        = QLatin1String("digikam");
    params.userName                      = QLatin1String("root");
    params.password                      = QString();
    params.internalServer                = true;
    params.internalServerDBPath          = dbDir.path();
    params.internalServerMysqlUpgradeCmd = mysqlUpgradeBin.path();
    params.internalServerMysqlServerCmd  = mysqlServerBin.path();
    params.internalServerMysqlAdminCmd   = mysqlAdminBin.path();
    params.internalServerMysqlInitCmd    = mysqlInitBin.path();
    params.hostName                      = QString();
    params.port                          = -1;
    params.connectOptions                = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing Mysql database...";
    QVERIFY2(AlbumManager::instance()->setDatabase(params, false, m_filesPath),
             "Cannot initialize Mysql database");

    QTest::qWait(3000);

    qCDebug(DIGIKAM_TESTS_LOG) << "Shutting down Mysql database";
    ScanController::instance()->shutDown();

    qCDebug(DIGIKAM_TESTS_LOG) << "Cleaning Mysql database";

    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();
    SimilarityDbAccess::cleanUpDatabase();

    DatabaseServerStarter::instance()->stopServerManagerProcess();
}

void DatabaseSwitchTest::testFromSqliteToMysql()
{
    // TODO: Not implemented.
}

void DatabaseSwitchTest::testFromMysqlToSqlite()
{
    startStopMysql(m_db1Dir);

    QTest::qWait(3000);

    // ---

    qCDebug(DIGIKAM_TESTS_LOG) << "Copy Sqlite database files to a temporary dir...";

    QDir sqliteDir(m_sqlitePath);
    sqliteDir.setFilter(QDir::Files | QDir::NoSymLinks);

    QFileInfoList list = sqliteDir.entryInfoList();

    for (int i = 0 ; i < list.size() ; ++i)
    {
        QString path = m_db2Dir.filePath(list.at(i).fileName().trimmed());

        QFile file(list.at(i).filePath());
        QVERIFY2(file.copy(path),
                 QString::fromLatin1("Cannot copy sqlite file %1 to %2")
                    .arg(file.fileName())
                    .arg(path)
                 .toLatin1().constData());
    }

    // ---

    DbEngineParameters params;
    params.databaseType = DbEngineParameters::SQLiteDatabaseType();
    params.setCoreDatabasePath(m_db2Dir.path() + QLatin1String("/digikam-core-test.db"));
    params.setThumbsDatabasePath(m_db2Dir.path() + QLatin1String("/digikam-thumbs-test.db"));
    params.setFaceDatabasePath(m_db2Dir.path() + QLatin1String("/digikam-faces-test.db"));
    params.setSimilarityDatabasePath(m_db2Dir.path() + QLatin1String("/digikam-similarity-test.db"));
    params.legacyAndDefaultChecks();

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing Sqlite database and switch...";
    AlbumManager::instance()->changeDatabase(params);
    qCDebug(DIGIKAM_TESTS_LOG) << "Database switch done";

    QTest::qWait(3000);

    qCDebug(DIGIKAM_TESTS_LOG) << "Shutting down Sqlite database";
    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();

    qCDebug(DIGIKAM_TESTS_LOG) << "Cleaning Sqlite database";
    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();
    SimilarityDbAccess::cleanUpDatabase();
}

#include "moc_databaseswitch_utest.cpp"

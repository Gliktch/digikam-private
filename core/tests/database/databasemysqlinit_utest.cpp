/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : unit test program for digiKam Mysql database init
 *
 * SPDX-FileCopyrightText: 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databasemysqlinit_utest.h"

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

QTEST_MAIN(DatabaseMysqlInitTest)

void DatabaseMysqlInitTest::cleanupTestCase()
{
    WSToolUtils::removeTemporaryDir(m_tempPath.toLatin1().data());
}

void DatabaseMysqlInitTest::initTestCase()
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

void DatabaseMysqlInitTest::testMysqlInit()
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
    params.internalServerDBPath          = m_tempDir.path();
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
    AlbumManager::instance()->cleanUp();

    qCDebug(DIGIKAM_TESTS_LOG) << "Cleaning Mysql database";
    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();
    SimilarityDbAccess::cleanUpDatabase();

    DatabaseServerStarter::instance()->stopServerManagerProcess();
}

#include "moc_databasemysqlinit_utest.cpp"

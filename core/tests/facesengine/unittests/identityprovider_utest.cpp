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

#include "identityprovider_utest.h"

// Qt includes

#include <QApplication>
#include <QSqlDatabase>
#include <QTimer>
#include <QCommandLineParser>
#include <QMultiMap>
#include <QUuid>

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
#include "identityprovider.h"

using namespace Digikam;

QTEST_MAIN(IdentityProviderTest)

void IdentityProviderTest::initTestCase()
{

    DbEngineParameters params;
    params.databaseType = DbEngineParameters::SQLiteDatabaseType();
    // params.setCoreDatabasePath(m_tempDir.path() + QLatin1String("/digikam-core-test.db"));
    // params.setThumbsDatabasePath(m_tempDir.path() + QLatin1String("/digikam-thumbs-test.db"));
    // params.setFaceDatabasePath(m_tempDir.path() + QLatin1String("/digikam-faces-test.db"));
    params.setFaceDatabasePath(QString::fromUtf8(":memory:"));
    // params.setSimilarityDatabasePath(m_tempDir.path() + QLatin1String("/digikam-similarity-test.db"));
    params.legacyAndDefaultChecks();

    CoreDbAccess::setParameters(params, CoreDbAccess::MainApplication);
}

void IdentityProviderTest::cleanupTestCase()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Cleaning Sqlite database";
    FaceDbAccess::cleanUpDatabase();
}

void IdentityProviderTest::testInit()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    QVERIFY2(IdentityProvider::instance() != nullptr,
             "Cannot initialize IdentityProvider");

}

void IdentityProviderTest::testAddIdentity()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    IdentityProvider* idProvider = IdentityProvider::instance();

    QMultiMap<QString, QString> attributes;
    attributes.insert(QLatin1String("uuid"), QUuid::createUuid().toString());


    QVERIFY2(!idProvider->addIdentity(attributes).isNull(),
             "Cannot add new identity");

}

void IdentityProviderTest::testFindIdentity()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    IdentityProvider* idProvider = IdentityProvider::instance();

    QMultiMap<QString, QString> attributes;
    attributes.insert(QLatin1String("uuid"), QUuid::createUuid().toString());
    idProvider->addIdentity(attributes);

    QVERIFY2(!idProvider->findIdentity(attributes).isNull(),
             "Cannot find identity by uuid");
}

void IdentityProviderTest::testDeleteIdentity()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    IdentityProvider* idProvider = IdentityProvider::instance();

    QMultiMap<QString, QString> attributes;
    attributes.insert(QLatin1String("uuid"), QUuid::createUuid().toString());
    Identity identity = idProvider->addIdentity(attributes);
    idProvider->deleteIdentity(identity);

    QVERIFY2(idProvider->findIdentity(attributes).isNull(),
             "delete identity failed");

}

void IdentityProviderTest::testIdentity()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    IdentityProvider* idProvider = IdentityProvider::instance();

    QMultiMap<QString, QString> attributes;
    attributes.insert(QLatin1String("uuid"), QUuid::createUuid().toString());
    Identity identity = idProvider->addIdentity(attributes);

    QVERIFY2(!idProvider->identity(identity.id()).isNull(),
             "retrieve identity by id failed");

}

void IdentityProviderTest::testAddTraining()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    IdentityProvider* idProvider = IdentityProvider::instance();

    QMultiMap<QString, QString> attributes;
    attributes.insert(QLatin1String("uuid"), QUuid::createUuid().toString());
    Identity identity = idProvider->addIdentity(attributes);
    cv::Mat features(1, 128, CV_32F);

    QVERIFY2(idProvider->addTraining(identity, QString::fromUtf8("hash_add"), features) != -1,
             "add training failed");
}

void IdentityProviderTest::testClearTraining()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Recognition Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        qWarning() << QLatin1String("Qt SQlite plugin is missing.");
        return;
    }

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    IdentityProvider* idProvider = IdentityProvider::instance();

    QMultiMap<QString, QString> attributes;
    attributes.insert(QLatin1String("uuid"), QUuid::createUuid().toString());
    Identity identity = idProvider->addIdentity(attributes);
    cv::Mat features(1, 128, CV_32F);
    idProvider->addTraining(identity, QString::fromUtf8("hash_clear"), features);

    QVERIFY2(idProvider->clearTraining(QString::fromUtf8("hash_clear")),
             "clear training failed");
}

#include "moc_identityprovider_utest.cpp"

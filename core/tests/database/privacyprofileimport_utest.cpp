/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

// Local includes

#include "privacyprofileinspector.h"
#include "privacyprofileimportstager.h"
#include "privacyprofilepreflight.h"
#include "privacyprofilepublication.h"
#include "privacysqlitesnapshot.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;

class PrivacyProfileImportTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void cleanup();
    void testVersionLabels();
    void testStockAndP1Inspection();
    void testAmbiguousSchemaIsRejected();
    void testOnlineBackupCapturesWalState();
    void testCanceledSnapshotIsNotPublished();
    void testStockProfileStagesAsP1();
    void testP1ProfileStagesWithoutConversion();
    void testPreparedPublicationBacksUpAndApplies();
    void testInterruptedPublicationReplays();
    void testIncompletePreparationIsIgnored();
    void testConflictingDisplacedFileBlocksReplay();
    void testCompletedPublicationCanBeRestored();
    void testProtectedStorePreflightPasses();
    void testProtectedStorePreflightFailsOnMissingContainer();
    void testProtectedStorePreflightFailsOnContainerSizeMismatch();
    void testProtectedStorePreflightFailsOnMissingStore();
    void testProtectedStorePreflightFailsOnMissingRootMarker();
    void testProtectedStorePreflightFailsOnProxySizeMismatch();
    void testP1WithProtectedItemsStagesOnlyAfterPreflight();
};

namespace
{

bool createSyntheticDatabase(const QString& path,
                             int schemaVersion,
                             bool p1Identity,
                             bool keepOpen,
                             QString* connectionName)
{
    *connectionName = QLatin1String("privacy-profile-fixture-") +
                      QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"),
                                                      *connectionName);
    database.setDatabaseName(path);

    if (!database.open())
    {
        return false;
    }

    QSqlQuery query(database);
    bool success = query.exec(QLatin1String(
                       "CREATE TABLE Settings (keyword TEXT PRIMARY KEY, value TEXT);")) &&
                   query.exec(QLatin1String(
                       "CREATE TABLE Images (id INTEGER PRIMARY KEY, status INTEGER NOT NULL);")) &&
                   query.exec(QLatin1String(
                       "CREATE TABLE AlbumRoots (id INTEGER PRIMARY KEY, status INTEGER NOT NULL, "
                       "specificPath TEXT);")) &&
                   query.exec(QString::fromUtf8(
                       "INSERT INTO Settings VALUES ('DBVersion', '%1');")
                                      .arg(schemaVersion)) &&
                   query.exec(QLatin1String(
                       "INSERT INTO Images VALUES (1, 1), (2, 3), (3, 1);")) &&
                   query.exec(QLatin1String(
                       "INSERT INTO AlbumRoots VALUES (1, 0, '/synthetic/photos');"));

    if (success && p1Identity)
    {
        success = query.exec(QLatin1String(
                      "INSERT INTO Settings VALUES "
                      "('DBSchemaFlavor', 'digikam-private'), "
                      "('DBSchemaFlavorVersion', '1'), "
                      "('DBSchemaBaseVersion', '17');")) &&
                  query.exec(QLatin1String(
                      "CREATE TABLE PrivacyItems (imageId INTEGER PRIMARY KEY);")) &&
                  query.exec(QLatin1String(
                      "CREATE TABLE PrivacyCategories (uuid TEXT PRIMARY KEY);")) &&
                  query.exec(QLatin1String(
                      "CREATE TABLE PrivacyStorageRoots "
                      "(uuid TEXT PRIMARY KEY, configuredPath TEXT, markerUuid TEXT);")) &&
                  query.exec(QLatin1String(
                      "CREATE TABLE PrivacyStores "
                      "(uuid TEXT PRIMARY KEY, rootUuid TEXT, cipherRelativePath TEXT, "
                      "configRelativePath TEXT);")) &&
                  query.exec(QLatin1String(
                      "CREATE TABLE PrivacyContainers "
                      "(uuid TEXT PRIMARY KEY, rootUuid TEXT, storeUuid TEXT, "
                      "objectRelativePath TEXT, protectedSize INTEGER);")) &&
                  query.exec(QLatin1String(
                      "CREATE TABLE PrivacyAssets "
                      "(itemUuid TEXT, publicRootUuid TEXT, publicRelativePath TEXT, "
                      "proxySize INTEGER);"));

        const QStringList remainingTables = {
            QLatin1String("PrivacyCredentials"),
            QLatin1String("PrivacyStoreBindings"),
            QLatin1String("PrivacyDerivatives"),
            QLatin1String("PrivacyTransactions"),
            QLatin1String("PrivacyTransactionJournals")
        };

        for (const QString& table : remainingTables)
        {
            success = success && query.exec(QString::fromUtf8(
                "CREATE TABLE %1 (id INTEGER PRIMARY KEY);").arg(table));
        }

        success = success &&
                  query.exec(QLatin1String("INSERT INTO PrivacyItems VALUES (1);")) &&
                  query.exec(QLatin1String(
                      "INSERT INTO PrivacyCategories VALUES ('synthetic-category');"));
    }

    if (!keepOpen)
    {
        database.close();
        database = QSqlDatabase();
        QSqlDatabase::removeDatabase(*connectionName);
        connectionName->clear();
    }

    return success;
}

bool createFullP1Database(const QString& path)
{
    CoreDbAccess::cleanUpDatabase();
    DbEngineParameters parameters = DbEngineParameters::parametersForSQLite(path);
    CoreDbAccess::setParameters(parameters);
    const bool success = CoreDbAccess::checkReadyForUse();
    CoreDbAccess::cleanUpDatabase();
    return success;
}

bool downgradeFixtureToStock17(const QString& path)
{
    const QString connectionName = QLatin1String("privacy-stock-fixture-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool success = false;

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        database.setDatabaseName(path);

        if (database.open())
        {
            QSqlQuery query(database);
            success = query.exec(QLatin1String("PRAGMA foreign_keys=OFF;")) &&
                      query.exec(QLatin1String(
                          "UPDATE Settings SET value='17' WHERE keyword='DBVersion';")) &&
                      query.exec(QLatin1String(
                          "UPDATE Settings SET value='17' WHERE keyword='DBVersionRequired';")) &&
                      query.exec(QLatin1String(
                          "DELETE FROM Settings WHERE keyword IN "
                          "('DBSchemaFlavor', 'DBSchemaFlavorVersion', 'DBSchemaBaseVersion');"));
            const QStringList privacyTables = {
                QLatin1String("PrivacyTransactionJournals"),
                QLatin1String("PrivacyTransactions"),
                QLatin1String("PrivacyDerivatives"),
                QLatin1String("PrivacyAssets"),
                QLatin1String("PrivacyContainers"),
                QLatin1String("PrivacyItems"),
                QLatin1String("PrivacyStoreBindings"),
                QLatin1String("PrivacyStores"),
                QLatin1String("PrivacyStorageRoots"),
                QLatin1String("PrivacyCredentials"),
                QLatin1String("PrivacyCategories")
            };

            for (const QString& table : privacyTables)
            {
                success = success && query.exec(
                    QString::fromUtf8("DROP TABLE %1;").arg(table));
            }

            database.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

bool setDatabaseMarker(const QString& path, const QString& value)
{
    const QString connectionName = QLatin1String("privacy-marker-write-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool success = false;

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        database.setDatabaseName(path);

        if (database.open())
        {
            QSqlQuery query(database);
            query.prepare(QLatin1String(
                "INSERT OR REPLACE INTO Settings (keyword, value) VALUES ('FixtureMarker', ?);"));
            query.addBindValue(value);
            success = query.exec();
            database.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

QString databaseMarker(const QString& path)
{
    const QString connectionName = QLatin1String("privacy-marker-read-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString value;

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        database.setConnectOptions(QLatin1String("QSQLITE_OPEN_READONLY"));
        database.setDatabaseName(path);

        if (database.open())
        {
            QSqlQuery query(database);

            if (query.exec(QLatin1String(
                    "SELECT value FROM Settings WHERE keyword='FixtureMarker';")) &&
                query.next())
            {
                value = query.value(0).toString();
            }

            database.close();
        }
    }

    QSqlDatabase::removeDatabase(connectionName);
    return value;
}

bool writeTextFile(const QString& path, const QByteArray& contents)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           (file.write(contents) == contents.size());
}

struct ProtectedFixture
{
    QString databasePath;
    QString publicRoot;
    QString managedRoot;
    QString containerPublic;
    QString containerStore;
    QString proxyPath;
    QString storeConfigPath;
    QString markerPublic;
    QString markerManaged;
};

bool buildProtectedFixture(const QString& directory, ProtectedFixture* const fixture)
{
    ProtectedFixture f;
    f.publicRoot = QDir(directory).filePath(QLatin1String("public"));
    f.managedRoot = QDir(directory).filePath(QLatin1String("managed"));
    f.databasePath = QDir(directory).filePath(QLatin1String("protected-p1.db"));
    f.containerPublic = QDir(f.publicRoot).filePath(
        QLatin1String("photo.jpg.digikam-private.zip"));
    f.containerStore = QDir(f.managedRoot).filePath(
        QLatin1String(".digikam-private/stores/store-1/objects/cipher.bin"));
    f.proxyPath = QDir(f.publicRoot).filePath(QLatin1String("photo.jpg"));
    f.storeConfigPath = QDir(f.managedRoot).filePath(
        QLatin1String(".digikam-private/stores/store-1/gocryptfs.conf"));
    f.markerPublic = QDir(f.publicRoot).filePath(
        QLatin1String(".digikam-private/root-marker-v1.json"));
    f.markerManaged = QDir(f.managedRoot).filePath(
        QLatin1String(".digikam-private/root-marker-v1.json"));

    QString connectionName;

    if (!createSyntheticDatabase(f.databasePath, 101, true, false, &connectionName))
    {
        return false;
    }

    const QString insertConnection = QLatin1String("privacy-preflight-insert-") +
                                     QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool success = false;

    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"),
                                                          insertConnection);
        database.setDatabaseName(f.databasePath);

        if (!database.open())
        {
            return false;
        }

        QSqlQuery query(database);
        query.prepare(QLatin1String(
            "INSERT INTO PrivacyStorageRoots VALUES (?, ?, ?);"));
        query.addBindValue(QLatin1String("root-public"));
        query.addBindValue(f.publicRoot);
        query.addBindValue(QLatin1String("marker-public"));
        success = query.exec();

        if (success)
        {
            query.prepare(QLatin1String(
                "INSERT INTO PrivacyStorageRoots VALUES (?, ?, ?);"));
            query.addBindValue(QLatin1String("root-managed"));
            query.addBindValue(f.managedRoot);
            query.addBindValue(QLatin1String("marker-managed"));
            success = query.exec();
        }

        if (success)
        {
            query.prepare(QLatin1String(
                "INSERT INTO PrivacyStores VALUES (?, ?, ?, ?);"));
            query.addBindValue(QLatin1String("store-1"));
            query.addBindValue(QLatin1String("root-managed"));
            query.addBindValue(QLatin1String(".digikam-private/stores/store-1"));
            query.addBindValue(QLatin1String("gocryptfs.conf"));
            success = query.exec();
        }

        if (success)
        {
            query.prepare(QLatin1String(
                "INSERT INTO PrivacyContainers VALUES (?, ?, ?, ?, ?);"));
            query.addBindValue(QLatin1String("container-public"));
            query.addBindValue(QLatin1String("root-public"));
            query.addBindValue(QVariant());
            query.addBindValue(QLatin1String("photo.jpg.digikam-private.zip"));
            query.addBindValue(23);
            success = query.exec();
        }

        if (success)
        {
            query.prepare(QLatin1String(
                "INSERT INTO PrivacyContainers VALUES (?, ?, ?, ?, ?);"));
            query.addBindValue(QLatin1String("container-store"));
            query.addBindValue(QVariant());
            query.addBindValue(QLatin1String("store-1"));
            query.addBindValue(QLatin1String("objects/cipher.bin"));
            query.addBindValue(31);
            success = query.exec();
        }

        if (success)
        {
            query.prepare(QLatin1String(
                "INSERT INTO PrivacyAssets VALUES (?, ?, ?, ?);"));
            query.addBindValue(QLatin1String("synthetic-item"));
            query.addBindValue(QLatin1String("root-public"));
            query.addBindValue(QLatin1String("photo.jpg"));
            query.addBindValue(19);
            success = query.exec();
        }

        database.close();
    }

    QSqlDatabase::removeDatabase(insertConnection);

    if (!success)
    {
        return false;
    }

    const QByteArray markerPublic(
        "{\"kind\":\"digikam-private-root-marker-v1\",\"markerUuid\":\"marker-public\"}");
    const QByteArray markerManaged(
        "{\"kind\":\"digikam-private-root-marker-v1\",\"markerUuid\":\"marker-managed\"}");
    success = writeTextFile(f.markerPublic, markerPublic) &&
              writeTextFile(f.markerManaged, markerManaged) &&
              writeTextFile(f.storeConfigPath, QByteArray("config")) &&
              writeTextFile(f.containerPublic, QByteArray(23, 'a')) &&
              writeTextFile(f.containerStore, QByteArray(31, 'b')) &&
              writeTextFile(f.proxyPath, QByteArray(19, 'c'));

    if (success)
    {
        *fixture = f;
    }

    return success;
}

PrivacyProfilePaths fixtureProfilePaths(const QString& root)
{
    PrivacyProfilePaths paths;
    paths.configHome = QDir(root).filePath(QLatin1String("config"));
    paths.dataHome = QDir(root).filePath(QLatin1String("data"));
    paths.cacheHome = QDir(root).filePath(QLatin1String("cache"));
    paths.stateHome = QDir(root).filePath(QLatin1String("state"));
    paths.transactionHome = QDir(root).filePath(QLatin1String("transactions"));
    paths.configFilePath = QDir(paths.configHome).filePath(QLatin1String("digikamrc"));
    paths.coreDatabasePath = QDir(paths.dataHome).filePath(QLatin1String("digikam4.db"));
    paths.thumbnailDatabasePath = QDir(paths.dataHome).filePath(
        QLatin1String("thumbnails-digikam.db"));
    return paths;
}

PrivacyProfileImportStageResult fixtureStage(const QString& candidatePath)
{
    PrivacyProfileImportStageResult staged;
    staged.success = true;
    staged.candidateCoreDatabasePath = candidatePath;
    staged.candidateSummary = PrivacyProfileInspector::inspectCoreDatabase(candidatePath);
    return staged;
}

} // namespace

void PrivacyProfileImportTest::cleanup()
{
    CoreDbAccess::cleanUpDatabase();
}

void PrivacyProfileImportTest::testVersionLabels()
{
    QCOMPARE(PrivacyProfileInspector::versionLabel(17), QLatin1String("digiKam 9.1"));
    QCOMPARE(PrivacyProfileInspector::versionLabel(16), QLatin1String("digiKam 9.0"));
    QCOMPARE(PrivacyProfileInspector::versionLabel(15), QLatin1String("digiKam 7.10"));
    QCOMPARE(PrivacyProfileInspector::versionLabel(14), QLatin1String("digiKam <7.5"));
    QCOMPARE(PrivacyProfileInspector::versionLabel(101), QLatin1String("digiKam Private P1"));
}

void PrivacyProfileImportTest::testStockAndP1Inspection()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QString connectionName;
    const QString stockPath = directory.filePath(QLatin1String("stock.db"));
    QVERIFY(createSyntheticDatabase(stockPath, 17, false, false, &connectionName));

    PrivacyProfileSummary stock = PrivacyProfileInspector::inspectCoreDatabase(stockPath);
    QCOMPARE(stock.schemaKind, PrivacyProfileSchemaKind::Stock);
    QCOMPARE(stock.versionLabel, QLatin1String("digiKam 9.1"));
    QCOMPARE(stock.activeItemCount, 2LL);
    QCOMPARE(stock.totalItemCount, 3LL);
    QCOMPARE(stock.collectionRoots, QStringList { QLatin1String("/synthetic/photos") });
    QVERIFY(stock.integrityOk);
    QVERIFY(stock.isUsable());

    const QString privatePath = directory.filePath(QLatin1String("private.db"));
    QVERIFY(createSyntheticDatabase(privatePath, 101, true, false, &connectionName));
    PrivacyProfileSummary privateProfile =
        PrivacyProfileInspector::inspectCoreDatabase(privatePath);
    QCOMPARE(privateProfile.schemaKind, PrivacyProfileSchemaKind::PrivateP1);
    QCOMPARE(privateProfile.protectedItemCount, 1LL);
    QCOMPARE(privateProfile.privacyCategoryCount, 1);
    QVERIFY(privateProfile.isPrivateProfile());
}

void PrivacyProfileImportTest::testAmbiguousSchemaIsRejected()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QString connectionName;
    const QString path = directory.filePath(QLatin1String("ambiguous.db"));
    QVERIFY(createSyntheticDatabase(path, 101, false, false, &connectionName));

    const PrivacyProfileSummary result =
        PrivacyProfileInspector::inspectCoreDatabase(path);
    QCOMPARE(result.schemaKind, PrivacyProfileSchemaKind::Unsupported);
    QVERIFY(!result.isUsable());
    QVERIFY(!result.error.isEmpty());
}

void PrivacyProfileImportTest::testOnlineBackupCapturesWalState()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QString connectionName;
    const QString sourcePath = directory.filePath(QLatin1String("wal-source.db"));
    const QString snapshotPath = directory.filePath(QLatin1String("snapshot.db"));
    QVERIFY(createSyntheticDatabase(sourcePath, 17, false, true, &connectionName));

    QSqlDatabase source = QSqlDatabase::database(connectionName);
    QSqlQuery query(source);
    QVERIFY(query.exec(QLatin1String("PRAGMA journal_mode=WAL;")));
    QVERIFY(query.exec(QLatin1String("PRAGMA wal_autocheckpoint=0;")));
    QVERIFY(query.exec(QLatin1String("INSERT INTO Images VALUES (4, 1);")));
    QVERIFY(QFileInfo::exists(sourcePath + QLatin1String("-wal")));

    int lastCopied = 0;
    const PrivacySqliteSnapshotResult result = PrivacySqliteSnapshot::create(
        sourcePath, snapshotPath,
        [&lastCopied](int copied, int)
        {
            lastCopied = copied;
        });
    QVERIFY2(result.success, qPrintable(result.error));
    QVERIFY(QFileInfo::exists(snapshotPath));
    QVERIFY(lastCopied > 0);

    const PrivacyProfileSummary snapshot =
        PrivacyProfileInspector::inspectCoreDatabase(snapshotPath);
    QVERIFY(snapshot.isUsable());
    QCOMPARE(snapshot.totalItemCount, 4LL);

    source.close();
    source = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

void PrivacyProfileImportTest::testCanceledSnapshotIsNotPublished()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QString connectionName;
    const QString sourcePath = directory.filePath(QLatin1String("source.db"));
    const QString snapshotPath = directory.filePath(QLatin1String("canceled.db"));
    QVERIFY(createSyntheticDatabase(sourcePath, 17, false, false, &connectionName));

    const PrivacySqliteSnapshotResult result = PrivacySqliteSnapshot::create(
        sourcePath, snapshotPath, PrivacySqliteSnapshot::Progress(),
        []()
        {
            return true;
        });
    QVERIFY(!result.success);
    QVERIFY(result.canceled);
    QVERIFY(!QFileInfo::exists(snapshotPath));
    const QStringList partials = QDir(directory.path()).entryList(
        QStringList { QLatin1String("canceled.db.partial-*") }, QDir::Files);
    QVERIFY(partials.isEmpty());
}

void PrivacyProfileImportTest::testStockProfileStagesAsP1()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = directory.filePath(QLatin1String("stock17.db"));
    QVERIFY(createFullP1Database(sourcePath));
    QVERIFY(downgradeFixtureToStock17(sourcePath));
    const PrivacyProfileSummary source =
        PrivacyProfileInspector::inspectCoreDatabase(sourcePath);
    QCOMPARE(source.schemaKind, PrivacyProfileSchemaKind::Stock);

    const PrivacyProfileImportStageResult staged = PrivacyProfileImportStager::stage(
        source, directory.filePath(QLatin1String("stock-stage")));
    QVERIFY2(staged.success, qPrintable(staged.error));
    QVERIFY(staged.candidateSummary.isPrivateProfile());
    QCOMPARE(staged.candidateSummary.schemaVersion, 101);
    QVERIFY(QFileInfo::exists(staged.candidateCoreDatabasePath));
}

void PrivacyProfileImportTest::testP1ProfileStagesWithoutConversion()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString sourcePath = directory.filePath(QLatin1String("source-p1.db"));
    QVERIFY(createFullP1Database(sourcePath));
    const PrivacyProfileSummary source =
        PrivacyProfileInspector::inspectCoreDatabase(sourcePath);
    QVERIFY(source.isPrivateProfile());

    const PrivacyProfileImportStageResult staged = PrivacyProfileImportStager::stage(
        source, directory.filePath(QLatin1String("p1-stage")));
    QVERIFY2(staged.success, qPrintable(staged.error));
    QVERIFY(staged.candidateSummary.isPrivateProfile());
    QCOMPARE(staged.candidateSummary.totalItemCount, source.totalItemCount);
    QVERIFY(QFileInfo::exists(staged.candidateCoreDatabasePath));
}

void PrivacyProfileImportTest::testPreparedPublicationBacksUpAndApplies()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const PrivacyProfilePaths paths = fixtureProfilePaths(directory.path());
    QVERIFY(paths.isValid());
    QVERIFY(QDir().mkpath(paths.configHome));
    QVERIFY(QDir().mkpath(paths.dataHome));
    QVERIFY(QDir().mkpath(paths.cacheHome));
    QVERIFY(QDir().mkpath(paths.stateHome));
    QVERIFY(createFullP1Database(paths.coreDatabasePath));
    QVERIFY(setDatabaseMarker(paths.coreDatabasePath, QLatin1String("target")));
    QVERIFY(writeTextFile(paths.configFilePath,
                          QByteArray("[General Settings]\nVersion=target\n")));
    QVERIFY(writeTextFile(QDir(paths.cacheHome).filePath(QLatin1String("retained.cache")),
                          QByteArray("cache")));
    QVERIFY(writeTextFile(QDir(paths.stateHome).filePath(QLatin1String("retained.state")),
                          QByteArray("state")));
    QVERIFY(writeTextFile(QDir(paths.dataHome).filePath(QLatin1String("recognition.db")),
                          QByteArray("recognition")));

    const QString candidatePath = directory.filePath(QLatin1String("candidate.db"));
    QVERIFY(createFullP1Database(candidatePath));
    QVERIFY(setDatabaseMarker(candidatePath, QLatin1String("candidate")));
    const QString sourceConfig = directory.filePath(QLatin1String("source-digikamrc"));
    QVERIFY(writeTextFile(sourceConfig,
                          QByteArray("[General Settings]\nVersion=source\n")));
    const PrivacyProfilePublicationResult prepared = PrivacyProfilePublication::prepare(
        fixtureStage(candidatePath), paths, sourceConfig);
    QVERIFY2(prepared.success, qPrintable(prepared.error));
    QVERIFY(QFileInfo::exists(QDir(prepared.backupDirectory).filePath(
        QLatin1String("cache/retained.cache"))));
    const QString backupCore = QDir(prepared.backupDirectory).filePath(
        QLatin1String("data/digikam4.db"));
    QCOMPARE(databaseMarker(backupCore), QLatin1String("target"));

    const PrivacyProfilePublicationResult applied =
        PrivacyProfilePublication::applyPending(paths.transactionHome);
    QVERIFY2(applied.success, qPrintable(applied.error));
    QCOMPARE(databaseMarker(paths.coreDatabasePath), QLatin1String("candidate"));
    QVERIFY(!QFileInfo::exists(QDir(paths.dataHome).filePath(
        QLatin1String("recognition.db"))));
    QVERIFY(QFileInfo::exists(QDir(applied.transactionDirectory).filePath(
        QLatin1String("displaced/recognition.db"))));

    QFile config(paths.configFilePath);
    QVERIFY(config.open(QIODevice::ReadOnly));
    const QByteArray configData = config.readAll();
    QVERIFY(configData.contains("Version=source"));
    QVERIFY(configData.contains("Database Type=QSQLITE"));

    const PrivacyProfilePublicationResult replay =
        PrivacyProfilePublication::applyPending(paths.transactionHome);
    QVERIFY2(replay.success, qPrintable(replay.error));
}

void PrivacyProfileImportTest::testInterruptedPublicationReplays()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const PrivacyProfilePaths paths = fixtureProfilePaths(directory.path());
    QVERIFY(QDir().mkpath(paths.configHome));
    QVERIFY(QDir().mkpath(paths.dataHome));
    QVERIFY(QDir().mkpath(paths.cacheHome));
    QVERIFY(QDir().mkpath(paths.stateHome));
    QVERIFY(createFullP1Database(paths.coreDatabasePath));
    QVERIFY(setDatabaseMarker(paths.coreDatabasePath, QLatin1String("target")));
    QVERIFY(writeTextFile(paths.configFilePath, QByteArray("[General Settings]\n")));
    const QString candidatePath = directory.filePath(QLatin1String("candidate.db"));
    QVERIFY(createFullP1Database(candidatePath));
    QVERIFY(setDatabaseMarker(candidatePath, QLatin1String("candidate")));

    const PrivacyProfilePublicationResult prepared = PrivacyProfilePublication::prepare(
        fixtureStage(candidatePath), paths);
    QVERIFY2(prepared.success, qPrintable(prepared.error));
    const QString displaced = QDir(prepared.transactionDirectory).filePath(
        QLatin1String("displaced"));
    QVERIFY(QDir().mkpath(displaced));
    QVERIFY(QFile::rename(paths.coreDatabasePath,
                          QDir(displaced).filePath(QLatin1String("digikam4.db"))));
    const QString transactionCandidate = QDir(prepared.transactionDirectory).filePath(
        QLatin1String("candidate/digikam4.db"));
    QVERIFY(QFile::copy(transactionCandidate,
                        paths.coreDatabasePath +
                        QLatin1String(".digikam-private-import-partial")));

    const PrivacyProfilePublicationResult replayed =
        PrivacyProfilePublication::applyPending(paths.transactionHome);
    QVERIFY2(replayed.success, qPrintable(replayed.error));
    QCOMPARE(databaseMarker(paths.coreDatabasePath), QLatin1String("candidate"));
}

void PrivacyProfileImportTest::testIncompletePreparationIsIgnored()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const PrivacyProfilePaths paths = fixtureProfilePaths(directory.path());
    const QString preparing = QDir(paths.transactionHome).filePath(
        QLatin1String("preparing-incomplete"));
    QVERIFY(QDir().mkpath(preparing));
    QVERIFY(writeTextFile(QDir(preparing).filePath(QLatin1String("partial-data")),
                          QByteArray("incomplete")));

    const PrivacyProfilePublicationResult result =
        PrivacyProfilePublication::applyPending(paths.transactionHome);
    QVERIFY2(result.success, qPrintable(result.error));
}

void PrivacyProfileImportTest::testConflictingDisplacedFileBlocksReplay()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const PrivacyProfilePaths paths = fixtureProfilePaths(directory.path());
    QVERIFY(QDir().mkpath(paths.configHome));
    QVERIFY(QDir().mkpath(paths.dataHome));
    QVERIFY(createFullP1Database(paths.coreDatabasePath));
    QVERIFY(setDatabaseMarker(paths.coreDatabasePath, QLatin1String("target")));
    const QString candidatePath = directory.filePath(QLatin1String("candidate.db"));
    QVERIFY(createFullP1Database(candidatePath));
    QVERIFY(setDatabaseMarker(candidatePath, QLatin1String("candidate")));

    const PrivacyProfilePublicationResult prepared = PrivacyProfilePublication::prepare(
        fixtureStage(candidatePath), paths);
    QVERIFY2(prepared.success, qPrintable(prepared.error));
    const QString displaced = QDir(prepared.transactionDirectory).filePath(
        QLatin1String("displaced/digikam4.db"));
    QVERIFY(writeTextFile(displaced, QByteArray("conflict")));

    const PrivacyProfilePublicationResult result =
        PrivacyProfilePublication::applyPending(paths.transactionHome);
    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QLatin1String("conflicting displaced")));
    QCOMPARE(databaseMarker(paths.coreDatabasePath), QLatin1String("target"));
}

void PrivacyProfileImportTest::testCompletedPublicationCanBeRestored()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const PrivacyProfilePaths paths = fixtureProfilePaths(directory.path());
    QVERIFY(QDir().mkpath(paths.configHome));
    QVERIFY(QDir().mkpath(paths.dataHome));
    QVERIFY(createFullP1Database(paths.coreDatabasePath));
    QVERIFY(setDatabaseMarker(paths.coreDatabasePath, QLatin1String("original")));
    QVERIFY(writeTextFile(paths.configFilePath,
                          QByteArray("[General Settings]\nVersion=original\n")));
    const QString candidatePath = directory.filePath(QLatin1String("candidate.db"));
    QVERIFY(createFullP1Database(candidatePath));
    QVERIFY(setDatabaseMarker(candidatePath, QLatin1String("replacement")));

    const PrivacyProfilePublicationResult prepared = PrivacyProfilePublication::prepare(
        fixtureStage(candidatePath), paths);
    QVERIFY2(prepared.success, qPrintable(prepared.error));
    QVERIFY(PrivacyProfilePublication::applyPending(paths.transactionHome).success);
    QCOMPARE(databaseMarker(paths.coreDatabasePath), QLatin1String("replacement"));

    const QList<PrivacyProfileBackup> backups =
        PrivacyProfilePublication::restorableBackups(paths.transactionHome);
    QCOMPARE(backups.size(), 1);
    QCOMPARE(databaseMarker(backups.constFirst().coreDatabasePath),
             QLatin1String("original"));
    const PrivacyProfilePublicationResult restore =
        PrivacyProfilePublication::prepareRestore(backups.constFirst(), paths);
    QVERIFY2(restore.success, qPrintable(restore.error));
    QVERIFY(PrivacyProfilePublication::applyPending(paths.transactionHome).success);
    QCOMPARE(databaseMarker(paths.coreDatabasePath), QLatin1String("original"));
}

void PrivacyProfileImportTest::testProtectedStorePreflightPasses()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));

    const PrivacyProfilePreflightResult result =
        PrivacyProfilePreflight::verify(fixture.databasePath);
    QVERIFY2(result.success, qPrintable(result.error));
    QCOMPARE(result.verifiedRootCount, 2);
    QCOMPARE(result.verifiedStoreCount, 1);
    QCOMPARE(result.failedRootCount, 0);
    QCOMPARE(result.failedStoreCount, 0);
    QCOMPARE(result.failedContainerCount, 0);
    QVERIFY(result.verifiedContainerCount >= 3);
}

void PrivacyProfileImportTest::testProtectedStorePreflightFailsOnMissingContainer()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));
    QVERIFY(QFile::remove(fixture.containerPublic));

    const PrivacyProfilePreflightResult result =
        PrivacyProfilePreflight::verify(fixture.databasePath);
    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QLatin1String("missing protected object")));
    QCOMPARE(result.failedContainerCount, 1);
}

void PrivacyProfileImportTest::testProtectedStorePreflightFailsOnContainerSizeMismatch()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));
    QVERIFY(writeTextFile(fixture.containerPublic, QByteArray(24, 'a')));

    const PrivacyProfilePreflightResult result =
        PrivacyProfilePreflight::verify(fixture.databasePath);
    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QLatin1String("protected object size mismatch")));
}

void PrivacyProfileImportTest::testProtectedStorePreflightFailsOnMissingStore()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));
    QVERIFY(QFile::remove(fixture.storeConfigPath));

    const PrivacyProfilePreflightResult result =
        PrivacyProfilePreflight::verify(fixture.databasePath);
    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QLatin1String("missing store configuration")));
    QCOMPARE(result.failedStoreCount, 1);
}

void PrivacyProfileImportTest::testProtectedStorePreflightFailsOnMissingRootMarker()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));
    QVERIFY(QFile::remove(fixture.markerManaged));

    const PrivacyProfilePreflightResult result =
        PrivacyProfilePreflight::verify(fixture.databasePath);
    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QLatin1String("missing managed-root marker")));
    QCOMPARE(result.failedRootCount, 1);
}

void PrivacyProfileImportTest::testProtectedStorePreflightFailsOnProxySizeMismatch()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));
    QVERIFY(writeTextFile(fixture.proxyPath, QByteArray(20, 'c')));

    const PrivacyProfilePreflightResult result =
        PrivacyProfilePreflight::verify(fixture.databasePath);
    QVERIFY(!result.success);
    QVERIFY(result.error.contains(QLatin1String("public proxy size mismatch")));
}

void PrivacyProfileImportTest::testP1WithProtectedItemsStagesOnlyAfterPreflight()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    ProtectedFixture fixture;
    QVERIFY(buildProtectedFixture(directory.path(), &fixture));
    const PrivacyProfileSummary source =
        PrivacyProfileInspector::inspectCoreDatabase(fixture.databasePath);
    QVERIFY(source.isPrivateProfile());
    QCOMPARE(source.protectedItemCount, 1LL);

    PrivacyProfileImportStageResult staged = PrivacyProfileImportStager::stage(
        source, directory.filePath(QLatin1String("stage-ok")));
    QVERIFY2(staged.success, qPrintable(staged.error));

    QVERIFY(QFile::remove(fixture.containerPublic));
    staged = PrivacyProfileImportStager::stage(
        source, directory.filePath(QLatin1String("stage-fail")));
    QVERIFY(!staged.success);
    QVERIFY(staged.error.contains(QLatin1String("Protected-store validation failed")));
}

QTEST_GUILESS_MAIN(PrivacyProfileImportTest)

#include "privacyprofileimport_utest.moc"

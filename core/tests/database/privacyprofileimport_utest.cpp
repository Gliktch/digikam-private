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
                      "CREATE TABLE PrivacyCategories (uuid TEXT PRIMARY KEY);"));

        const QStringList remainingTables = {
            QLatin1String("PrivacyCredentials"),
            QLatin1String("PrivacyStorageRoots"),
            QLatin1String("PrivacyStores"),
            QLatin1String("PrivacyStoreBindings"),
            QLatin1String("PrivacyContainers"),
            QLatin1String("PrivacyAssets"),
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

QTEST_GUILESS_MAIN(PrivacyProfileImportTest)

#include "privacyprofileimport_utest.moc"

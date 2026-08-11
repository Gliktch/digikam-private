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
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

// Local includes

#include "privacyprofileinspector.h"
#include "privacysqlitesnapshot.h"

using namespace Digikam;

class PrivacyProfileImportTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testVersionLabels();
    void testStockAndP1Inspection();
    void testAmbiguousSchemaIsRejected();
    void testOnlineBackupCapturesWalState();
    void testCanceledSnapshotIsNotPublished();
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

} // namespace

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

QTEST_GUILESS_MAIN(PrivacyProfileImportTest)

#include "privacyprofileimport_utest.moc"

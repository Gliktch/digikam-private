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

#include <QDomDocument>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTest>
#include <QUuid>

// Local includes

#include "coredbschemaupdater.h"
#include "privacyservice.h"

using namespace Digikam;

class PrivacyFoundationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testSchemaActions();
    void testSqliteSchemaActionsExecute();
    void testStorageRecordValidation();
    void testSessionLockState();
    void testUnknownCategoryFailsClosed();
};

namespace
{

PrivacyCategory makeCategory(const QString& uuid, const QString& name)
{
    PrivacyCategory category;
    category.uuid      = uuid;
    category.name      = name;
    category.lifecycleState = PrivacyCategoryLifecycleState::Active;
    category.currentCredentialGeneration = 1;
    category.createdAt = QDateTime::currentDateTime();

    return category;
}

PrivacyItem makeItem(qlonglong imageId, const QString& uuid, const QString& categoryUuid)
{
    PrivacyItem item;
    item.imageId      = imageId;
    item.uuid         = uuid;
    item.categoryUuid = categoryUuid;

    return item;
}

QString actionText(const QDomElement& database, const QString& actionName)
{
    const QDomNodeList actions = database.elementsByTagName(QLatin1String("dbaction"));

    for (int i = 0 ; i < actions.count() ; ++i)
    {
        const QDomElement action = actions.at(i).toElement();

        if (action.attribute(QLatin1String("name")) == actionName)
        {
            return action.text();
        }
    }

    return QString();
}

QDomElement databaseElement(const QDomDocument& document, const QString& databaseName)
{
    const QDomNodeList databases = document.elementsByTagName(QLatin1String("database"));

    for (int i = 0 ; i < databases.count() ; ++i)
    {
        const QDomElement database = databases.at(i).toElement();

        if (database.attribute(QLatin1String("name")) == databaseName)
        {
            return database;
        }
    }

    return QDomElement();
}

QStringList actionStatements(const QDomElement& database, const QString& actionName)
{
    const QDomNodeList actions = database.elementsByTagName(QLatin1String("dbaction"));

    for (int i = 0 ; i < actions.count() ; ++i)
    {
        const QDomElement action = actions.at(i).toElement();

        if (action.attribute(QLatin1String("name")) != actionName)
        {
            continue;
        }

        QStringList statements;

        for (QDomElement statement = action.firstChildElement(QLatin1String("statement")) ;
             !statement.isNull() ;
             statement = statement.nextSiblingElement(QLatin1String("statement")))
        {
            statements << statement.text().trimmed();
        }

        return statements;
    }

    return {};
}

bool executeSqliteSchemaScenario(const QDomElement& database, bool update, QString* errorMessage)
{
    const QString connectionName = QLatin1String("privacy-schema-") +
                                   QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool success                 = true;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
        db.setDatabaseName(QLatin1String(":memory:"));

        if (!db.open())
        {
            *errorMessage = db.lastError().text();
            success       = false;
        }

        QSqlQuery query(db);

        if (success && update)
        {
            success = query.exec(QLatin1String("CREATE TABLE AlbumRoots (id INTEGER PRIMARY KEY);")) &&
                      query.exec(QLatin1String("CREATE TABLE Images "
                                               "(id INTEGER PRIMARY KEY, name TEXT NOT NULL);"));
        }

        const QStringList actionNames = update
                                      ? QStringList { QLatin1String("UpdateSchemaFromV17ToV18") }
                                      : QStringList { QLatin1String("CreateDB"),
                                                      QLatin1String("CreateIndices"),
                                                      QLatin1String("CreateTriggers") };

        for (const QString& actionName : actionNames)
        {
            const QStringList statements = actionStatements(database, actionName);

            if (statements.isEmpty())
            {
                *errorMessage = QLatin1String("Missing action: ") + actionName;
                success       = false;
                break;
            }

            for (const QString& statement : statements)
            {
                if (!query.exec(statement))
                {
                    *errorMessage = actionName + QLatin1String(": ") + query.lastError().text();
                    success       = false;
                    break;
                }
            }

            if (!success)
            {
                break;
            }
        }

        const QString imageInsert = update
                                  ? QLatin1String("INSERT INTO Images (id, name) VALUES (1, 'proxy.jpg');")
                                  : QLatin1String("INSERT INTO Images (id, name, status, category) "
                                                  "VALUES (1, 'proxy.jpg', 1, 1);");

        if (success && !query.exec(imageInsert))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO AlbumRoots "
                                      "(id" ) + (update ? QLatin1String(") VALUES (1);")
                                                     : QLatin1String(", status, type) VALUES (1, 0, 1);"))))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyCategories "
                                      "(uuid, name, backend, presentationMode, unlockedThumbnailMode, "
                                      "lifecycleState, currentCredentialGeneration, schemaVersion, createdAt) "
                                      "VALUES ('10000000-0000-0000-0000-000000000001', "
                                      "'Category', 1, 2, 1, 2, 1, 1, '2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            (!query.exec(QLatin1String("SELECT tagVisibilityMode FROM PrivacyCategories;")) ||
             !query.next() ||
             (query.value(0).toInt() !=
              static_cast<int>(PrivacyTagVisibilityMode::UnlockedOnly))))
        {
            *errorMessage = query.lastError().text().isEmpty()
                          ? QLatin1String("Privacy tag-visibility schema default is not UnlockedOnly")
                          : query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyCredentials "
                                      "(categoryUuid, generation, encodingVersion, envelopeFormat, envelopeBlob, "
                                      "envelopeHashAlgorithm, envelopeHash, recoveryMode, recoveryState, "
                                      "recoveryRecordVersion, createdAt) VALUES "
                                      "('10000000-0000-0000-0000-000000000001', 1, 'utf8-nfc-v1', "
                                      "'gocryptfs-config-v2', X'0102', 'sha256', 'credential-hash', 0, 0, 1, "
                                      "'2026-08-08T00:00:00'), "
                                      "('10000000-0000-0000-0000-000000000001', 2, 'utf8-nfc-v1', "
                                      "'gocryptfs-config-v2', X'0304', 'sha256', 'pending-credential-hash', 0, 0, 1, "
                                      "'2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyStorageRoots "
                                      "(uuid, kind, albumRootId, configuredPath, identityVersion, identityData, "
                                      "markerUuid, schemaVersion, createdAt) VALUES "
                                      "('30000000-0000-0000-0000-000000000001', 1, 1, '/collection', 1, X'01', NULL, 1, "
                                      "'2026-08-08T00:00:00'), "
                                      "('30000000-0000-0000-0000-000000000002', 2, NULL, '/vault', 1, X'02', "
                                      "'40000000-0000-0000-0000-000000000002', 1, "
                                      "'2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            query.exec(QLatin1String("INSERT INTO PrivacyStorageRoots "
                                     "(uuid, kind, albumRootId, configuredPath, identityVersion, identityData, "
                                     "schemaVersion, createdAt) VALUES "
                                     "('30000000-0000-0000-0000-000000000003', 1, 1, '/other', 1, X'03', 1, "
                                     "'2026-08-08T00:00:00');")))
        {
            *errorMessage = QLatin1String("Duplicate privacy album-root mapping unexpectedly succeeded");
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyStorageRoots "
                                      "(uuid, kind, albumRootId, configuredPath, identityVersion, identityData, "
                                      "markerUuid, schemaVersion, createdAt) VALUES "
                                      "('30000000-0000-0000-0000-000000000004', 2, NULL, '/vault-two', 1, X'04', "
                                      "'40000000-0000-0000-0000-000000000004', 1, "
                                      "'2026-08-08T00:00:00');")))
        {
            *errorMessage = QLatin1String("A second nullable managed-root mapping was rejected");
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyStores "
                                      "(uuid, categoryUuid, rootUuid, format, formatVersion, cipherRelativePath, "
                                      "configRelativePath, configGeneration, lifecycleState, schemaVersion, createdAt) "
                                      "VALUES ('40000000-0000-0000-0000-000000000001', "
                                      "'10000000-0000-0000-0000-000000000001', "
                                      "'30000000-0000-0000-0000-000000000002', 'gocryptfs', 2, "
                                      "'stores/category', 'stores/category/gocryptfs.conf', 1, 2, 1, "
                                      "'2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyStoreBindings "
                                      "(categoryUuid, role, storeUuid, schemaVersion) VALUES "
                                      "('10000000-0000-0000-0000-000000000001', 1, "
                                      "'40000000-0000-0000-0000-000000000001', 1), "
                                      "('10000000-0000-0000-0000-000000000001', 3, "
                                      "'40000000-0000-0000-0000-000000000001', 1);")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyItems "
                                      "(imageId, uuid, categoryUuid, presentationVersion, generation, transactionState) "
                                      "VALUES (1, '20000000-0000-0000-0000-000000000001', "
                                      "'10000000-0000-0000-0000-000000000001', 1, 0, 0);")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            query.exec(QLatin1String("INSERT INTO PrivacyItems "
                                     "(imageId, uuid, categoryUuid, presentationVersion, generation, transactionState) "
                                     "VALUES (1, '20000000-0000-0000-0000-000000000002', "
                                     "'10000000-0000-0000-0000-000000000001', 1, 0, 0);")))
        {
            *errorMessage = QLatin1String("Duplicate image mapping unexpectedly succeeded");
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyContainers "
                                      "(uuid, itemUuid, kind, rootUuid, objectRelativePath, protectedSize, "
                                      "protectedHashAlgorithm, protectedHash, formatVersion, credentialGeneration, "
                                      "state, createdAt, updatedAt) VALUES "
                                      "('50000000-0000-0000-0000-000000000001', "
                                      "'20000000-0000-0000-0000-000000000001', 1, "
                                      "'30000000-0000-0000-0000-000000000001', "
                                      "'album/proxy.jpg.digikam-private.zip', 123, 'sha256', 'container-hash', "
                                      "1, 1, 2, '2026-08-08T00:00:00', '2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyAssets "
                                      "(itemUuid, role, ordinal, originalName, publicRootUuid, publicRelativePath, "
                                      "containerUuid, protectedRelativePath, hashAlgorithm, originalHash, originalSize) "
                                      "VALUES ('20000000-0000-0000-0000-000000000001', 1, 0, 'proxy.jpg', "
                                      "'30000000-0000-0000-0000-000000000001', 'album/proxy.jpg', "
                                      "'50000000-0000-0000-0000-000000000001', 'members/original.jpg', "
                                      "'sha256', 'original-hash', 100);")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyDerivatives "
                                      "(itemUuid, kind, ordinal, storeUuid, protectedRelativePath, "
                                      "sourceHashAlgorithm, sourceOriginalHash, derivativeFormat, "
                                      "derivativeHashAlgorithm, derivativeHash, derivativeSize, "
                                      "presentationVersion, generation, createdAt) VALUES "
                                      "('20000000-0000-0000-0000-000000000001', 1, 0, "
                                      "'40000000-0000-0000-0000-000000000001', 'clear/item/thumb.jpg', "
                                      "'sha256', 'original-hash', 'jpeg', 'sha256', 'derivative-hash', 20, 1, 1, "
                                      "'2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyTransactions "
                                      "(uuid, categoryUuid, itemUuid, type, state, generation, "
                                      "fromCredentialGeneration, toCredentialGeneration, payloadFormatVersion, "
                                      "createdAt, updatedAt) VALUES "
                                      "('60000000-0000-0000-0000-000000000001', "
                                      "'10000000-0000-0000-0000-000000000001', "
                                      "'20000000-0000-0000-0000-000000000001', 1, 1, 1, 1, 2, 1, "
                                      "'2026-08-08T00:00:00', '2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success &&
            !query.exec(QLatin1String("INSERT INTO PrivacyTransactionJournals "
                                      "(transactionUuid, rootUuid, journalRelativePath, journalFormatVersion, "
                                      "stage, updatedAt) VALUES "
                                      "('60000000-0000-0000-0000-000000000001', "
                                      "'30000000-0000-0000-0000-000000000001', 'journals/public.cbor', 1, 1, "
                                      "'2026-08-08T00:00:00'), "
                                      "('60000000-0000-0000-0000-000000000001', "
                                      "'30000000-0000-0000-0000-000000000002', 'journals/store.cbor', 1, 1, "
                                      "'2026-08-08T00:00:00');")))
        {
            *errorMessage = query.lastError().text();
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyItems WHERE imageId=1;")))
        {
            *errorMessage = QLatin1String("Privacy item with assets unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM Images WHERE id=1;")))
        {
            *errorMessage = QLatin1String("Protected image deletion unexpectedly succeeded");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyCategories;")))
        {
            *errorMessage = QLatin1String("Populated category deletion unexpectedly succeeded");
            success       = false;
        }

        if (success &&
            query.exec(QLatin1String("DELETE FROM PrivacyCredentials WHERE generation=2;")))
        {
            *errorMessage = QLatin1String("Transaction credential generation unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyCredentials;")))
        {
            *errorMessage = QLatin1String("Referenced credential generation unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyContainers;")))
        {
            *errorMessage = QLatin1String("Container with protected records unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyTransactions;")))
        {
            *errorMessage = QLatin1String("Transaction with journals unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyStores;")))
        {
            *errorMessage = QLatin1String("Bound privacy store unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM PrivacyStorageRoots;")))
        {
            *errorMessage = QLatin1String("Referenced privacy root unexpectedly deleted");
            success       = false;
        }

        if (success && query.exec(QLatin1String("DELETE FROM AlbumRoots WHERE id=1;")))
        {
            *errorMessage = QLatin1String("Referenced album root unexpectedly deleted");
            success       = false;
        }

        db.close();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return success;
}

} // namespace

void PrivacyFoundationTest::testSchemaActions()
{
    QCOMPARE(CoreDbSchemaUpdater::schemaVersion(), 18);

    QFile file(QString::fromUtf8(PRIVACY_DB_CONFIG_PATH));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.errorString()));

    QDomDocument document;
    QVERIFY2(document.setContent(&file), "Database configuration XML is invalid");

    const QDomNodeList databases = document.elementsByTagName(QLatin1String("database"));
    QCOMPARE(databases.count(), 2);

    const QStringList tableNames = {
        QLatin1String("PrivacyCategories"),
        QLatin1String("PrivacyCredentials"),
        QLatin1String("PrivacyStorageRoots"),
        QLatin1String("PrivacyStores"),
        QLatin1String("PrivacyStoreBindings"),
        QLatin1String("PrivacyItems"),
        QLatin1String("PrivacyContainers"),
        QLatin1String("PrivacyAssets"),
        QLatin1String("PrivacyDerivatives"),
        QLatin1String("PrivacyTransactions"),
        QLatin1String("PrivacyTransactionJournals")
    };

    for (int i = 0 ; i < databases.count() ; ++i)
    {
        const QDomElement database = databases.at(i).toElement();
        const QString createText   = actionText(database, QLatin1String("CreateDB"));
        const QString updateText   = actionText(database, QLatin1String("UpdateSchemaFromV17ToV18"));

        QVERIFY2(!createText.isEmpty(), qPrintable(database.attribute(QLatin1String("name"))));
        QVERIFY2(!updateText.isEmpty(), qPrintable(database.attribute(QLatin1String("name"))));

        for (const QString& tableName : tableNames)
        {
            QVERIFY2(createText.contains(tableName), qPrintable(tableName));
            QVERIFY2(updateText.contains(tableName), qPrintable(tableName));
            QVERIFY2(!actionText(database, QLatin1String("Migrate_Read_") + tableName).isEmpty(),
                     qPrintable(tableName));
            QVERIFY2(!actionText(database, QLatin1String("Migrate_Write_") + tableName).isEmpty(),
                     qPrintable(tableName));
        }

        const QString imagePrimaryKey =
            (database.attribute(QLatin1String("name")) == QLatin1String("QSQLITE"))
                ? QLatin1String("imageId INTEGER PRIMARY KEY")
                : QLatin1String("imageId BIGINT PRIMARY KEY");
        QVERIFY2(createText.contains(imagePrimaryKey), qPrintable(imagePrimaryKey));
        QVERIFY2(updateText.contains(imagePrimaryKey), qPrintable(imagePrimaryKey));
        QVERIFY(createText.contains(QLatin1String("categoryUuid")));
        QVERIFY(createText.contains(QLatin1String("tagVisibilityMode INTEGER NOT NULL DEFAULT 1")));
        QVERIFY(updateText.contains(QLatin1String("tagVisibilityMode INTEGER NOT NULL DEFAULT 1")));
        QVERIFY(actionText(database, QLatin1String("Migrate_Read_PrivacyCategories"))
                    .contains(QLatin1String("tagVisibilityMode")));
        QVERIFY(actionText(database, QLatin1String("Migrate_Write_PrivacyCategories"))
                    .contains(QLatin1String(":tagVisibilityMode")));
        QVERIFY(createText.contains(QLatin1String("ON DELETE RESTRICT")));
        QVERIFY(createText.contains(QLatin1String("UNIQUE(albumRootId)")) ||
                createText.contains(QLatin1String("UNIQUE INDEX PrivacyStorageRoots_AlbumRoot")));
        QVERIFY(updateText.contains(QLatin1String("UNIQUE(albumRootId)")) ||
                updateText.contains(QLatin1String("UNIQUE INDEX PrivacyStorageRoots_AlbumRoot")));
        QVERIFY(!createText.contains(QLatin1String("protectedObjectPath")));
        QVERIFY(!createText.contains(QLatin1String("journalPath")));
        QVERIFY(!createText.contains(QLatin1String("online"), Qt::CaseInsensitive));

        for (const QString& tableName : tableNames)
        {
            const QString copyText = actionText(database, QLatin1String("Migrate_Write_") + tableName);
            QVERIFY2(!copyText.contains(QLatin1String("IGNORE"), Qt::CaseInsensitive),
                     qPrintable(tableName));
        }

        if (database.attribute(QLatin1String("name")) == QLatin1String("QMYSQL"))
        {
            QVERIFY(updateText.contains(QLatin1String("CREATE TABLE IF NOT EXISTS PrivacyCategories")));
        }
    }
}

void PrivacyFoundationTest::testSqliteSchemaActionsExecute()
{
    if (!QSqlDatabase::isDriverAvailable(QLatin1String("QSQLITE")))
    {
        QSKIP("Qt SQLite driver is unavailable");
    }

    QFile file(QString::fromUtf8(PRIVACY_DB_CONFIG_PATH));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.errorString()));

    QDomDocument document;
    QVERIFY2(document.setContent(&file), "Database configuration XML is invalid");

    const QDomElement sqlite = databaseElement(document, QLatin1String("QSQLITE"));
    QVERIFY(!sqlite.isNull());

    QString errorMessage;
    QVERIFY2(executeSqliteSchemaScenario(sqlite, false, &errorMessage), qPrintable(errorMessage));
    QVERIFY2(executeSqliteSchemaScenario(sqlite, true,  &errorMessage), qPrintable(errorMessage));
}

void PrivacyFoundationTest::testStorageRecordValidation()
{
    const QString categoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
    const QString itemUuid     = QLatin1String("20000000-0000-0000-0000-000000000001");
    const QString rootUuid     = QLatin1String("30000000-0000-0000-0000-000000000001");
    const QString storeUuid    = QLatin1String("40000000-0000-0000-0000-000000000001");
    const QString markerUuid   = QLatin1String("40000000-0000-0000-0000-000000000002");
    const QString containerUuid = QLatin1String("50000000-0000-0000-0000-000000000001");

    QCOMPARE(static_cast<int>(PrivacyDerivativeKind::ClearThumbnail), 1);
    QCOMPARE(static_cast<int>(PrivacyDerivativeKind::BlurredPresentation), 2);

    PrivacyCategory category = makeCategory(categoryUuid, QLatin1String("Category"));
    QCOMPARE(category.unlockedThumbnailMode, PrivacyUnlockedThumbnailMode::FocusedClear);
    QCOMPARE(category.tagVisibilityMode, PrivacyTagVisibilityMode::UnlockedOnly);
    category.unlockedThumbnailMode = static_cast<PrivacyUnlockedThumbnailMode>(99);
    QVERIFY(!category.isValid());
    category.unlockedThumbnailMode = PrivacyUnlockedThumbnailMode::FocusedClear;
    category.tagVisibilityMode = static_cast<PrivacyTagVisibilityMode>(99);
    QVERIFY(!category.isValid());
    category.tagVisibilityMode = PrivacyTagVisibilityMode::UnlockedOnly;
    QVERIFY(category.isValid());

    PrivacyCredential credential;
    credential.categoryUuid          = categoryUuid;
    credential.generation            = 1;
    credential.encodingVersion       = QLatin1String("utf8-nfc-v1");
    credential.envelopeFormat        = QLatin1String("gocryptfs-config-v2");
    credential.envelopeBlob          = QByteArray("opaque");
    credential.envelopeHashAlgorithm = QLatin1String("sha256");
    credential.envelopeHash          = QLatin1String("hash");
    credential.createdAt             = QDateTime::currentDateTimeUtc();
    QVERIFY(credential.isValid());

    credential.encodingVersion = QLatin1String("unknown-encoding");
    QVERIFY(!credential.isValid());
    credential.encodingVersion = QLatin1String("utf8-nfc-v1");

    PrivacyStorageRoot root;
    root.uuid            = rootUuid;
    root.kind            = PrivacyStorageRootKind::ManagedStoreRoot;
    root.configuredPath  = QLatin1String("/configured/hint");
    root.identityVersion = 1;
    root.identityData    = QByteArray("opaque-root-identity");
    root.markerUuid      = markerUuid;
    root.createdAt       = QDateTime::currentDateTimeUtc();
    QVERIFY(root.isValid());

    PrivacyStore store;
    store.uuid               = storeUuid;
    store.categoryUuid       = categoryUuid;
    store.rootUuid           = rootUuid;
    store.format             = QLatin1String("gocryptfs");
    store.formatVersion      = 2;
    store.cipherRelativePath = QLatin1String("stores/category");
    store.configRelativePath = QLatin1String("stores/category/gocryptfs.conf");
    store.configGeneration   = 1;
    store.lifecycleState     = PrivacyStoreLifecycleState::Active;
    store.createdAt          = QDateTime::currentDateTimeUtc();
    QVERIFY(store.isValid());

    store.cipherRelativePath = QLatin1String("/absolute/path");
    QVERIFY(!store.isValid());
    store.cipherRelativePath = QLatin1String("stores/../escape");
    QVERIFY(!store.isValid());
    store.cipherRelativePath = QLatin1String("stores/category");
    store.configRelativePath = QLatin1String("other/gocryptfs.conf");
    QVERIFY(!store.isValid());

    PrivacyContainer container;
    container.uuid                    = containerUuid;
    container.itemUuid                = itemUuid;
    container.kind                    = PrivacyContainerKind::StrongObject;
    container.storeUuid               = storeUuid;
    container.objectRelativePath      = QLatin1String("originals/item");
    container.protectedSize           = 100;
    container.protectedHashAlgorithm  = QLatin1String("sha256");
    container.protectedHash           = QLatin1String("hash");
    container.formatVersion           = 1;
    container.credentialGeneration    = 1;
    container.state                   = PrivacyContainerState::Verified;
    container.createdAt               = QDateTime::currentDateTimeUtc();
    container.updatedAt               = container.createdAt;
    QVERIFY(container.isValid());

    container.rootUuid = rootUuid;
    QVERIFY(!container.isValid());
    container.rootUuid.clear();

    container.objectRelativePath = QLatin1String("objects/item");
    QVERIFY(!container.isValid());
    container.objectRelativePath = QLatin1String("originals/item");

    container.kind                    = PrivacyContainerKind::CasualArchive;
    container.storeUuid.clear();
    container.rootUuid                = rootUuid;
    container.objectRelativePath      = QLatin1String("album/item.zip");
    QVERIFY(!container.isValid());
    container.objectRelativePath      = QLatin1String("album/item.jpg.digikam-private.zip");
    QVERIFY(container.isValid());

    PrivacyDerivative derivative;
    derivative.itemUuid                = itemUuid;
    derivative.kind                    = PrivacyDerivativeKind::ClearThumbnail;
    derivative.ordinal                 = 0;
    derivative.storeUuid               = storeUuid;
    derivative.protectedRelativePath   = QLatin1String("derivatives/clear-thumb.jpg");
    derivative.sourceHashAlgorithm     = QLatin1String("sha256");
    derivative.sourceOriginalHash      = QLatin1String("original-hash");
    derivative.derivativeFormat        = QLatin1String("jpeg");
    derivative.derivativeHashAlgorithm = QLatin1String("sha256");
    derivative.derivativeHash          = QLatin1String("derivative-hash");
    derivative.derivativeSize          = 20;
    derivative.presentationVersion     = 1;
    derivative.generation              = 1;
    derivative.createdAt               = QDateTime::currentDateTimeUtc();
    QVERIFY(derivative.isValid());

    derivative.protectedRelativePath = QLatin1String("../clear-thumb.jpg");
    QVERIFY(!derivative.isValid());
    derivative.protectedRelativePath = QLatin1String("clear/clear-thumb.jpg");
    QVERIFY(!derivative.isValid());

    derivative.kind                  = PrivacyDerivativeKind::BlurredPresentation;
    derivative.protectedRelativePath = QLatin1String("derivatives/blurred-presentation.jpg");
    QVERIFY(derivative.isValid());

    derivative.kind = static_cast<PrivacyDerivativeKind>(99);
    QVERIFY(!derivative.isValid());
}

void PrivacyFoundationTest::testSessionLockState()
{
    const QString categoryAUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
    const QString categoryBUuid = QLatin1String("10000000-0000-0000-0000-000000000002");

    const QList<PrivacyCategory> categories = {
        makeCategory(categoryAUuid, QLatin1String("Category A")),
        makeCategory(categoryBUuid, QLatin1String("Category B"))
    };

    const QList<PrivacyItem> items = {
        makeItem(1, QLatin1String("20000000-0000-0000-0000-000000000001"), categoryAUuid),
        makeItem(2, QLatin1String("20000000-0000-0000-0000-000000000002"), categoryBUuid)
    };

    PrivacyService service(categories, items);

    const quint64 initialCategoryAEpoch = service.categoryEpoch(categoryAUuid);
    const quint64 initialCategoryBEpoch = service.categoryEpoch(categoryBUuid);
    QVERIFY(initialCategoryAEpoch > 0);
    QVERIFY(initialCategoryBEpoch > 0);
    QVERIFY(initialCategoryAEpoch != initialCategoryBEpoch);
    QCOMPARE(service.itemCategoryEpoch(1), initialCategoryAEpoch);
    QCOMPARE(service.itemGeneration(1), 0LL);
    QVERIFY(service.compareAndSetItemGeneration(1, 0, 1));
    QCOMPARE(service.itemGeneration(1), 1LL);
    QVERIFY(!service.compareAndSetItemGeneration(1, 0, 2));
    QVERIFY(!service.compareAndSetItemGeneration(1, 1, 1));
    QCOMPARE(service.categoryEpoch(QLatin1String("ffffffff-ffff-ffff-ffff-ffffffffffff")),
             quint64(0));
    QCOMPARE(service.itemCategoryEpoch(999), quint64(0));
    QCOMPARE(service.itemGeneration(999), -1LL);

    PrivacyServiceItemState sampledState;
    QVERIFY(service.sessionStateForItem(1, &sampledState));
    QVERIFY(sampledState.protectedItem);
    QCOMPARE(sampledState.access, PrivacyItemAccess::Locked);
    QCOMPARE(sampledState.categoryEpoch, initialCategoryAEpoch);
    QCOMPARE(sampledState.itemGeneration, 1LL);
    QVERIFY(service.sessionStateForItem(999, &sampledState));
    QVERIFY(!sampledState.protectedItem);

    QCOMPARE(service.itemAccess(1), PrivacyItemAccess::Locked);
    QCOMPARE(service.itemAccess(2), PrivacyItemAccess::Locked);
    QVERIFY(service.isProtected(1));
    QCOMPARE(service.categoryUuidForItem(1), categoryAUuid);
    QVERIFY(!service.mayAccessOriginal(1));
    QVERIFY(!service.mayAnalyze(2));
    QVERIFY(!service.mayAccessManualTags(1));
    QVERIFY(!service.mayAccessManualTags(2));
    QVERIFY(service.mayAccessManualTags(999));
    const quint64 preTagVisibilityEpoch = service.categoryEpoch(categoryBUuid);
    QVERIFY(!service.setCategoryTagVisibilityMode(categoryBUuid,
                                                  PrivacyTagVisibilityMode::AlwaysVisible,
                                                  false));
    QCOMPARE(service.categoryEpoch(categoryBUuid), preTagVisibilityEpoch);
    QVERIFY(service.setCategoryTagVisibilityMode(categoryBUuid,
                                                 PrivacyTagVisibilityMode::AlwaysVisible,
                                                 true));
    QVERIFY(service.categoryEpoch(categoryBUuid) > preTagVisibilityEpoch);
    QVERIFY(service.mayAccessManualTags(2));

    QVERIFY(service.setCategoryUnlocked(categoryAUuid, true));
    const quint64 unlockedCategoryAEpoch = service.categoryEpoch(categoryAUuid);
    QVERIFY(unlockedCategoryAEpoch > initialCategoryBEpoch);
    QCOMPARE(service.itemCategoryEpoch(1), unlockedCategoryAEpoch);
    QVERIFY(service.setCategoryUnlocked(categoryAUuid, true));
    QCOMPARE(service.categoryEpoch(categoryAUuid), unlockedCategoryAEpoch);
    QCOMPARE(service.itemAccess(1), PrivacyItemAccess::Unlocked);
    QCOMPARE(service.itemAccess(2), PrivacyItemAccess::Locked);
    QVERIFY(service.mayAccessOriginal(1));
    QVERIFY(!service.mayAccessOriginal(2));
    QVERIFY(service.mayAccessManualTags(1));
    QVERIFY(service.mayAccessManualTags(2));
    QVERIFY(!service.mayAnalyze(1));

    QCOMPARE(service.itemAccess(999), PrivacyItemAccess::Unprotected);
    QVERIFY(!service.isProtected(999));
    QVERIFY(service.mayAccessOriginal(999));
    QVERIFY(service.mayAnalyze(999));

    service.lockAll();
    const quint64 lockedCategoryAEpoch = service.categoryEpoch(categoryAUuid);
    const quint64 lockedCategoryBEpoch = service.categoryEpoch(categoryBUuid);
    QVERIFY(lockedCategoryAEpoch > unlockedCategoryAEpoch);
    QVERIFY(lockedCategoryBEpoch > initialCategoryBEpoch);
    QVERIFY(lockedCategoryAEpoch != lockedCategoryBEpoch);
    QCOMPARE(service.itemAccess(1), PrivacyItemAccess::Locked);
    QVERIFY(!service.mayAccessManualTags(1));
    QVERIFY(service.mayAccessManualTags(2));

    QVERIFY(service.setCategoryTagVisibilityMode(categoryBUuid,
                                                 PrivacyTagVisibilityMode::UnlockedOnly,
                                                 false));
    QVERIFY(service.categoryEpoch(categoryBUuid) > lockedCategoryBEpoch);
    QVERIFY(!service.mayAccessManualTags(2));

    QVERIFY(service.setCategoryUnlocked(categoryBUuid, true));
    const quint64 preResetCategoryBEpoch = service.categoryEpoch(categoryBUuid);
    service.reset(categories, items);
    QVERIFY(service.categoryEpoch(categoryAUuid) > preResetCategoryBEpoch);
    QVERIFY(service.categoryEpoch(categoryBUuid) > service.categoryEpoch(categoryAUuid));
    QCOMPARE(service.itemAccess(2), PrivacyItemAccess::Locked);
}

void PrivacyFoundationTest::testUnknownCategoryFailsClosed()
{
    PrivacyService uninitializedService;

    QVERIFY(!uninitializedService.isInitialized());
    QVERIFY(uninitializedService.isProtected(999));
    QCOMPARE(uninitializedService.itemAccess(999), PrivacyItemAccess::Locked);
    QVERIFY(!uninitializedService.mayAccessOriginal(999));
    QVERIFY(!uninitializedService.mayAnalyze(999));
    QVERIFY(!uninitializedService.mayAccessManualTags(999));

    const PrivacyItem item = makeItem(3,
                                      QLatin1String("20000000-0000-0000-0000-000000000003"),
                                      QLatin1String("30000000-0000-0000-0000-000000000003"));

    PrivacyService service({}, { item });

    QVERIFY(service.isInitialized());
    QCOMPARE(service.itemAccess(3), PrivacyItemAccess::Locked);
    QVERIFY(!service.mayAccessOriginal(3));
    QVERIFY(!service.mayAnalyze(3));
    QVERIFY(!service.mayAccessManualTags(3));
    QVERIFY(!service.setCategoryUnlocked(item.categoryUuid, true));

    PrivacyItem malformedItem = item;
    malformedItem.imageId     = 4;
    malformedItem.uuid.clear();
    malformedItem.categoryUuid.clear();

    PrivacyService malformedService({}, { malformedItem });

    QVERIFY(malformedService.isProtected(4));
    QCOMPARE(malformedService.itemAccess(4), PrivacyItemAccess::Locked);
    QVERIFY(!malformedService.mayAccessOriginal(4));
    QVERIFY(!malformedService.mayAccessManualTags(4));

    const QString categoryUuid = QLatin1String("10000000-0000-0000-0000-000000000001");
    const PrivacyCategory category = makeCategory(categoryUuid, QLatin1String("Category"));
    const PrivacyItem duplicateA = makeItem(5,
                                             QLatin1String("20000000-0000-0000-0000-000000000005"),
                                             categoryUuid);
    const PrivacyItem duplicateB = makeItem(5,
                                             QLatin1String("20000000-0000-0000-0000-000000000006"),
                                             categoryUuid);
    PrivacyService duplicateService({ category }, { duplicateA, duplicateB });

    QVERIFY(duplicateService.setCategoryUnlocked(categoryUuid, true));
    QCOMPARE(duplicateService.itemAccess(5), PrivacyItemAccess::Locked);
    QCOMPARE(duplicateService.itemCategoryEpoch(5), quint64(0));
    QCOMPARE(duplicateService.itemGeneration(5), -1LL);
    PrivacyServiceItemState duplicateState;
    QVERIFY(!duplicateService.sessionStateForItem(5, &duplicateState));
    QVERIFY(!duplicateService.mayAccessManualTags(5));
}

QTEST_GUILESS_MAIN(PrivacyFoundationTest)

#include "privacyfoundation_utest.moc"

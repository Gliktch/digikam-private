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

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

// Unix includes

#include <sys/stat.h>
#include <unistd.h>

// Local includes

#include "privacyexternalcheckouttransaction.h"

using namespace Digikam;

namespace
{

const QString CategoryUuid = QStringLiteral("11111111-1111-4111-8111-111111111111");
const QString RootUuid = QStringLiteral("22222222-2222-4222-8222-222222222222");
const QString StoreRootUuid = QStringLiteral("66666666-6666-4666-8666-666666666666");
const QString StoreUuid = QStringLiteral("77777777-7777-4777-8777-777777777777");
const QString StoreMarkerUuid = QStringLiteral("88888888-8888-4888-8888-888888888888");
const QString ItemUuid = QStringLiteral("33333333-3333-4333-8333-333333333333");
const QString ContainerUuid = QStringLiteral("44444444-4444-4444-8444-444444444444");
const QString TransactionUuid = QStringLiteral("55555555-5555-4555-8555-555555555555");
const QByteArray OriginalBytes("synthetic original bytes");
const QByteArray SidecarBytes("synthetic sidecar bytes");
const QByteArray ProxyBytes("synthetic opaque proxy");

QByteArray digest(const QByteArray& bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

bool writeFile(const QString& path, const QByteArray& bytes)
{
    QFile file(path);
    return (file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
            (file.write(bytes) == bytes.size()) && file.flush());
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

class FakePersistence final : public PrivacyExternalCheckoutPersistence
{
public:

    bool loadSnapshot(PrivacyRepositorySnapshot* const output) const override
    {
        *output = snapshot;
        return true;
    }

    bool beginExternalCheckout(
        const PrivacyTransaction& transaction,
        const PrivacyTransactionJournal& journal) override
    {
        for (const PrivacyTransaction& existing : std::as_const(snapshot.transactions))
        {
            if ((existing.uuid == transaction.uuid) ||
                (existing.isActive() &&
                 ((existing.itemUuid == transaction.itemUuid) ||
                  (existing.categoryUuid == transaction.categoryUuid))))
            {
                return false;
            }
        }

        begunTransaction = transaction;
        snapshot.transactions << transaction;
        snapshot.transactionJournals << journal;
        return true;
    }

    bool compareAndUpdateTransaction(
        const PrivacyTransaction& transaction,
        PrivacyTransactionState expectedState,
        qlonglong expectedGeneration) override
    {
        if (failState == transaction.state)
        {
            return false;
        }

        for (PrivacyTransaction& existing : snapshot.transactions)
        {
            if (existing.uuid == transaction.uuid)
            {
                if ((existing.state != expectedState) ||
                    (existing.generation != expectedGeneration))
                {
                    return false;
                }

                existing = transaction;
                return true;
            }
        }

        return false;
    }

    bool compareAndUpdateJournal(
        const PrivacyTransactionJournal& journal,
        int expectedStage) override
    {
        for (PrivacyTransactionJournal& existing : snapshot.transactionJournals)
        {
            if ((existing.transactionUuid == journal.transactionUuid) &&
                (existing.rootUuid == journal.rootUuid))
            {
                if (existing.stage != expectedStage)
                {
                    return false;
                }

                existing = journal;
                return true;
            }
        }

        return false;
    }

public:

    PrivacyRepositorySnapshot snapshot;
    PrivacyTransaction begunTransaction;
    PrivacyTransactionState failState = static_cast<PrivacyTransactionState>(0);
};

struct Fixture
{
    QTemporaryDir directory;
    FakePersistence persistence;
    PrivacyStorageRoot publicRoot;
    PrivacyJournalRootExpectation publicExpectation;
    PrivacyStorageRoot storeRoot;
    PrivacyJournalRootExpectation storeExpectation;
    PrivacyExternalCheckoutStoreAccess storeAccess;
    PrivacyExternalCheckoutRequest request;
    QString proxyPath;
    QString plaintextRoot;

    bool initialize()
    {
        if (!directory.isValid() ||
            !QDir(directory.path()).mkpath(QStringLiteral("public/album")) ||
            !QDir(directory.path()).mkpath(QStringLiteral("store")) ||
            !QDir(directory.path()).mkpath(QStringLiteral("mount")))
        {
            return false;
        }

        const QString publicPath = QDir(directory.path()).filePath(
            QStringLiteral("public"));
        const QString storePath = QDir(directory.path()).filePath(
            QStringLiteral("store"));
        plaintextRoot = QDir(directory.path()).filePath(QStringLiteral("mount"));
        proxyPath = QDir(publicPath).filePath(QStringLiteral("album/photo.jpg"));

        if (!writeFile(proxyPath, ProxyBytes))
        {
            return false;
        }

        struct stat publicStat = {};
        struct stat storeStat = {};

        if ((::stat(QFile::encodeName(publicPath).constData(), &publicStat) != 0) ||
            (::stat(QFile::encodeName(storePath).constData(), &storeStat) != 0))
        {
            return false;
        }

        const QDateTime now = QDateTime::currentDateTimeUtc();
        publicRoot.uuid = RootUuid;
        publicRoot.kind = PrivacyStorageRootKind::AlbumRoot;
        publicRoot.albumRootId = 1;
        publicRoot.configuredPath = publicPath;
        publicRoot.identityVersion = 1;
        publicRoot.identityData = QByteArrayLiteral("synthetic-root-v1");
        publicRoot.createdAt = now;
        publicExpectation.rootUuid = RootUuid;
        publicExpectation.device = static_cast<quint64>(publicStat.st_dev);
        publicExpectation.inode = static_cast<quint64>(publicStat.st_ino);
        publicExpectation.identitySha256 = digest(publicRoot.identityData);

        storeRoot.uuid = StoreRootUuid;
        storeRoot.kind = PrivacyStorageRootKind::ManagedStoreRoot;
        storeRoot.albumRootId = -1;
        storeRoot.configuredPath = storePath;
        storeRoot.identityVersion = 1;
        storeRoot.identityData = QByteArrayLiteral("synthetic-store-root-v1");
        storeRoot.markerUuid = StoreMarkerUuid;
        storeRoot.createdAt = now;

        const QString metadataPath = QDir(storePath).filePath(
            QStringLiteral(".digikam-private"));

        if (!QDir().mkpath(metadataPath) ||
            (::chmod(QFile::encodeName(metadataPath).constData(), 0700) != 0))
        {
            return false;
        }

        QJsonObject marker;
        marker.insert(QStringLiteral("kind"),
                      QStringLiteral("digikam-private-root-marker-v1"));
        marker.insert(QStringLiteral("markerUuid"), StoreMarkerUuid);
        marker.insert(QStringLiteral("rootUuid"), StoreRootUuid);
        const QString markerPath = QDir(metadataPath).filePath(
            QStringLiteral("root-marker-v1.json"));

        if (!writeFile(markerPath,
                       QJsonDocument(marker).toJson(QJsonDocument::Compact)) ||
            (::chmod(QFile::encodeName(markerPath).constData(), 0600) != 0))
        {
            return false;
        }

        storeExpectation.rootUuid = StoreRootUuid;
        storeExpectation.markerUuid = StoreMarkerUuid;
        storeExpectation.device = static_cast<quint64>(storeStat.st_dev);
        storeExpectation.inode = static_cast<quint64>(storeStat.st_ino);
        storeExpectation.identitySha256 = digest(storeRoot.identityData);

        PrivacyCategory category;
        category.uuid = CategoryUuid;
        category.name = QStringLiteral("Synthetic private category");
        category.backend = PrivacyBackend::Casual;
        category.lifecycleState = PrivacyCategoryLifecycleState::Active;
        category.currentCredentialGeneration = 1;
        category.createdAt = now;

        PrivacyStore store;
        store.uuid = StoreUuid;
        store.categoryUuid = CategoryUuid;
        store.rootUuid = StoreRootUuid;
        store.format = QStringLiteral("gocryptfs");
        store.formatVersion = 1;
        store.cipherRelativePath = QStringLiteral("cipher");
        store.configRelativePath = QStringLiteral("gocryptfs.conf");
        store.configGeneration = 1;
        store.lifecycleState = PrivacyStoreLifecycleState::Active;
        store.createdAt = now;

        PrivacyStoreBinding authority;
        authority.categoryUuid = CategoryUuid;
        authority.role = PrivacyStoreRole::CredentialAuthority;
        authority.storeUuid = StoreUuid;
        PrivacyStoreBinding derivatives = authority;
        derivatives.role = PrivacyStoreRole::Derivatives;

        PrivacyItem item;
        item.imageId = 42;
        item.uuid = ItemUuid;
        item.categoryUuid = CategoryUuid;
        item.generation = 3;
        item.transactionState = 0;

        PrivacyContainer container;
        container.uuid = ContainerUuid;
        container.itemUuid = ItemUuid;
        container.kind = PrivacyContainerKind::CasualArchive;
        container.rootUuid = RootUuid;
        container.objectRelativePath = QStringLiteral("album/photo.jpg.digikam-private.zip");
        container.protectedSize = 99;
        container.protectedHashAlgorithm = QStringLiteral("sha256");
        container.protectedHash = QString::fromLatin1(digest(QByteArrayLiteral("container")).toHex());
        container.formatVersion = 1;
        container.credentialGeneration = 1;
        container.state = PrivacyContainerState::Verified;
        container.createdAt = now;
        container.updatedAt = now;

        PrivacyAsset asset;
        asset.itemUuid = ItemUuid;
        asset.role = PrivacyAsset::PrimaryMediaRole;
        asset.ordinal = 0;
        asset.originalName = QStringLiteral("photo.jpg");
        asset.publicRootUuid = RootUuid;
        asset.publicRelativePath = QStringLiteral("album/photo.jpg");
        asset.containerUuid = ContainerUuid;
        asset.protectedRelativePath = QStringLiteral("assets/1/0/photo.jpg");
        asset.hashAlgorithm = QStringLiteral("sha256");
        asset.originalHash = QString::fromLatin1(digest(OriginalBytes).toHex());
        asset.originalSize = OriginalBytes.size();
        asset.originalModificationDate = now;

        persistence.snapshot.categories << category;
        persistence.snapshot.storageRoots << publicRoot << storeRoot;
        persistence.snapshot.stores << store;
        persistence.snapshot.storeBindings << authority << derivatives;
        persistence.snapshot.items << item;
        persistence.snapshot.containers << container;
        persistence.snapshot.assets << asset;

        PrivacyExternalCheckoutAssetSource source;
        source.role = asset.role;
        source.ordinal = asset.ordinal;
        source.producer = [](int descriptor, QString* detail)
        {
            const ssize_t count = ::write(descriptor, OriginalBytes.constData(),
                                          static_cast<size_t>(OriginalBytes.size()));

            if (count != OriginalBytes.size())
            {
                if (detail)
                {
                    *detail = QStringLiteral("synthetic producer write failed");
                }

                return false;
            }

            return true;
        };

        request.imageId = item.imageId;
        request.categoryUuid = CategoryUuid;
        request.transactionUuid = TransactionUuid;
        request.publicRoot = publicRoot;
        request.publicRootExpectation = publicExpectation;
        request.storeUuid = StoreUuid;
        request.storeRoot = storeRoot;
        request.storeRootExpectation = storeExpectation;
        request.storePlaintextRoot = plaintextRoot;
        request.sources << source;
        storeAccess.storeUuid = StoreUuid;
        storeAccess.root = storeRoot;
        storeAccess.rootExpectation = storeExpectation;
        storeAccess.plaintextRoot = plaintextRoot;
        return true;
    }

    void addSidecar()
    {
        PrivacyAsset asset = persistence.snapshot.assets.constFirst();
        asset.role = 2;
        asset.originalName = QStringLiteral("photo.xmp");
        asset.publicRelativePath = QStringLiteral("album/photo.xmp");
        asset.protectedRelativePath = QStringLiteral("assets/2/0/photo.xmp");
        asset.originalHash = QString::fromLatin1(digest(SidecarBytes).toHex());
        asset.originalSize = SidecarBytes.size();
        persistence.snapshot.assets << asset;

        PrivacyExternalCheckoutAssetSource source;
        source.role = asset.role;
        source.ordinal = asset.ordinal;
        source.producer = [](int descriptor, QString*)
        {
            return (::write(descriptor, SidecarBytes.constData(),
                            static_cast<size_t>(SidecarBytes.size())) ==
                    SidecarBytes.size());
        };
        request.sources << source;
    }
};

} // namespace

class PrivacyExternalCheckoutTransactionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreateAuthorizeAndUnchangedCleanup();
    void testChangedAndUnexpectedContentArePreserved();
    void testCreatedAndExposedRestartRecovery();
    void testAuthenticatedResumeRecreatesMissingCheckout();
    void testRelockingRestartCompletesMissingCleanup();
};

void PrivacyExternalCheckoutTransactionTest::testCreateAuthorizeAndUnchangedCleanup()
{
    Fixture fixture;
    QVERIFY(fixture.initialize());
    PrivacyExternalCheckoutTransactionEngine engine(fixture.persistence);
    const PrivacyExternalCheckoutResult created = engine.create(fixture.request);
    QVERIFY2(created.status == PrivacyExternalCheckoutStatus::Ready,
             qPrintable(created.detail));
    QCOMPARE(created.assets.size(), 1);
    QCOMPARE(readFile(fixture.proxyPath), ProxyBytes);
    const QString checkout = created.assets.constFirst().checkoutUrl.toLocalFile();
    QCOMPARE(readFile(checkout), OriginalBytes);
    QCOMPARE(fixture.persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Prepared);

    const PrivacyExternalCheckoutResult authorized = engine.authorizeLaunch(
        fixture.storeAccess, TransactionUuid);
    QVERIFY2(authorized.status == PrivacyExternalCheckoutStatus::Ready,
             qPrintable(authorized.detail));
    QCOMPARE(fixture.persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Exposed);
    const PrivacyExternalCheckoutResult reconciled = engine.reconcile(
        fixture.storeAccess, TransactionUuid);
    QVERIFY2(reconciled.status == PrivacyExternalCheckoutStatus::CompletedUnchanged,
             qPrintable(reconciled.detail));
    QVERIFY(!QFileInfo::exists(checkout));
    QCOMPARE(readFile(fixture.proxyPath), ProxyBytes);
}

void PrivacyExternalCheckoutTransactionTest::testChangedAndUnexpectedContentArePreserved()
{
    for (const bool createUnexpected : { false, true })
    {
        Fixture fixture;
        QVERIFY(fixture.initialize());
        PrivacyExternalCheckoutTransactionEngine engine(fixture.persistence);
        const PrivacyExternalCheckoutResult created = engine.create(fixture.request);
        QVERIFY2(created.succeeded(), qPrintable(created.detail));
        QVERIFY(engine.authorizeLaunch(fixture.storeAccess,
                                       TransactionUuid).succeeded());
        const QString checkout = created.assets.constFirst().checkoutUrl.toLocalFile();

        if (createUnexpected)
        {
            QVERIFY(writeFile(QFileInfo(checkout).dir().filePath(QStringLiteral("new-output.txt")),
                              QByteArrayLiteral("new output")));
        }
        else
        {
            QVERIFY(writeFile(checkout, QByteArrayLiteral("externally changed")));
        }

        const PrivacyExternalCheckoutResult reconciled = engine.reconcile(
            fixture.storeAccess, TransactionUuid);
        QCOMPARE(reconciled.status, PrivacyExternalCheckoutStatus::ChangesPending);
        const PrivacyExternalCheckoutResult recovered = engine.recover(
            fixture.storeRoot, fixture.storeExpectation, TransactionUuid);
        QCOMPARE(recovered.status, PrivacyExternalCheckoutStatus::ChangesPending);
        QVERIFY(QFileInfo::exists(checkout));
        QCOMPARE(fixture.persistence.snapshot.transactions.constFirst().state,
                 PrivacyTransactionState::NeedsReconciliation);
        QCOMPARE(readFile(fixture.proxyPath), ProxyBytes);
    }
}

void PrivacyExternalCheckoutTransactionTest::testCreatedAndExposedRestartRecovery()
{
    Fixture fixture;
    QVERIFY(fixture.initialize());
    PrivacyExternalCheckoutTransactionEngine engine(fixture.persistence);
    const PrivacyExternalCheckoutResult created = engine.create(fixture.request);
    QVERIFY2(created.succeeded(), qPrintable(created.detail));
    fixture.persistence.snapshot.transactions[0] = fixture.persistence.begunTransaction;
    PrivacyExternalCheckoutTransactionEngine restarted(fixture.persistence);
    const PrivacyExternalCheckoutResult recovered = restarted.recover(
        fixture.storeRoot, fixture.storeExpectation, TransactionUuid);
    QVERIFY2(recovered.status == PrivacyExternalCheckoutStatus::AuthenticationRequired,
             qPrintable(recovered.detail));
    const PrivacyExternalCheckoutResult resumed =
        restarted.resumeAuthenticatedCreate(fixture.request);
    QVERIFY2(resumed.status == PrivacyExternalCheckoutStatus::Ready,
             qPrintable(resumed.detail));
    QVERIFY2(restarted.authorizeLaunch(fixture.storeAccess,
                                      TransactionUuid).succeeded(),
             "resumed checkout could not be authorized");
    const PrivacyExternalCheckoutResult reconciled = restarted.reconcile(
        fixture.storeAccess, TransactionUuid);
    QVERIFY2(reconciled.status == PrivacyExternalCheckoutStatus::CompletedUnchanged,
             qPrintable(reconciled.detail));
    QVERIFY(!QFileInfo::exists(created.assets.constFirst().checkoutUrl.toLocalFile()));
}

void PrivacyExternalCheckoutTransactionTest::testAuthenticatedResumeRecreatesMissingCheckout()
{
    Fixture fixture;
    QVERIFY(fixture.initialize());
    fixture.addSidecar();
    PrivacyExternalCheckoutTransactionEngine engine(fixture.persistence);
    const PrivacyExternalCheckoutResult created = engine.create(fixture.request);
    QVERIFY2(created.succeeded(), qPrintable(created.detail));
    QCOMPARE(created.assets.size(), 2);
    const QString retained = created.assets.constFirst().checkoutUrl.toLocalFile();
    const QString missing = created.assets.constLast().checkoutUrl.toLocalFile();
    struct stat retainedBefore = {};
    QVERIFY(::stat(QFile::encodeName(retained).constData(), &retainedBefore) == 0);
    QVERIFY(QFile::remove(missing));
    fixture.persistence.snapshot.transactions[0] = fixture.persistence.begunTransaction;
    PrivacyExternalCheckoutTransactionEngine restarted(fixture.persistence);
    QCOMPARE(restarted.recover(fixture.storeRoot, fixture.storeExpectation,
                               TransactionUuid).status,
             PrivacyExternalCheckoutStatus::AuthenticationRequired);
    const PrivacyExternalCheckoutResult resumed =
        restarted.resumeAuthenticatedCreate(fixture.request);
    QVERIFY2(resumed.status == PrivacyExternalCheckoutStatus::Ready,
             qPrintable(resumed.detail));
    struct stat retainedAfter = {};
    QVERIFY(::stat(QFile::encodeName(retained).constData(), &retainedAfter) == 0);
    QCOMPARE(retainedAfter.st_ino, retainedBefore.st_ino);
    QCOMPARE(readFile(retained), OriginalBytes);
    QCOMPARE(readFile(missing), SidecarBytes);
    QCOMPARE(readFile(fixture.proxyPath), ProxyBytes);
    QCOMPARE(fixture.persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Prepared);
}

void PrivacyExternalCheckoutTransactionTest::testRelockingRestartCompletesMissingCleanup()
{
    Fixture fixture;
    QVERIFY(fixture.initialize());
    PrivacyExternalCheckoutTransactionEngine engine(fixture.persistence);
    const PrivacyExternalCheckoutResult created = engine.create(fixture.request);
    QVERIFY2(created.succeeded(), qPrintable(created.detail));
    QVERIFY(engine.authorizeLaunch(fixture.storeAccess,
                                   TransactionUuid).succeeded());
    fixture.persistence.failState = PrivacyTransactionState::Complete;
    QCOMPARE(engine.reconcile(fixture.storeAccess, TransactionUuid).status,
             PrivacyExternalCheckoutStatus::PersistenceFailure);
    QVERIFY(!QFileInfo::exists(created.assets.constFirst().checkoutUrl.toLocalFile()));
    QCOMPARE(fixture.persistence.snapshot.transactions.constFirst().state,
             PrivacyTransactionState::Relocking);
    fixture.persistence.failState = static_cast<PrivacyTransactionState>(0);
    PrivacyExternalCheckoutTransactionEngine restarted(fixture.persistence);
    const PrivacyExternalCheckoutResult recovered = restarted.recover(
        fixture.storeRoot, fixture.storeExpectation, TransactionUuid);
    QVERIFY2(recovered.status == PrivacyExternalCheckoutStatus::AuthenticationRequired,
             qPrintable(recovered.detail));
    const PrivacyExternalCheckoutResult reconciled = restarted.reconcile(
        fixture.storeAccess, TransactionUuid);
    QVERIFY2(reconciled.status == PrivacyExternalCheckoutStatus::CompletedUnchanged,
             qPrintable(reconciled.detail));
}

QTEST_MAIN(PrivacyExternalCheckoutTransactionTest)

#include "privacyexternalcheckouttransaction_utest.moc"

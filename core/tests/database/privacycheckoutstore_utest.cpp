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
#include <QTemporaryDir>
#include <QTest>

#if defined(Q_OS_UNIX)

#   include <unistd.h>

#endif

// Local includes

#include "privacycheckoutstore.h"

using namespace Digikam;

namespace
{

const QString TransactionUuid =
    QLatin1String("70000000-0000-0000-0000-000000000001");

bool ownerOnly(const QString& path)
{
    return QFile::setPermissions(path, QFileDevice::ReadOwner |
                                       QFileDevice::WriteOwner |
                                       QFileDevice::ExeOwner);
}

bool writeAll(int descriptor, const QByteArray& bytes)
{
#if defined(Q_OS_UNIX)
    qint64 offset = 0;

    while (offset < bytes.size())
    {
        const ssize_t count = ::write(
            descriptor, bytes.constData() + offset,
            static_cast<size_t>(bytes.size() - offset));

        if (count <= 0)
        {
            return false;
        }

        offset += count;
    }

    return true;
#else
    Q_UNUSED(descriptor);
    Q_UNUSED(bytes);
    return false;
#endif
}

std::unique_ptr<PrivacyCheckoutStore> openStore(QTemporaryDir& directory,
                                                QString* detail)
{
    if (!directory.isValid() || !ownerOnly(directory.path()))
    {
        return {};
    }

    PrivacyCheckoutStoreError error = PrivacyCheckoutStoreError::None;
    return PrivacyCheckoutStore::open(directory.path(), &error, detail);
}

} // namespace

class PrivacyCheckoutStoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCanonicalRelativePaths();
    void testMaterializeInventoryAndRuntimeResolution();
    void testInventoryDoesNotTraverseLinksAndBoundsDeletion();
    void testAtomicMoveToRecoveryAndConflict();
};

void PrivacyCheckoutStoreTest::testCanonicalRelativePaths()
{
    QCOMPARE(PrivacyCheckoutStore::transactionRelativePath(TransactionUuid),
             QStringLiteral("checkouts/%1").arg(TransactionUuid));
    QCOMPARE(PrivacyCheckoutStore::workRelativePath(TransactionUuid),
             QStringLiteral("checkouts/%1/work").arg(TransactionUuid));
    QCOMPARE(PrivacyCheckoutStore::workFileRelativePath(
                 TransactionUuid, QLatin1String("item.jpg")),
             QStringLiteral("checkouts/%1/work/item.jpg").arg(TransactionUuid));
    QCOMPARE(PrivacyCheckoutStore::transactionRelativePath(
                 TransactionUuid, PrivacyCheckoutStoreLocation::Recovery),
             QStringLiteral("recovery/%1").arg(TransactionUuid));
    QVERIFY(PrivacyCheckoutStore::workFileRelativePath(
                TransactionUuid, QLatin1String("../item.jpg")).isEmpty());
    QVERIFY(PrivacyCheckoutStore::workRelativePath(
                QLatin1String("not-a-uuid")).isEmpty());
}

void PrivacyCheckoutStoreTest::testMaterializeInventoryAndRuntimeResolution()
{
    QTemporaryDir directory;
    QString detail;
    std::unique_ptr<PrivacyCheckoutStore> store = openStore(directory, &detail);
    QVERIFY2(store, qPrintable(detail));
    QString workPath;
    QVERIFY2(store->createOrOpenTransaction(TransactionUuid, &workPath,
                                             nullptr, &detail),
             qPrintable(detail));
    QCOMPARE(workPath, PrivacyCheckoutStore::workRelativePath(TransactionUuid));
    QVERIFY(store->reopenTransaction(
        TransactionUuid, PrivacyCheckoutStoreLocation::Checkout,
        nullptr, nullptr, &detail));

    const QByteArray original = QByteArrayLiteral("synthetic original bytes");
    const QByteArray hash = QCryptographicHash::hash(
        original, QCryptographicHash::Sha256);
    int producerCalls = 0;
    QString relative;
    const PrivacyCheckoutStore::FileProducer producer =
        [&original, &producerCalls](int descriptor, QString*)
        {
            ++producerCalls;
            return writeAll(descriptor, original);
        };
    QVERIFY2(store->createFile(TransactionUuid, QLatin1String("item.jpg"),
                               original.size(), hash, producer, &relative,
                               nullptr, &detail), qPrintable(detail));
    QCOMPARE(relative, PrivacyCheckoutStore::workFileRelativePath(
                          TransactionUuid, QLatin1String("item.jpg")));
    QVERIFY(store->createFile(TransactionUuid, QLatin1String("item.jpg"),
                              original.size(), hash, producer, nullptr,
                              nullptr, &detail));
    QCOMPARE(producerCalls, 1);

    PrivacyCheckoutInventory inventory;
    QVERIFY2(store->inventory(TransactionUuid,
                              PrivacyCheckoutStoreLocation::Checkout,
                              &inventory, nullptr, &detail), qPrintable(detail));
    QCOMPARE(inventory.entries.size(), 1);
    QCOMPARE(inventory.entries.constFirst().storeRelativePath, relative);
    QCOMPARE(inventory.entries.constFirst().kind,
             PrivacyCheckoutEntryKind::RegularFile);
    QCOMPARE(inventory.entries.constFirst().sha256, hash);
    QVERIFY(store->validateInventory(inventory, nullptr, &detail));

    const QString runtime = store->runtimePathForEntry(
        inventory, relative, nullptr, &detail);
    QVERIFY2(!runtime.isEmpty(), qPrintable(detail));
    QVERIFY(QDir::isAbsolutePath(runtime));
    QFile file(runtime);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), original);

    QFile changed(runtime);
    QVERIFY(changed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(changed.write("changed"), qint64(7));
    changed.close();
    PrivacyCheckoutStoreError error = PrivacyCheckoutStoreError::None;
    QVERIFY(!store->createFile(TransactionUuid, QLatin1String("item.jpg"),
                               original.size(), hash, producer, nullptr,
                               &error, &detail));
    QCOMPARE(error, PrivacyCheckoutStoreError::Conflict);
}

void PrivacyCheckoutStoreTest::testInventoryDoesNotTraverseLinksAndBoundsDeletion()
{
#if !defined(Q_OS_UNIX)
    QSKIP("POSIX descriptor-confinement test");
#else
    QTemporaryDir directory;
    QTemporaryDir outside;
    QString detail;
    std::unique_ptr<PrivacyCheckoutStore> store = openStore(directory, &detail);
    QVERIFY2(store, qPrintable(detail));
    QVERIFY(store->createOrOpenTransaction(TransactionUuid, nullptr,
                                            nullptr, &detail));
    const QString work = directory.filePath(
        PrivacyCheckoutStore::workRelativePath(TransactionUuid));
    QVERIFY(QDir(work).mkpath(QLatin1String("outputs/nested")));
    const QString outputPath = work + QLatin1String("/outputs/nested/result.xmp");
    QFile output(outputPath);
    QVERIFY(output.open(QIODevice::WriteOnly));
    QVERIFY(output.write("sidecar") == 7);
    output.close();
    const QString outsidePath = outside.filePath(QLatin1String("valuable.txt"));
    QFile valuable(outsidePath);
    QVERIFY(valuable.open(QIODevice::WriteOnly));
    QVERIFY(valuable.write("outside bytes") == 13);
    valuable.close();
    const QString linkPath = work + QLatin1String("/outside-link");
    QVERIFY(QFile::link(outsidePath, linkPath));

    PrivacyCheckoutInventory inventory;
    QVERIFY2(store->inventory(TransactionUuid,
                              PrivacyCheckoutStoreLocation::Checkout,
                              &inventory, nullptr, &detail), qPrintable(detail));
    QCOMPARE(inventory.entries.size(), 4);
    const auto link = std::find_if(
        inventory.entries.cbegin(), inventory.entries.cend(),
        [](const PrivacyCheckoutInventoryEntry& entry)
        {
            return (entry.kind == PrivacyCheckoutEntryKind::SymbolicLink);
        });
    QVERIFY(link != inventory.entries.cend());
    QVERIFY(store->runtimePathForEntry(inventory, link->storeRelativePath,
                                       nullptr, &detail).isEmpty());

    valuable.setFileName(outsidePath);
    QVERIFY(valuable.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(valuable.write("changed outside") == 15);
    valuable.close();
    QVERIFY2(store->validateInventory(inventory, nullptr, &detail),
             qPrintable(detail));

    QFile unexpected(work + QLatin1String("/late-output"));
    QVERIFY(unexpected.open(QIODevice::WriteOnly));
    QVERIFY(unexpected.write("late") == 4);
    unexpected.close();
    PrivacyCheckoutStoreError error = PrivacyCheckoutStoreError::None;
    QVERIFY(!store->removeExact(inventory, &error, &detail));
    QCOMPARE(error, PrivacyCheckoutStoreError::Conflict);
    QVERIFY(QFileInfo::exists(outputPath));
    QVERIFY(QFileInfo::exists(linkPath));

    QVERIFY(QFile::remove(unexpected.fileName()));
    QVERIFY(store->inventory(TransactionUuid,
                             PrivacyCheckoutStoreLocation::Checkout,
                             &inventory, nullptr, &detail));
    QVERIFY2(store->removeExact(inventory, nullptr, &detail),
             qPrintable(detail));
    QVERIFY(!QFileInfo::exists(work));
    valuable.setFileName(outsidePath);
    QVERIFY(valuable.open(QIODevice::ReadOnly));
    QCOMPARE(valuable.readAll(), QByteArrayLiteral("changed outside"));
#endif
}

void PrivacyCheckoutStoreTest::testAtomicMoveToRecoveryAndConflict()
{
    QTemporaryDir directory;
    QString detail;
    std::unique_ptr<PrivacyCheckoutStore> store = openStore(directory, &detail);
    QVERIFY2(store, qPrintable(detail));
    QVERIFY(store->createOrOpenTransaction(TransactionUuid, nullptr,
                                            nullptr, &detail));
    QString recovery;
    QVERIFY2(store->moveToRecovery(TransactionUuid, &recovery,
                                   nullptr, &detail), qPrintable(detail));
    QCOMPARE(recovery, PrivacyCheckoutStore::transactionRelativePath(
                           TransactionUuid,
                           PrivacyCheckoutStoreLocation::Recovery));
    PrivacyCheckoutStoreError error = PrivacyCheckoutStoreError::None;
    QVERIFY(!store->reopenTransaction(
        TransactionUuid, PrivacyCheckoutStoreLocation::Checkout,
        nullptr, &error, &detail));
    QCOMPARE(error, PrivacyCheckoutStoreError::Missing);
    QVERIFY(store->reopenTransaction(
        TransactionUuid, PrivacyCheckoutStoreLocation::Recovery,
        nullptr, nullptr, &detail));

    QVERIFY(store->createOrOpenTransaction(TransactionUuid, nullptr,
                                            nullptr, &detail));
    QVERIFY(!store->moveToRecovery(TransactionUuid, nullptr, &error, &detail));
    QCOMPARE(error, PrivacyCheckoutStoreError::Conflict);
    QVERIFY(store->reopenTransaction(
        TransactionUuid, PrivacyCheckoutStoreLocation::Checkout,
        nullptr, nullptr, &detail));
}

QTEST_GUILESS_MAIN(PrivacyCheckoutStoreTest)

#include "privacycheckoutstore_utest.moc"

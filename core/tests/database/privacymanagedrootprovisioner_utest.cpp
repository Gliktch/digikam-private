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
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

// C++ includes

#include <thread>

// POSIX includes

#include <sys/stat.h>
#include <unistd.h>

// Local includes

#include "privacycontracts.h"
#include "privacymanagedrootprovisioner.h"
#include "privacyruntime.h"

using namespace Digikam;

namespace
{

bool makePrivateDirectory(const QString& path)
{
    return (QDir().mkdir(path) &&
            QFile::setPermissions(path, QFileDevice::ReadOwner |
                                        QFileDevice::WriteOwner |
                                        QFileDevice::ExeOwner));
}

bool writePrivateFile(const QString& path, const QByteArray& bytes)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
        (file.write(bytes) != bytes.size()))
    {
        return false;
    }

    file.close();

    return QFile::setPermissions(path, QFileDevice::ReadOwner |
                                       QFileDevice::WriteOwner);
}

QString uuidText()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

class ScopedUmask
{
public:

    explicit ScopedUmask(mode_t value)
        : previous(::umask(value))
    {
    }

    ~ScopedUmask()
    {
        ::umask(previous);
    }

private:

    mode_t previous = 0;
};

} // namespace

class PrivacyManagedRootProvisionerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreateReuseAndRuntimeVerification();
    void testRestrictiveUmaskIsCorrected();
    void testConcurrentProvisioningConverges();
    void testRollbackRequiresUnusedExactRoot();
    void testRejectsInvalidAndUnsafeRoots();
    void testRejectsOccupiedMalformedAndHardlinkedMarkers();
};

void PrivacyManagedRootProvisionerTest::testCreateReuseAndRuntimeVerification()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = temporary.filePath(QLatin1String("store-root"));
    QVERIFY(makePrivateDirectory(rootPath));

    const PrivacyManagedRootProvisionResult created =
        PrivacyManagedRootProvisioner::provision(rootPath + QLatin1String("/./"));
    QVERIFY2(created.succeeded(), qPrintable(created.detail));
    QCOMPARE(created.status, PrivacyManagedRootProvisionStatus::ReadyCreated);
    QVERIFY(created.createdMarker());
    QVERIFY(created.root.isValid());
    QCOMPARE(created.root.configuredPath, rootPath);

    const QFileInfo metadata(rootPath + QLatin1String("/.digikam-private"));
    const QFileInfo marker(rootPath +
                           QLatin1String("/.digikam-private/root-marker-v1.json"));
    QVERIFY(metadata.isDir());
    QVERIFY(marker.isFile());
    QCOMPARE(metadata.permissions() & (QFileDevice::WriteGroup |
                                       QFileDevice::WriteOther),
             QFileDevice::Permissions());
    QCOMPARE(marker.permissions() & (QFileDevice::ReadGroup |
                                     QFileDevice::WriteGroup |
                                     QFileDevice::ExeGroup |
                                     QFileDevice::ReadOther |
                                     QFileDevice::WriteOther |
                                     QFileDevice::ExeOther),
             QFileDevice::Permissions());

    const QSharedPointer<const PrivacyRootVerifier> verifier =
        createDefaultPrivacyRootVerifier();
    QVERIFY(verifier);
    QCOMPARE(verifier->verify(created.root),
             PrivacyRootRuntimeState::VerifiedAvailable);

    const PrivacyManagedRootProvisionResult reused =
        PrivacyManagedRootProvisioner::provision(rootPath);
    QVERIFY2(reused.succeeded(), qPrintable(reused.detail));
    QCOMPARE(reused.status, PrivacyManagedRootProvisionStatus::ReadyExisting);
    QVERIFY(!reused.createdMarker());
    QCOMPARE(reused.root.uuid, created.root.uuid);
    QCOMPARE(reused.root.markerUuid, created.root.markerUuid);
    QCOMPARE(reused.root.identityData, created.root.identityData);
}

void PrivacyManagedRootProvisionerTest::testRestrictiveUmaskIsCorrected()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = temporary.filePath(QLatin1String("umask-root"));
    QVERIFY(makePrivateDirectory(rootPath));
    PrivacyManagedRootProvisionResult result;

    {
        const ScopedUmask restrictive(0777);
        result = PrivacyManagedRootProvisioner::provision(rootPath);
    }

    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    const QFileInfo metadata(rootPath + QLatin1String("/.digikam-private"));
    const QFileInfo marker(rootPath +
                           QLatin1String("/.digikam-private/root-marker-v1.json"));
    const QFileDevice::Permissions allPermissions =
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
        QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
        QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;
    QCOMPARE(metadata.permissions() & allPermissions,
             QFileDevice::Permissions(QFileDevice::ReadOwner |
                                      QFileDevice::WriteOwner |
                                      QFileDevice::ExeOwner));
    QCOMPARE(marker.permissions() & allPermissions,
             QFileDevice::Permissions(QFileDevice::ReadOwner |
                                      QFileDevice::WriteOwner));
}

void PrivacyManagedRootProvisionerTest::testConcurrentProvisioningConverges()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = temporary.filePath(QLatin1String("concurrent-root"));
    QVERIFY(makePrivateDirectory(rootPath));

    PrivacyManagedRootProvisionResult first;
    PrivacyManagedRootProvisionResult second;
    std::thread firstThread([&first, &rootPath]()
    {
        first = PrivacyManagedRootProvisioner::provision(rootPath);
    });
    std::thread secondThread([&second, &rootPath]()
    {
        second = PrivacyManagedRootProvisioner::provision(rootPath);
    });
    firstThread.join();
    secondThread.join();

    QVERIFY2(first.succeeded(), qPrintable(first.detail));
    QVERIFY2(second.succeeded(), qPrintable(second.detail));
    QCOMPARE(first.root.uuid, second.root.uuid);
    QCOMPARE(first.root.markerUuid, second.root.markerUuid);
    QCOMPARE(static_cast<int>(first.createdMarker()) +
             static_cast<int>(second.createdMarker()), 1);
}

void PrivacyManagedRootProvisionerTest::testRollbackRequiresUnusedExactRoot()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString rootPath = temporary.filePath(QLatin1String("rollback-root"));
    QVERIFY(makePrivateDirectory(rootPath));

    const PrivacyManagedRootProvisionResult result =
        PrivacyManagedRootProvisioner::provision(rootPath);
    QVERIFY2(result.succeeded(), qPrintable(result.detail));
    QVERIFY(result.createdMarker());

    const QString candidate = rootPath +
        QLatin1String("/.digikam-private/root-marker-v1.json.tmp-") + uuidText();
    QVERIFY(writePrivateFile(candidate, QByteArrayLiteral("stale-candidate")));
    QVERIFY(!PrivacyManagedRootProvisioner::rollbackUnused(result));
    QVERIFY(QFile::remove(candidate));

    const QString blocker = rootPath + QLatin1String("/.digikam-private/store-data");
    QVERIFY(writePrivateFile(blocker, QByteArrayLiteral("occupied")));
    QVERIFY(!PrivacyManagedRootProvisioner::rollbackUnused(result));
    QVERIFY(QFileInfo::exists(rootPath +
                             QLatin1String("/.digikam-private/root-marker-v1.json")));
    QVERIFY(QFile::remove(blocker));
    QVERIFY(PrivacyManagedRootProvisioner::rollbackUnused(result));
    QVERIFY(!QFileInfo::exists(rootPath + QLatin1String("/.digikam-private")));
    QVERIFY(!PrivacyManagedRootProvisioner::rollbackUnused(result));
}

void PrivacyManagedRootProvisionerTest::testRejectsInvalidAndUnsafeRoots()
{
    QCOMPARE(PrivacyManagedRootProvisioner::provision(QLatin1String("relative")).status,
             PrivacyManagedRootProvisionStatus::InvalidPath);
    QCOMPARE(PrivacyManagedRootProvisioner::provision(QLatin1String("/")).status,
             PrivacyManagedRootProvisionStatus::InvalidPath);

    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    QCOMPARE(PrivacyManagedRootProvisioner::provision(
                 temporary.filePath(QLatin1String("missing"))).status,
             PrivacyManagedRootProvisionStatus::PathUnavailable);

    const QString unsafePath = temporary.filePath(QLatin1String("unsafe"));
    QVERIFY(QDir().mkdir(unsafePath));
    QVERIFY(QFile::setPermissions(unsafePath, QFileDevice::ReadOwner |
                                             QFileDevice::WriteOwner |
                                             QFileDevice::ExeOwner |
                                             QFileDevice::WriteGroup));
    QCOMPARE(PrivacyManagedRootProvisioner::provision(unsafePath).status,
             PrivacyManagedRootProvisionStatus::UnsafeRoot);

    const QString targetPath = temporary.filePath(QLatin1String("target"));
    const QString linkPath = temporary.filePath(QLatin1String("link"));
    QVERIFY(makePrivateDirectory(targetPath));
    QVERIFY(::symlink(QFile::encodeName(targetPath).constData(),
                      QFile::encodeName(linkPath).constData()) == 0);
    QCOMPARE(PrivacyManagedRootProvisioner::provision(linkPath).status,
             PrivacyManagedRootProvisionStatus::UnsafeRoot);
}

void PrivacyManagedRootProvisionerTest::
testRejectsOccupiedMalformedAndHardlinkedMarkers()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());

    const QString occupiedRoot = temporary.filePath(QLatin1String("occupied-root"));
    QVERIFY(makePrivateDirectory(occupiedRoot));
    QVERIFY(makePrivateDirectory(occupiedRoot + QLatin1String("/.digikam-private")));
    QVERIFY(writePrivateFile(occupiedRoot + QLatin1String("/.digikam-private/unknown"),
                             QByteArrayLiteral("unknown")));
    QCOMPARE(PrivacyManagedRootProvisioner::provision(occupiedRoot).status,
             PrivacyManagedRootProvisionStatus::InvalidMarker);

    const QString malformedRoot = temporary.filePath(QLatin1String("malformed-root"));
    QVERIFY(makePrivateDirectory(malformedRoot));
    QVERIFY(makePrivateDirectory(malformedRoot + QLatin1String("/.digikam-private")));
    QVERIFY(writePrivateFile(
        malformedRoot + QLatin1String("/.digikam-private/root-marker-v1.json"),
        QByteArrayLiteral("not-json")));
    QCOMPARE(PrivacyManagedRootProvisioner::provision(malformedRoot).status,
             PrivacyManagedRootProvisionStatus::InvalidMarker);

    const QString linkedRoot = temporary.filePath(QLatin1String("linked-root"));
    QVERIFY(makePrivateDirectory(linkedRoot));
    QVERIFY(makePrivateDirectory(linkedRoot + QLatin1String("/.digikam-private")));
    const QByteArray validMarker = PrivacyRootIdentityCodec::encodeManagedRootMarkerV1(
        uuidText(), uuidText());
    const QString markerPath = linkedRoot +
        QLatin1String("/.digikam-private/root-marker-v1.json");
    const QString linkedPath = temporary.filePath(QLatin1String("marker-link"));
    QVERIFY(writePrivateFile(markerPath, validMarker));
    QVERIFY(::link(QFile::encodeName(markerPath).constData(),
                   QFile::encodeName(linkedPath).constData()) == 0);
    QCOMPARE(PrivacyManagedRootProvisioner::provision(linkedRoot).status,
             PrivacyManagedRootProvisionStatus::InvalidMarker);
}

QTEST_GUILESS_MAIN(PrivacyManagedRootProvisionerTest)

#include "privacymanagedrootprovisioner_utest.moc"

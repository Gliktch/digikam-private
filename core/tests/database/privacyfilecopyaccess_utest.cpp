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
#include <QTimeZone>

// Local includes

#include "fctask.h"

using namespace Digikam;
using namespace DigikamGenericFileCopyPlugin;

namespace
{

class SyntheticSourceHandle final : public DItemAccessSourceHandle
{
public:

    SyntheticSourceHandle(
        const DItemAccessEntry& entry,
        const QList<DItemAssociatedAccessEntry>& associatedEntries)
        : DItemAccessSourceHandle(entry, associatedEntries)
    {
    }
};

class SyntheticAccessHandle final : public DItemAccessHandle
{
public:

    SyntheticAccessHandle(
        const DItemAccessEntry& entry,
        const QList<DItemAssociatedAccessEntry>& associatedEntries)
        : DItemAccessHandle({ entry }, {}, false),
          m_entry(entry),
          m_associatedEntries(associatedEntries)
    {
    }

    QSharedPointer<DItemAccessSourceHandle> acquireSource(
        const QUrl& logicalUrl,
        const QSharedPointer<DItemAccessCancellationToken>& cancellation) const override
    {
        if ((logicalUrl != m_entry.logicalUrl) ||
            (cancellation && cancellation->isCanceled()))
        {
            return {};
        }

        QSharedPointer<DItemAccessSourceHandle> source(
            new SyntheticSourceHandle(m_entry, m_associatedEntries));
        return source->validateAccess()
             ? source : QSharedPointer<DItemAccessSourceHandle>();
    }

private:

    DItemAccessEntry                  m_entry;
    QList<DItemAssociatedAccessEntry> m_associatedEntries;
};

class ExecutableFCTask final : public FCTask
{
public:

    using FCTask::FCTask;

    void execute()
    {
        run();
    }
};

DItemAccessFileFacts facts(qint64 seconds,
                           QFileDevice::Permissions permissions)
{
    DItemAccessFileFacts result;
    result.modificationDate = QDateTime::fromSecsSinceEpoch(
        seconds, QTimeZone::UTC);
    result.permissions = permissions;
    result.available = true;
    return result;
}

bool writeBytes(const QString& path, const QByteArray& bytes)
{
    QFile file(path);
    return (file.open(QIODevice::WriteOnly) &&
            (file.write(bytes) == bytes.size()) && file.flush());
}

QFileDevice::Permissions comparablePermissions()
{
    return QFileDevice::ReadOwner  | QFileDevice::WriteOwner |
           QFileDevice::ExeOwner   | QFileDevice::ReadGroup  |
           QFileDevice::WriteGroup | QFileDevice::ExeGroup   |
           QFileDevice::ReadOther  | QFileDevice::WriteOther |
           QFileDevice::ExeOther;
}

} // namespace

class PrivacyFileCopyAccessTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPreparedAssetSetPreservesBytesAndFacts();
    void testPersistentProxySymlinkRemainsUsable();
};

void PrivacyFileCopyAccessTest::testPreparedAssetSetPreservesBytesAndFacts()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(QDir().mkpath(directory.filePath(QLatin1String("source"))));
    QVERIFY(QDir().mkpath(directory.filePath(QLatin1String("output"))));

    const QString sourceDirectory = directory.filePath(QLatin1String("source"));
    const QString outputDirectory = directory.filePath(QLatin1String("output"));
    const QString primaryPath = QDir(sourceDirectory).filePath(
        QLatin1String("opaque-primary"));
    const QString xmpPath = QDir(sourceDirectory).filePath(
        QLatin1String("opaque-xmp"));
    const QString configuredPath = QDir(sourceDirectory).filePath(
        QLatin1String("opaque-config"));
    const QString companionPath = QDir(sourceDirectory).filePath(
        QLatin1String("opaque-companion"));

    QVERIFY(writeBytes(primaryPath, QByteArrayLiteral("primary-bytes")));
    QVERIFY(writeBytes(xmpPath, QByteArrayLiteral("xmp-bytes")));
    QVERIFY(writeBytes(configuredPath, QByteArrayLiteral("config-bytes")));
    QVERIFY(writeBytes(companionPath, QByteArrayLiteral("companion-bytes")));

    DItemAccessEntry primary;
    primary.logicalUrl = QUrl::fromLocalFile(
        QDir(sourceDirectory).filePath(QLatin1String("sample.jpg")));
    primary.physicalUrl = QUrl::fromLocalFile(primaryPath);
    primary.fileFacts = facts(
        1700000001,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QVERIFY(primary.isValid());

    QList<DItemAssociatedAccessEntry> associatedEntries;

    const auto appendAssociated =
        [&associatedEntries, &sourceDirectory](
            const QString& logicalName,
            const QString& physicalPath,
            DItemAssociatedRole role,
            int ordinal,
            const DItemAccessFileFacts& fileFacts)
        {
            DItemAssociatedAccessEntry associated;
            associated.logicalUrl = QUrl::fromLocalFile(
                QDir(sourceDirectory).filePath(logicalName));
            associated.physicalUrl = QUrl::fromLocalFile(physicalPath);
            associated.role = static_cast<int>(role);
            associated.ordinal = ordinal;
            associated.fileFacts = fileFacts;
            associatedEntries << associated;
        };

    appendAssociated(
        QLatin1String("sample.jpg.xmp"), xmpPath,
        DItemAssociatedRole::XmpSidecar, 0,
        facts(1700000002, QFileDevice::ReadOwner |
                          QFileDevice::WriteOwner |
                          QFileDevice::ReadGroup));
    appendAssociated(
        QLatin1String("sample.pp3"), configuredPath,
        DItemAssociatedRole::ConfiguredSidecar, 0,
        facts(1700000003, QFileDevice::ReadOwner |
                          QFileDevice::WriteOwner |
                          QFileDevice::ReadGroup |
                          QFileDevice::ReadOther));
    appendAssociated(
        QLatin1String("sample.mov"), companionPath,
        DItemAssociatedRole::CompanionMedia, 0,
        facts(1700000004, QFileDevice::ReadOwner));

    for (const DItemAssociatedAccessEntry& associated : associatedEntries)
    {
        QVERIFY(associated.isValid());
    }

    QSharedPointer<DItemAccessHandle> accessHandle(
        new SyntheticAccessHandle(primary, associatedEntries));
    QVERIFY(accessHandle->isValid());

    FCContainer settings;
    settings.destUrl = QUrl::fromLocalFile(outputDirectory);
    settings.behavior = FCContainer::CopyFile;
    settings.sidecars = true;
    settings.overwrite = true;

    bool processed = false;
    ExecutableFCTask task(primary, settings, accessHandle);
    connect(&task, &FCTask::signalUrlProcessed, this,
            [&processed](const QUrl&, const QUrl&)
            {
                processed = true;
            });
    task.execute();
    QVERIFY(processed);

    struct ExpectedOutput
    {
        QString              name;
        QByteArray           bytes;
        DItemAccessFileFacts fileFacts;
    };

    const QList<ExpectedOutput> outputs = {
        { QLatin1String("sample.jpg"), QByteArrayLiteral("primary-bytes"),
          primary.fileFacts },
        { QLatin1String("sample.jpg.xmp"), QByteArrayLiteral("xmp-bytes"),
          associatedEntries.at(0).fileFacts },
        { QLatin1String("sample.pp3"), QByteArrayLiteral("config-bytes"),
          associatedEntries.at(1).fileFacts },
        { QLatin1String("sample.mov"), QByteArrayLiteral("companion-bytes"),
          associatedEntries.at(2).fileFacts }
    };

    for (const ExpectedOutput& expected : outputs)
    {
        const QString path = QDir(outputDirectory).filePath(expected.name);
        QFile output(path);
        QVERIFY2(output.open(QIODevice::ReadOnly),
                 qPrintable(QLatin1String("Missing output: ") + path));
        QCOMPARE(output.readAll(), expected.bytes);
        output.close();

        const QFileInfo info(path);
        QCOMPARE(info.permissions() & comparablePermissions(),
                 expected.fileFacts.permissions);
        QCOMPARE(info.lastModified().toSecsSinceEpoch(),
                 expected.fileFacts.modificationDate.toSecsSinceEpoch());
    }
}

void PrivacyFileCopyAccessTest::testPersistentProxySymlinkRemainsUsable()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(QDir().mkpath(directory.filePath(QLatin1String("output"))));

    const QString proxyPath = directory.filePath(
        QLatin1String("verified-proxy"));
    const QString outputDirectory = directory.filePath(QLatin1String("output"));
    QVERIFY(writeBytes(proxyPath, QByteArrayLiteral("proxy-bytes")));

    DItemAccessEntry proxy;
    proxy.logicalUrl = QUrl::fromLocalFile(
        directory.filePath(QLatin1String("sample.jpg")));
    proxy.physicalUrl = QUrl::fromLocalFile(proxyPath);
    proxy.placeholder = true;
    QVERIFY(proxy.isValid());

    QSharedPointer<DItemAccessHandle> accessHandle(
        new SyntheticAccessHandle(proxy, {}));

    FCContainer settings;
    settings.destUrl = QUrl::fromLocalFile(outputDirectory);
    settings.behavior = FCContainer::FullSymLink;
    settings.overwrite = true;

    {
        ExecutableFCTask task(proxy, settings, accessHandle);
        task.execute();
    }

    accessHandle.clear();

    const QString linkPath = QDir(outputDirectory).filePath(
        QLatin1String("sample.jpg"));
    const QFileInfo linkInfo(linkPath);
    QVERIFY(linkInfo.isSymLink());

    QFile linkedFile(linkPath);
    QVERIFY(linkedFile.open(QIODevice::ReadOnly));
    QCOMPARE(linkedFile.readAll(), QByteArrayLiteral("proxy-bytes"));
}

QTEST_GUILESS_MAIN(PrivacyFileCopyAccessTest)

#include "privacyfilecopyaccess_utest.moc"

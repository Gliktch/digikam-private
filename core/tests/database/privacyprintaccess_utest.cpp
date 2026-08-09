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
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

// Local includes

#include "advprintphoto.h"
#include "advprinttask.h"

using namespace Digikam;
using namespace DigikamGenericPrintCreatorPlugin;

namespace
{

class SyntheticSourceHandle final : public DItemAccessSourceHandle
{
public:

    explicit SyntheticSourceHandle(const DItemAccessEntry& entry)
        : DItemAccessSourceHandle(entry)
    {
    }
};

class SyntheticAccessHandle final : public DItemAccessHandle
{
public:

    explicit SyntheticAccessHandle(const DItemAccessEntry& entry)
        : SyntheticAccessHandle(entry, {}, true)
    {
    }

    SyntheticAccessHandle(const DItemAccessEntry& entry,
                          const QList<QUrl>& excluded,
                          bool provideSource)
        : DItemAccessHandle({ entry }, excluded, false),
          m_entry(entry),
          m_provideSource(provideSource)
    {
    }

    QSharedPointer<DItemAccessSourceHandle> acquireSource(
        const QUrl& logicalUrl,
        const QSharedPointer<DItemAccessCancellationToken>& cancellation) const override
    {
        if (!m_provideSource || (logicalUrl != m_entry.logicalUrl) ||
            (cancellation && cancellation->isCanceled()))
        {
            return {};
        }

        QSharedPointer<DItemAccessSourceHandle> source(
            new SyntheticSourceHandle(m_entry));
        return source->validateAccess()
             ? source : QSharedPointer<DItemAccessSourceHandle>();
    }

private:

    DItemAccessEntry m_entry;
    bool             m_provideSource = true;
};

class ExecutableAdvPrintTask final : public AdvPrintTask
{
public:

    using AdvPrintTask::AdvPrintTask;

    void execute()
    {
        run();
    }
};

bool writeImage(const QString& path, const QColor& color)
{
    QImage image(16, 16, QImage::Format_RGB32);
    image.fill(color);
    return image.save(path, "PNG");
}

} // namespace

class PrivacyPrintAccessTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testPrintUsesOwnedPreparedSource();
    void testExcludedPhotoIsNotPrinted();
    void testSourceFailureLeavesNoOutput();
};

void PrivacyPrintAccessTest::testPrintUsesOwnedPreparedSource()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString logicalPath = directory.filePath(
        QLatin1String("logical-placeholder.png"));
    const QString preparedPath = directory.filePath(
        QLatin1String("prepared-original.png"));
    const QString outputPath = directory.filePath(QLatin1String("output"));
    QVERIFY(QDir().mkpath(outputPath));
    QVERIFY(writeImage(logicalPath, Qt::blue));
    QVERIFY(writeImage(preparedPath, Qt::red));

    DItemAccessEntry entry;
    entry.logicalUrl = QUrl::fromLocalFile(logicalPath);
    entry.physicalUrl = QUrl::fromLocalFile(preparedPath);
    QVERIFY(entry.isValid());

    const QSharedPointer<DItemAccessHandle> accessHandle(
        new SyntheticAccessHandle(entry));
    QVERIFY(accessHandle->isValid());

    AdvPrintSettings settings;
    settings.printerName = settings.outputName(AdvPrintSettings::FILES);
    settings.outputPath = outputPath;
    settings.imageFormat = AdvPrintSettings::PNG;
    settings.disableCrop = true;

    AdvPrintPhoto* const photo = new AdvPrintPhoto(16, nullptr);
    photo->m_url = entry.logicalUrl;
    photo->m_cropRegion = QRect(0, 0, 16, 16);
    settings.photos << photo;

    AdvPrintPhotoSize layouts;
    layouts.m_dpi = 300;
    QRect page(0, 0, 64, 64);
    QRect imageArea(0, 0, 64, 64);
    layouts.m_layouts << &page << &imageArea;
    settings.outputLayouts = &layouts;

    bool completed = false;
    QStringList messages;
    ExecutableAdvPrintTask task(&settings, AdvPrintTask::PRINT,
                                QSize(), 0, accessHandle);
    connect(&task, &AdvPrintTask::signalComplete, this,
            [&completed](bool success)
            {
                completed = success;
            });
    connect(&task, &AdvPrintTask::signalMessage, this,
            [&messages](const QString& message, bool)
            {
                messages << message;
            });
    task.execute();
    QVERIFY2(completed, qPrintable(messages.join(QLatin1String(" | "))));

    const QImage output(QDir(outputPath).filePath(
        QLatin1String("output_1.PNG")));
    QVERIFY(!output.isNull());
    QCOMPARE(output.pixelColor(output.width() / 2, output.height() / 2),
             QColor(Qt::red));
}

void PrivacyPrintAccessTest::testExcludedPhotoIsNotPrinted()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString outputPath = directory.filePath(QLatin1String("output"));
    QVERIFY(QDir().mkpath(outputPath));

    const QString includedLogical = directory.filePath(
        QLatin1String("included-placeholder.png"));
    const QString includedPrepared = directory.filePath(
        QLatin1String("included-original.png"));
    const QString excludedLogical = directory.filePath(
        QLatin1String("excluded-placeholder.png"));
    QVERIFY(writeImage(includedLogical, Qt::blue));
    QVERIFY(writeImage(includedPrepared, Qt::red));
    QVERIFY(writeImage(excludedLogical, Qt::green));

    DItemAccessEntry entry;
    entry.logicalUrl = QUrl::fromLocalFile(includedLogical);
    entry.physicalUrl = QUrl::fromLocalFile(includedPrepared);
    const QSharedPointer<DItemAccessHandle> accessHandle(
        new SyntheticAccessHandle(
            entry, { QUrl::fromLocalFile(excludedLogical) }, true));
    QVERIFY(accessHandle->isValid());

    AdvPrintSettings settings;
    settings.printerName = settings.outputName(AdvPrintSettings::FILES);
    settings.outputPath = outputPath;
    settings.imageFormat = AdvPrintSettings::PNG;
    settings.disableCrop = true;

    for (const QString& path : { includedLogical, excludedLogical })
    {
        AdvPrintPhoto* const photo = new AdvPrintPhoto(16, nullptr);
        photo->m_url = QUrl::fromLocalFile(path);
        photo->m_cropRegion = QRect(0, 0, 16, 16);
        settings.photos << photo;
    }

    AdvPrintPhotoSize layouts;
    layouts.m_dpi = 300;
    QRect page(0, 0, 64, 64);
    QRect imageArea(0, 0, 64, 64);
    layouts.m_layouts << &page << &imageArea;
    settings.outputLayouts = &layouts;

    bool completed = false;
    ExecutableAdvPrintTask task(&settings, AdvPrintTask::PRINT,
                                QSize(), 0, accessHandle);
    connect(&task, &AdvPrintTask::signalComplete, this,
            [&completed](bool success)
            {
                completed = success;
            });
    task.execute();
    QVERIFY(completed);

    const QString firstOutput = QDir(outputPath).filePath(
        QLatin1String("output_1.PNG"));
    const QImage output(firstOutput);
    QVERIFY(!output.isNull());
    QCOMPARE(output.pixelColor(output.width() / 2, output.height() / 2),
             QColor(Qt::red));
    QVERIFY(!QFileInfo::exists(QDir(outputPath).filePath(
        QLatin1String("output_2.PNG"))));
}

void PrivacyPrintAccessTest::testSourceFailureLeavesNoOutput()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString outputPath = directory.filePath(QLatin1String("output"));
    QVERIFY(QDir().mkpath(outputPath));
    const QString logicalPath = directory.filePath(QLatin1String("logical.png"));
    const QString preparedPath = directory.filePath(QLatin1String("prepared.png"));
    QVERIFY(writeImage(logicalPath, Qt::blue));
    QVERIFY(writeImage(preparedPath, Qt::red));

    DItemAccessEntry entry;
    entry.logicalUrl = QUrl::fromLocalFile(logicalPath);
    entry.physicalUrl = QUrl::fromLocalFile(preparedPath);
    const QSharedPointer<DItemAccessHandle> accessHandle(
        new SyntheticAccessHandle(entry, {}, false));

    AdvPrintSettings settings;
    settings.printerName = settings.outputName(AdvPrintSettings::FILES);
    settings.outputPath = outputPath;
    settings.imageFormat = AdvPrintSettings::PNG;
    settings.disableCrop = true;
    AdvPrintPhoto* const photo = new AdvPrintPhoto(16, nullptr);
    photo->m_url = entry.logicalUrl;
    photo->m_cropRegion = QRect(0, 0, 16, 16);
    settings.photos << photo;

    AdvPrintPhotoSize layouts;
    layouts.m_dpi = 300;
    QRect page(0, 0, 64, 64);
    QRect imageArea(0, 0, 64, 64);
    layouts.m_layouts << &page << &imageArea;
    settings.outputLayouts = &layouts;

    bool completed = true;
    ExecutableAdvPrintTask task(&settings, AdvPrintTask::PRINT,
                                QSize(), 0, accessHandle);
    connect(&task, &AdvPrintTask::signalComplete, this,
            [&completed](bool success)
            {
                completed = success;
            });
    task.execute();
    QVERIFY(!completed);
    QVERIFY(!QFileInfo::exists(QDir(outputPath).filePath(
        QLatin1String("output_1.PNG"))));
}

QTEST_MAIN(PrivacyPrintAccessTest)

#include "privacyprintaccess_utest.moc"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : an unit-test tool to erase metadata tag from photo with DMetadata
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "erasemetadatatag_utest.h"

// Qt includes

#include <QFile>

QTEST_MAIN(EraseMetadataTagTest)

EraseMetadataTagTest::EraseMetadataTagTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void EraseMetadataTagTest::testEraseMetadataTag()
{
    MetaEngineSettingsContainer settings;

    settings.metadataWritingMode = DMetadata::WRITE_TO_FILE_ONLY;
    eraseMetadataTag(m_originalImageFolder + QLatin1String("2008-05_DSC_0294.JPG"),
                     QLatin1String("Exif.Nikon3.RetouchHistory"),
                     settings);
}

void EraseMetadataTagTest::eraseMetadataTag(const QString& file,
                                            const QString& metadataTag,
                                            const MetaEngineSettingsContainer& settings)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:" << file;
    QString path = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:" << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setSettings(settings);
    ret = meta->load(path);
    QVERIFY(ret);

    // Check if tag contents exits and drop it.
    QByteArray ba;

    if (metadataTag.startsWith(QLatin1String("Exif")))
    {
        ba = meta->getExifTagData(metadataTag.toLatin1().data());
        QVERIFY(!ba.isNull());
        qCDebug(DIGIKAM_TESTS_LOG) << metadataTag << "found (" << ba.size() << "bytes)";

        ret = meta->removeExifTag(metadataTag.toLatin1().data());
        QVERIFY(ret);
    }
    else if (metadataTag.startsWith(QLatin1String("Iptc")))
    {
        ba = meta->getIptcTagData(metadataTag.toLatin1().data());
        QVERIFY(!ba.isNull());
        qCDebug(DIGIKAM_TESTS_LOG) << metadataTag << "found (" << ba.size() << "bytes)";

        ret = meta->removeIptcTag(metadataTag.toLatin1().data());
        QVERIFY(ret);
    }
    else if (metadataTag.startsWith(QLatin1String("Xmp")))
    {
        ba = meta->getXmpTagString(metadataTag.toLatin1().data()).toLatin1();
        QVERIFY(!ba.isNull());
        qCDebug(DIGIKAM_TESTS_LOG) << metadataTag << "found (" << ba.size() << "bytes)";

        ret = meta->removeXmpTag(metadataTag.toLatin1().data());
        QVERIFY(ret);
    }
    else
    {
        QFAIL(QString::fromLatin1("Metadata Tag not supported (%1)").arg(metadataTag).toLatin1().data());
    }

    ret = meta->applyChanges();
    QVERIFY(ret);

    // Re-read and check if really null

    QScopedPointer<DMetadata> meta2(new DMetadata);
    meta2->setSettings(settings);
    ret = meta2->load(path);
    QVERIFY(ret);

    if (metadataTag.startsWith(QLatin1String("Exif")))
    {
        ba = meta2->getExifTagData(metadataTag.toLatin1().data());
        QVERIFY(ba.isNull());
    }
    else if (metadataTag.startsWith(QLatin1String("Iptc")))
    {
        ba = meta2->getIptcTagData(metadataTag.toLatin1().data());
        QVERIFY(ba.isNull());
    }
    else if (metadataTag.startsWith(QLatin1String("Xmp")))
    {
        ba = meta2->getXmpTagString(metadataTag.toLatin1().data()).toLatin1();
        QVERIFY(ba.isNull());
    }
    else
    {
        QFAIL(QString::fromLatin1("Metadata Tag not supported (%1)").arg(metadataTag).toLatin1().data());
    }
}

#include "moc_erasemetadatatag_utest.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read/write metadata from XMP sidecar with DMetadata.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "usexmpsidecar_utest.h"

// Qt includes

#include <QFile>

QTEST_MAIN(UseXmpSidecarTest)

UseXmpSidecarTest::UseXmpSidecarTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void UseXmpSidecarTest::testUseXmpSidecar()
{
    MetaEngineSettingsContainer settings;

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITE_TO_SIDECAR_ONLY;
    useXmpSidecar(m_originalImageFolder + QLatin1String("_27A1417.CR2"), settings);

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITE_TO_SIDECAR_ONLY_FOR_READ_ONLY_FILES;
    useXmpSidecar(m_originalImageFolder + QLatin1String("_27A1417.CR2"), settings);

    settings.useXMPSidecar4Reading = false;
    settings.metadataWritingMode = DMetadata::WRITE_TO_FILE_ONLY;
    useXmpSidecar(m_originalImageFolder + QLatin1String("_27A1434.JPG"), settings);

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITE_TO_SIDECAR_ONLY;
    useXmpSidecar(m_originalImageFolder + QLatin1String("_27A1434.JPG"), settings);

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITE_TO_SIDECAR_AND_FILE;
    useXmpSidecar(m_originalImageFolder + QLatin1String("_27A1434.JPG"), settings);
}

void UseXmpSidecarTest::useXmpSidecar(const QString& file,
                                      const MetaEngineSettingsContainer& settings)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:          " << file;

    QString path    = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());
    QString pathXmp = path + QLatin1String(".xmp");

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:    " << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary XMP target file:" << pathXmp;

    ret = !pathXmp.isNull();
    QVERIFY(ret);

    QFile::remove(pathXmp);
    QFile xmpTarget(file + QLatin1String(".xmp"));
    ret = xmpTarget.copy(pathXmp);
    QVERIFY(ret);

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setSettings(settings);
    ret = meta->load(path);
    QVERIFY(ret);

    meta->setItemRating(3);
    meta->setItemPickLabel(2);
    meta->setItemColorLabel(1);

    QStringList tags = QStringList() << QString::fromUtf8("City/Paris/Eiffel Tower")
                                     << QString::fromUtf8("Animals/Dog/Illka")
                                     << QString::fromUtf8("People/Family/Agnès");
    meta->setItemTagsPath(tags);
    ret = meta->applyChanges(true);
    QVERIFY(ret);

    QScopedPointer<DMetadata> meta2(new DMetadata);
    meta2->setSettings(settings);
    ret = meta2->load(path);
    QVERIFY(ret);

    int val = meta2->getItemRating();
    QCOMPARE(val, 3);

    val = meta2->getItemPickLabel();
    QCOMPARE(val, 2);

    val = meta2->getItemColorLabel();
    QCOMPARE(val, 1);

    QStringList newTags;
    ret = meta2->getItemTagsPath(newTags);
    QVERIFY(ret);

    int count = tags.count();

    for (const QString& tag : std::as_const(newTags))
    {
        if (tags.contains(tag))
        {
            --count;
        }
    }

    QCOMPARE(count, 0);
}

#include "moc_usexmpsidecar_utest.cpp"

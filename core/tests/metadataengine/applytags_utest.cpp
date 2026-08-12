/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read metadata and apply tag paths to item with DMetadata.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applytags_utest.h"

// Qt includes

#include <QFile>

QTEST_MAIN(ApplyTagsTest)

ApplyTagsTest::ApplyTagsTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void ApplyTagsTest::testApplyTagsToMetadata()
{
    MetaEngineSettingsContainer settings;

    // For bug #400436

    settings.metadataWritingMode = DMetadata::WRITE_TO_FILE_ONLY;

    applyTags(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"),
              QStringList() << QLatin1String("nature"),
              settings,
              true,
              true);

    // For bug #397189
    // The bug is known for Exiv2-0.26, only test the newer versions

    bool ok = true;

    if ((MetaEngine::Exiv2Version().section(QLatin1Char('.'), 0, 1).toDouble(&ok) > 0.26) && ok)
    {
        settings.metadataWritingMode = DMetadata::WRITE_TO_FILE_ONLY;

        applyTags(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"),
                  QStringList() << QLatin1String("test"),
                  settings,
                  true,       // NOTE: image is corrupted => no expected crash
                  true);
    }
}

void ApplyTagsTest::applyTags(const QString& file,
                              const QStringList& tags,
                              const MetaEngineSettingsContainer& settings,
                              bool  expectedRead,
                              bool  expectedWrite)
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
    QCOMPARE(ret, expectedRead);

    meta->setItemTagsPath(tags);
    ret = meta->applyChanges(true);
    QCOMPARE(ret, expectedWrite);

    if (expectedWrite)
    {
        QScopedPointer<DMetadata> meta2(new DMetadata);
        meta2->setSettings(settings);
        QStringList newTags;
        ret = meta2->load(path);
        QVERIFY(ret);

        ret = meta2->getItemTagsPath(newTags);
        QVERIFY(ret);

        for (const QString& tag : std::as_const(tags))
        {
            ret = newTags.contains(tag);
            QVERIFY(ret);
        }
    }
}

#include "moc_applytags_utest.cpp"

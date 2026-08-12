/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-27
 * Description : an unit-test to test XMP sidecar creation with DMetadata
 *
 * SPDX-FileCopyrightText: 2010      by Jakob Malm <jakob dot malm at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "createxmpsidecar_utest.h"

// Qt includes

#include <QFile>

QTEST_MAIN(CreateXmpSidecarTest)

CreateXmpSidecarTest::CreateXmpSidecarTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void CreateXmpSidecarTest::testCreateXmpSidecar()
{
    MetaEngineSettingsContainer settings;
    settings.metadataWritingMode = DMetadata::WRITE_TO_SIDECAR_ONLY;

    createXmpSidecar(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"), settings);

    createXmpSidecar(m_originalImageFolder + QLatin1String("IMG_2520.CR2"),         settings);
}

void CreateXmpSidecarTest::createXmpSidecar(const QString& file,
                                            const MetaEngineSettingsContainer& settings)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:          " << file;

    QString path    = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());
    QString pathXmp = path + QLatin1String(".xmp");

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:    " << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    // Copy image file in temporary dir.
    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    // Check if no xmp sidecar relevant is present.

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary XMP target file to create:" << pathXmp;

    ret = !pathXmp.isNull();
    QVERIFY(ret);

    QFile::remove(pathXmp);

    // Export metadata from image to a fresh xmp sidecar.
    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setSettings(settings);
    ret = meta->load(path);
    QVERIFY(ret);

    ret = meta->save(path);
    QVERIFY(ret);

    QFile sidecar(pathXmp);
    ret = sidecar.exists();
    QVERIFY(ret);

    qCDebug(DIGIKAM_TESTS_LOG) << "Sidecar" << pathXmp << "size :" << sidecar.size();

    // Check if xmp sidecar are created and can be loaded
    QScopedPointer<DMetadata> meta2(new DMetadata);
    ret = meta2->load(pathXmp);
    QVERIFY(ret);
}

#include "moc_createxmpsidecar_utest.cpp"

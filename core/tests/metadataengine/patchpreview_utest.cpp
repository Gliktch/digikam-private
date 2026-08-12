/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-10-25
 * Description : An unit-test to extract preview and patch with DMetadata.
 *               This stage is used by Export tools.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "patchpreview_utest.h"

// Qt includes

#include <QImage>
#include <QByteArray>

// Local includes

#include "dpluginloader.h"
#include "previewloadthread.h"

QTEST_MAIN(PatchPreviewTest)

PatchPreviewTest::PatchPreviewTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void PatchPreviewTest::testExtractPreviewAndFixMetadata()
{
    patchPreview(m_originalImageFolder + QLatin1String("IMG_2520.CR2"), true, 1024, 100); // See bug #400140
}

void PatchPreviewTest::patchPreview(const QString& file, bool rescale, int maxDim, int imageQuality)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:" << file;

    QString path = m_tempDir.filePath(QFileInfo(file).fileName().trimmed()) +
                   QLatin1String(".jpg");

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:" << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    // Load preview from original image.

    QImage image = PreviewLoadThread::loadHighQualitySynchronously(file).copyQImage();

    if (image.isNull())
    {
        image.load(file);
    }

    ret = image.isNull();
    QVERIFY(!ret);

    if (rescale && (image.width() > maxDim || image.height() > maxDim))
    {
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Save preview in temporary directory.

    ret = image.save(path, "JPEG", imageQuality);
    QVERIFY(ret);

    // Load metadata from original image.
    QScopedPointer<DMetadata> meta(new DMetadata);
    ret = meta->load(file);
    QVERIFY(ret);

    QByteArray exif = meta->getExifEncoded();
    QByteArray iptc = meta->getIptc();
    QByteArray xmp  = meta->getXmp();

    // Backport metadata to preview file.

    meta->load(path);
    QVERIFY(ret);

    meta->setExif(exif);
    meta->setIptc(iptc);
    meta->setXmp(xmp);
    meta->setItemDimensions(image.size());
    meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
    meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
    ret = meta->applyChanges(true);
    QVERIFY(ret);
}

void PatchPreviewTest::initTestCase()
{
    AbstractUnitTest::initTestCase();

    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();
}

void PatchPreviewTest::cleanupTestCase()
{
    AbstractUnitTest::cleanupTestCase();

    DPluginLoader::instance()->cleanUp();
}

#include "moc_patchpreview_utest.cpp"

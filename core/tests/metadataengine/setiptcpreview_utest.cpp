/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to set IPTC Preview
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setiptcpreview_utest.h"

// Qt includes

#include <QFile>
#include <QImage>

QTEST_MAIN(SetIptcPreviewTest)

SetIptcPreviewTest::SetIptcPreviewTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void SetIptcPreviewTest::testSetIptcPreview()
{
    setIptcPreview(m_originalImageFolder + QLatin1String("Luftbild_H6_7.tif"));
}

void SetIptcPreviewTest::setIptcPreview(const QString& file)
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

    QImage  preview;
    QImage  image(path);
    QVERIFY(!image.isNull());

    QSize previewSize = image.size();
    previewSize.scale(1280, 1024, Qt::KeepAspectRatio);

    // Ensure that preview is not upscaled
    if (previewSize.width() >= (int)image.width())
        preview = image.copy();
    else
        preview = image.scaled(previewSize.width(), previewSize.height(), Qt::IgnoreAspectRatio).copy();

    QVERIFY(!preview.isNull());

    QScopedPointer<DMetadata> meta(new DMetadata);
    ret = meta->load(path);
    QVERIFY(ret);

    meta->setItemPreview(preview);
    ret = meta->applyChanges();
    QVERIFY(ret);

    QImage preview2;
    QScopedPointer<DMetadata> meta2(new DMetadata);
    ret = meta2->load(path);
    QVERIFY(ret);

    ret = meta2->getItemPreview(preview2);
    QVERIFY(ret);

    QVERIFY(!preview2.isNull());

    QCOMPARE(preview.size(), preview2.size());
}

#include "moc_setiptcpreview_utest.cpp"

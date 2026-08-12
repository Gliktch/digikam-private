/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-21
 * Description : an unit-test to set and clear faces in Picassa format with DMetadata
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setxmpface_utest.h"

// Qt includes

#include <QFile>
#include <QMultiMap>
#include <QRectF>

QTEST_MAIN(SetXmpFaceTest)

SetXmpFaceTest::SetXmpFaceTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void SetXmpFaceTest::testSetXmpFace()
{
    setXmpFace(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"));
}

void SetXmpFaceTest::setXmpFace(const QString& file)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:          " << file;

    QString filePath = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:    " << filePath;

    bool ret = !filePath.isNull();
    QVERIFY(ret);

    // Copy image file in temporary dir.
    QFile::remove(filePath);
    QFile target(file);
    ret = target.copy(filePath);
    QVERIFY(ret);

    QScopedPointer<DMetadata> meta(new DMetadata);
    ret = meta->load(filePath);
    QVERIFY(ret);

    qCDebug(DIGIKAM_TESTS_LOG) << "Add region with face tags in file...";

    QMultiMap<QString, QVariant> faces;

    QString name  = QLatin1String("Bob Marley");
    QRectF rect(10, 100, 60, 80);
    faces.insert(name, QVariant(rect));

    QString name2 = QLatin1String("Alice in wonderland");
    QRectF rect2(20, 40, 90, 70);
    faces.insert(name2, QVariant(rect2));

    ret = meta->setItemFacesMap(faces, true);
    QVERIFY(ret);

    ret = meta->applyChanges();
    QVERIFY(ret);

    qCDebug(DIGIKAM_TESTS_LOG) << "Check if face tags are well assigned in file...";

    QScopedPointer<DMetadata> meta2(new DMetadata);
    ret = meta2->load(filePath);
    QVERIFY(ret);

    QMultiMap<QString, QVariant> faces2;
    ret = meta2->getItemFacesMap(faces2);
    QVERIFY(ret);

    QVERIFY(!faces2.isEmpty());
    QVERIFY(faces2.contains(name));
    QVERIFY(faces2.contains(name2));
    QVERIFY(faces2.value(name)  == rect);
    QVERIFY(faces2.value(name2) == rect2);
    ret = meta2->applyChanges();
    QVERIFY(ret);

    qCDebug(DIGIKAM_TESTS_LOG) << "Clear face tags from file...";

    QScopedPointer<DMetadata> meta3(new DMetadata);
    ret = meta3->load(filePath);
    QVERIFY(ret);

    meta3->removeItemFacesMap();
    ret = meta3->applyChanges();
    QVERIFY(ret);

    qCDebug(DIGIKAM_TESTS_LOG) << "Check if face tags are well removed from file...";

    QScopedPointer<DMetadata> meta4(new DMetadata);
    ret = meta4->load(filePath);
    QVERIFY(ret);

    QMultiMap<QString, QVariant> faces4;
    ret = meta4->getItemFacesMap(faces4);
    QVERIFY(!ret);   // Empty map must be returned
}

#include "moc_setxmpface_utest.cpp"

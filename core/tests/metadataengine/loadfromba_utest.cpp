/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit test to load metadata from byte array
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadfromba_utest.h"

// Qt includes

#include <QFile>
#include <QDataStream>
#include <QImage>
#include <QByteArray>

QTEST_MAIN(LoadFromBATest)

LoadFromBATest::LoadFromBATest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void LoadFromBATest::testLoadFromByteArray()
{
    loadFromByteArray(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"));
}

void LoadFromBATest::loadFromByteArray(const QString& file)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:" << file;
    QString path = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:" << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    QString baFile(path + QLatin1String("ba.dat"));

    QImage image(file);
    ret = image.save(baFile, "PNG");
    QVERIFY(ret);

    QFile baf(baFile);
    ret = baf.open(QIODevice::ReadOnly);
    QVERIFY(ret);

    QByteArray data;
    data.resize(baf.size());
    QDataStream stream(&baf);
    ret = stream.readRawData(data.data(), data.size());
    baf.close();
    QVERIFY(ret);

    QScopedPointer<DMetadata> meta(new DMetadata);
    ret = meta->loadFromData(data);
    QVERIFY(ret);
}

#include "moc_loadfromba_utest.cpp"

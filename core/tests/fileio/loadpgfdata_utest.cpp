/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : an unit-test to load PGF data and convert to QImage
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadpgfdata_utest.h"

// Qt includes

#include <QTest>
#include <QFile>
#include <QIODevice>
#include <QImage>

// Local includes

#include "digikam_debug.h"
#include "dtestdatadir.h"
#include "pgfutils.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(LoadPGFDataTest)

LoadPGFDataTest::LoadPGFDataTest(QObject* const parent)
    : QObject(parent)
{
}

void LoadPGFDataTest::testLoadData()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Using LibPGF version: " << PGFUtils::libPGFVersion();
    QImage img;

    // Write PGF file.

    QString fname = DTestDataDir::TestData(QString::fromUtf8("core/tests/fileio"))
                       .root().path() + QLatin1String("/raw.pgf");
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << fname;

    QFile file(fname);

    QVERIFY2(file.open(QIODevice::ReadOnly), "Cannot read Raw PGF stream...");

    QByteArray data(file.size(), '\x00');
    QDataStream stream(&file);
    stream.readRawData(data.data(), data.size());

    // PGF => QImage conversion

    QVERIFY2(PGFUtils::readPGFImageData(data, img), "Failed to read Raw PGF stream...");

    qCDebug(DIGIKAM_TESTS_LOG) << "Raw PGF stream image size:"         << img.size();
    qCDebug(DIGIKAM_TESTS_LOG) << "Raw PGF stream image color depth:"  << img.depth();
    qCDebug(DIGIKAM_TESTS_LOG) << "Raw PGF stream image color format:" << img.format();

    QVERIFY2(!img.isNull(), "Raw PGF stream image is null...");
    QVERIFY2(img.size()  == QSize(256, 170), "Incorrect Raw PGF stream image size...");
    QVERIFY2(img.depth() == 32, "Incorrect Raw PGF stream image color depth...");
    QVERIFY2(img.format() == QImage::Format_ARGB32, "Incorrect Raw PGF stream image color format...");
}

#include "moc_loadpgfdata_utest.cpp"

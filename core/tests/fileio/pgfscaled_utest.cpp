/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to load PGF file scaled to QImage
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "pgfscaled_utest.h"

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

QTEST_GUILESS_MAIN(PGFScaledTest)

PGFScaledTest::PGFScaledTest(QObject* const parent)
    : QObject(parent)
{
}

void PGFScaledTest::testPGFScaled()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Using LibPGF version: " << PGFUtils::libPGFVersion();
    QImage img;

    QString fname = DTestDataDir::TestData(QString::fromUtf8("core/tests/fileio"))
                       .root().path() + QLatin1String("/4324477432.pgf");
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << fname;

    QFile   file(fname);

    QVERIFY2(file.open(QIODevice::ReadOnly), "Cannot read Raw PGF file...");

    // PGF => QImage conversion

    QVERIFY2(PGFUtils::loadPGFScaled(img, file.fileName(), 1280), "Cannot load PGF scaled file");

    qCDebug(DIGIKAM_TESTS_LOG) << "PGF image size:"         << img.size();
    qCDebug(DIGIKAM_TESTS_LOG) << "PGF image color depth:"  << img.depth();
    qCDebug(DIGIKAM_TESTS_LOG) << "PGF image color format:" << img.format();

    QVERIFY2(!img.isNull(), "PGF image is null...");
    QVERIFY2(img.size()  == QSize(2016, 1512), "Incorrect PGF image size...");
    QVERIFY2(img.depth() == 32, "Incorrect PGF image color depth...");
    QVERIFY2(img.format() == QImage::Format_RGB32, "Incorrect PGF image color format...");
}

#include "moc_pgfscaled_utest.cpp"

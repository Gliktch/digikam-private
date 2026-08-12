/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-02-17
 * Description : an unit-test to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "raw2dng_utest.h"

// Qt includes

#include <QApplication>
#include <QTest>
#include <QFileInfo>
#include <QDir>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "dimg.h"
#include "dngwriter.h"
#include "wstoolutils.h"
#include "dpluginloader.h"
#include "dtestdatadir.h"
#include "drawdecoding.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(Raw2DngTest)

Raw2DngTest::Raw2DngTest(QObject* const parent)
    : QObject(parent)
{
}

void Raw2DngTest::testRaw2Dng()
{
    MetaEngine::initializeExiv2();
    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();

    QVERIFY2(!DPluginLoader::instance()->allPlugins().isEmpty(),
             "Not able to found digiKam plugin in standard paths. Test is aborted...");

    QString fname = DTestDataDir::TestData(QString::fromUtf8("core/tests/metadataengine"))
                    .root().path() + QLatin1String("/IMG_2520.CR2");
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << fname;

    QString tmpPath   = QString::fromLatin1(QTest::currentAppName());
    tmpPath.replace(QLatin1String("./"), QString());
    QDir tmpDir   = WSToolUtils::makeTemporaryDir(tmpPath.toLatin1().data());
    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary Dir :" << tmpDir.path();

    DNGWriter dngProcessor;
    dngProcessor.setInputFile(fname);
    QString path = tmpDir.filePath(QFileInfo(fname).fileName().trimmed());
    dngProcessor.setOutputFile(QString::fromLatin1("%1.dng").arg(path));
    QVERIFY2(!dngProcessor.convert(), "Cannot convert RAW to DNG");

    qCDebug(DIGIKAM_TESTS_LOG) << "Loading DNG file with DImg:" << dngProcessor.outputFile();

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img;

    QVERIFY2(img.load(dngProcessor.outputFile(), nullptr, DRawDecoding(settings)), "Cannot load DNG file");

    qCDebug(DIGIKAM_TESTS_LOG) << "DNG image size:"         << img.size();
    qCDebug(DIGIKAM_TESTS_LOG) << "DNG image bits depth:"   << img.bitsDepth();
    qCDebug(DIGIKAM_TESTS_LOG) << "DNG image format:"       << img.format();

    QVERIFY2(!img.isNull(), "DNG image is null...");
    QVERIFY2(img.size()  == QSize(6264, 4180), "Incorrect DNG image size...");
    QVERIFY2(img.bitsDepth() == 16, "Incorrect DNG image bits depth...");
    QVERIFY2(img.format() == QLatin1String("RAW"), "Incorrect DNG image format...");

    DPluginLoader::instance()->cleanUp();
}

#include "moc_raw2dng_utest.cpp"

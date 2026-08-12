/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : an unit-test to check DImg image loader
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgloader_utest.h"

// Qt includes

#include <QApplication>
#include <QTest>
#include <QFileInfo>
#include <QList>
#include <QDir>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "dpluginloader.h"
#include "dtestdatadir.h"
#include "drawdecoding.h"

QTEST_GUILESS_MAIN(DImgLoaderTest)

DImgLoaderTest::DImgLoaderTest(QObject* const parent)
    : QObject(parent)
{
}

void DImgLoaderTest::testDImgLoader()
{
    MetaEngine::initializeExiv2();
    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();

    QVERIFY2(!DPluginLoader::instance()->allPlugins().isEmpty(),
             "Not able to found digiKam plugin in standard paths. Test is aborted...");

    QString fname = DTestDataDir::TestData(QString::fromUtf8("core/tests/dimg"))
                    .root().path() + QLatin1String("/DSC00636.JPG");
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << fname;

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(fname, nullptr, DRawDecoding(settings));

    QVERIFY2(!img.isNull(), "Cannot load JPEG image with DImg plugin");
    QVERIFY2(img.size() == QSize(100, 67), "Incorrect JPEG image size...");
    QVERIFY2(img.save(fname + QLatin1String(".png"), QLatin1String("PNG")),
             "Cannot save PNG image with DImg plugin");

    DPluginLoader::instance()->cleanUp();
}

#include "moc_dimgloader_utest.cpp"

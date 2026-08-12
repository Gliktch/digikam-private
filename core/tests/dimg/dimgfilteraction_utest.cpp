/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-13
 * Description : a test for applying FilterActions
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgfilteraction_utest.h"

// Qt includes

#include <QPixmap>
#include <QLabel>
#include <QDialog>
#include <QHBoxLayout>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "dpluginloader.h"
#include "dimagehistory.h"
#include "drawdecoding.h"
#include "filteractionfilter.h"
#include "dtestdatadir.h"

QTEST_MAIN(DImgFilterActionTest)

DImgFilterActionTest::DImgFilterActionTest(QObject* const parent)
    : QObject(parent)
{
}

void DImgFilterActionTest::testDRawDecoding()
{
    DRawDecoding params;

    params.rawPrm.sixteenBitsImage      = true;
    params.rawPrm.autoBrightness        = true;
    params.rawPrm.whiteBalance          = DRawDecoderSettings::AERA;
    params.rawPrm.RGBInterpolate4Colors = true;
    params.rawPrm.RAWQuality            = DRawDecoderSettings::DHT;
    params.rawPrm.NRType                = DRawDecoderSettings::WAVELETSNR;
    params.rawPrm.outputColorSpace      = DRawDecoderSettings::ADOBERGB;

    FilterAction action;
    params.writeToFilterAction(action);

    qCDebug(DIGIKAM_TESTS_LOG) << action.parameters();

    DRawDecoding params2 = DRawDecoding::fromFilterAction(action);
    QVERIFY(params == params2);
}

/**
 * The data for this test had to be regenerated because the test was failing
 * The regeneration was performed like this:
 * 1. Create a folder in digikam and add the DSC00636.JPG
 * 2. Go to image editor and choose auto-levels
 * 3. Apply first option and then Save As New Version -> Save in new Format: PNG
 * 4. Repeat the operation until you get multiple versions of this file
 */
void DImgFilterActionTest::testActions()
{
    QStringList files = imageDir().entryList(QDir::Files);
    files.removeOne(originalImage());

    DImg original(imageDir().filePath(originalImage()));
    QVERIFY(!original.isNull());

    for (const QString& fileName : std::as_const(files))
    {
        DImg ref(imageDir().filePath(fileName));
        QVERIFY(!ref.isNull());
        DImageHistory history = ref.getItemHistory();

        FilterActionFilter filter;
        filter.setFilterActions(history.allActions());
        QVERIFY(filter.isReproducible() || filter.isComplexAction());

        filter.setupFilter(original.copy());
        filter.startFilterDirectly();
        qCDebug(DIGIKAM_TESTS_LOG) << filter.filterActions().size();

        DImg img     = filter.getTargetImage();

        QVERIFY(ref.size() == img.size());

        bool isEqual = true;
        DImg diff(ref.width(), ref.height(), ref.sixteenBit());
        diff.fill(DColor(Qt::black));

        for (uint x = 0 ; x < ref.width() ; ++x)
        {
            for (uint y = 0 ; y < ref.height() ; ++y)
            {
                DColor cref = ref.getPixelColor(x,y);
                DColor cres = img.getPixelColor(x,y);

                if ((cref.red()   != cres.red())   ||
                    (cref.green() != cres.green()) ||
                    (cref.blue()  != cres.blue()))
                {
                    diff.setPixelColor(x,y, DColor(Qt::white));
                    isEqual = false;
                }
            }
        }

        if (!isEqual)
        {
            showDiff(original, ref, img, diff);
        }

        QVERIFY(isEqual);
    }
}

void DImgFilterActionTest::showDiff(const DImg& orig, const DImg& ref,
                                    const DImg& result, const DImg& diff)
{
    QDialog d;
    QLabel l1, l2, l3, l4;
    l1.setPixmap(orig.convertToPixmap());
    l2.setPixmap(ref.convertToPixmap());
    l3.setPixmap(result.convertToPixmap());
    l4.setPixmap(diff.convertToPixmap());
    QHBoxLayout layout(&d);
    layout.addWidget(&l1);
    layout.addWidget(&l2);
    layout.addWidget(&l3);
    layout.addWidget(&l4);
    d.setLayout(&layout);
    d.exec();
}

void DImgFilterActionTest::initTestCase()
{
    // initialize Exiv2 before doing any multitasking

    MetaEngine::initializeExiv2();
    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();
}

void DImgFilterActionTest::cleanupTestCase()
{
    DPluginLoader::instance()->cleanUp();
}

QString DImgFilterActionTest::originalImage()
{
    // picture taken by me, downscaled to 100x66 and metadata stripped off

    return QString::fromUtf8("DSC00636.JPG");
}

QDir DImgFilterActionTest::imageDir()
{
    QString filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/dimg"))
                           .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << filesPath;

    return filesPath;
}

#include "moc_dimgfilteraction_utest.cpp"

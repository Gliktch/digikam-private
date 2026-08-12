/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : unit test for DImg image loader with multithreading
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "loadsavethread_utest.h"

// Qt includes

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

QTEST_GUILESS_MAIN(LoadSaveThreadTest)

LoadSaveThreadTest::LoadSaveThreadTest(QObject* const parent)
    : QObject(parent)
{
}

void LoadSaveThreadTest::testLoadSaveThread()
{
    MetaEngine::initializeExiv2();
    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();

    if (DPluginLoader::instance()->allPlugins().isEmpty())
    {
        qWarning() << QLatin1String("Not able to found digiKam plugin in standard paths. Test is aborted...");
        return;
    }

    qRegisterMetaType<LoadingDescription>("LoadingDescription");
    qRegisterMetaType<DImg>("DImg");

    m_thread = new LoadSaveThread;

    connect(m_thread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotImageLoaded(LoadingDescription,DImg)));

    connect(m_thread, SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    connect(m_thread, SIGNAL(signalLoadingProgress(LoadingDescription,float)),
            this, SLOT(slotLoadingProgress(LoadingDescription,float)));

    connect(m_thread, SIGNAL(signalSavingProgress(QString,float)),
            this, SLOT(slotSavingProgress(QString,float)));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    m_fname = DTestDataDir::TestData(QString::fromUtf8("core/tests/fileio"))
                  .root().path() + QLatin1String("/4324477432.pgf");
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << m_fname;

    LoadingDescription desc(m_fname, DRawDecoding(settings));

    m_thread->load(desc);

    int i = 0;

    do
    {
        QTest::qWait(1000);
        i++;
    }
    while ((i < 10) && !(m_loaded && m_saved));

    QVERIFY2(m_loadedProgress, "Failed to progress image loading");
    QVERIFY2(m_savedProgress,  "Failed to progress image saving");
    QVERIFY2(m_loaded,         "Failed to load image");
    QVERIFY2(m_saved,          "Failed to save image");

    DPluginLoader::instance()->cleanUp();
}

void LoadSaveThreadTest::slotLoadingProgress(const LoadingDescription& desc, float p)
{
    QFileInfo fi(desc.filePath);
    qCDebug(DIGIKAM_TESTS_LOG) << "Loading " << fi.baseName() << " : " << p << " %";
    QVERIFY2(desc.filePath == m_fname, "File to load do not match");
    m_loadedProgress = true;
}

void LoadSaveThreadTest::slotImageLoaded(const LoadingDescription& desc, const DImg& img)
{
    QFileInfo fi(desc.filePath);
    qCDebug(DIGIKAM_TESTS_LOG) << "Image " << fi.baseName() << " loaded";

    QVERIFY2(desc.filePath == m_fname, "File loaded do not match");

    qCDebug(DIGIKAM_TESTS_LOG) << "DImg image size:"         << img.size();

    QVERIFY2(!img.isNull(), "DImg image is null...");
    QVERIFY2(img.size()  == QSize(4032, 3024), "Incorrect DImg image size...");

    m_loaded      = true;
    m_outFilePath = fi.baseName() + QString::fromUtf8(".out.png");
    DImg image    = img;
    m_thread->save(image, m_outFilePath, QLatin1String("PNG"));
}

void LoadSaveThreadTest::slotSavingProgress(const QString& filePath, float p)
{
    QFileInfo fi(filePath);
    qCDebug(DIGIKAM_TESTS_LOG) << "Saving " << fi.baseName() << " : " << p << " %";
    QVERIFY2(filePath == m_outFilePath, "File to save do not match");
    m_savedProgress = true;
}

void LoadSaveThreadTest::slotImageSaved(const QString& filePath, bool b)
{
    QFileInfo fi(filePath);
    qCDebug(DIGIKAM_TESTS_LOG) << fi.baseName() << " saved : " << (b ? "ok" : "pb");

    QVERIFY2(filePath == m_outFilePath, "File saved do not match");
    QVERIFY2(b, "Failed to save PNG file");

    m_saved = true;
}

#include "moc_loadsavethread_utest.cpp"

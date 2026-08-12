/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <user dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgabstracthistory_utest.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QTest>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "digikam_config.h"
#include "bcgfilter.h"
#include "curvesfilter.h"
#include "autolevelsfilter.h"
#include "infraredfilter.h"
#include "blurfilter.h"
#include "editortooliface.h"
#include "editorwindow.h"
#include "iccsettings.h"
#include "imageiface.h"
#include "iofilesettings.h"
#include "dtestdatadir.h"

DImgAbstractHistoryTest::DImgAbstractHistoryTest(QObject* const parent)
    : QObject(parent),
      m_im   (nullptr)
{
}

QString DImgAbstractHistoryTest::imagePath()
{
    // NOTE: We will use same data dir than album model tests.

    QString filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/albummodel"))
                           .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << filesPath;

    return filesPath;
}

QDebug operator<<(QDebug dbg, const HistoryImageId& id)
{
    dbg.nospace() << " { ";
    dbg.nospace() << id.m_uuid;
    dbg.space()   << id.m_type;
    dbg.space()   << id.m_fileName;
    dbg.space()   << id.m_filePath;
    dbg.space()   << id.m_creationDate;
    dbg.space()   << id.m_uniqueHash;
    dbg.space()   << id.m_fileSize;
    dbg.space()   << id.m_originalUUID;
    dbg.nospace() << " } ";

    return dbg;
}

HistoryImageId DImgAbstractHistoryTest::id1() const
{
    HistoryImageId id(QLatin1String("abc123"));
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName(QLatin1String("file1.jpg"));
    id.setPathOnDisk(QLatin1String("/home/user/file1.jpg"));
    id.setUniqueHash(QLatin1String("cde567"), 987654);

    return id;
}

HistoryImageId DImgAbstractHistoryTest::id2() const
{
    HistoryImageId id(QLatin1String("12345"));
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName(QLatin1String("file2.jpg"));
    id.setPathOnDisk(QLatin1String("/home/user/file2.jpg"));
    id.setUniqueHash(QLatin1String("abc654"), 876549);

    return id;
}

HistoryImageId DImgAbstractHistoryTest::id3() const
{
    HistoryImageId id(QLatin1String("abcdef"));
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName(QLatin1String("file3.jpg"));
    id.setPathOnDisk(QLatin1String("/home/user/file3.jpg"));
    id.setUniqueHash(QLatin1String("c76543"), 765489);

    return id;
}

HistoryImageId DImgAbstractHistoryTest::id4() const
{
    HistoryImageId id(QLatin1String("aaabbb"));
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName(QLatin1String("file4.jpg"));
    id.setPathOnDisk(QLatin1String("/home/user/file4.jpg"));
    id.setUniqueHash(QLatin1String("c96542"), 654987);

    return id;
}

FilterAction DImgAbstractHistoryTest::action1() const
{
    FilterAction fa(QLatin1String("digikam:BCGFilter"), 1);
    fa.addParameter(QLatin1String("contrast"), 1);
    fa.addParameter(QLatin1String("channel"), 1);
    fa.addParameter(QLatin1String("brightness"), 1);
    fa.addParameter(QLatin1String("gamma"), 1.2);

    return fa;
}

FilterAction DImgAbstractHistoryTest::action2() const
{
    FilterAction fa(QLatin1String("digikam:SomeComplexFilter"), 1, FilterAction::ComplexFilter);

    return fa;
}

DImageHistory DImgAbstractHistoryTest::history1() const
{
    DImageHistory history;

    history << id1();

    history << action1();

    history << id2();

    history << action2();

    history << id3() << id4();

    return history;
}

void DImgAbstractHistoryTest::applyFilters1()
{
    ImageIface iface;

    BCGFilter filter1(iface.original(), this);
    filter1.startFilterDirectly();
    iface.setOriginal(QLatin1String("Brightness / Contrast / Gamma"), filter1.filterAction(), filter1.getTargetImage());

    CurvesFilter filter2(iface.original(), this);
    filter2.startFilterDirectly();
    iface.setOriginal(QLatin1String("Curves"), filter2.filterAction(), filter2.getTargetImage());
}

void DImgAbstractHistoryTest::applyFilters2()
{
    ImageIface iface;

    AutoLevelsFilter filter(iface.original(), iface.original(), this);
    filter.startFilterDirectly();
    iface.setOriginal(QLatin1String(""), filter.filterAction(), filter.getTargetImage());
}

void DImgAbstractHistoryTest::applyFilters3()
{
    ImageIface iface;

    InfraredFilter filter(iface.original(), this);
    filter.startFilterDirectly();
    iface.setOriginal(QLatin1String(""), filter.filterAction(), filter.getTargetImage());
}

void DImgAbstractHistoryTest::applyFilters4()
{
    ImageIface iface;

    BlurFilter filter(iface.original(), this);
    filter.startFilterDirectly();
    iface.setOriginal(QLatin1String(""), filter.filterAction(), filter.getTargetImage());
}

QString DImgAbstractHistoryTest::tempFileName(const QString& purpose) const
{
    return (
            QString::fromUtf8("digikamtests-")       +
            QLatin1String(metaObject()->className()) +
            QLatin1Char('-')                         +
            purpose                                  +
            QLatin1Char('-')                         +
            QTime::currentTime().toString()
           );
}

QString DImgAbstractHistoryTest::tempFilePath(const QString& purpose) const
{
    return (
            QDir::tempPath() +
            QLatin1Char('/') +
            tempFileName(purpose)
           );
}

void DImgAbstractHistoryTest::initBaseTestCase()
{
    // initialize Exiv2 before doing any multi-tasking

    MetaEngine::initializeExiv2();

    QDir dir(qApp->applicationDirPath());
    qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
    DPluginLoader::instance()->init();

    ICCSettingsContainer c = IccSettings::instance()->settings();
    c.enableCM             = false;
    IccSettings::instance()->setSettings(c);

    m_im                   = new EditorCore();
    EditorCore::setDefaultInstance(m_im);

    connect(m_im, SIGNAL(signalImageLoaded(QString,bool)),
            this, SLOT(slotImageLoaded(QString,bool)));

    connect(m_im, SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    m_tempFile = tempFilePath(QLatin1String("tempfile"));
}

void DImgAbstractHistoryTest::cleanupBaseTestCase()
{
    delete m_im;

    DPluginLoader::instance()->cleanUp();

    QFile file(m_tempFile);
    file.remove();
}

void DImgAbstractHistoryTest::slotImageLoaded(const QString&, bool)
{
}

void DImgAbstractHistoryTest::slotImageSaved(const QString&, bool)
{
}

#include "moc_dimgabstracthistory_utest.cpp"

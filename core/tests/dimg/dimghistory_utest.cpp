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

#include "dimghistory_utest.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "editortooliface.h"
#include "editorwindow.h"
#include "imageiface.h"
#include "iofilesettings.h"
#include "dpluginloader.h"

using namespace Digikam;

QTEST_MAIN(DImgHistoryTest)

DImgHistoryTest::DImgHistoryTest(QObject* const parent)
    : DImgAbstractHistoryTest(parent)
{
}

void DImgHistoryTest::initTestCase()
{
    initBaseTestCase();
}

void DImgHistoryTest::cleanupTestCase()
{
    cleanupBaseTestCase();
}

void DImgHistoryTest::testXml()
{
    DImageHistory history  = history1();

    QString xml            = history.toXml();

    DImageHistory history2 = DImageHistory::fromXml(xml);
    QString xml2           = history2.toXml();

    QCOMPARE(xml, xml2);

    // Does not need to work: Some fields in the ids like originalUUID are filled out even if not in XML
    //QCOMPARE(history2, historyWithoutCurrent);
}

void DImgHistoryTest::testDImg()
{
    QDir imageDir(imagePath());
    imageDir.setNameFilters(QStringList() << QLatin1String("*.jpg"));
    QList<QFileInfo> imageFiles = imageDir.entryInfoList();

    IOFileSettings container;
    m_im->load(imageFiles.first().filePath(), &container);

    m_loop.exec();   // krazy:exclude=crashy

    DImageHistory history = m_im->getImg()->getItemHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Current);
    QVERIFY(history.entries().last().referredImages.isEmpty());

    m_im->saveAs(m_tempFile, &container, true, QString(), QString());

    m_loop.exec();   // krazy:exclude=crashy

    history = m_im->getImg()->getItemHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Current);
    QCOMPARE(history.entries().last().referredImages.size(), 1);
    QCOMPARE(history.entries().last().referredImages.first().m_type, HistoryImageId::Intermediate);

    m_im->switchToLastSaved();

    history = m_im->getImg()->getOriginalImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history.entries().last().referredImages.size(), 1);
    QCOMPARE(history.entries().last().referredImages.first().m_type, HistoryImageId::Current);
}

void DImgHistoryTest::slotImageLoaded(const QString&, bool success)
{
    QVERIFY(success);

    applyFilters1();

    DImageHistory h = m_im->getImg()->getItemHistory();
    h.adjustReferredImages();

    for (int i = 0 ; i < 3 ; ++i)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << i << h.entries().at(i).referredImages.size();

        if (h.entries().at(i).referredImages.size())
        {
            qCDebug(DIGIKAM_TESTS_LOG) << " " << i << h.entries().at(i).referredImages.first().m_type;
        }
    }

    m_loop.quit();
}

void DImgHistoryTest::slotImageSaved(const QString& fileName, bool success)
{
    QVERIFY(success);

    m_im->setLastSaved(fileName);

    DImg img(fileName);
    DImageHistory history = img.getItemHistory();
    qCDebug(DIGIKAM_TESTS_LOG) << history.toXml();

    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history.action(1).category(), FilterAction::ReproducibleFilter);
    QCOMPARE(history.action(2).category(), FilterAction::ReproducibleFilter);

    DImageHistory history2 = DImageHistory::fromXml(history.toXml());
    QCOMPARE(history2.size(), 3);
    QCOMPARE(history2.entries().first().referredImages.size(), 1);
    QCOMPARE(history2.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history2.action(1).category(), FilterAction::ReproducibleFilter);
    QCOMPARE(history2.action(2).category(), FilterAction::ReproducibleFilter);

    m_loop.quit();
}

#include "moc_dimghistory_utest.cpp"

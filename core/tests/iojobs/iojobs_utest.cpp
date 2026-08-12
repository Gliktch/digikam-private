/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-02
 * Description : Test the functions for dealing with DatabaseFields
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iojobs_utest.h"

// Qt includes

#include <QApplication>
#include <QTest>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QThreadPool>

// Local includes

#include "iojob.h"
#include "digikam_debug.h"
#include "dtestdatadir.h"

using namespace Digikam;

const QString testFileName   = QLatin1String("test.png");
const QString filePath       = DTestDataDir::TestData(QString::fromUtf8("core/tests/iojobs"))
                                   .root().path() + QLatin1Char('/') + testFileName;
const QString srcFolderName  = QLatin1String("src");
const QString dstFolderPath  = QLatin1String("dst");
const QString testFolderName = QLatin1String("test");
const QString testFilePath   = (QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFileName);
const QString testFolderPath = (QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFolderName + QLatin1Char('/'));
const QString destPath       = (QDir::currentPath() + QLatin1Char('/') + dstFolderPath + QLatin1Char('/'));

void IOJobsTest::init()
{
    QFile f(filePath);

    QDir d;
    (void)d.mkpath(QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFolderName);
    (void)d.mkpath(QDir::currentPath() + QLatin1Char('/') + dstFolderPath);

    QVERIFY2(f.copy(QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFileName),
             qPrintable(QLatin1String("Could not copy the test target to src folder")));

    QVERIFY2(f.copy(QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFolderName + QLatin1Char('/') + testFileName),
             qPrintable(QLatin1String("Could not copy the test target to src folder")));
}

void IOJobsTest::cleanup()
{
    QDir src(QDir::currentPath() + QLatin1Char('/') + srcFolderName);
    QDir dst(QDir::currentPath() + QLatin1Char('/') + dstFolderPath);

    QVERIFY2(src.removeRecursively() && dst.removeRecursively(),
             qPrintable(QLatin1String("Could not cleanup")));
}

void IOJobsTest::copyAndMove()
{
    QFETCH(QString, src);
    QFETCH(QString, dst);
    QFETCH(bool, isMove);
    QFETCH(bool, shouldExistInDst);
    QFETCH(QString, pathToCheckInDst);

    QFileInfo srcFi(src);
    QFileInfo dstFi(dst);

    QUrl srcUrl              = QUrl::fromLocalFile(srcFi.absoluteFilePath());
    QUrl dstUrl              = QUrl::fromLocalFile(dstFi.absoluteFilePath());
    int operation            = (isMove ? IOJobData::MoveFiles : IOJobData::CopyFiles);

    IOJobData* const data    = new IOJobData(operation, QList<QUrl>() << srcUrl, dstUrl);
    CopyOrMoveJob* const job = new CopyOrMoveJob(data);

    QThreadPool::globalInstance()->start(job);
    QThreadPool::globalInstance()->waitForDone();

    delete job;
    delete data;

    QFileInfo dstFiAfterCopy(pathToCheckInDst);

    QCOMPARE(dstFiAfterCopy.exists(), shouldExistInDst);
    srcFi.refresh();
    QCOMPARE(srcFi.exists(), !isMove);
}

void IOJobsTest::copyAndMove_data()
{
    QTest::addColumn<QString>("src");
    QTest::addColumn<QString>("dst");
    QTest::addColumn<bool>("isMove");
    QTest::addColumn<bool>("shouldExistInDst");
    QTest::addColumn<QString>("pathToCheckInDst");

    QTest::newRow(qPrintable(QLatin1String("Copying file")))
            << testFilePath
            << destPath
            << false
            << true
            << (destPath + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Moving file")))
            << testFilePath
            << destPath
            << true
            << true
            << (destPath + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Copying Folder")))
            << testFolderPath
            << destPath
            << false
            << true
            << (destPath + testFolderName + QLatin1Char('/') + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Moving Folder")))
            << testFolderPath
            << destPath
            << true
            << true
            << (destPath + testFolderName + QLatin1Char('/') + testFileName);
}

void IOJobsTest::permanentDel()
{
    QFETCH(QString, srcToDel);

    QFileInfo fi(srcToDel);

    QUrl fileUrl          = QUrl::fromLocalFile(fi.absoluteFilePath());

    IOJobData* const data = new IOJobData(IOJobData::Delete, QList<QUrl>() << fileUrl);
    DeleteJob* job        = new DeleteJob(data);

    QThreadPool::globalInstance()->start(job);
    QThreadPool::globalInstance()->waitForDone();

    delete job;
    delete data;

    fi.refresh();
    QVERIFY(!fi.exists());
}

void IOJobsTest::permanentDel_data()
{
    QTest::addColumn<QString>("srcToDel");

    QTest::newRow(qPrintable(QLatin1String("Deleting File")))
            << testFilePath;

    QTest::newRow(qPrintable(QLatin1String("Deleting Folder")))
            << testFolderPath;
}

QTEST_GUILESS_MAIN(IOJobsTest)

#include "moc_iojobs_utest.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a unit-test to write metadata with ExifTool and EXV container.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolapplychanges_utest.h"

// Qt includes

#include <QApplication>
#include <QTest>
#include <QFileInfo>
#include <QDir>

// Local includes

#include "digikam_debug.h"
#include "wstoolutils.h"
#include "dtestdatadir.h"
#include "metaengine.h"
#include "exiftoolparser.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(ExifToolApplyChangesTest)

ExifToolApplyChangesTest::ExifToolApplyChangesTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void ExifToolApplyChangesTest::testExifToolApplyChanges()
{
    QString file = m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG");
    qCDebug(DIGIKAM_TESTS_LOG) << "File to process:" << file;
    QString path = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qCDebug(DIGIKAM_TESTS_LOG) << "Temporary target file:" << path;

    bool ret = !path.isNull();
    QVERIFY(ret);

    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    QScopedPointer<DMetadata> meta(new DMetadata);

    QVERIFY2(meta->load(path),
             QString::fromLatin1("Cannot load %1").arg(meta->getFilePath()).toLatin1().constData());

    meta->setImageDateTime(QDateTime::currentDateTime(), true);

    QString exvPath = path + QLatin1String("_changes.exv");
    QVERIFY2(meta->exportChanges(exvPath),
             QString::fromLatin1("Cannot export metadata to %1").arg(exvPath).toLatin1().constData());

    QScopedPointer<ExifToolParser> parser(new ExifToolParser(qApp));

    QVERIFY2(parser->applyChanges(path, exvPath),
             QString::fromLatin1("Cannot apply changes with ExifTool on %1").arg(path).toLatin1().constData());
}

#include "moc_exiftoolapplychanges_utest.cpp"

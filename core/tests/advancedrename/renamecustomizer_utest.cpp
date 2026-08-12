/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "renamecustomizer_utest.h"

// Qt includes

#include <QFileInfo>
#include <QTest>
#include <QUrl>

// Local includes

#include "advancedrenamemanager.h"
#include "defaultrenameparser.h"
#include "parsesettings.h"
#include "renamecustomizer.h"
#include "digikam_debug.h"
#include "dtestdatadir.h"

using namespace Digikam;

QString createFilePath(const QString& file)
{
    const QString path = DTestDataDir::TestData(QString::fromUtf8("core/tests/advancerename"))
                         .root().path() + QLatin1Char('/') + file;
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data File:" << path;

    return path;
}

const QString fileName  = QLatin1String("advancedrename_testimage.jpg");
const QString fileName2 = QLatin1String("advancedrename_testimage2.jpg");
const QString fileName3 = QLatin1String("001a.jpg");
const QString fileName4 = QLatin1String("test.png");
const QString fileName5 = QLatin1String("myfile.jpg");
const QString fileName6 = QLatin1String("my_file.jpg");
const QString fileName7 = QLatin1String("holiday_spain_2011_img001.jpg");
const QString fileName8 = QLatin1String("my images.jpg");
const QString fileName9 = QLatin1String("holiday_spain_2011_001img.jpg");

const QString filePath  = createFilePath(fileName);
const QString filePath2 = createFilePath(fileName2);
const QString filePath3 = createFilePath(fileName3);
const QString filePath4 = createFilePath(fileName4);
const QString filePath5 = createFilePath(fileName5);
const QString filePath6 = createFilePath(fileName6);
const QString filePath7 = createFilePath(fileName7);
const QString filePath8 = createFilePath(fileName8);
const QString filePath9 = createFilePath(fileName9);

QTEST_MAIN(RenameCustomizerTest)

void RenameCustomizerTest::newName_should_return_empty_string_with_empty_filename_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QTest::newRow("empty")          << QString::fromUtf8("")      << QString::fromUtf8("");
    QTest::newRow("whitespaces")    << QString::fromUtf8("    ")  << QString::fromUtf8("");
}

void RenameCustomizerTest::newName_should_return_empty_string_with_empty_filename()
{
    QFETCH(QString, filename);
    QFETCH(QString, result);

    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    QCOMPARE(customizer.newName(filename), result);
}

void RenameCustomizerTest::setCaseType_set_to_none()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setChangeCase(RenameCustomizer::NONE);
    QCOMPARE(customizer.changeCase(), RenameCustomizer::NONE);
}

void RenameCustomizerTest::setCaseType_set_to_upper()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setChangeCase(RenameCustomizer::UPPER);
    QCOMPARE(customizer.changeCase(), RenameCustomizer::UPPER);
}

void RenameCustomizerTest::setCaseType_set_to_lower()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setChangeCase(RenameCustomizer::LOWER);
    QCOMPARE(customizer.changeCase(), RenameCustomizer::LOWER);
}

void RenameCustomizerTest::setUseDefault_true()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setUseDefault(true);
    QVERIFY(customizer.useDefault());
}

void RenameCustomizerTest::setUseDefault_false()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setUseDefault(false);
    QVERIFY(customizer.useDefault() == false);
}

void RenameCustomizerTest::setUseDefault_case_none_should_deliver_original_filename()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setUseDefault(true);
    customizer.setChangeCase(RenameCustomizer::NONE);
    QCOMPARE(customizer.newName(QLatin1String("TeSt.png")), QLatin1String("TeSt.png"));
}

void RenameCustomizerTest::setUseDefault_case_upper_should_deliver_uppercase_filename()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setUseDefault(true);
    customizer.setChangeCase(RenameCustomizer::UPPER);
    QCOMPARE(customizer.newName(QLatin1String("TeSt.png")), QLatin1String("TEST.PNG"));
}

void RenameCustomizerTest::setUseDefault_case_lower_should_deliver_lowercase_filename()
{
    RenameCustomizer customizer(nullptr, QLatin1String("Unit Tests"));
    customizer.setUseDefault(true);
    customizer.setChangeCase(RenameCustomizer::LOWER);
    QCOMPARE(customizer.newName(QLatin1String("TeSt.pnG")), QLatin1String("test.png"));
}

#include "moc_renamecustomizer_utest.cpp"

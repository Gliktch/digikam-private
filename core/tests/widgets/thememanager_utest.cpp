/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-07-18
 * Description : a test for the theme manager
 *
 * SPDX-FileCopyrightText: 2026 by Andreas Winther <git dot tumble747 at simplelogin dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thememanager_utest.h"

// Qt includes

#include <QMenu>
#include <QTest>

// Local includes

#include "thememanager.h"

using namespace Digikam;

QTEST_MAIN(ThemeManagerTest)

ThemeManagerTest::ThemeManagerTest(QObject* const parent)
    : QObject(parent)
{
}

void ThemeManagerTest::testCallsBeforeMenuRegistration()
{
    ThemeManager* const tm = ThemeManager::instance();

    // Every public method must be safe to call before a theme menu
    // has been registered with setThemeMenuAction().

    QCOMPARE(tm->currentThemeName(), tm->defaultThemeName());

    tm->setCurrentTheme(QLatin1String("Does Not Exist"));
    QCOMPARE(tm->currentThemeName(), tm->defaultThemeName());

    tm->updateThemeMenu();
    QCOMPARE(tm->currentThemeName(), tm->defaultThemeName());
}

void ThemeManagerTest::testAfterMenuRegistration()
{
    ThemeManager* const tm = ThemeManager::instance();
    QMenu menu;

    tm->setThemeMenuAction(&menu);

    QCOMPARE(tm->currentThemeName(), tm->defaultThemeName());
    QVERIFY(!menu.actions().isEmpty());

    tm->updateThemeMenu();
    QCOMPARE(tm->currentThemeName(), tm->defaultThemeName());
}

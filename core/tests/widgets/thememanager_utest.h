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

#ifndef DIGIKAM_THEME_MANAGER_UTEST_H
#define DIGIKAM_THEME_MANAGER_UTEST_H

// Qt includes

#include <QObject>

class ThemeManagerTest : public QObject
{
    Q_OBJECT

public:

    explicit ThemeManagerTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testCallsBeforeMenuRegistration();
    void testAfterMenuRegistration();
};

#endif // DIGIKAM_THEME_MANAGER_UTEST_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-06
 * Description : test for the filesaveoptionsbox
 *
 * SPDX-FileCopyrightText: 2009 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

class FileSaveOptionsBoxTest : public QObject
{
    Q_OBJECT

public:

    explicit FileSaveOptionsBoxTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testDiscoverFormat_data();
    void testDiscoverFormat();

    void testDiscoverFormatDefault();

protected Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
};

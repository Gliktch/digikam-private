/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : unit test program for digiKam sqlite database init
 *
 * SPDX-FileCopyrightText: 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>
#include <QDir>
#include <QString>

class IdentityProviderTest : public QObject
{
    Q_OBJECT

public:

    explicit IdentityProviderTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void testInit();
    void testAddIdentity();
    void testFindIdentity();
    void testDeleteIdentity();
    void testIdentity();
    void testAddTraining();
    void testClearTraining();


private:

};

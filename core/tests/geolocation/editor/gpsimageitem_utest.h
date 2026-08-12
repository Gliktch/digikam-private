/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-28
 * Description : Test loading and saving of data in GPSItemContainer.
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

// Local includes

class TestGPSItemContainer : public QObject
{
    Q_OBJECT

public:

    explicit TestGPSItemContainer(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void testNoOp();
    void testBasicLoading();
};

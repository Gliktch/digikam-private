/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-07
 * Description : test for the simple datatypes and helper functions
 *
 * SPDX-FileCopyrightText: 2010-2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

class TestPrimitives : public QObject
{
    Q_OBJECT

public:

    explicit TestPrimitives(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testNoOp();
    void testParseLatLonString();
    void testParseXYStringToPoint();
    void testParseBoundsString();
    void testNormalizeBounds_data();
    void testNormalizeBounds();
    void testGroupStateComputer();
};

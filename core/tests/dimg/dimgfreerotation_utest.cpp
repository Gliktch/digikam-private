/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the freerotation tool
 *
 * SPDX-FileCopyrightText: 2009 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgfreerotation_utest.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QTest>

// Local includes

#include "freerotationfilter.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(DImgFreeRotationTest)

DImgFreeRotationTest::DImgFreeRotationTest(QObject* const parent)
    : QObject(parent)
{
}

void DImgFreeRotationTest::testCalculateAngle_data()
{
    QTest::addColumn<QPoint>("p1");
    QTest::addColumn<QPoint>("p2");
    QTest::addColumn<double>("result");

    QTest::newRow("empty")
            << QPoint()
            << QPoint()
            << 0.0;

    QTest::newRow("invalid")
            << QPoint(-1, -3)
            << QPoint(2, 2)
            << 0.0;

    QTest::newRow("p1=p2")
            << QPoint(10, 10)
            << QPoint(10, 10)
            << 0.0;

    QTest::newRow("p1.x=p2.x")
            << QPoint(10, 10)
            << QPoint(200, 10)
            << 0.0;

    QTest::newRow("p1.y=p2.y")
            << QPoint(10, 10)
            << QPoint(10, 200)
            << 90.0;

    QTest::newRow("-45 degrees")
            << QPoint(10, 10)
            << QPoint(20, 20)
            << -45.0;

    QTest::newRow("45 degrees")
            << QPoint(10, 20)
            << QPoint(20, 10)
            << 45.0;

    QTest::newRow("28.82 degrees")
            << QPoint(0, 241)
            << QPoint(438, 0)
            << 28.82;

    QTest::newRow("-28.82 degrees")
            << QPoint(0, 0)
            << QPoint(438, 241)
            << -28.82;

    // point layout shouldn't matter

    QPoint p1(10, 20);
    QPoint p2(20, 10);

    QTest::newRow("layout1")
            << p1
            << p2
            << 45.0;

    QTest::newRow("layout2")
            << p2
            << p1
            << 45.0;
}

void DImgFreeRotationTest::testCalculateAngle()
{
    QFETCH(QPoint, p1);
    QFETCH(QPoint, p2);
    QFETCH(double, result);

    double angle = FreeRotationFilter::calculateAngle(p1, p2);
    QCOMPARE(myRound(angle, 2), result);
}

double DImgFreeRotationTest::myRound(double val, int accuracy)
{
    double tmp = pow(10.0, accuracy);
    double x   = val;

    x *= tmp;
    x += 0.5;
    x  = floor(x);
    x /= tmp;

    return x;
}

#include "moc_dimgfreerotation_utest.cpp"

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

#pragma once

// Qt includes

#include <QObject>

class DImgFreeRotationTest : public QObject
{
    Q_OBJECT

public:

    explicit DImgFreeRotationTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testCalculateAngle();
    void testCalculateAngle_data();

private:

    double myRound(double val, int accuracy);
};

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-11-13
 * Description : a test for applying FilterActions
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>
#include <QEventLoop>
#include <QDir>

// Local includes

#include "dimg.h"

using namespace Digikam;

class DImgFilterActionTest : public QObject
{
    Q_OBJECT

public:

    explicit DImgFilterActionTest(QObject* const parent = nullptr);

public:

    QDir    imageDir();
    QString originalImage();

    void showDiff(const DImg& orig, const DImg& ref, const DImg& result, const DImg& diff);

private Q_SLOTS:

    void testDRawDecoding();
    void testActions();

    void initTestCase();
    void cleanupTestCase();
};

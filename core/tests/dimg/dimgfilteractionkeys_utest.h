/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-06-09
 * Description : a test for verifying unique keys in FilterAction for all filters
 *
 * SPDX-FileCopyrightText: 2021 by Anjani Kumar <anjanik012 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

// Local includes

#include "dimg.h"
#include "dimgfiltermanager.h"

using namespace Digikam;

class DImgFilterActionKeysTest : public QObject
{
    Q_OBJECT

public:

    explicit DImgFilterActionKeysTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void testAllFilters();

private:

    bool hasUniqueKeys(const FilterAction& action);
};

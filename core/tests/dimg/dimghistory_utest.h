/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
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

// Local includes

#include "editorcore.h"
#include "dimgabstracthistory_utest.h"

class DImgHistoryTest : public DImgAbstractHistoryTest
{
    Q_OBJECT

public:

    explicit DImgHistoryTest(QObject* const parent = nullptr);

public Q_SLOTS:

    void slotImageLoaded(const QString&, bool) override;
    void slotImageSaved(const QString&, bool) override;

private Q_SLOTS:

    void testXml();
    void testDImg();

    void initTestCase();
    void cleanupTestCase();
};

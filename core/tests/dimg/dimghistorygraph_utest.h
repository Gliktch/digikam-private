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
#include <QDir>

// Local includes

#include "dimgabstracthistory_utest.h"

class DImgHistoryGraphTest : public DImgAbstractHistoryTest
{
    Q_OBJECT

public:

    explicit DImgHistoryGraphTest(QObject* const parent = nullptr);

public Q_SLOTS:

    void slotImageLoaded(const QString&, bool) override;
    void slotImageSaved(const QString&, bool)  override;

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void testGraph();
    void testHistory();

private:

    void rescan();
    void testEditing();

private:

    QDir             collectionDir;
    QString          dbFile;
    QStringList      readOnlyImages;
    QList<qlonglong> ids;
};

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : a test for the StateSavingObject class
 *
 * SPDX-FileCopyrightText: 2009 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

// Local includes

#include "statesavingobject.h"

class KConfigGroup;

class StateSavingObjectTest: public QObject
{
    Q_OBJECT

public:

    explicit StateSavingObjectTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testGroupName();
    void testPrefixUsage();
    void testDirectCalling();
    void testDirectChildrenLoading();
    void testDirectChildrenSaving();
    void testRecursiveChildrenLoading();
    void testRecursiveChildrenSaving();
};

// ----------------------------------------------------------

class StubStateSaver: public QObject,
                      public Digikam::StateSavingObject
{
    Q_OBJECT

public:

    explicit StubStateSaver(QObject* const parent = nullptr);
    ~StubStateSaver() override;

    KConfigGroup getGroup();
    QString getEntryKey(const QString& base);

    void doLoadState() override;
    void doSaveState() override;

    bool loadCalled();
    bool saveCalled();

    unsigned int numLoadCalls();
    unsigned int numSaveCalls();

private:

    class Private;
    Private* const d = nullptr;
};

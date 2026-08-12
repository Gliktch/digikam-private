/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-17
 * Description : Test parsing gpx data.
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

class TestTracks : public QObject
{
    Q_OBJECT

public:

    explicit TestTracks(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testNoOp();
    void testQDateTimeParsing();
    void testCustomDateTimeParsing();
    void testSaxLoader();
    void testSaxLoaderError();
    void testFileLoading();
};

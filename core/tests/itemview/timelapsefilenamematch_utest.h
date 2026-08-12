/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-06-13
 * Description : a test for the timelapse filename matcher
 *
 * SPDX-FileCopyrightText: 2026 by François Martin <kde at fmartin dot ch>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

class TimelapseFilenameMatchTest : public QObject
{
    Q_OBJECT

public:

    explicit TimelapseFilenameMatchTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }


private Q_SLOTS:

    void testParseNumberedFilename_data();
    void testParseNumberedFilename();
    void testDirectlyPreceeds_data();
    void testDirectlyPreceeds();
    void testSequenceOrder_data();
    void testSequenceOrder();
};

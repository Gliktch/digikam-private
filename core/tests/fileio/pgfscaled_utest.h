/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to load PGF file scaled to QImage
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

class PGFScaledTest : public QObject
{
    Q_OBJECT

public:

    explicit PGFScaledTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testPGFScaled();
};

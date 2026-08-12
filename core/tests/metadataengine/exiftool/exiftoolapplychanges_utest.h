/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a unit-test to write metadata with ExifTool and EXV container.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QObject>

// Local includes

#include "abstractunittest.h"

class ExifToolApplyChangesTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit ExifToolApplyChangesTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testExifToolApplyChanges();
};

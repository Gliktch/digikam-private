/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : a test for the CameraNameHelper
 *
 * SPDX-FileCopyrightText: 2009 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

class CameraNameHelperTest : public QObject
{
    Q_OBJECT

public:

    explicit CameraNameHelperTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testCreateCameraName();
    void testCreateCameraName_data();

    void testSameDevices();
    void testSameDevices_data();

    void testCameraName_data();
    void testCameraName();

    void testCameraNameAutoDetected_data();
    void testCameraNameAutoDetected();
};

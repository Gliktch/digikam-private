/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : an unit-test to check DImg image loader
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

#include "dimg.h"
#include "loadingdescription.h"

using namespace Digikam;

class DImgLoaderTest : public QObject
{
    Q_OBJECT

public:

    explicit DImgLoaderTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testDImgLoader();
};

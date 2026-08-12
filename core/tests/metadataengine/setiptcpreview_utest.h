/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to set IPTC Preview
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractunittest.h"

class SetIptcPreviewTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit SetIptcPreviewTest(QObject* const parent = nullptr);

private:

    void setIptcPreview(const QString& file);

private Q_SLOTS:

    void testSetIptcPreview();
};

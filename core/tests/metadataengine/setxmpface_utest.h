/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-21
 * Description : an unit-test to set and clear faces in Picassa format with DMetadata
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

using namespace Digikam;

class SetXmpFaceTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit SetXmpFaceTest(QObject* const parent = nullptr);

private:

    void setXmpFace(const QString& file);

private Q_SLOTS:
/*
    virtual void cleanup()
    {
    }
*/
    void testSetXmpFace();
};

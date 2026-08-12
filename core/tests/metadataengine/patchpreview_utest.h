/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-10-25
 * Description : An unit-test to extract preview and patch with DMetadata.
 *               This stage is used by Export tools.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractunittest.h"

class PatchPreviewTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit PatchPreviewTest(QObject* const parent = nullptr);

private:

    void patchPreview(const QString& file, bool rescale, int maxDim, int imageQuality);

private Q_SLOTS:

    void testExtractPreviewAndFixMetadata();

    void initTestCase()    override;
    void cleanupTestCase() override;
};

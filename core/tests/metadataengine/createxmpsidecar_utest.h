/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-27
 * Description : an unit-test to test XMP sidecar creation with DMetadata
 *
 * SPDX-FileCopyrightText: 2010      by Jakob Malm <jakob dot malm at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

class CreateXmpSidecarTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit CreateXmpSidecarTest(QObject* const parent = nullptr);

private:

    void createXmpSidecar(const QString& file,
                          const MetaEngineSettingsContainer& settings);

private Q_SLOTS:

    void testCreateXmpSidecar();
};

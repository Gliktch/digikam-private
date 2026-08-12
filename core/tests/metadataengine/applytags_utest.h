/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read metadata and apply tag paths to item with DMetadata.
 *
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

class ApplyTagsTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit ApplyTagsTest(QObject* const parent = nullptr);

private:

    void applyTags(const QString& file,
                   const QStringList& tags,
                   const MetaEngineSettingsContainer& settings,
                   bool  expectedRead,
                   bool  expectedWrite);

private Q_SLOTS:

    void testApplyTagsToMetadata();
};

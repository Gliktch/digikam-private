/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : An unit-test to print all available metadata tags provided by Exiv2.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class PrintTagsListTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit PrintTagsListTest(QObject* const parent = nullptr);

private:

    void parseTagsList(const DMetadata::TagsMap& tags);

private Q_SLOTS:

    void testPrintAllAvailableStdExifTags();
    void testPrintAllAvailableMakernotesTags();
    void testPrintAllAvailableIptcTags();
    void testPrintAllAvailableXmpTags();
};

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit-test to print metadata tags from file using DMetadata.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractunittest.h"
#include "dmetadata.h"

using namespace Digikam;

class PrintMetadataTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit PrintMetadataTest(QObject* const parent = nullptr);

private:

    void printMetadataMap(const DMetadata::MetaDataMap& map);
    void printMetadata(const QString& filePath, bool exif, bool iptc, bool xmp, bool expectedRead);

    /// @note 'expected' paramareters want mean that we expect a metadata tags map non empty
    void loadExif(const DMetadata& meta, bool expected);
    void loadIptc(const DMetadata& meta, bool expected);
    void loadXmp(const DMetadata& meta, bool expected);

private Q_SLOTS:

    void testPrintMetadata();
};

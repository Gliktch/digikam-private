/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit-test to print item info from file using DMetadata.
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

class PrintItemInfoTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit PrintItemInfoTest(QObject* const parent = nullptr);

private:

    void printMetadataMap(const DMetadata::MetaDataMap& map);
    void printItemInfo(const QString& filePath,
                       bool com, bool ttl,            // Comments and titles
                       bool cnt, bool loc, bool isb,  // Iptc
                       bool pho, bool vid,            // Media
                       bool key, bool xsb, bool cat,  // Xmp
                       bool expectedRead
                      );

    /// @note 'expected' paramareters want mean that we expect a info container non empty
    void printComments(const DMetadata& meta, bool expected);
    void printTitles(const DMetadata& meta, bool expected);

    void printIptcContact(const DMetadata& meta, bool expected);
    void printIptcLocation(const DMetadata& meta, bool expected);
    void printIptcSubjects(const DMetadata& meta, bool expected);

    void printPhotoInfo(const DMetadata& meta, bool expected);
    void printVideoInfo(const DMetadata& meta, bool expected);

    void printXmpKeywords(const DMetadata& meta, bool expected);
    void printXmpSubjects(const DMetadata& meta, bool expected);
    void printXmpSubCategories(const DMetadata& meta, bool expected);

private Q_SLOTS:

    void testPrintItemInfo();
};

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

#include "printiteminfo_utest.h"

// Qt includes

#include <QTextStream>
#include <QStringList>

QTEST_MAIN(PrintItemInfoTest)

PrintItemInfoTest::PrintItemInfoTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void PrintItemInfoTest::testPrintItemInfo()
{
    //                                                   Expected tags to found in Comments,    Titles,
    //                                                                             IptcContact, IptcLocation, IptcSubjects,
    //                                                                             PhotoInfo,   VideoInfo,
    //                                                                             XmpKeywords, XmpSubjects,  XmpSubCategories, expectedRead
    printItemInfo(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"),        false,       false,
                                                                                   false,       false,        false,
                                                                                   true,        true,
                                                                                   true,        false,        false,            true);

    printItemInfo(m_originalImageFolder + QLatin1String("_27A1417.CR2"),           false,       false,
                                                                                   false,       false,        false,
                                                                                   true,        true,
                                                                                   false,       false,        false,            true);

    printItemInfo(m_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"),   false,       false,
                                                                                   false,       false,        false,
                                                                                   true,        false,
                                                                                   false,       false,        false,            true);

    // The file cannot be loaded with Exiv2-0.26, only test the newer versions

    bool ok = true;

    if ((MetaEngine::Exiv2Version().section(QLatin1Char('.'), 0, 1).toDouble(&ok) > 0.26) && ok && m_hasExifTool)
    {
        printItemInfo(m_originalImageFolder + QLatin1String("20160821035715.jpg"), false,       false,
                                                                                   false,       false,        false,
                                                                                   true,        true,
                                                                                   false,       false,        false,            true);
    }
}

void PrintItemInfoTest::printItemInfo(const QString& filePath,
                                      bool com, bool ttl,            // Comments and titles
                                      bool cnt, bool loc, bool isb,  // Iptc
                                      bool pho, bool vid,            // Media
                                      bool key, bool xsb, bool cat,  // Xmp
                                      bool expectedRead
                                     )
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    bool ret = meta->load(filePath);
    QCOMPARE(ret, expectedRead);

    // Comments and titles
    printComments(*meta,         com);
    printTitles(*meta,           ttl);

    // Iptc
    printIptcContact(*meta,      cnt);
    printIptcLocation(*meta,     loc);
    printIptcSubjects(*meta,     isb),

    // Media
    printPhotoInfo(*meta,        pho);
    printVideoInfo(*meta,        vid);

    // Xmp
    printXmpKeywords(*meta,      key);
    printXmpSubjects(*meta,      xsb);
    printXmpSubCategories(*meta, cat);
}

void PrintItemInfoTest::printComments(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Comments from %1 --------------------------").arg(meta.getFilePath());

    CaptionsMap map = meta.getItemComments();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printTitles(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Titles from %1 --").arg(meta.getFilePath());

    CaptionsMap map = meta.getItemTitles();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printIptcContact(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- IptcContact from %1 --").arg(meta.getFilePath());

    IptcCoreContactInfo map = meta.getCreatorContactInfo();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printIptcLocation(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- IptcLocation from %1 --").arg(meta.getFilePath());

    IptcCoreLocationInfo map = meta.getIptcCoreLocation();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printIptcSubjects(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- IptcSubjects from %1 --").arg(meta.getFilePath());

    QStringList map = meta.getIptcCoreSubjects();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printPhotoInfo(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- PhotoInfo from %1 --").arg(meta.getFilePath());

    PhotoInfoContainer map = meta.getPhotographInformation();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printVideoInfo(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- VideoInfo from %1 --").arg(meta.getFilePath());

    VideoInfoContainer map = meta.getVideoInformation();
    QCOMPARE(!map.isEmpty(), expected);

    qCDebug(DIGIKAM_TESTS_LOG) << map;
}

void PrintItemInfoTest::printXmpKeywords(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- XmpKeywords from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        QStringList map = meta.getXmpKeywords();
        QCOMPARE(!map.isEmpty(), expected);

        qCDebug(DIGIKAM_TESTS_LOG) << map;
    }
    else
    {
        qWarning() << QLatin1String("Exiv2 has no XMP support...");
    }
}

void PrintItemInfoTest::printXmpSubjects(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- XmpSubjects from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        QStringList map = meta.getXmpSubjects();
        QCOMPARE(!map.isEmpty(), expected);

        qCDebug(DIGIKAM_TESTS_LOG) << map;
    }
    else
    {
        qWarning() << QLatin1String("Exiv2 has no XMP support...");
    }
}

void PrintItemInfoTest::printXmpSubCategories(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- XmpSubCategories from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        QStringList map = meta.getXmpSubCategories();
        QCOMPARE(!map.isEmpty(), expected);

        qCDebug(DIGIKAM_TESTS_LOG) << map;
    }
    else
    {
        qWarning() << QLatin1String("Exiv2 has no XMP support...");
    }
}

#include "moc_printiteminfo_utest.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-06-13
 * Description : a test for the timelapse filename matcher
 *
 * SPDX-FileCopyrightText: 2026 by François Martin <kde at fmartin dot ch>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timelapsefilenamematch_utest.h"

// Qt includes

#include <QStringList>
#include <QTest>

// Local includes

#include "digikam_debug.h"
#include "timelapsefilenamematch.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(TimelapseFilenameMatchTest)

void TimelapseFilenameMatchTest::testParseNumberedFilename_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("prefix");
    QTest::addColumn<qulonglong>("value");
    QTest::addColumn<QString>("suffix");

    QTest::newRow("sony arw")
            << QString::fromLatin1("DSC06975.ARW")
            << QString::fromLatin1("DSC")
            << 6975ULL
            << QString::fromLatin1(".ARW");

    QTest::newRow("nikon nef")
            << QString::fromLatin1("DSC_0001.NEF")
            << QString::fromLatin1("DSC_")
            << 1ULL
            << QString::fromLatin1(".NEF");

    QTest::newRow("nikon adobergb")
            << QString::fromLatin1("_DSC0001.NEF")
            << QString::fromLatin1("_DSC")
            << 1ULL
            << QString::fromLatin1(".NEF");

    QTest::newRow("nikon dscn")
            << QString::fromLatin1("DSCN0001.NEF")
            << QString::fromLatin1("DSCN")
            << 1ULL
            << QString::fromLatin1(".NEF");

    QTest::newRow("fuji dscf")
            << QString::fromLatin1("DSCF0001.RAF")
            << QString::fromLatin1("DSCF")
            << 1ULL
            << QString::fromLatin1(".RAF");

    QTest::newRow("fuji dscf underscore")
            << QString::fromLatin1("DSCF_0001.JPG")
            << QString::fromLatin1("DSCF_")
            << 1ULL
            << QString::fromLatin1(".JPG");

    QTest::newRow("canon cr3 extension contains digit")
            << QString::fromLatin1("IMG_0001.CR3")
            << QString::fromLatin1("IMG_")
            << 1ULL
            << QString::fromLatin1(".CR3");

    QTest::newRow("canon adobergb cr2")
            << QString::fromLatin1("_MG_0001.CR2")
            << QString::fromLatin1("_MG_")
            << 1ULL
            << QString::fromLatin1(".CR2");

    QTest::newRow("canon custom code cr3")
            << QString::fromLatin1("_J4A0011.CR3")
            << QString::fromLatin1("_J4A")
            << 11ULL
            << QString::fromLatin1(".CR3");

    QTest::newRow("heif")
            << QString::fromLatin1("IMG_0001.HEIF")
            << QString::fromLatin1("IMG_")
            << 1ULL
            << QString::fromLatin1(".HEIF");

    QTest::newRow("png")
            << QString::fromLatin1("IMG_0001.PNG")
            << QString::fromLatin1("IMG_")
            << 1ULL
            << QString::fromLatin1(".PNG");

    QTest::newRow("tiff")
            << QString::fromLatin1("IMG_0001.TIFF")
            << QString::fromLatin1("IMG_")
            << 1ULL
            << QString::fromLatin1(".TIFF");

    QTest::newRow("panasonic rw2 extension contains digit")
            << QString::fromLatin1("P1010461.RW2")
            << QString::fromLatin1("P")
            << 1010461ULL
            << QString::fromLatin1(".RW2");

    QTest::newRow("pentax")
            << QString::fromLatin1("IMGP0001.PEF")
            << QString::fromLatin1("IMGP")
            << 1ULL
            << QString::fromLatin1(".PEF");

    QTest::newRow("pentax alternate")
            << QString::fromLatin1("_IGP0001.PEF")
            << QString::fromLatin1("_IGP")
            << 1ULL
            << QString::fromLatin1(".PEF");

    QTest::newRow("dji")
            << QString::fromLatin1("DJI_0001.DNG")
            << QString::fromLatin1("DJI_")
            << 1ULL
            << QString::fromLatin1(".DNG");

    QTest::newRow("samsung nx")
            << QString::fromLatin1("SAM_0001.SRW")
            << QString::fromLatin1("SAM_")
            << 1ULL
            << QString::fromLatin1(".SRW");

    QTest::newRow("google pixel burst")
            << QString::fromLatin1("IMG_20260612_161829_BURST001.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST")
            << 1ULL
            << QString::fromLatin1(".JPG");

    QTest::newRow("google pixel burst cover")
            << QString::fromLatin1("IMG_20260612_161829_BURST000_COVER.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST")
            << 0ULL
            << QString::fromLatin1("_COVER.JPG");

    QTest::newRow("google pixel burst cover one-based")
            << QString::fromLatin1("IMG_20260612_161829_BURST001_COVER.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST")
            << 1ULL
            << QString::fromLatin1("_COVER.JPG");

    QTest::newRow("google pixel timestamp")
            << QString::fromLatin1("PXL_20260612_161829123.JPG")
            << QString::fromLatin1("PXL_20260612_")
            << 161829123ULL
            << QString::fromLatin1(".JPG");

    QTest::newRow("google pixel night mode")
            << QString::fromLatin1("PXL_20260612_161829123.NIGHT.JPG")
            << QString::fromLatin1("PXL_20260612_")
            << 161829123ULL
            << QString::fromLatin1(".NIGHT.JPG");

    QTest::newRow("phone timestamp")
            << QString::fromLatin1("20220111_153300.JPG")
            << QString::fromLatin1("20220111_")
            << 153300ULL
            << QString::fromLatin1(".JPG");

    QTest::newRow("hp point and shoot")
            << QString::fromLatin1("HPIM0001.JPG")
            << QString::fromLatin1("HPIM")
            << 1ULL
            << QString::fromLatin1(".JPG");

    QTest::newRow("gopro chaptered mp4")
            << QString::fromLatin1("GX010001.MP4")
            << QString::fromLatin1("GX")
            << 10001ULL
            << QString::fromLatin1(".MP4");

    QTest::newRow("gopro hero mp4")
            << QString::fromLatin1("GH013607.MP4")
            << QString::fromLatin1("GH")
            << 13607ULL
            << QString::fromLatin1(".MP4");
}

void TimelapseFilenameMatchTest::testParseNumberedFilename()
{
    QFETCH(QString,   filename);
    QFETCH(QString,   prefix);
    QFETCH(qulonglong, value);
    QFETCH(QString,   suffix);

    const TimelapseFilenameMatch match(filename);

    qCDebug(DIGIKAM_TESTS_LOG) << "Parsed filename prefix:" << match.prefix
                               << "value:" << match.value
                               << "suffix:" << match.suffix;

    QVERIFY(match.containsValue);
    QCOMPARE(match.prefix, prefix);
    QCOMPARE(match.value, value);
    QCOMPARE(match.suffix, suffix);
}

void TimelapseFilenameMatchTest::testDirectlyPreceeds_data()
{
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("second");
    QTest::addColumn<bool>("expected");

    QTest::newRow("camera raw sequence")
            << QString::fromLatin1("DSC06975.ARW")
            << QString::fromLatin1("DSC06976.ARW")
            << true;

    QTest::newRow("camera raw before rollover sequence")
            << QString::fromLatin1("DSC99998.ARW")
            << QString::fromLatin1("DSC99999.ARW")
            << true;

    QTest::newRow("camera raw rollover sequence")
            << QString::fromLatin1("DSC99999.ARW")
            << QString::fromLatin1("DSC00000.ARW")
            << true;

    QTest::newRow("camera raw after rollover sequence")
            << QString::fromLatin1("DSC00000.ARW")
            << QString::fromLatin1("DSC00001.ARW")
            << true;

    QTest::newRow("timelapse jpg sequence")
            << QString::fromLatin1("IMG0001.JPG")
            << QString::fromLatin1("IMG0002.JPG")
            << true;

    QTest::newRow("nikon nef sequence")
            << QString::fromLatin1("DSC_0001.NEF")
            << QString::fromLatin1("DSC_0002.NEF")
            << true;

    QTest::newRow("nikon adobergb sequence")
            << QString::fromLatin1("_DSC0001.NEF")
            << QString::fromLatin1("_DSC0002.NEF")
            << true;

    QTest::newRow("nikon dscn sequence")
            << QString::fromLatin1("DSCN0001.NEF")
            << QString::fromLatin1("DSCN0002.NEF")
            << true;

    QTest::newRow("fuji dscf sequence")
            << QString::fromLatin1("DSCF0001.RAF")
            << QString::fromLatin1("DSCF0002.RAF")
            << true;

    QTest::newRow("fuji dscf underscore sequence")
            << QString::fromLatin1("DSCF_0001.JPG")
            << QString::fromLatin1("DSCF_0002.JPG")
            << true;

    QTest::newRow("canon cr3 sequence")
            << QString::fromLatin1("IMG_0001.CR3")
            << QString::fromLatin1("IMG_0002.CR3")
            << true;

    QTest::newRow("canon adobergb cr2 sequence")
            << QString::fromLatin1("_MG_0001.CR2")
            << QString::fromLatin1("_MG_0002.CR2")
            << true;

    QTest::newRow("canon custom code cr3 sequence")
            << QString::fromLatin1("_J4A0011.CR3")
            << QString::fromLatin1("_J4A0012.CR3")
            << true;

    QTest::newRow("heif sequence")
            << QString::fromLatin1("IMG_0001.HEIF")
            << QString::fromLatin1("IMG_0002.HEIF")
            << true;

    QTest::newRow("png sequence")
            << QString::fromLatin1("IMG_0001.PNG")
            << QString::fromLatin1("IMG_0002.PNG")
            << true;

    QTest::newRow("tiff sequence")
            << QString::fromLatin1("IMG_0001.TIFF")
            << QString::fromLatin1("IMG_0002.TIFF")
            << true;

    QTest::newRow("panasonic rw2 sequence")
            << QString::fromLatin1("P1010461.RW2")
            << QString::fromLatin1("P1010462.RW2")
            << true;

    QTest::newRow("pentax sequence")
            << QString::fromLatin1("IMGP0001.PEF")
            << QString::fromLatin1("IMGP0002.PEF")
            << true;

    QTest::newRow("pentax alternate sequence")
            << QString::fromLatin1("_IGP0001.PEF")
            << QString::fromLatin1("_IGP0002.PEF")
            << true;

    QTest::newRow("dji sequence")
            << QString::fromLatin1("DJI_0001.DNG")
            << QString::fromLatin1("DJI_0002.DNG")
            << true;

    QTest::newRow("samsung nx sequence")
            << QString::fromLatin1("SAM_0001.SRW")
            << QString::fromLatin1("SAM_0002.SRW")
            << true;

    QTest::newRow("google pixel burst sequence")
            << QString::fromLatin1("IMG_20260612_161829_BURST001.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST002.JPG")
            << true;

    QTest::newRow("google pixel burst cover sequence")
            << QString::fromLatin1("IMG_20260612_161829_BURST000_COVER.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST001.JPG")
            << true;

    QTest::newRow("google pixel burst one-based cover sequence")
            << QString::fromLatin1("IMG_20260612_161829_BURST001_COVER.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST002.JPG")
            << true;

    QTest::newRow("google pixel timestamp sequence")
            << QString::fromLatin1("PXL_20260612_161829123.JPG")
            << QString::fromLatin1("PXL_20260612_161829124.JPG")
            << true;

    QTest::newRow("google pixel timestamp next second")
            << QString::fromLatin1("PXL_20260612_161829123.JPG")
            << QString::fromLatin1("PXL_20260612_161830123.JPG")
            << true;

    QTest::newRow("google pixel timestamp day rollover")
            << QString::fromLatin1("PXL_20260612_235959123.JPG")
            << QString::fromLatin1("PXL_20260613_000000123.JPG")
            << true;

    QTest::newRow("google pixel night mode sequence")
            << QString::fromLatin1("PXL_20260612_161829123.NIGHT.JPG")
            << QString::fromLatin1("PXL_20260612_161829124.NIGHT.JPG")
            << true;

    QTest::newRow("phone timestamp sequence")
            << QString::fromLatin1("20220111_153300.JPG")
            << QString::fromLatin1("20220111_153301.JPG")
            << true;

    QTest::newRow("phone timestamp minute rollover")
            << QString::fromLatin1("20220111_153359.JPG")
            << QString::fromLatin1("20220111_153400.JPG")
            << true;

    QTest::newRow("phone timestamp day rollover")
            << QString::fromLatin1("20220111_235959.JPG")
            << QString::fromLatin1("20220112_000000.JPG")
            << true;

    QTest::newRow("hp point and shoot sequence")
            << QString::fromLatin1("HPIM0001.JPG")
            << QString::fromLatin1("HPIM0002.JPG")
            << true;

    QTest::newRow("gopro photo sequence")
            << QString::fromLatin1("GOPR0001.JPG")
            << QString::fromLatin1("GOPR0002.JPG")
            << true;

    QTest::newRow("gopro chaptered video sequence")
            << QString::fromLatin1("GX010001.MP4")
            << QString::fromLatin1("GX010002.MP4")
            << true;

    QTest::newRow("gopro hero video sequence")
            << QString::fromLatin1("GH013607.MP4")
            << QString::fromLatin1("GH013608.MP4")
            << true;

    QTest::newRow("gap in numbering")
            << QString::fromLatin1("DSC06975.ARW")
            << QString::fromLatin1("DSC06977.ARW")
            << false;

    QTest::newRow("rollover gap")
            << QString::fromLatin1("DSC99999.ARW")
            << QString::fromLatin1("DSC00001.ARW")
            << false;

    QTest::newRow("rollover requires same width")
            << QString::fromLatin1("DSC999.ARW")
            << QString::fromLatin1("DSC0000.ARW")
            << false;

    QTest::newRow("counter gap that looks like time without date prefix")
            << QString::fromLatin1("DSC001533.JPG")
            << QString::fromLatin1("DSC001600.JPG")
            << false;

    QTest::newRow("timestamp with different prefix text")
            << QString::fromLatin1("PXL_20260612_235959123.JPG")
            << QString::fromLatin1("IMG_20260613_000000123.JPG")
            << false;

    QTest::newRow("different suffix")
            << QString::fromLatin1("DSC06975.ARW")
            << QString::fromLatin1("DSC06976.JPG")
            << false;

    QTest::newRow("different numbered extension suffix")
            << QString::fromLatin1("IMG_0001.CR2")
            << QString::fromLatin1("IMG_0002.CR3")
            << false;

    QTest::newRow("different burst suffix")
            << QString::fromLatin1("IMG_20260612_161829_BURST001_EDIT.JPG")
            << QString::fromLatin1("IMG_20260612_161829_BURST002.JPG")
            << false;

    QTest::newRow("different prefix")
            << QString::fromLatin1("DSC06975.ARW")
            << QString::fromLatin1("IMG06976.ARW")
            << false;

    QTest::newRow("first filename without number")
            << QString::fromLatin1("README")
            << QString::fromLatin1("README2")
            << false;
}

void TimelapseFilenameMatchTest::testDirectlyPreceeds()
{
    QFETCH(QString, first);
    QFETCH(QString, second);
    QFETCH(bool,    expected);

    const TimelapseFilenameMatch firstMatch(first);
    const TimelapseFilenameMatch secondMatch(second);

    qCDebug(DIGIKAM_TESTS_LOG) << first << "directly precedes" << second
                               << firstMatch.directlyPreceeds(secondMatch);

    QCOMPARE(firstMatch.directlyPreceeds(secondMatch), expected);
}

void TimelapseFilenameMatchTest::testSequenceOrder_data()
{
    QTest::addColumn<QStringList>("filenames");
    QTest::addColumn<QStringList>("expectedOrder");

    QTest::newRow("regular sequence")
            << QStringList
               {
                   QString::fromLatin1("DSC06975.ARW"),
                   QString::fromLatin1("DSC06976.ARW"),
                   QString::fromLatin1("DSC06977.ARW")
               }
            << QStringList
               {
                   QString::fromLatin1("DSC06975.ARW"),
                   QString::fromLatin1("DSC06976.ARW"),
                   QString::fromLatin1("DSC06977.ARW")
               };

    QTest::newRow("camera raw rollover sequence")
            << QStringList
               {
                   QString::fromLatin1("DSC00000.ARW"),
                   QString::fromLatin1("DSC00001.ARW"),
                   QString::fromLatin1("DSC00002.ARW"),
                   QString::fromLatin1("DSC99998.ARW"),
                   QString::fromLatin1("DSC99999.ARW")
               }
            << QStringList
               {
                   QString::fromLatin1("DSC99998.ARW"),
                   QString::fromLatin1("DSC99999.ARW"),
                   QString::fromLatin1("DSC00000.ARW"),
                   QString::fromLatin1("DSC00001.ARW"),
                   QString::fromLatin1("DSC00002.ARW")
               };

    QTest::newRow("camera raw rollover after other group")
            << QStringList
               {
                   QString::fromLatin1("DSC00000.ARW"),
                   QString::fromLatin1("DSC00001.ARW"),
                   QString::fromLatin1("DSC06975.ARW"),
                   QString::fromLatin1("DSC06976.ARW"),
                   QString::fromLatin1("DSC99998.ARW"),
                   QString::fromLatin1("DSC99999.ARW")
               }
            << QStringList
               {
                   QString::fromLatin1("DSC99998.ARW"),
                   QString::fromLatin1("DSC99999.ARW"),
                   QString::fromLatin1("DSC00000.ARW"),
                   QString::fromLatin1("DSC00001.ARW"),
                   QString::fromLatin1("DSC06975.ARW"),
                   QString::fromLatin1("DSC06976.ARW")
               };

    QTest::newRow("camera raw rollover with unrelated file")
            << QStringList
               {
                   QString::fromLatin1("DSC00000.JPG"),
                   QString::fromLatin1("DSC00001.JPG"),
                   QString::fromLatin1("DSC99999.JPG"),
                   QString::fromLatin1("IMG0001.JPG")
               }
            << QStringList
               {
                   QString::fromLatin1("DSC99999.JPG"),
                   QString::fromLatin1("DSC00000.JPG"),
                   QString::fromLatin1("DSC00001.JPG"),
                   QString::fromLatin1("IMG0001.JPG")
               };

    QTest::newRow("no rollover without high counter")
            << QStringList
               {
                   QString::fromLatin1("DSC00000.ARW"),
                   QString::fromLatin1("DSC00001.ARW"),
                   QString::fromLatin1("DSC06975.ARW")
               }
            << QStringList
               {
                   QString::fromLatin1("DSC00000.ARW"),
                   QString::fromLatin1("DSC00001.ARW"),
                   QString::fromLatin1("DSC06975.ARW")
               };

    QTest::newRow("different suffix at counter bounds")
            << QStringList
               {
                   QString::fromLatin1("DSC00000.JPG"),
                   QString::fromLatin1("DSC99999.ARW")
               }
            << QStringList
               {
                   QString::fromLatin1("DSC00000.JPG"),
                   QString::fromLatin1("DSC99999.ARW")
               };
}

void TimelapseFilenameMatchTest::testSequenceOrder()
{
    QFETCH(QStringList, filenames);
    QFETCH(QStringList, expectedOrder);

    QList<TimelapseFilenameMatch> matches;
    matches.reserve(filenames.size());

    const QStringList& constFilenames = filenames;

    for (const QString& filename : constFilenames)
    {
        matches.append(TimelapseFilenameMatch(filename));
    }

    const QList<qsizetype> sequenceOrder = timelapseFilenameSequenceOrder(matches);
    QStringList actualOrder;
    actualOrder.reserve(sequenceOrder.size());

    for (const qsizetype index : sequenceOrder)
    {
        actualOrder.append(filenames.at(index));
    }

    QCOMPARE(actualOrder, expectedOrder);
}

#include "moc_timelapsefilenamematch_utest.cpp"

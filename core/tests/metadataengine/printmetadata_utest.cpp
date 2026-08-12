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

#include "printmetadata_utest.h"

// Qt includes

#include <QTextStream>

QTEST_MAIN(PrintMetadataTest)

PrintMetadataTest::PrintMetadataTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void PrintMetadataTest::printMetadataMap(const DMetadata::MetaDataMap& map)
{
    QString output;
    QTextStream stream(&output);
    stream << Qt::endl;

    qCDebug(DIGIKAM_TESTS_LOG) << "Found" << map.size() << "tags:" << Qt::endl;

    for (DMetadata::MetaDataMap::const_iterator it = map.constBegin() ;
         it != map.constEnd() ; ++it)
    {
        QString key   = it.key();
        QString value = it.value();

        // None of these strings can be null, event if strings are translated.

        QVERIFY(!key.isNull());

        if (key != QLatin1String("Exif.Photo.UserComment"))
        {
            QVERIFY(!value.isNull());
        }

        QString tagName = key.simplified();
        tagName.append(QString().fill(QLatin1Char(' '), 48 - tagName.length()));

        QString tagVal  = value.simplified();

        if (tagVal.length() > 48)
        {
            tagVal.truncate(48);
            tagVal.append(QString::fromLatin1("... (%1 bytes)").arg(value.length()));
        }

        stream << tagName << " : " << tagVal << Qt::endl;
    }

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << output;
}

void PrintMetadataTest::testPrintMetadata()
{
    //                                                    Expected tags to found in Exif,  Iptc,  Xmp,   expectedRead
    printMetadata(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"),         true,  true,  true,  true);
    printMetadata(m_originalImageFolder + QLatin1String("_27A1417.CR2"),            true,  false, true,  true);
    printMetadata(m_originalImageFolder + QLatin1String("2008-05_DSC_0294.JPG"),    true,  true,  true,  true);

    if (m_hasExifTool)
    {
        printMetadata(m_originalImageFolder + QLatin1String("kepler_xray_he.fits"), false, false, false, true);
    }

    // The file cannot be loaded with Exiv2-0.26, only test the newer versions

    bool ok = true;

    if ((MetaEngine::Exiv2Version().section(QLatin1Char('.'), 0, 1).toDouble(&ok) > 0.26) && ok)
    {
//        printMetadata(m_originalImageFolder + QLatin1String("20160821035715.jpg"), false, false, false, false);
    }
}

void PrintMetadataTest::printMetadata(const QString& filePath, bool exif, bool iptc, bool xmp, bool expectedRead)
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    bool ret = meta->load(filePath);
    QCOMPARE(ret, expectedRead);

    loadExif(*meta, exif);
    loadIptc(*meta, iptc);
    loadXmp(*meta,  xmp);
}

void PrintMetadataTest::loadExif(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Exif metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getExifTagsDataList();
    QCOMPARE(!map.isEmpty(), expected);

    printMetadataMap(map);
}

void PrintMetadataTest::loadIptc(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Iptc metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getIptcTagsDataList();
    QCOMPARE(!map.isEmpty(), expected);

    printMetadataMap(map);
}

void PrintMetadataTest::loadXmp(const DMetadata& meta, bool expected)
{
    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Xmp metadata from %1 --").arg(meta.getFilePath());

    if (meta.supportXmp())
    {
        DMetadata::MetaDataMap map = meta.getXmpTagsDataList();
        QCOMPARE(!map.isEmpty(), expected);

        printMetadataMap(map);
    }
    else
    {
        qWarning() << QLatin1String("Exiv2 has no XMP support...");
    }
}

#include "moc_printmetadata_utest.cpp"

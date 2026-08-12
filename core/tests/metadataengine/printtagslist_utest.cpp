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

#include "printtagslist_utest.h"

// Qt includes

#include <QTextStream>

QTEST_MAIN(PrintTagsListTest)

PrintTagsListTest::PrintTagsListTest(QObject* const parent)
    : AbstractUnitTest(parent)
{
}

void PrintTagsListTest::parseTagsList(const DMetadata::TagsMap& tags)
{
    QString output;
    QTextStream stream(&output);
    stream << Qt::endl;

    qCDebug(DIGIKAM_TESTS_LOG) << "Found" << tags.size() << "tags:" << Qt::endl;

    for (DMetadata::TagsMap::const_iterator it = tags.constBegin() ; it != tags.constEnd() ; ++it )
    {
        QString     key    = it.key();
        QStringList values = it.value();
        QString     name   = values[0];
        QString     title  = values[1];
        QString     desc   = values[2];

        // None of these strings can be null, event if strings are translated.
        QVERIFY(!key.isNull());
        QVERIFY(!name.isNull());
        QVERIFY(!title.isNull());
        QVERIFY(!desc.isNull());

        stream << key << Qt::endl
               << "    " << name  << Qt::endl
               << "    " << title << Qt::endl
               << "    " << desc  << Qt::endl;
    }

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << output;
}

void PrintTagsListTest::testPrintAllAvailableStdExifTags()
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    qCDebug(DIGIKAM_TESTS_LOG) << "-- Standard Exif Tags -------------------------------------------------------------";

    DMetadata::TagsMap exiftags = meta->getStdExifTagsList();
    QVERIFY(!exiftags.isEmpty());

    parseTagsList(exiftags);
}

void PrintTagsListTest::testPrintAllAvailableMakernotesTags()
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    qCDebug(DIGIKAM_TESTS_LOG) << "-- Makernote Tags -----------------------------------------------------------------";

    DMetadata::TagsMap mntags = meta->getMakernoteTagsList();

    QVERIFY(!mntags.isEmpty());

    parseTagsList(mntags);
}

void PrintTagsListTest::testPrintAllAvailableIptcTags()
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    qCDebug(DIGIKAM_TESTS_LOG) << "-- Standard Iptc Tags -----------------------------------------------------------------";

    DMetadata::TagsMap iptctags = meta->getIptcTagsList();

    QVERIFY(!iptctags.isEmpty());

    parseTagsList(iptctags);
}

void PrintTagsListTest::testPrintAllAvailableXmpTags()
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    qCDebug(DIGIKAM_TESTS_LOG) << "-- Standard Xmp Tags -----------------------------------------------------------------";

    DMetadata::TagsMap xmptags = meta->getXmpTagsList();

    if (meta->supportXmp())
    {
        QVERIFY(!xmptags.isEmpty());

        parseTagsList(xmptags);
    }
    else
    {
        qWarning() << QLatin1String("Exiv2 has no XMP support...");
    }
}

#include "moc_printtagslist_utest.cpp"

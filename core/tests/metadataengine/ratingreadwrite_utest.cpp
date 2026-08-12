/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : metadata settings tests for getImageRating and setImageRating.
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ratingreadwrite_utest.h"

// Qt includes

#include <QTest>
#include <QString>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(RatingReadWriteTest)

RatingReadWriteTest::RatingReadWriteTest(QObject* const parent)
    : QObject(parent)
{
}

void RatingReadWriteTest::initTestCase()
{
    MetaEngine::initializeExiv2();
    qCDebug(DIGIKAM_TESTS_LOG) << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();
}

void RatingReadWriteTest::cleanupTestCase()
{
}

void RatingReadWriteTest::testSimpleReadAfterWrite()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);

    // Trick dmetadata, so it will think that we have a file path
    dmeta->setFilePath(QLatin1String("random.org"));
    int rez = -1;

    qCDebug(DIGIKAM_TESTS_LOG) << dmeta->supportXmp();

    for (int i = 0; i < 6; i++)
    {
        dmeta->setItemRating(i);
        rez = dmeta->getItemRating();
        QCOMPARE(rez, i);
    }
}

void RatingReadWriteTest::testWriteToDisabledNamespaces()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);
    dmeta->setFilePath(QLatin1String("random.org"));

    DMetadataSettingsContainer dmsettings;

    QList<int> defaultVal, microsoftMappings, iptcMappings;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    iptcMappings << 8 << 6 << 5 << 4 << 2 << 1;

    NamespaceEntry ratingNs2;
    ratingNs2.namespaceName = QLatin1String("Xmp.acdsee.rating");
    ratingNs2.convertRatio  = defaultVal;
    ratingNs2.nsType        = NamespaceEntry::RATING;
    ratingNs2.index         = 1;
    ratingNs2.subspace = NamespaceEntry::XMP;
    ratingNs2.isDisabled    = true;

    NamespaceEntry ratingNs3;
    ratingNs3.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.Rating");
    ratingNs3.convertRatio  = microsoftMappings;
    ratingNs3.nsType        = NamespaceEntry::RATING;
    ratingNs3.index         = 2;
    ratingNs3.subspace = NamespaceEntry::XMP;


    dmsettings.getWriteMapping(NamespaceEntry::DM_RATING_CONTAINER()).clear();
    dmsettings.getWriteMapping(NamespaceEntry::DM_RATING_CONTAINER())
             << ratingNs2
             << ratingNs3;

    for (int i = 0; i < 6; i++)
    {
        dmeta->setItemRating(i, dmsettings);

        QString data;
        bool ok;

        data    = dmeta->getXmpTagString("Xmp.acdsee.rating", false);

        QVERIFY(data.isEmpty());

        data    = dmeta->getXmpTagString("Xmp.MicrosoftPhoto.Rating", false);
        int rez = data.toInt(&ok);

        QCOMPARE(ok, true);
        QCOMPARE(rez, microsoftMappings.at(i));
    }
}

void RatingReadWriteTest::testReadFromDisabledNamespaces()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);
    dmeta->setFilePath(QLatin1String("random.org"));

    DMetadataSettingsContainer dmsettings;

    QList<int> defaultVal, microsoftMappings, iptcMappings;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    iptcMappings << 8 << 6 << 5 << 4 << 2 << 1;

    NamespaceEntry ratingNs2;
    ratingNs2.namespaceName = QLatin1String("Xmp.acdsee.rating");
    ratingNs2.convertRatio  = defaultVal;
    ratingNs2.nsType        = NamespaceEntry::RATING;
    ratingNs2.index         = 1;
    ratingNs2.subspace = NamespaceEntry::XMP;
    ratingNs2.isDisabled    = true;

    NamespaceEntry ratingNs3;
    ratingNs3.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.Rating");
    ratingNs3.convertRatio  = microsoftMappings;
    ratingNs3.nsType        = NamespaceEntry::RATING;
    ratingNs3.index         = 2;
    ratingNs3.subspace = NamespaceEntry::XMP;


    dmsettings.getReadMapping(NamespaceEntry::DM_RATING_CONTAINER()).clear();
    dmsettings.getReadMapping(NamespaceEntry::DM_RATING_CONTAINER())
             << ratingNs2
             << ratingNs3;

    for (int i = 0; i < 6; i++)
    {

        dmeta->setXmpTagString("Xmp.acdsee.rating", QString::number(5-i));
        dmeta->setXmpTagString("Xmp.MicrosoftPhoto.Rating", QString::number(microsoftMappings.at(i)));

        int rez = dmeta->getItemRating(dmsettings);
        QCOMPARE(rez, i);
    }
}

#include "moc_ratingreadwrite_utest.cpp"

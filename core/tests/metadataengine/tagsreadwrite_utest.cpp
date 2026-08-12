/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : metadata settings tests for getImageTagPaths and setImageTagPaths.
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagsreadwrite_utest.h"

// Qt includes

#include <QTest>
#include <QString>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(TagsReadWriteTest)

TagsReadWriteTest::TagsReadWriteTest(QObject* const parent)
    : QObject(parent)
{
}

void TagsReadWriteTest::initTestCase()
{
    MetaEngine::initializeExiv2();
    qCDebug(DIGIKAM_TESTS_LOG) << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();

    this->tagSet1  << QLatin1String("/root/child1/child2")
                   << QLatin1String("/root/extra/child2/triple")
                   << QLatin1String("/root/extra/ch223/triple");

    this->tagSet2  << QLatin1String("/root/child1/chilasdfasdf")
                   << QLatin1String("/root/exrebhtra/chsdrild2/asdfad")
                   << QLatin1String("/root/exfgvdtra/ch2gfg23/tridfgvle");

    this->tagSet3  << QLatin1String("/rowet/child1/crehild2")
                   << QLatin1String("/rsdfsoot/extsdera/chihgld2/triple")
                   << QLatin1String("/roosdst/extfamnbra/ch2hg23/triple");
}

void TagsReadWriteTest::cleanupTestCase()
{
}

void TagsReadWriteTest::testSimpleReadAfterWrite()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);
    QStringList tagPaths2;

    dmeta->setItemTagsPath(this->tagSet1);
    dmeta->getItemTagsPath(tagPaths2);

    QCOMPARE(tagSet1, tagPaths2);
}

void TagsReadWriteTest::testWriteToDisabledNamespaces()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);

    DMetadataSettingsContainer dmsettings;
    QStringList empty;
    QStringList secondNamespace;

    NamespaceEntry tagNs2;
    tagNs2.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP");
    tagNs2.tagPaths      = NamespaceEntry::TAGPATH;
    tagNs2.separator     = QLatin1Char('/');
    tagNs2.nsType        = NamespaceEntry::TAGS;
    tagNs2.index         = 1;
    tagNs2.specialOpts   = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace      = NamespaceEntry::XMP;
    tagNs2.isDisabled    = true;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1Char('|');
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getWriteMapping(NamespaceEntry::DM_TAG_CONTAINER()).clear();
    dmsettings.getWriteMapping(NamespaceEntry::DM_TAG_CONTAINER())
             << tagNs2
             << tagNs3;

    dmeta->setItemTagsPath(tagSet1, dmsettings);

    empty           = dmeta->getXmpTagStringBag("Xmp.MicrosoftPhoto.LastKeywordXMP", false);

    QCOMPARE(empty, QStringList());

    secondNamespace = dmeta->getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    secondNamespace.replaceInStrings(QLatin1String("|"), QLatin1String("/"));

    QCOMPARE(secondNamespace, tagSet1);
}

void TagsReadWriteTest::testReadFromDisabledNamespaces()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);

    DMetadataSettingsContainer dmsettings;
    QStringList toWrite;
    QStringList actual;

    NamespaceEntry tagNs2;
    tagNs2.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP");
    tagNs2.tagPaths      = NamespaceEntry::TAGPATH;
    tagNs2.separator     = QLatin1Char('/');
    tagNs2.nsType        = NamespaceEntry::TAGS;
    tagNs2.index         = 1;
    tagNs2.specialOpts   = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace      = NamespaceEntry::XMP;
    tagNs2.isDisabled    = true;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1Char('|');
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER()).clear();
    dmsettings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER())
             << tagNs2
             << tagNs3;

    dmeta->setXmpTagStringBag("Xmp.MicrosoftPhoto.LastKeywordXMP", tagSet1);

    toWrite = tagSet2;
    toWrite.replaceInStrings(QLatin1String("/"), QLatin1String("|"));
    dmeta->setXmpTagStringBag("Xmp.lr.hierarchicalSubject", toWrite);

    dmeta->getItemTagsPath(actual, dmsettings);

    QCOMPARE(actual, tagSet2);
}

void TagsReadWriteTest::testTagSeparatorWrite()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);

    DMetadataSettingsContainer dmsettings;
    QStringList readResult;
    QStringList expected;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1Char('|');
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getWriteMapping(NamespaceEntry::DM_TAG_CONTAINER()).clear();
    dmsettings.getWriteMapping(NamespaceEntry::DM_TAG_CONTAINER())
             << tagNs3;

    dmeta->setItemTagsPath(tagSet1, dmsettings);

    readResult = dmeta->getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    expected   = tagSet1;
    expected.replaceInStrings(QLatin1String("/"), QLatin1String("|"));

    QCOMPARE(readResult, expected);
}

void TagsReadWriteTest::testTagSeparatorRead()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);

    DMetadataSettingsContainer dmsettings;
    QStringList toWrite;
    QStringList actual;
    QStringList reference;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1Char('|');
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER()).clear();
    dmsettings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER())
             << tagNs3;

    toWrite   = tagSet1;
    toWrite.replaceInStrings(QLatin1String("/"), QLatin1String("|"));

    dmeta->setXmpTagStringBag("Xmp.lr.hierarchicalSubject", toWrite);

    reference = dmeta->getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    QCOMPARE(reference, toWrite);

    dmeta->getItemTagsPath(actual, dmsettings);

    QCOMPARE(actual, tagSet1);
}

void TagsReadWriteTest::testTagReadAlternativeNameSpace()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);

    DMetadataSettingsContainer dmsettings;
    QStringList toWrite;
    QStringList actual;
    QStringList reference;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1Char('|');
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER()).clear();
    dmsettings.getReadMapping(NamespaceEntry::DM_TAG_CONTAINER())
             << tagNs3;

    toWrite   = tagSet1;
    toWrite.replaceInStrings(QLatin1String("/"), QLatin1String("|"));

    dmeta->setXmpTagStringSeq("Xmp.lr.HierarchicalSubject", toWrite);

    // We write some data to alternative namespace
    reference = dmeta->getXmpTagStringSeq("Xmp.lr.HierarchicalSubject", false);

    QCOMPARE(reference, toWrite);

    dmeta->getItemTagsPath(actual, dmsettings);

    QCOMPARE(actual, tagSet1);
}

#include "moc_tagsreadwrite_utest.cpp"

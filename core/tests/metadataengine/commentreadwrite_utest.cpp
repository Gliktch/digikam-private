/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : metadata settings tests for getImageComment and setImageComment.
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "commentreadwrite_utest.h"

// Qt includes

#include <QTest>
#include <QString>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"

QTEST_GUILESS_MAIN(CommentReadWriteTest)

using namespace Digikam;

CommentReadWriteTest::CommentReadWriteTest(QObject* const parent)
    : QObject(parent)
{
}

void CommentReadWriteTest::initTestCase()
{
    MetaEngine::initializeExiv2();
    qCDebug(DIGIKAM_TESTS_LOG) << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();

    MetaEngine::AltLangMap authorsMap,  authorsMap2;
    MetaEngine::AltLangMap datesMap,    datesMap2;
    MetaEngine::AltLangMap commentsMap, commentsMap2;
    QString commonAuthor, commonAuthor2;

    authorsMap.insert(QLatin1String("x-default"), QLatin1String("Veaceslav"));
    commentsMap.insert(QLatin1String("x-default"), QLatin1String("Veaceslav's comment"));
    commonAuthor  = QLatin1String("Veaceslav");

    commentSet1.setData(commentsMap, authorsMap, commonAuthor, datesMap);

    authorsMap2.insert(QLatin1String("x-default"), QLatin1String("Munteanu"));
    commentsMap2.insert(QLatin1String("x-default"), QLatin1String("Munteanu's comment"));
    commonAuthor2 = QLatin1String("Munteanu");

    commentSet2.setData(commentsMap2, authorsMap2, commonAuthor2, datesMap2);
}

void CommentReadWriteTest::cleanupTestCase()
{
}

void CommentReadWriteTest::testSimpleReadAfterWrite()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);
    CaptionsMap result;

    // Trick dmetadata, so it will think that we have a file path
    dmeta->setFilePath(QLatin1String("random.org"));

    dmeta->setItemComments(commentSet1);

    result = dmeta->getItemComments();

    QString rezAuthor  = result.value(QLatin1String("x-default")).author;
    QString rezComment = result.value(QLatin1String("x-default")).caption;

    QCOMPARE(rezAuthor,  commentSet1.value(QLatin1String("x-default")).author);
    QCOMPARE(rezComment, commentSet1.value(QLatin1String("x-default")).caption);
}

void CommentReadWriteTest::testWriteToDisabledNamespaces()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);
    dmeta->setFilePath(QLatin1String("random.org"));

    MetaEngine::AltLangMap commentsMap;
    QString commentString;

    DMetadataSettingsContainer dmsettings;

    NamespaceEntry commNs1;
    commNs1.namespaceName   = QLatin1String("Xmp.dc.description");
    commNs1.nsType          = NamespaceEntry::COMMENT;
    commNs1.specialOpts     = NamespaceEntry::COMMENT_ATLLANGLIST;
    commNs1.index           = 0;
    commNs1.subspace        = NamespaceEntry::XMP;
    commNs1.isDisabled      = true;

    NamespaceEntry commNs2;
    commNs2.namespaceName   = QLatin1String("Xmp.exif.UserComment");
    commNs2.nsType          = NamespaceEntry::COMMENT;
    commNs2.specialOpts     = NamespaceEntry::COMMENT_ALTLANG;
    commNs2.index           = 1;
    commNs2.subspace        = NamespaceEntry::XMP;

    dmsettings.setUnifyReadWrite(false);

    dmsettings.getWriteMapping(NamespaceEntry::DM_COMMENT_CONTAINER()).clear();
    dmsettings.getWriteMapping(NamespaceEntry::DM_COMMENT_CONTAINER())
             << commNs1
             << commNs2;

    bool rez = dmeta->setItemComments(commentSet1, dmsettings);

    QVERIFY(rez);

    commentsMap   = dmeta->getXmpTagStringListLangAlt("Xmp.dc.description", false);

    QCOMPARE(commentsMap.value(QLatin1String("x-default")), QString());

    commentString = dmeta->getXmpTagStringLangAlt("Xmp.exif.UserComment", QString(), false);

    QCOMPARE(commentString, commentSet1.value(QLatin1String("x-default")).caption);
}

void CommentReadWriteTest::testReadFromDisabledNamespaces()
{
    QScopedPointer<DMetadata> dmeta(new DMetadata);
    dmeta->setFilePath(QLatin1String("random.org"));

    CaptionsMap rez;

    DMetadataSettingsContainer dmsettings;

    NamespaceEntry commNs1;
    commNs1.namespaceName   = QLatin1String("Xmp.dc.description");
    commNs1.nsType          = NamespaceEntry::COMMENT;
    commNs1.specialOpts     = NamespaceEntry::COMMENT_ATLLANGLIST;
    commNs1.index           = 0;
    commNs1.subspace        = NamespaceEntry::XMP;
    commNs1.isDisabled      = true;

    NamespaceEntry commNs2;
    commNs2.namespaceName   = QLatin1String("Xmp.exif.UserComment");
    commNs2.nsType          = NamespaceEntry::COMMENT;
    commNs2.specialOpts     = NamespaceEntry::COMMENT_ALTLANG;
    commNs2.index           = 1;
    commNs2.subspace        = NamespaceEntry::XMP;

    dmsettings.setUnifyReadWrite(false);
    dmsettings.getReadMapping(NamespaceEntry::DM_COMMENT_CONTAINER()).clear();
    dmsettings.getReadMapping(NamespaceEntry::DM_COMMENT_CONTAINER())
             << commNs1
             << commNs2;

    dmeta->setXmpTagStringListLangAlt("Xmp.dc.description", commentSet1.toAltLangMap());
    dmeta->setXmpTagStringLangAlt("Xmp.exif.UserComment", commentSet2.value(QLatin1String("x-default")).caption, QString());

    rez = dmeta->getItemComments(dmsettings);
    QCOMPARE(rez.value(QLatin1String("x-default")).caption, commentSet2.value(QLatin1String("x-default")).caption);
}

#include "moc_commentreadwrite_utest.cpp"

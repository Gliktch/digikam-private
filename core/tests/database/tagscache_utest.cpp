/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-20
 * Description : Unit tests for TagsCache class
 *
 * SPDX-FileCopyrightText: 2021 by David Haslam, <dch dot code at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagscache_utest.h"

// C++ includes

#include <set>

// Qt includes

#include <QString>
#include <QDebug>
#include <QSqlDatabase>

// Local includes

#include "coredb.h"
#include "coredbaccess.h"

TagsCacheTest::TagsCacheTest(QObject* const parent)
    : QObject  (parent),
      tagsCache(nullptr)
{
}

TagsCacheTest::~TagsCacheTest()
{
}

void TagsCacheTest::initTestCase()
{
    qDebug() << "initTestCase()";

    Digikam::DbEngineParameters db_params
    (
        QString::fromUtf8("QSQLITE"),      // databaseType
        QString::fromUtf8(":memory:"),     // databaseNameCore
        QString::fromUtf8("")              // connectOptions
    );

    Digikam::CoreDbAccess::setParameters(db_params);
    QVERIFY(Digikam::CoreDbAccess::checkReadyForUse());
    tagsCache = Digikam::TagsCache::instance();
}

void TagsCacheTest::cleanupTestCase()
{
    qDebug() << "cleanupTestCase()";
}

void TagsCacheTest::init()
{
    QCOMPARE(countTags(), 0);
}

void TagsCacheTest::cleanup()
{
    auto coredb     = Digikam::CoreDbAccess().db();
    const auto tags = coredb->getTagShortInfos();

    for (const auto& tag : tags)
    {
        coredb->deleteTag(tag.id);
    }

    QCOMPARE(countTags(), 0);
}

void TagsCacheTest::testSimpleHierarchy()
{
    auto id       = tagsCache->createTag(QLatin1String("Top/Middle/Bottom"));

    dumpTags();
    QCOMPARE(countTags(), 3);

    auto bottomId = tagsCache->tagForPath(QLatin1String("Top/Middle/Bottom"));
    auto middleId = tagsCache->parentTag(id);
    auto topId    = tagsCache->parentTag(middleId);

    QCOMPARE(id, bottomId);
    QVERIFY(bottomId);
    QVERIFY(middleId);
    QVERIFY(topId);
    QVERIFY(tagsCache->hasTag(bottomId));
    QVERIFY(tagsCache->hasTag(middleId));
    QVERIFY(tagsCache->hasTag(topId));
    QCOMPARE(tagsCache->tagName(id),       QLatin1String("Bottom"));
    QCOMPARE(tagsCache->tagName(middleId), QLatin1String("Middle"));
    QCOMPARE(tagsCache->tagName(topId),    QLatin1String("Top"));

    auto bottomIds = tagsCache->tagsForName(QLatin1String("Bottom"));

    QCOMPARE(bottomIds.size(), 1);
    QCOMPARE(bottomIds[0], bottomId);
}

void TagsCacheTest::testComplexHierarchy()
{
    tagsCache->createTag(QLatin1String("Top/Middle"));
    tagsCache->createTag(QLatin1String("Top/Middle/First"));
    tagsCache->createTag(QLatin1String("Top/Middle/Second"));
    tagsCache->createTag(QLatin1String("Super/Top"));
    tagsCache->createTag(QLatin1String("Super/Top/First"));
    tagsCache->createTag(QLatin1String("Super/Top/Second"));
    tagsCache->createTag(QLatin1String("Super/Top/Third"));
    tagsCache->createTag(QLatin1String("Mixed Up/Third/Second"));
    tagsCache->createTag(QLatin1String("Mixed Up/Third/First"));

    QCOMPARE(countTags(), 13);

    auto top_ids   = tagsCache->tagsForName(QLatin1String("Top"));
    QCOMPARE(top_ids.size(), 2);

    auto first_ids = tagsCache->tagsForName(QLatin1String("First"));
    QCOMPARE(first_ids.size(), 3);
}

void TagsCacheTest::testRepeatedNames()
{
    tagsCache->createTag(QLatin1String("Repeat Me/Repeat Me/Repeat Me/Repeat Me"));

    const auto repeatMeIds = tagsCache->tagsForName(QLatin1String("Repeat Me"));
    QCOMPARE(repeatMeIds.size(), 4);

    // all ids should be unique

    std::set<int> set;

    for (const auto& id : repeatMeIds)
    {
        auto result = set.insert(id);
        QVERIFY(result.second);
    }
}

void TagsCacheTest::testDuplicateTop()
{
    tagsCache->createTag(QLatin1String("Top/Middle/Bottom"));
    tagsCache->createTag(QLatin1String("Super/Top/Middle/Bottom"));
    tagsCache->createTag(QLatin1String("First/Super/Top"));
    dumpTags();
    QCOMPARE(countTags(), 10);

    // the single word 'Top' should match the top tag

    auto id1 = tagsCache->tagForPath(QLatin1String("Top"));
    QVERIFY(id1);
    QCOMPARE(tagsCache->tagPath(id1), QLatin1String("/Top"));

    // and it doesn't matter if there's a leading slash

    auto id2 = tagsCache->tagForPath(QLatin1String("/Top"));
    QVERIFY(id2);
    QCOMPARE(id1, id2);
    QCOMPARE(tagsCache->tagPath(id2), QLatin1String("/Top"));

    // a more complex request

    auto id3 = tagsCache->tagForPath(QLatin1String("Super/Top"));
    QVERIFY(id3);
    QCOMPARE(tagsCache->tagPath(id3), QLatin1String("/Super/Top"));
}

// utilities

int TagsCacheTest::countTags()
{
    const auto coredb = Digikam::CoreDbAccess().db();                // cppcheck-suppress constVariablePointer
    auto tags         = coredb->getTagShortInfos();

    return tags.size();
}

// debug

void TagsCacheTest::dumpTables()
{
    auto sqlnames = QSqlDatabase::connectionNames();
    auto sqldb    = QSqlDatabase::database(sqlnames[0]);
    qDebug() << sqldb.tables();
}

void TagsCacheTest::dumpTags()
{
    const auto coredb = Digikam::CoreDbAccess().db();                // cppcheck-suppress constVariablePointer
    auto tags         = coredb->getTagShortInfos();

    for (auto it = tags.begin() ; it != tags.end() ; ++it)
    {
        qDebug() << it->id << it->pid << it->name;
    }
}

QTEST_GUILESS_MAIN(TagsCacheTest)

#include "moc_tagscache_utest.cpp"

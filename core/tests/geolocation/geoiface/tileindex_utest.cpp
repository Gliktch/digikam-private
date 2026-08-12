/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-12
 * Description : Test the TileIndex class
 *
 * SPDX-FileCopyrightText: 2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tileindex_utest.h"

// C++ includes

#include <cmath>

// Local includes

#include "tileindex.h"

using namespace Digikam;

void TestTileIndex::testNoOp()
{
}

void TestTileIndex::testBasics()
{
    {
        // an empty index:
        TileIndex index1;
        QCOMPARE(index1.indexCount(), 0);
        QCOMPARE(index1.level(), 0);
        QVERIFY(index1.toIntList().isEmpty());

        TileIndex index2 = TileIndex::fromIntList(QIntList());
        QCOMPARE(index2.indexCount(), 0);
        QCOMPARE(index2.level(), 0);
        QVERIFY(index2.toIntList().isEmpty());
    }

    {
        // appending indexes:
        TileIndex index1;
        index1.appendLinearIndex(1);
        QCOMPARE(index1.indexCount(), 1);
        QCOMPARE(index1.level(), 0);
        QCOMPARE(index1.at(0), 1);
        QCOMPARE(index1.lastIndex(), 1);
    }
}

void TestTileIndex::testResizing()
{
    TileIndex i1 = TileIndex::fromIntList(QIntList()<<1<<2<<3<<4);
    QCOMPARE(i1.indexCount(), 4);

    TileIndex i2 = i1.mid(1, 2);
    QCOMPARE(i2.indexCount(), 2);
    QCOMPARE(i2.at(0), 2);
    QCOMPARE(i2.at(1), 3);

    TileIndex i3 = i1;
    QCOMPARE(i3.indexCount(), 4);
    i3.oneUp();
    QCOMPARE(i3.indexCount(), 3);
    i3.oneUp();
    QCOMPARE(i3.indexCount(), 2);
}

void TestTileIndex::testIntListInteraction()
{
    {
        for (int l = 0; l <= TileIndex::MaxLevel; ++l)
        {
            QIntList myList;
            TileIndex i1;

            for (int i = 0; i <= l; ++i)
            {
                i1.appendLinearIndex(i);
                myList << i;
            }

            const TileIndex i2 = TileIndex::fromIntList(myList);
            QVERIFY(TileIndex::indicesEqual(i1, i2, l));

            const QIntList il2 = i1.toIntList();
            QCOMPARE(myList, il2);
        }
    }
}

void TestTileIndex::testFromCoordinates()
{
    auto comp = [] (std::initializer_list<int> list, double lat, double lon, int level)
    {
        auto tileIndex = TileIndex::fromCoordinates(GeoCoordinates(lat, lon), level);
        QCOMPARE(tileIndex.toIntList(), QIntList{list});
        // should this work?
        // QCOMPARE(TileIndex::fromCoordinates(tileIndex.toCoordinates(), level).toIntList(), QIntList{list});
    };

    QList<double> latSizes;
    QList<double> lonSizes;
    double latOne = -90;
    double lonOne = -180;
    double lat123 = -90;
    double lon321 = -180;
    for (int i = 0; i <= TileIndex::MaxLevel; ++i) {
        latSizes.push_back(180. * std::pow(10, -i-1));
        lonSizes.push_back(360. * std::pow(10, -i-1));
        // construct coordinate at always the second tile on level i
        latOne += latSizes.back();
        lonOne += lonSizes.back();
        // construct coordinate at always the i-th tile in latitude and (9-i)-th tile in longitude at level i
        lat123 += i * latSizes.back();
        lon321 += (9-i) * lonSizes.back();
    }

    comp({0,0,0,0,0,0,0,0,0,0}, -90,-180, TileIndex::MaxLevel);
    comp({99,99,99,99,99,99,99,99,99,99}, 90,180, TileIndex::MaxLevel);
    comp({55,0,0,0,0,0,0,0,0,0}, 0,0, TileIndex::MaxLevel);
    comp({11,11,11,11,11,11,11,11,11,11}, latOne, lonOne, TileIndex::MaxLevel);
    comp({55,0,0,0,0,73,0,0,0,0}, latSizes[5] * 7,lonSizes[5] * 3, TileIndex::MaxLevel);
    comp({55,0,0,0,0,3,0,0,90,0}, latSizes[8] * 9,lonSizes[5] * 3, TileIndex::MaxLevel);
    comp({81,0,0,0,0,3,0,0,90,0}, latSizes[0] * 3 + latSizes[8] * 9, lonSizes[0] * -4 + lonSizes[5] * 3, TileIndex::MaxLevel);
    comp({9,18,27,36,45,54,63,72,81,90}, lat123, lon321, TileIndex::MaxLevel);
    comp({9,18,27,36,45,54}, lat123, lon321, 5);
 }

void TestTileIndex::testToCoordinates()
{
    auto comp = [] (std::initializer_list<int> list, double lat, double lon)
    {
        auto coordinates = TileIndex::fromIntList(list).toCoordinates();
        QCOMPARE(coordinates.lat(), lat);
        QCOMPARE(coordinates.lon(), lon);
    };

    comp({55,0,0,0,0}, 0, 0);
    comp({55,0,0,14,0}, std::pow(TileIndex::Tiling, -4.0) * 180.0, 4.0 * std::pow(TileIndex::Tiling, -4.0) * 360.0);
    comp({55,0,0,0,0,0,0,0,0,99}, 9.0 * std::pow(TileIndex::Tiling, -10.0) * 180.0, 9.0 * std::pow(TileIndex::Tiling, -10.0) * 360.0);
    comp({99,99,99,99,99,99,99,99,99,99}, 90.0 - std::pow(TileIndex::Tiling, -10.0) * 180.0, 180.0 - std::pow(TileIndex::Tiling, -10.0) * 360.0);
}

void TestTileIndex::testToCoordinatesCorners()
{
    auto comp = [] (std::initializer_list<int> list)
    {
        auto tileIndex = TileIndex::fromIntList(list);
        auto coordinates = tileIndex.toCoordinates();
        double latHeight = std::pow(0.1, list.size()) * 180.0;
        double lonWidth = std::pow(0.1, list.size()) * 360.0;
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerSW).lat(), coordinates.lat());
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerSW).lon(), coordinates.lon());
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerNW).lat(), coordinates.lat() + latHeight);
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerNW).lon(), coordinates.lon());
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerNE).lat(), coordinates.lat() + latHeight);
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerNE).lon(), coordinates.lon() + lonWidth);
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerSE).lat(), coordinates.lat());
        QCOMPARE(tileIndex.toCoordinates(TileIndex::CornerSE).lon(), coordinates.lon() + lonWidth);
    };

    comp({55,0,0,0,0});
    comp({55,0,0,0,0,0,0,0,0,99});
    comp({99,99,99,99,99,99,99,99,99,99});
    comp({0,0,0,0,0,0,0,0,0,0});
}


/**
 * TileIndex is declared as Q_MOVABLE_TYPE, and here we verify that it still works with QList.
 */
void TestTileIndex::testMovable()
{
    {
        TileIndex i1;

        for (int i = 0; i <= TileIndex::MaxLevel; ++i)
        {
            i1.appendLinearIndex(i);
        }

        TileIndex::List l1;

        for (int i = 0; i < 10; ++i)
        {
            l1 << i1;
        }

        TileIndex::List l2 = l1;
        l2[0]              = l1.at(0);

        for (int i = 0; i < l1.count(); ++i)
        {
            QVERIFY(TileIndex::indicesEqual(i1, l2.at(i), TileIndex::MaxLevel));
        }
    }

//     QBENCHMARK
//     {
//         TileIndex i1;
//
//         for (int i = 0; i <= TileIndex::MaxLevel; ++i)
//         {
//             i1.appendLinearIndex(i);
//         }
//
//         TileIndex::List l1;
//
//         for (int i = 0; i < 100000; ++i)
//         {
//             l1 << i1;
//         }
//
// //         QBENCHMARK
//         {
//             for (int i = 0; i < 100; ++i)
//             {
//                 TileIndex::List l2 = l1;
//                 l2[0]              = i1;
//             }
//         }
//     }
}

QTEST_GUILESS_MAIN(TestTileIndex)

#include "moc_tileindex_utest.cpp"

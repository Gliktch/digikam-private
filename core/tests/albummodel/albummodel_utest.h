/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-11
 * Description : test cases for the various album models
 *
 * SPDX-FileCopyrightText: 2009 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QHash>
#include <QAbstractItemModel>

namespace Digikam
{
class PAlbum;
class TAlbum;
class AlbumModel;
}

class AlbumModelTest: public QObject
{
    Q_OBJECT

public:

    explicit AlbumModelTest(QObject* const parent = nullptr);
    ~AlbumModelTest() override;

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void testPAlbumModel();
    void testDisablePAlbumCount();
    void testDAlbumModel();
    void testDAlbumCount();
    void testDAlbumContainsAlbums();
    void testDAlbumSorting();
    void testTAlbumModel();
    void testSAlbumModel();
    void testStartAlbumModel();

    void deletePAlbum(Digikam::PAlbum* album);

    void setLastPAlbumCountHash(const QHash<int, int>& hash);

    /**
     * slots for ensuring signal order while scanning albums.
     */
    void slotStartModelRowsInserted(const QModelIndex& parent, int start, int end);
    void slotStartModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:

    void ensureItemCounts();

private:

    const QString        albumCategory;

    QString              dbPath;
    QString              tempSuffix;

    Digikam::PAlbum*     palbumRoot0;
    Digikam::PAlbum*     palbumRoot1;
    Digikam::PAlbum*     palbumRoot2;
    Digikam::PAlbum*     palbumChild0Root0;
    Digikam::PAlbum*     palbumChild1Root0;
    Digikam::PAlbum*     palbumChild2Root0;
    Digikam::PAlbum*     palbumChild0Root1;

    Digikam::TAlbum*     rootTag;
    Digikam::TAlbum*     talbumRoot0;
    Digikam::TAlbum*     talbumRoot1;
    Digikam::TAlbum*     talbumChild0Root0;
    Digikam::TAlbum*     talbumChild1Root0;
    Digikam::TAlbum*     talbumChild0Child1Root0;
    Digikam::TAlbum*     talbumChild0Root1;

    QHash<int, int>       palbumCountHash;

    /**
     * This model is used to ensure that adding and changing signals are emitted
     * correctly if the model is created before the scanning starts.
     */
    Digikam::AlbumModel* startModel;
    QList<int>           addedIds;
    QString              imagesPath;
};

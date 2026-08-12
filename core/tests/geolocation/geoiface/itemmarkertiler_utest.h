/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : test for the model holding markers
 *
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

// Local includes

#include "itemmarkertiler.h"
#include "geomodelhelper.h"

using namespace Digikam;

class MarkerModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit MarkerModelHelper(QAbstractItemModel* const itemModel,
                               QItemSelectionModel* const itemSelectionModel);
    ~MarkerModelHelper()                                          override;

    QAbstractItemModel*  model()                            const override;
    QItemSelectionModel* selectionModel()                   const override;
    bool itemCoordinates(const QModelIndex& index,
                         GeoCoordinates* const coordinates) const override;

private Q_SLOTS:

    void slotDataChanged(const QModelIndex& topLeft,
                         const QModelIndex& bottomRight);

private:

    QAbstractItemModel* const  m_itemModel;
    QItemSelectionModel* const m_itemSelectionModel;
};

// --------------------------------------------------------------------------------

class TestItemMarkerTiler : public QObject
{
    Q_OBJECT

public:

    explicit TestItemMarkerTiler(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testNoOp();
    void testIndices();
    void testAddMarkers1();
    void testRemoveMarkers1();
    void testRemoveMarkers2();
    void testMoveMarkers1();
    void testMoveMarkers2();
    void testIteratorWholeWorldNoBackingModel();
    void testIteratorWholeWorld();
    void testIteratorPartial1();
    void testIteratorPartial2();
    void testPreExistingMarkers();
    void testSelectionState1();
    void benchmarkIteratorWholeWorld();
};

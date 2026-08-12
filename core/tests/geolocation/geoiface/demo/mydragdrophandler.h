/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-18
 * Description : Drag-and-drop handler for geolocation interface used in the demo
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>
#include <QMimeData>
#include <QTreeWidgetItem>

// Local includes

#include "geodragdrophandler.h"

using namespace Digikam;

class MyDragData : public QMimeData
{
    Q_OBJECT

public:

    MyDragData()
        : QMimeData     (),
          draggedIndices()
    {
    }

    QList<QPersistentModelIndex> draggedIndices;
};

class MyDragDropHandler : public GeoDragDropHandler
{
    Q_OBJECT

public:

    explicit MyDragDropHandler(QAbstractItemModel* const pModel, QObject* const parent = nullptr);
    ~MyDragDropHandler()                                                                    override;

    Qt::DropAction accepts(const QDropEvent* e) override;
    bool           dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates)    override;
    QMimeData*     createMimeData(const QList<QPersistentModelIndex>& modelIndices)         override;

private:

    QAbstractItemModel* const model;
};

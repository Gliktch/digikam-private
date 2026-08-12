/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : Drag and drop handler for the item list
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>

// Local includes

#include "mapdragdrophandler.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ItemListDragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit ItemListDragDropHandler(QObject* const parent = nullptr);
    ~ItemListDragDropHandler() override = default;

    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

// -------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT GPSItemListDragDropHandler : public ItemListDragDropHandler
{
    Q_OBJECT

public:

    explicit GPSItemListDragDropHandler(QObject* const parent = nullptr);
    ~GPSItemListDragDropHandler() override = default;

    QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) override;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "searchresultmodel.h"
#include "geomodelhelper.h"
#include "searchresultbackend.h"
#include "gpsundocommand.h"
#include "gpsitemmodel.h"

using namespace Digikam;

namespace DigikamGenericGeolocationEditPlugin
{

class SearchResultModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit SearchResultModelHelper(SearchResultModel* const resultModel,
                                     QItemSelectionModel* const selectionModel,
                                     GPSItemModel* const imageModel,
                                     QObject* const parent = nullptr);
    ~SearchResultModelHelper()                                                                override;

    void setVisibility(const bool state);

    QAbstractItemModel* model()                                                         const override;
    QItemSelectionModel* selectionModel()                                               const override;
    bool itemCoordinates(const QModelIndex& index,
                         GeoCoordinates* const coordinates)                             const override;
    bool itemIcon(const QModelIndex& index, QPoint* const offset,
                  QSize* const size, QPixmap* const pixmap,
                  QUrl* const url)                                                      const override;
    PropertyFlags modelFlags()                                                          const override;
    PropertyFlags itemFlags(const QModelIndex& index)                                   const override;
    void snapItemsTo(const QModelIndex& targetIndex,
                     const QList<QModelIndex>& snappedIndices)                                override;

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGeolocationEditPlugin

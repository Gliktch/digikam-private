/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : A tool to edit geolocation
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QModelIndex>

// Local includes

#include "geomodelhelper.h"
#include "digikam_export.h"

namespace Digikam
{

class GPSItemModel;
class GPSUndoCommand;
class MapWidget;

class DIGIKAM_EXPORT GPSGeoIfaceModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit GPSGeoIfaceModelHelper(GPSItemModel* const model,
                                    QItemSelectionModel* const selectionModel,
                                    QObject* const parent = nullptr);
    ~GPSGeoIfaceModelHelper()                                                                             override;

    QAbstractItemModel*  model()                                                                    const override;
    QItemSelectionModel* selectionModel()                                                           const override;

    bool itemCoordinates(const QModelIndex& index,
                         GeoCoordinates* const coordinates)                                         const override;

    PropertyFlags modelFlags()                                                                      const override;

    QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                          const QSize& size)                                              override;

    QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                          const int sortKey)                              override;

    void onIndicesMoved(const QList<QPersistentModelIndex>& movedMarkers,
                        const GeoCoordinates& targetCoordinates,
                        const QPersistentModelIndex& targetSnapIndex)                                     override;

    void addUngroupedModelHelper(GeoModelHelper* const newModelHelper);

private Q_SLOTS:

    void slotThumbnailFromModel(const QPersistentModelIndex& index,
                                const QPixmap& pixmap);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

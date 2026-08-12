/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Helper class to access models
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QItemSelectionModel>
#include <QPixmap>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QString>

// Local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GeoModelHelper : public QObject
{
    Q_OBJECT

public:

    enum PropertyFlag
    {
        FlagNull    = 0,
        FlagVisible = 1,
        FlagMovable = 2,
        FlagSnaps   = 4
    };

    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

public:

    explicit GeoModelHelper(QObject* const parent = nullptr);
    ~GeoModelHelper() override = default;

    void snapItemsTo(const QModelIndex& targetIndex,
                     const QList<QPersistentModelIndex>& snappedIndices);

    /// these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel* model() const = 0;
    virtual QItemSelectionModel* selectionModel() const = 0;
    virtual bool itemCoordinates(const QModelIndex& index,
                                 GeoCoordinates* const coordinates) const = 0;
    virtual PropertyFlags modelFlags() const;

    /// these are necessary for ungrouped models
    virtual bool itemIcon(const QModelIndex& index,
                          QPoint* const offset,
                          QSize* const size,
                          QPixmap* const pixmap,
                          QUrl* const url) const;
    virtual PropertyFlags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex,
                             const QList<QModelIndex>& snappedIndices);

    /// these are used by MarkerModel for grouped models
    virtual QPixmap pixmapFromRepresentativeIndex(const QPersistentModelIndex& index,
                                                  const QSize& size);
    virtual QPersistentModelIndex bestRepresentativeIndexFromList(const QList<QPersistentModelIndex>& list,
                                                                  const int sortKey);

    virtual void onIndicesClicked(const QList<QPersistentModelIndex>& clickedIndices);
    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices,
                                const GeoCoordinates& targetCoordinates,
                                const QPersistentModelIndex& targetSnapIndex);

Q_SIGNALS:

    void signalVisibilityChanged();
    void signalThumbnailAvailableForIndex(const QPersistentModelIndex& index,
                                          const QPixmap& pixmap);
    void signalModelChangedDrastically();
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GeoModelHelper::PropertyFlags)

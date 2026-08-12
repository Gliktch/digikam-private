/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-06
 * Description : Helper class for geomap interaction
 *
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QObject>
#include <QSize>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "geocoordinates.h"
#include "geogroupstate.h"
#include "gpsiteminfo.h"
#include "mapwidget.h"

namespace Digikam
{

class DIGIKAM_EXPORT GPSItemInfoSorter : public QObject
{
    Q_OBJECT

public:

    enum SortOption
    {
        SortYoungestFirst = 0,
        SortOldestFirst   = 1,
        SortRating        = 2
    };
    Q_DECLARE_FLAGS(SortOptions, SortOption)

public:

    explicit GPSItemInfoSorter(QObject* const parent);
    ~GPSItemInfoSorter() override;

    void addToMapWidget(MapWidget* const mapWidget);

    void setSortOptions(const SortOptions sortOptions);
    SortOptions getSortOptions() const;

public:

    static bool fitsBetter(const GPSItemInfo& oldInfo,
                           const GeoGroupState oldState,
                           const GPSItemInfo& newInfo,
                           const GeoGroupState newState,
                           const GeoGroupState globalGroupState,
                           const SortOptions sortOptions);

private Q_SLOTS:

    void slotSortOptionTriggered();

private:

    void initializeSortMenu();

private:

     /// @note disabled
    GPSItemInfoSorter() = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GPSItemInfoSorter::SortOptions)

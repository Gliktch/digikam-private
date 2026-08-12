/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

class QAbstractItemModel;

namespace Marble
{

class GeoDataCoordinates;
class GeoDataPlacemark;
class MarbleModel;
class ReverseGeocodingTask;

class DIGIKAM_EXPORT ReverseGeocodingRunnerManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * @param pluginManager The plugin manager that gives access to RunnerPlugins
     * @param parent Optional parent object
     */
    explicit ReverseGeocodingRunnerManager(const MarbleModel* marbleModel, QObject* parent = nullptr);

    ~ReverseGeocodingRunnerManager() override;

    /**
     * Find the address and other meta information for a given geoposition.
     * @see reverseGeocoding is asynchronous with currently one result
     * returned using the @see reverseGeocodingFinished signal.
     * @see searchReverseGeocoding is blocking.
     * @see reverseGeocodingFinished signal indicates all runners are finished.
     */
    void reverseGeocoding(const GeoDataCoordinates& coordinates);
    QString searchReverseGeocoding(const GeoDataCoordinates& coordinates, int timeout = 30000);

Q_SIGNALS:

    /**
     * The reverse geocoding request is finished, the result is stored
     * in the given placemark. This signal is emitted when the first
     * runner found a result, subsequent results are discarded and do not
     * Emit further signals. If no result is found, this signal is emitted
     * with an empty (default constructed) placemark.
     */
    void reverseGeocodingFinished(const GeoDataCoordinates& coordinates, const GeoDataPlacemark& placemark);

    /**
     * Emitted whenever all runners are finished for the query
     */
    void reverseGeocodingFinished();

private:

    Q_PRIVATE_SLOT(d, void addReverseGeocodingResult(const GeoDataCoordinates& coordinates, const GeoDataPlacemark& placemark))
    Q_PRIVATE_SLOT(d, void cleanupReverseGeocodingTask(ReverseGeocodingTask* task))

private:

    class Private;
    friend class Private;
    Private* const d = nullptr;
};

} // namespace Marble

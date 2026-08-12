/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Correlator for tracks and images
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "trackmanager.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TrackCorrelator : public QObject
{
    Q_OBJECT

public:

    enum CorrelationFlags
    {
        CorrelationFlagCoordinates  = 1,
        CorrelationFlagInterpolated = 2,
        CorrelationFlagAltitude     = 3
    };

public:

    class Correlation
    {
    public:

        Correlation() = default;

    public:

        typedef QList<Correlation>    List;

        QDateTime                     dateTime;
        QVariant                      userData;
        int                           nSatellites   = -1;
        qreal                         hDop          = -1.0;
        qreal                         pDop          = -1.0;
        int                           fixType       = -1;
        qreal                         speed         = -1.0;
        CorrelationFlags              flags         = CorrelationFlagCoordinates;
        GeoCoordinates                coordinates;
    };

    // -------------------------------------

    class CorrelationOptions
    {
    public:

        CorrelationOptions() = default;

    public:

        bool interpolate            = false;
        int  interpolationDstTime   = 0;
        int  maxGapTime             = 0;
        int  secondsOffset          = 0;
        int  timeZoneOffset         = 0;
    };

public:

    explicit TrackCorrelator(TrackManager* const trackManager, QObject* const parent = nullptr);
    ~TrackCorrelator() override;

    void correlate(const Correlation::List& itemsToCorrelate, const CorrelationOptions& options);
    void cancelCorrelation();

Q_SIGNALS:

    void signalItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
    void signalAllItemsCorrelated();
    void signalCorrelationCanceled();

private Q_SLOTS:

    void slotThreadItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
    void slotThreadFinished();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TrackCorrelator::Correlation::List)

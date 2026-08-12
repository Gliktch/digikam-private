/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file loading and managing
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QDateTime>
#include <QUrl>

// Local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "digikam_export.h"

class TestGPXParsing;

namespace Digikam
{

class DIGIKAM_EXPORT TrackManager : public QObject
{
    Q_OBJECT

public:

    /// @note we assume here that we will never load more than uint32_max tracks.
    typedef quint32 Id;

    enum ChangeFlag
    {
        ChangeTrackPoints = 1,
        ChangeMetadata    = 2,

        ChangeRemoved     = 4,
        ChangeAdd         = ChangeTrackPoints | ChangeMetadata
    };

    typedef QPair<Id, ChangeFlag> TrackChanges;

public:

    class  TrackPoint
    {
    public:

        TrackPoint() = default;

        static bool EarlierThan(const TrackPoint& a, const TrackPoint& b);

    public:

        QDateTime                 dateTime;
        GeoCoordinates            coordinates;
        int                       nSatellites   = -1;
        qreal                     hDop          = -1.0;
        qreal                     pDop          = -1.0;
        int                       fixType       = -1;
        qreal                     speed         = -1.0;

        typedef QList<TrackPoint> List;
    };

    // -------------------------------------

    class Track
    {
    public:

        enum Flags
        {
            FlagVisible = 1,
            FlagDefault = FlagVisible
        };

    public:

        Track()
        {
            qRegisterMetaType<TrackChanges>("TrackChanges");
        }

        QUrl                 url;
        QList<TrackPoint>    points;

        /// 0 means no track id assigned yet
        Id                   id     = 0;
        QColor               color  = Qt::red;
        Flags                flags  = FlagDefault;

        typedef QList<Track> List;
    };

public:

    explicit TrackManager(QObject* const parent = nullptr);
    ~TrackManager()                                   override;

    void loadTrackFiles(const QList<QUrl>& urls);
    QList<QPair<QUrl, QString> > readLoadErrors();
    void clear();

    const Track& getTrack(const int index)      const;
    Track::List getTrackList()                  const;
    int trackCount()                            const;

    quint64 getNextFreeTrackId();
    Track   getTrackById(const quint64 trackId) const;
    QColor  getNextFreeTrackColor();

    void setVisibility(const bool value);
    bool getVisibility()                        const;

Q_SIGNALS:

    void signalTrackFilesReadyAt(const int startIndex, const int endIndex);
    void signalAllTrackFilesReady();
    void signalTracksChanged(const QList<TrackManager::TrackChanges>& trackChanges);
    void signalVisibilityChanged(const bool newValue);

private Q_SLOTS:

    void slotTrackFilesReadyAt(int beginIndex, int endIndex);
    void slotTrackFilesFinished();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file reader
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QXmlStreamReader>

// Local includes

#include "trackmanager.h"
#include "digikam_export.h"

class TestTracks;

namespace Digikam
{

class DIGIKAM_EXPORT TrackReader : public QObject
{
    Q_OBJECT

public:

    class TrackReadResult
    {
    public:

        TrackReadResult() = default;

    public:

        TrackManager::Track            track;
        bool                           isValid = false;
        QString                        loadError;

        typedef QList<TrackReadResult> List;
    };

public:

    explicit TrackReader(TrackReadResult* const dataTarget);
    ~TrackReader()                                          override;

    static TrackReadResult loadTrackFile(const QUrl& url);
    static QDateTime parseTime(const QString& tstring);

private:

    void parseTrack(QXmlStreamReader& xml);
    explicit TrackReader(QObject*);

private:

    class Private;
    const QScopedPointer<Private> d;

    friend class ::TestTracks;
};

} // namespace Digikam

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

#include <QRunnable>
#include <QString>

// Local includes

#include "GeoDataCoordinates.h"
#include "GeoDataDocument.h"
#include "GeoDataLatLonBox.h"

namespace Marble
{

class MarbleModel;
class ParsingRunner;
class SearchRunner;
class ReverseGeocodingRunner;
class ParsingRunnerManager;
class SearchRunnerManager;
class ReverseGeocodingRunnerManager;

/**
 * A RunnerTask that executes a placemark search
 */
class SearchTask : public QObject, public QRunnable
{
    Q_OBJECT

public:

    SearchTask(SearchRunner* runner, SearchRunnerManager* manager, const MarbleModel* model, const QString& searchTerm, const GeoDataLatLonBox& preferred);

    /**
     * @reimp
     */
    void run() override;

Q_SIGNALS:

    void finished(SearchTask* task);

private:

    SearchRunner* const m_runner = nullptr;
    QString             m_searchTerm;
    GeoDataLatLonBox    m_preferredBbox;
};

/** A RunnerTask that executes reverse geocoding */
class ReverseGeocodingTask : public QObject, public QRunnable
{
    Q_OBJECT

public:

    ReverseGeocodingTask(ReverseGeocodingRunner* runner, ReverseGeocodingRunnerManager* manager, const MarbleModel* model, const GeoDataCoordinates& coordinates);

    /**
     * @reimp
     */
    void run() override;

Q_SIGNALS:

    void finished(ReverseGeocodingTask* task);

private:

    ReverseGeocodingRunner* const   m_runner = nullptr;
    GeoDataCoordinates              m_coordinates;
};

/** A RunnerTask that executes a file Parsing */
class ParsingTask : public QObject,
    public QRunnable
{
    Q_OBJECT

public:

    ParsingTask(ParsingRunner* runner, ParsingRunnerManager* manager, const QString& fileName, DocumentRole role);

    /**
     * @reimp
     */
    void run() override;

Q_SIGNALS:

    void parsed(GeoDataDocument* document, const QString& error);
    void finished();

private:

    ParsingRunner* const    m_runner  = nullptr;
    QString                 m_fileName;
    DocumentRole            m_role;
    ParsingRunnerManager*   m_manager = nullptr;
};

} // Namespace Marble

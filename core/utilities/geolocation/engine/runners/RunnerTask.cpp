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

#include "RunnerTask.h"

// Local includes

#include "ParsingRunner.h"
#include "ParsingRunnerManager.h"
#include "SearchRunner.h"
#include "SearchRunnerManager.h"
#include "ReverseGeocodingRunner.h"
#include "ReverseGeocodingRunnerManager.h"
#include "digikam_debug.h"

namespace Marble
{

SearchTask::SearchTask(SearchRunner* runner, SearchRunnerManager* manager, const MarbleModel* model, const QString& searchTerm, const GeoDataLatLonBox& preferred) :
    QObject        (),
    m_runner       (runner),
    m_searchTerm   (searchTerm),
    m_preferredBbox(preferred)
{
    connect(m_runner, SIGNAL(searchFinished(QVector<GeoDataPlacemark*>)),
            manager, SLOT(addSearchResult(QVector<GeoDataPlacemark*>)));

    m_runner->setModel(model);
}

void SearchTask::run()
{
    m_runner->search(m_searchTerm, m_preferredBbox);
    m_runner->deleteLater();

    Q_EMIT finished(this);
}

ReverseGeocodingTask::ReverseGeocodingTask(ReverseGeocodingRunner* runner, ReverseGeocodingRunnerManager* manager, const MarbleModel* model, const GeoDataCoordinates& coordinates) :
    QObject      (),
    m_runner     (runner),
    m_coordinates(coordinates)
{
    connect(m_runner, SIGNAL(reverseGeocodingFinished(GeoDataCoordinates,GeoDataPlacemark)),
            manager, SLOT(addReverseGeocodingResult(GeoDataCoordinates,GeoDataPlacemark)));

    m_runner->setModel(model);
}

void ReverseGeocodingTask::run()
{
    m_runner->reverseGeocoding(m_coordinates);
    m_runner->deleteLater();

    Q_EMIT finished(this);
}

ParsingTask::ParsingTask(ParsingRunner* runner, ParsingRunnerManager* manager, const QString& fileName, DocumentRole role) :
    QObject     (),
    m_runner    (runner),
    m_fileName  (fileName),
    m_role      (role),
    m_manager   (manager)
{
    connect(this, SIGNAL(parsed(GeoDataDocument*,QString)),
            m_manager, SLOT(addParsingResult(GeoDataDocument*,QString)));
}

void ParsingTask::run()
{
    QString error;
    GeoDataDocument* document = m_runner->parseFile(m_fileName, m_role, error);

    Q_EMIT parsed(document, error);

    m_runner->deleteLater();

    Q_EMIT finished();
}

} // namespace Marble

#include "moc_RunnerTask.cpp"

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
#include <QVector>

// Local includes

#include "GeoDataLatLonBox.h"
#include "digikam_export.h"

class QAbstractItemModel;
class QString;

namespace Marble
{

class GeoDataPlacemark;
class MarbleModel;
class SearchTask;

class DIGIKAM_EXPORT SearchRunnerManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * @param pluginManager The plugin manager that gives access to RunnerPlugins
     * @param parent Optional parent object
     */
    explicit SearchRunnerManager(const MarbleModel* marbleModel, QObject* parent = nullptr);

    ~SearchRunnerManager() override;

    /**
     * Search for placemarks matching the given search term.
     * @see findPlacemark is asynchronous with results returned using the
     * @see searchResultChanged signal.
     * @see searchPlacemark is blocking.
     * @see searchFinished signal indicates all runners are finished.
     */
    void findPlacemarks(const QString& searchTerm, const GeoDataLatLonBox& preferred = GeoDataLatLonBox());
    QVector<GeoDataPlacemark*> searchPlacemarks(const QString& searchTerm, const GeoDataLatLonBox& preferred = GeoDataLatLonBox(), int timeout = 30000);

Q_SIGNALS:

    /**
     * Placemarks were added to or removed from the model
     * @todo FIXME: this sounds like a duplication of QAbstractItemModel signals
     */
    void searchResultChanged(QAbstractItemModel* model);
    void searchResultChanged(const QVector<GeoDataPlacemark*>& result);

    /**
     * The search request for the given search term has finished, i.e. all
     * runners are finished and reported their results via the
     * @see searchResultChanged signal
     */
    void searchFinished(const QString& searchTerm);

    /**
     * Emitted whenever all runners are finished for the query
     */
    void placemarkSearchFinished();

private:

    Q_PRIVATE_SLOT(d, void addSearchResult(const QVector<GeoDataPlacemark*>& result))
    Q_PRIVATE_SLOT(d, void cleanupSearchTask(SearchTask* task))

private:

    class Private;
    friend class Private;
    Private* const d = nullptr;
};

} // namespace Marble

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

/**
 * MarblePlacemarkModel exposes the model for Placemarks
 */

#pragma once

// Qt includes

#include <QAbstractListModel>
#include <QModelIndex>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class GeoDataCoordinates;
class GeoDataPlacemark;

/**
 * This class represents a model of all place marks which
 * are currently available through a given PlacemarkManager.
 */
class DIGIKAM_EXPORT MarblePlacemarkModel : public QAbstractListModel
{
    friend class PlacemarkManager;

    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:

    /**
     * The roles of the place marks.
     */
    enum Roles
    {
        GeoTypeRole = Qt::UserRole + 1,  ///< The geo type (e.g. city or mountain)
        DescriptionRole,                 ///< The description
        CoordinateRole,                  ///< The GeoDataCoordinates coordinate
        PopulationRole,                  ///< The population
        AreaRole,                        ///< The area size
        CountryCodeRole,                 ///< The country code
        StateRole,                       ///< The state
        VisualCategoryRole,              ///< The category
        StyleRole,                       ///< The style
        PopularityIndexRole,             ///< The popularity index
        PopularityRole,                  ///< The popularity
        ObjectPointerRole,               ///< The pointer to a specific object
        GmtRole,                         ///< The Greenwich Mean Time
        DstRole,                         ///< The Daylight Saving Time
        GeometryRole,                    ///< The GeoDataGeometry geometry
        LongitudeRole,                   ///< The longitude in degree (for use in QML)
        LatitudeRole,                    ///< The latitude in degree (for use in QML)
        IconPathRole                     ///< Path to the image, if known
    };

    /**
     * Creates a new place mark model.
     *
     * @param parent The parent object.
     */
    explicit MarblePlacemarkModel(QObject* parent = nullptr);

    /**
     * Destroys the place mark model.
     */
    ~MarblePlacemarkModel() override;

    void setPlacemarkContainer(QVector<GeoDataPlacemark*>* container);

    /**
     * Return the number of Placemarks in the Model.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Return the supported role names
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Return the data according to the index.
     *
     * @param index  the index of the data
     * @param role   which part of the data to return.  @see Roles
     */
    QVariant data(const QModelIndex& index, int role) const override;

    QModelIndexList approxMatch(const QModelIndex& start, int role,
                                const QVariant& value, int hits = 1,
                                Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

    /**
     * This method is used by the PlacemarkManager to add new
     * place marks to the model.
     */
    void addPlacemarks(int start,
                       int length);

    /**
     * This method is used by the PlacemarkManager to remove
     * place marks from the model.
     */
    void removePlacemarks(const QString& containerName,
                          int start,
                          int length);

Q_SIGNALS:

    void countChanged();

private:

    Q_DISABLE_COPY(MarblePlacemarkModel)

    class Private;
    Private* const d = nullptr;
    QHash<int, QByteArray> m_roleNames;
};

} // namespace Marble

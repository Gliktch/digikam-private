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
 * PlacemarkLayout is responsible for drawing the Placemarks on the map
 */

#pragma once

// Qt includes

#include <QHash>
#include <QRect>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QPointer>

// Local includes

#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"

class QAbstractItemModel;
class QItemSelectionModel;
class QPoint;
class QModelIndex;

namespace Marble
{

class GeoDataCoordinates;
class GeoPainter;
class MarbleClock;
class PlacemarkPainter;
class TileId;
class VisiblePlacemark;
class ViewportParams;
class StyleBuilder;

/**
 * Layouts the place marks with a passed QPainter.
 */
class PlacemarkLayout : public QObject
{
    Q_OBJECT

public:

    /**
     * Creates a new place mark layout.
     */
    PlacemarkLayout(QAbstractItemModel*  placemarkModel,
                    QItemSelectionModel* selectionModel,
                    MarbleClock* clock,
                    const StyleBuilder* styleBuilder,
                    QObject* parent = nullptr);

    /**
     * Destroys the place mark painter.
     */
    ~PlacemarkLayout() override;

    /**
     * @reimp
     */
    QVector<VisiblePlacemark*> generateLayout(const ViewportParams* viewport, int tileLevel);

    /**
     * Returns a list of model indexes that are at position @p pos.
     */
    QVector<const GeoDataFeature*> whichPlacemarkAt(const QPoint& pos);

    QString runtimeTrace() const;

    QList<VisiblePlacemark*> visiblePlacemarks() const;

    bool hasPlacemarkAt(const QPoint& pos);

public Q_SLOTS:

    // earth
    void setShowPlaces(bool show);
    void setShowCities(bool show);
    void setShowTerrain(bool show);
    void setShowOtherPlaces(bool show);

    // other planets
    void setShowLandingSites(bool show);
    void setShowCraters(bool show);
    void setShowMaria(bool show);

    void requestStyleReset();
    void addPlacemarks(const QModelIndex& index, int first, int last);
    void removePlacemarks(const QModelIndex& index, int first, int last);
    void resetCacheData();

Q_SIGNALS:

    void repaintNeeded();

private:

    /**
     * Returns a the maximum height of all possible labels.
     * WARNING: This is a really slow method as it traverses all placemarks
     * to check the labelheight.
     * FIXME: Once a StyleManager that manages all styles has been implemented
     * just traverse all existing styles.
     */
    static int maxLabelHeight();

    void styleReset();
    void clearCache();

    static QSet<TileId> visibleTiles(const ViewportParams& viewport, int tileLevel);
    bool layoutPlacemark(const GeoDataPlacemark* placemark, const GeoDataCoordinates& coordinates, qreal x, qreal y, bool selected);

    /**
     * Returns the coordinates at which an icon should be drawn for the @p placemark.
     * @p ok is set to true if the coordinates are valid and should be used for drawing,
     * it is set to false otherwise.
     */
    GeoDataCoordinates placemarkIconCoordinates(const GeoDataPlacemark* placemark) const;

    QRectF  roomForLabel(const GeoDataStyle::ConstPtr& style,
                         const qreal x, const qreal y,
                         const QString& labelText, const VisiblePlacemark* placemark) const;
    bool    hasRoomForPixmap(const qreal y, const VisiblePlacemark* placemark) const;

    bool    placemarksOnScreenLimit(const QSize& screenSize) const;

private:

    Q_DISABLE_COPY(PlacemarkLayout)

private:

    QAbstractItemModel*                                 m_placemarkModel        = nullptr;
    QItemSelectionModel* const                          m_selectionModel        = nullptr;
    MarbleClock* const                                  m_clock                 = nullptr;

    QVector<VisiblePlacemark*>                          m_paintOrder;
    QString                                             m_runtimeTrace;
    int                                                 m_labelArea;
    QHash<const GeoDataPlacemark*, VisiblePlacemark*>   m_visiblePlacemarks;
    QVector< QVector< VisiblePlacemark* > >             m_rowsection;

    /// map providing the list of placemark belonging in TileId as key
    QMap<TileId, QList<const GeoDataPlacemark*> >       m_placemarkCache;
    QSet<qint64>                                        m_osmIds;

    const QSet<GeoDataPlacemark::GeoDataVisualCategory> m_acceptedVisualCategories;

    // earth
    bool                                                m_showPlaces;
    bool                                                m_showCities;
    bool                                                m_showTerrain;
    bool                                                m_showOtherPlaces;

    // other planets
    bool                                                m_showLandingSites;
    bool                                                m_showCraters;
    bool                                                m_showMaria;

    int                                                 m_maxLabelHeight;
    bool                                                m_styleResetRequested;
    const StyleBuilder*                                 m_styleBuilder          = nullptr;
    // Referencing these properties by value
    // instead of using a more fragile pointer
    bool                                                m_lastPlacemarkAvailable;
    QRectF                                              m_lastPlacemarkLabelRect;
    QRectF                                              m_lastPlacemarkSymbolRect;
};

} // namespace Marble

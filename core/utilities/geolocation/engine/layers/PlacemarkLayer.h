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
 * PlacemarkLayer is responsible for drawing the Placemarks on the map
 */

#pragma once

// Qt includes

#include <QObject>
#include <QVector>
#include <QPainter>

// Local includes

#include "LayerInterface.h"
#include "PlacemarkLayout.h"

class QAbstractItemModel;
class QItemSelectionModel;
class QString;

namespace Marble
{

class GeoPainter;
class GeoSceneLayer;
class MarbleClock;
class ViewportParams;
class StyleBuilder;

struct Fragment
{
    QVarLengthArray<QPainter::PixmapFragment, 16> fragments;
    QPixmap pixmap;
};

class PlacemarkLayer : public QObject,
                       public LayerInterface
{
    Q_OBJECT

public:

    PlacemarkLayer(QAbstractItemModel* placemarkModel,
                   QItemSelectionModel* selectionModel,
                   MarbleClock* clock,
                   const StyleBuilder* styleBuilder,
                   QObject* parent = nullptr);
    ~PlacemarkLayer()                                                             override;

    /**
     * @reimp
     */
    QStringList renderPosition()                                            const override;

    /**
     * @reimp
     */
    qreal zValue()                                                          const override;

    /**
     * @reimp
     */
    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("NONE"),
                GeoSceneLayer* layer = nullptr)                                   override;

    RenderState renderState()                                               const override;

    QString runtimeTrace()                                                  const override;

    /**
     * Returns a list of model indexes that are at position @p pos.
     */
    QVector<const GeoDataFeature*> whichPlacemarkAt(const QPoint& pos);

    bool hasPlacemarkAt(const QPoint& pos);

    bool isDebugModeEnabled()                                               const;
    void setDebugModeEnabled(bool enabled);

    void setLevelTagDebugModeEnabled(bool enabled);
    bool levelTagDebugModeEnabled()                                         const;
    void setDebugLevelTag(int level);

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
    void setTileLevel(int tileLevel);

Q_SIGNALS:

    void repaintNeeded();

private:

    void renderDebug(GeoPainter* painter, ViewportParams* viewport,
                     const QVector<VisiblePlacemark*>& placemarks)          const;

private:

    PlacemarkLayout m_layout;
    bool            m_debugModeEnabled          = false;
    bool            m_levelTagDebugModeEnabled  = false;
    int             m_tileLevel                 = 0;
    int             m_debugLevelTag             = 0;
};

} // namespace Marble

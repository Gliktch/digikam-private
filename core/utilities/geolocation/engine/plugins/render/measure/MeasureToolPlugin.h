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
 * MeasureToolPlugin enables Marble to set and display measure points
 */

#pragma once

// Qt includes

#include <QFont>
#include <QPen>
#include <QAction>
#include <QPixmap>

// Local includes

#include "DialogConfigurationInterface.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataLineString.h"
#include "RenderPlugin.h"
#include "MarbleWidget.h"

namespace Marble
{

class MeasureConfigDialog;

class MeasureToolPlugin : public RenderPlugin,
    public DialogConfigurationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.marble.MeasureToolPlugin")
    Q_INTERFACES(Marble::RenderPluginInterface)
    Q_INTERFACES(Marble::DialogConfigurationInterface)
    MARBLE_PLUGIN(MeasureToolPlugin)

public:

    enum PaintMode
    {
        Polygon = 0,
        Circular
    };

public:

    explicit MeasureToolPlugin(const MarbleModel* marbleModel = nullptr);

    QStringList backendTypes() const override;
    QString renderPolicy() const override;
    QStringList renderPosition() const override;
    QString name() const override;
    QString guiString() const override;
    QString nameId() const override;

    QString version() const override;

    QString description() const override;

    QString copyrightYears() const override;

    QVector<PluginAuthor> pluginAuthors() const override;

    QIcon icon() const override;

    void initialize() override;

    bool isInitialized() const override;

    bool render(GeoPainter* painter, ViewportParams* viewport, const QString& renderPos, GeoSceneLayer* layer = nullptr) override;

    QDialog* configDialog() override;
    QHash<QString, QVariant> settings() const override;
    void setSettings(const QHash<QString, QVariant>& settings) override;

Q_SIGNALS:

    void  numberOfMeasurePointsChanged(int newNumber);

public Q_SLOTS:

    bool  eventFilter(QObject* object, QEvent* event) override;

private:

    void  drawMeasurePoints(GeoPainter* painter);
    void  drawInfobox(GeoPainter* painter) const;
    void  drawSegments(GeoPainter* painter);
    void  addContextItems();
    void  removeContextItems();

private Q_SLOTS:

    void  setNumberOfMeasurePoints(int number);
    void  addMeasurePointEvent();

    void  addMeasurePoint(qreal lon, qreal lat);
    void  removeLastMeasurePoint();
    void  removeMeasurePoints();

    void writeSettings();

private:

    Q_DISABLE_COPY(MeasureToolPlugin)

    static QString meterToPreferredUnit(qreal meters, bool isSquare = false);

private:

    // The line strings in the distance path.
    GeoDataLineString       m_measureLineString;
    GeoDataLatLonAltBox     m_latLonAltBox;

    QPixmap                 m_mark;
    QFont                   m_font_regular;
    int                     m_fontascent;

    QPen                    m_pen;

    QAction*                m_addMeasurePointAction        = nullptr;
    QAction*                m_removeLastMeasurePointAction = nullptr;
    QAction*                m_removeMeasurePointsAction    = nullptr;
    QAction*                m_separator                    = nullptr;

    MarbleWidget*           m_marbleWidget                 = nullptr;

    MeasureConfigDialog*    m_configDialog                 = nullptr;

    bool                    m_showDistanceLabel;
    bool                    m_showBearingLabel;
    bool                    m_showBearingChangeLabel;

    bool                    m_showPolygonArea;
    bool                    m_showCircularArea;
    bool                    m_showRadius;
    bool                    m_showPerimeter;
    bool                    m_showCircumference;

    qreal                   m_totalDistance;
    qreal                   m_polygonArea;
    qreal                   m_circularArea;
    qreal                   m_radius;
    qreal                   m_perimeter;
    qreal                   m_circumference;

    PaintMode               m_paintMode;
};

} // namespace Marble

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

#include <QHash>
#include <QColor>
#include <QPixmap>
#include <QSvgWidget>
#include <QSvgRenderer>

// Local includes

#include "GeoDataLatLonAltBox.h"
#include "AbstractFloatItem.h"
#include "DialogConfigurationInterface.h"

namespace Ui
{
class OverviewMapConfigWidget;
}

namespace Marble
{

/**
 * @short The class that creates an overview map.
 *
 */
class OverviewMap : public AbstractFloatItem,
    public DialogConfigurationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.marble.OverviewMap")
    Q_INTERFACES(Marble::RenderPluginInterface)
    Q_INTERFACES(Marble::DialogConfigurationInterface)
    MARBLE_PLUGIN(OverviewMap)

public:

    OverviewMap();
    explicit OverviewMap(const MarbleModel* marbleModel);
    ~OverviewMap()                                              override;

    QStringList backendTypes()                            const override;

    QString name()                                        const override;

    QString guiString()                                   const override;

    QString nameId()                                      const override;

    QString version()                                     const override;

    QString description()                                 const override;

    QString copyrightYears()                              const override;

    QVector<PluginAuthor> pluginAuthors()                 const override;

    QIcon icon()                                          const override;

    QDialog* configDialog()                                     override;

    void initialize()                                           override;

    bool isInitialized()                                  const override;

    void setProjection(const ViewportParams* viewport)          override;

    void paintContent(QPainter* painter)                        override;

    /**
     * @return: The settings of the item.
     */
    QHash<QString, QVariant> settings()                   const override;

    /**
     * Set the settings of the item.
     */
    void setSettings(const QHash<QString, QVariant>& settings)  override;

public Q_SLOTS:

    void readSettings();
    void writeSettings();
    void updateSettings();

protected:

    bool eventFilter(QObject* object, QEvent* e) override;

private:

    void changeBackground(const QString& target);
    QSvgWidget* currentWidget() const;
    void setCurrentWidget(QSvgWidget* widget);
    void loadPlanetMaps();
    void loadMapSuggestions();

private:

    QString                         m_target;
    QSvgRenderer                    m_svgobj;
    QHash<QString, QSvgWidget*>     m_svgWidgets;
    QHash<QString, QString>         m_svgPaths;
    QStringList                     m_planetID;
    QPixmap                         m_worldmap;
    QHash<QString, QVariant>         m_settings;
    QColor                          m_posColor;
    QSizeF                          m_defaultSize;

    Ui::OverviewMapConfigWidget*    ui_configWidget = nullptr;
    QDialog*                        m_configDialog  = nullptr;

    GeoDataLatLonAltBox             m_latLonAltBox;
    qreal                           m_centerLat     = 0.0;
    qreal                           m_centerLon     = 0.0;
    bool                            m_mapChanged    = false;

private Q_SLOTS:

    void chooseCustomMap();
    void synchronizeSpinboxes();
    void showCurrentPlanetPreview() const;
    void choosePositionIndicatorColor();
    void useMapSuggestion(int index);
};

} // namespace Marble

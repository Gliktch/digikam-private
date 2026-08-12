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

// Local includes

#include "AbstractFloatItem.h"
#include "DialogConfigurationInterface.h"

namespace Ui
{
class MapScaleConfigWidget;
}

namespace Marble
{

/**
 * @short The class that creates a map scale.
 */
class MapScaleFloatItem : public AbstractFloatItem,
    public DialogConfigurationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.marble.MapScaleFloatItem")
    Q_INTERFACES(Marble::RenderPluginInterface)
    Q_INTERFACES(Marble::DialogConfigurationInterface)
    MARBLE_PLUGIN(MapScaleFloatItem)

public:

    explicit MapScaleFloatItem(const MarbleModel* marbleModel = nullptr);
    ~MapScaleFloatItem()                                          override;

    QStringList backendTypes()                              const override;

    QString name()                                          const override;

    QString guiString()                                     const override;

    QString nameId()                                        const override;

    QString version()                                       const override;

    QString description()                                   const override;

    QString copyrightYears()                                const override;

    QVector<PluginAuthor> pluginAuthors()                   const override;

    QIcon icon()                                            const override;

    void initialize()                                             override;

    bool isInitialized()                                    const override;

    void setProjection(const ViewportParams* viewport) override;

    void paintContent(QPainter* painter)                          override;

    QDialog* configDialog()                                       override;

protected:

    void contextMenuEvent(QWidget* w, QContextMenuEvent* e)       override;
    void toolTipEvent(QHelpEvent* e)                              override;

private Q_SLOTS:

    void readSettings();
    void writeSettings();
    void toggleRatioScaleVisibility();
    void toggleMinimized();

private:

    void calcScaleBar();

private:

    QDialog*                    m_configDialog      = nullptr;
    Ui::MapScaleConfigWidget*   ui_configWidget     = nullptr;

    int                         m_radius            = 0;

    QString                     m_target;

    int                         m_leftBarMargin     = 0;
    int                         m_rightBarMargin    = 0;
    int                         m_scaleBarWidth     = 0;
    int                         m_viewportWidth     = 0;
    int                         m_scaleBarHeight    = 5;
    qreal                       m_scaleBarDistance  = 0.0;

    qreal                       m_pixel2Length      = 0.0;
    int                         m_bestDivisor       = 0;
    int                         m_pixelInterval     = 0;
    int                         m_valueInterval     = 0;

    QString                     m_ratioString;

    bool                        m_scaleInitDone     = false;

    bool                        m_showRatioScale    = false;

    QMenu*                      m_contextMenu       = nullptr;

    QAction*                    m_minimizeAction    = nullptr;
    bool                        m_minimized         = false;
    int                         m_widthScaleFactor  = 2;
};

} // namespace Marble

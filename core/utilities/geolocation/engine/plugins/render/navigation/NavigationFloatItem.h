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

#include <QMenu>

// Local includes

#include "MarbleGlobal.h"
#include "AbstractFloatItem.h"

namespace Ui
{
class Navigation;
}

namespace Marble
{

class MarbleWidget;
class WidgetGraphicsItem;

/**
 * @short Provides a float item with zoom and move controls
 */
class NavigationFloatItem : public AbstractFloatItem
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.marble.NavigationFloatItem")

    Q_INTERFACES(Marble::RenderPluginInterface)

    MARBLE_PLUGIN(NavigationFloatItem)

public:

    explicit NavigationFloatItem(const MarbleModel* marbleModel = nullptr);
    ~NavigationFloatItem() override;

    QStringList backendTypes() const override;

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

    void setProjection(const ViewportParams* viewport) override;

    static QPixmap pixmap(const QString& Id);

    QHash<QString, QVariant> settings() const override;

    void setSettings(const QHash<QString, QVariant>& settings) override;

protected:

    bool eventFilter(QObject* object, QEvent* e) override;
    void paintContent(QPainter* painter) override;
    void contextMenuEvent(QWidget* w, QContextMenuEvent* e) override;

private Q_SLOTS:

    /** Map theme was changed, adjust controls */
    void selectTheme(const QString& theme);

    /** Enable/disable zoom in/out buttons */
    void updateButtons(int zoomValue);

    void activateCurrentPositionButton();
    void activateHomeButton();
    void centerOnCurrentLocation();

private:

    /** MarbleWidget this float item is installed as event filter for */
    MarbleWidget* m_marbleWidget                    = nullptr;

    /** The GraphicsItem presenting the widgets. NavigationFloatItem doesn't take direct ownership
        of this */
    WidgetGraphicsItem* m_widgetItem                = nullptr;

    /** Navigation controls */
    Ui::Navigation* m_navigationWidget              = nullptr;

    /** Used Profile */
    MarbleGlobal::Profiles m_profiles               = MarbleGlobal::Default;

    /** Radius of the viewport last time */
    int m_oldViewportRadius                         = 0;

    int m_maxZoom                                   = 0;
    int m_minZoom                                   = 0;

    QMenu* m_contextMenu                            = nullptr;
    QAction* m_activateCurrentPositionButtonAction  = nullptr;
    QAction* m_activateHomeButtonAction             = nullptr;

    bool m_showHomeButton                           = true;
};

} // namespace Marble

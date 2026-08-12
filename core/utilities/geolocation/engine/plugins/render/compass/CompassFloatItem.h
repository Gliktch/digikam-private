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

#include <QPixmap>

// Local includes

#include "AbstractFloatItem.h"
#include "DialogConfigurationInterface.h"

class QSvgRenderer;

namespace Ui
{
class CompassConfigWidget;
}

namespace Marble
{

/**
 * @short The class that creates a compass
 */
class CompassFloatItem : public AbstractFloatItem,
    public DialogConfigurationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.marble.CompassFloatItem")
    Q_INTERFACES(Marble::RenderPluginInterface)
    Q_INTERFACES(Marble::DialogConfigurationInterface)
    MARBLE_PLUGIN(CompassFloatItem)

public:

    CompassFloatItem();
    explicit CompassFloatItem(const MarbleModel* marbleModel);
    ~CompassFloatItem()                                               override;

    QStringList backendTypes()                                  const override;

    QString name()                                              const override;

    QString guiString()                                         const override;

    QString nameId()                                            const override;

    QString version()                                           const override;

    QString description()                                       const override;

    QString copyrightYears()                                    const override;

    QVector<PluginAuthor> pluginAuthors()                       const override;

    QIcon icon()                                                const override;

    void initialize()                                                 override;

    bool isInitialized()                                        const override;

    QPainterPath backgroundShape()                              const override;

    void setProjection(const ViewportParams* viewport)                override;

    void paintContent(QPainter* painter)                              override;

    QDialog* configDialog()                                           override;

    QHash<QString, QVariant> settings()                         const override;

    void setSettings(const QHash<QString, QVariant>& settings)        override;

private Q_SLOTS:

    void readSettings();
    void writeSettings();

private:

    Q_DISABLE_COPY(CompassFloatItem)

private:

    bool                     m_isInitialized    = false;

    QSvgRenderer*            m_svgobj           = nullptr;
    QPixmap                  m_compass;

    /// allowed values: -1, 0, 1; default here: 0. FIXME: Declare enum
    int                      m_polarity         = 0;

    int                      m_themeIndex;
    QDialog*                 m_configDialog     = nullptr;
    Ui::CompassConfigWidget* m_uiConfigWidget   = nullptr;
};

} // namespace Marble

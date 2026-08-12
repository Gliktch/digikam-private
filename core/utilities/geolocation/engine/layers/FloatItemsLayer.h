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
#include <QList>
#include <QRegion>

// Local includes

#include "LayerInterface.h"

namespace Marble
{

class AbstractFloatItem;
class RenderPlugin;

/**
 * @brief Layer for handling the rendering of screen-positioned, 2D float items.
 */
class FloatItemsLayer : public QObject,
                        public LayerInterface
{
    Q_OBJECT

public:

    explicit FloatItemsLayer(QObject* parent = nullptr);

    QStringList renderPosition() const override;

    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("NONE"), GeoSceneLayer* layer = nullptr) override;

    void addFloatItem(AbstractFloatItem* floatItem);

    /**
     * @brief Returns a list of all FloatItems of the layer
     * @return the list of the floatItems
     */
    QList<AbstractFloatItem*> floatItems() const;

    QString runtimeTrace() const override;

Q_SIGNALS:

    /**
     * @brief Signal that a render item has been initialized
     */
    void renderPluginInitialized(RenderPlugin* renderPlugin);

    /**
     * This signal is emitted when the repaint of the view was requested by a plugin.
     * If available with the @p dirtyRegion which is the region the view will change in.
     * If dirtyRegion.isEmpty() returns true, the whole viewport has to be repainted.
     */
    void repaintNeeded(const QRegion& dirtyRegion = QRegion());

    void visibilityChanged(const QString& nameId, bool visible);

    void pluginSettingsChanged();

private Q_SLOTS:

    void updateVisibility(bool visible, const QString& nameId);

private:

    QList<AbstractFloatItem*> m_floatItems;
};

} // namespace Marble

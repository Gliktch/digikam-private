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

#include <QList>
#include <QObject>
#include <QRegion>

class QPoint;
class QString;

namespace Marble
{

class AbstractDataPlugin;
class AbstractDataPluginItem;
class GeoPainter;
class ViewportParams;
class RenderPlugin;
class RenderState;
class LayerInterface;

/**
 * @short Handles rendering of all active layers in the correct order
 */
class LayerManager : public QObject
{
    Q_OBJECT

public:

    explicit LayerManager(QObject* parent = nullptr);
    ~LayerManager() override;

    void renderLayers(GeoPainter* painter, ViewportParams* viewport);

    bool showBackground() const;

    bool showRuntimeTrace() const;

    void addRenderPlugin(RenderPlugin* renderPlugin);

    /**
     * @brief Returns a list of all DataPlugins on the layer
     * @return the list of DataPlugins
     */
    QList<AbstractDataPlugin*> dataPlugins()  const;

    /**
     * @brief Returns all items of dataPlugins on the position curpos
     */
    QList<AbstractDataPluginItem*> whichItemAt(const QPoint& curpos) const;

    /**
     * @brief Add a layer to be included in rendering.
     */
    void addLayer(LayerInterface* layer);

    /**
     * @brief Remove a layer from being included in rendering.
     */
    void removeLayer(LayerInterface* layer);

    QList<LayerInterface*> internalLayers() const;

    RenderState renderState() const;

Q_SIGNALS:

    /**
     * @brief Signal that a render item has been initialized
     */
    void renderPluginInitialized(RenderPlugin* renderPlugin);

    /**
     * This signal is emitted when the settings of a plugin changed.
     */
    void pluginSettingsChanged();

    /**
     * This signal is emitted when the repaint of the view was requested by a plugin.
     * If available with the @p dirtyRegion which is the region the view will change in.
     * If dirtyRegion.isEmpty() returns true, the whole viewport has to be repainted.
     */
    void repaintNeeded(const QRegion& dirtyRegion = QRegion());

    void visibilityChanged(const QString& nameId, bool visible);

public Q_SLOTS:

    void setShowBackground(bool show);

    void setShowRuntimeTrace(bool show);

private:

    Q_PRIVATE_SLOT(d, void updateVisibility(bool, const QString&))

private:

    Q_DISABLE_COPY(LayerManager)

private:

    class Private;
    friend class Private;
    Private* const d = nullptr;
};

} // namespace Marble

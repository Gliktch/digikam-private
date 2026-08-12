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

#include "RenderPlugin.h"
#include "digikam_export.h"

namespace Marble
{

class ViewportParams;
class GeoSceneLayer;
class AbstractDataPluginItem;
class AbstractDataPluginModel;
class AbstractDataPluginPrivate;

/**
 * @short An abstract class for plugins that show data that has a geo coordinate
 *
 * This is the abstract class for plugins that show data on Marble map.
 * It takes care of painting all items it gets from the corresponding AbstractDataPluginModel
 * that has to be set on initialisation.
 *
 * The user has to set the nameId as well as the number of items to fetch.
 * Additionally it should be useful to set standard values via setEnabled (often true)
 * and setVisible (often false) in the constructor of a subclass.
 **/
class DIGIKAM_EXPORT AbstractDataPlugin : public RenderPlugin
{
    Q_OBJECT

    Q_PROPERTY(bool favoriteItemsOnly READ isFavoriteItemsOnly WRITE setFavoriteItemsOnly NOTIFY favoriteItemsOnlyChanged)
    /** @todo FIXME Qt Quick segfaults if using the real class here instead of QObject */
    Q_PROPERTY(QObject* favoritesModel READ favoritesModel NOTIFY favoritesModelChanged)
    Q_PROPERTY(int numberOfItems READ numberOfItems WRITE setNumberOfItems NOTIFY changedNumberOfItems)

public:

    explicit AbstractDataPlugin(const MarbleModel* marbleModel);

    ~AbstractDataPlugin() override;

    bool isInitialized() const override;

    /**
     * @brief Returns the name(s) of the backend that the plugin can render
     */
    QStringList backendTypes() const override;

    /**
     * @brief Return how the plugin settings should be used.
     */
    QString renderPolicy() const override;

    /**
     * @brief Preferred level in the layer stack for the rendering
     */
    QStringList renderPosition() const override;

    /**
     * @brief Renders the content provided by the plugin on the viewport.
     * @return @c true  Returns whether the rendering has been successful
     */
    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("NONE"), GeoSceneLayer* layer = nullptr) override;

    /**
     * @return The model associated with the plugin.
     */
    AbstractDataPluginModel* model();
    const AbstractDataPluginModel* model() const;

    /**
     * Set the model of the plugin.
     */
    void setModel(AbstractDataPluginModel* model);

    /**
     * Set the number of items to be shown at the same time.
     */
    void setNumberOfItems(quint32 number);

    /**
     * @return The number of items to be shown at the same time.
     */
    quint32 numberOfItems() const;

    /**
     * This function returns all items at the position @p curpos. Depending on where they have
     * been painted the last time.
     *
     * @return The items at the given position.
     */
    QList<AbstractDataPluginItem*> whichItemAt(const QPoint& curpos);

    /**
     * Function for returning the type of plugin this is for.
     * This affects where in the menu tree the action() is placed.
     *
     * @return: The type of render plugin this is.
     */
    RenderType renderType() const override;

    /** Convenience method to set the favorite item state on the current model */
    void setFavoriteItemsOnly(bool favoriteOnly);

    bool isFavoriteItemsOnly() const;

    QObject* favoritesModel();

private Q_SLOTS:

    virtual void favoriteItemsChanged(const QStringList& favoriteItems);

    void delayedUpdate();

Q_SIGNALS:

    void changedNumberOfItems(quint32 number);

    void favoriteItemsOnlyChanged();

    void favoritesModelChanged();

private:

    AbstractDataPluginPrivate* const d = nullptr;
};

} // namespace Marble

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
#include <QStandardItemModel>
#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Marble
{

class GeoSceneDocument;
class GeoDataPhotoOverlay;

/**
 * @short The class that handles map themes that are locally available .
 *
 * This class which is able to check for maps that are locally available.
 * After parsing the data it only stores the name, description and path
 * into a QStandardItemModel.
 *
 * The MapThemeManager is not owned by the MarbleWidget/Map itself.
 * Instead it is owned by the widget or application that contains
 * MarbleWidget/Map ( usually: the ControlView convenience class )
 *
 * For convenience MarbleThemeManager provides a static helper class
 * that loads the properties of a map theme into a GeoSceneDocument
 * object.
 *
 * @see GeoSceneDocument
 */

class DIGIKAM_EXPORT MapThemeManager : public QObject
{
    Q_OBJECT

public:

    explicit MapThemeManager(QObject* parent = nullptr);
    ~MapThemeManager() override;

    /**
     * @brief Returns a list of all locally available map theme IDs
     */
    QStringList mapThemeIds() const;

    /**
     * @brief Provides a model of the locally existing themes.
     *
     * This method provides a QStandardItemModel of all themes
     * that are available via MarbleDirs.
     */
    QStandardItemModel* mapThemeModel();

    /**
     * @brief Provides a model of all installed planets.
     */
    QStandardItemModel* celestialBodiesModel();

    /**
     * @brief Returns the map theme as a GeoSceneDocument object
     * @param mapThemeStringID  the string ID that refers to the map theme
     *
     * This helper method should only get used by MarbleModel to load the
     * current theme into memory or by the MapThemeManager.
     */
    static GeoSceneDocument* loadMapTheme(const QString& mapThemeStringID);

    /**
     * @brief Returns a map as a GeoSceneDocument object created from a GeoDataPhotoOverlay
     */
    static GeoSceneDocument* createMapThemeFromOverlay(const GeoDataPhotoOverlay* overlayData);

    /**
     * @brief Deletes the map theme with the specified map theme ID.
     * @param mapThemeId ID of the map theme to be deleted
     *
     * Deletion will only succeed for local map themes, that is, if the map
     * theme's directory structure resides in the user's home directory.
     */
    static void deleteMapTheme(const QString& mapThemeId);

Q_SIGNALS:

    /**
     * @brief This signal will be emitted, when the themes change.
     */
    void themesChanged();

private:

    Q_PRIVATE_SLOT(d, void directoryChanged(const QString& path))
    Q_PRIVATE_SLOT(d, void fileChanged(const QString& path))

    Q_DISABLE_COPY(MapThemeManager)

private:

    class Private;
    friend class Private;
    Private* const d = nullptr;
};

} // namespace Marble

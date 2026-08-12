/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-21
 * Description : Geolocation Settings.
 *
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "geolocationsettingscontainer.h"
#include "setupgeolocation.h"

namespace Marble
{
    class MarbleWidget;
}

using namespace Marble;

namespace Digikam
{

class MapWidget;

class DIGIKAM_EXPORT GeolocationSettings : public QObject
{
    Q_OBJECT

public:

    GeolocationSettings();
    ~GeolocationSettings() override;

    /**
     * Global container for Metadata settings. All accessor methods are thread-safe.
     */
    static GeolocationSettings* instance();

    /**
     * Returns the current Metadata settings.
     */
    GeolocationSettingsContainer settings() const;

    /**
     * Sets the current Metadata settings and writes them to config.
     */
    void setSettings(const GeolocationSettingsContainer& settings);

    /**
     * Store one MapWidget instance in the collection.
     */
    void registerWidget(MapWidget* const widget);

    /**
     * Remove one MapWidget instance in the collection.
     */
    void unregisterWidget(MapWidget* const widget);

    /**
     * Apply the current settings to a previously registered MapWidget.
     */
    void applySettingsToWidget(MapWidget* const widget);

    /**
     * Return the first registered MarbleWidget instance stored in the collection.
     * If no valid instance is found, nullptr is returned.
     */
    MarbleWidget* mainMarbleWidget() const;

    void openSetupGeolocation(SetupGeolocation::GeolocationTab tab);

    void reloadGoogleMaps();

Q_SIGNALS:

    void signalSettingsChanged();

    void signalGeolocationSettingsChanged(const GeolocationSettingsContainer& current,
                                          const GeolocationSettingsContainer& previous);

    void signalSetupGeolocation(int tab);

private:

    /// @note disabledd
    explicit GeolocationSettings(QObject*);

    void readFromConfig();

    void applySettingsToWidgets(const GeolocationSettingsContainer& settings);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

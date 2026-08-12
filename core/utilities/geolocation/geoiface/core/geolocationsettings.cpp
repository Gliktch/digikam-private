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

#include "geolocationsettings.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QList>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_debug.h"
#include "backendgooglemaps.h"
#include "backendmarble.h"
#include "mapwidget.h"
#include "MarbleGlobal.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "MarbleWidgetInputHandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN GeolocationSettings::Private
{
public:

    Private() = default;

public:

    GeolocationSettingsContainer settings;
    QMutex                       mutex;

    const QString                configGroup = QLatin1String("Marble Settings");

    QList<MapWidget*>            widgets;

public:

    GeolocationSettingsContainer readFromConfig() const;
    void                         writeToConfig()  const;
    GeolocationSettingsContainer setSettings(const GeolocationSettingsContainer& s);

    void applySettingsToWidget(MapWidget* const w, const GeolocationSettingsContainer& settings);
};

GeolocationSettingsContainer GeolocationSettings::Private::readFromConfig() const
{
    GeolocationSettingsContainer s;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    s.readFromConfig(group);

    return s;
}

void GeolocationSettings::Private::writeToConfig() const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroup);
    settings.writeToConfig(group);
}

GeolocationSettingsContainer GeolocationSettings::Private::setSettings(const GeolocationSettingsContainer& s)
{
    QMutexLocker lock(&mutex);
    GeolocationSettingsContainer old;
    old      = settings;
    settings = s;

    return old;
}

void GeolocationSettings::Private::applySettingsToWidget(MapWidget* const w,
                                                         const GeolocationSettingsContainer& settings)
{
    if (w)
    {
        const auto backs = w->backends();

        for (MapBackend* const b : backs)
        {
            BackendMarble* const mb = dynamic_cast<BackendMarble*>(b);

            if (mb && mb->isReady())
            {
                MarbleWidget* const mw = static_cast<MarbleWidget*>(mb->mapWidget());
                qCDebug(DIGIKAM_GEOIFACE_LOG) << "Apply settings to Marble Widget instance:" << mw;

                MarbleGlobal::getInstance()->locale()->setMeasurementSystem(settings.distanceUnit);
                mw->model()->setPersistentTileCacheLimit(settings.persistentTileCacheLimit * 1024);
                mw->setDefaultFont(settings.mapFont);
                mw->setMapQualityForViewContext(settings.stillQuality,     Marble::Still);
                mw->setMapQualityForViewContext(settings.animationQuality, Marble::Animation);
                mw->setDefaultAngleUnit(settings.angleUnit);
                mw->inputHandler()->setInertialEarthRotationEnabled(settings.inertialRotation);
                mw->inputHandler()->setMouseViewRotationEnabled(settings.mouseRotation);
                mw->setVolatileTileCacheLimit(settings.volatileTileCacheLimit * 1024);
                mw->setShowGrid(settings.showGrid);
                mw->setShowCrosshairs(settings.showCross);
                mw->setShowAtmosphere(settings.showAtmos);
                mw->setShowSunShading(settings.showSunShading);
                mw->setShowCities(settings.showCities);
                mw->setShowRelief(settings.showRelief);
                mw->update();
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN GeolocationSettingsCreator
{
public:

    GeolocationSettings object;
};

Q_GLOBAL_STATIC(GeolocationSettingsCreator, creator)

// -----------------------------------------------------------------------------------------------

GeolocationSettings::GeolocationSettings()
    : d(new Private)
{
    readFromConfig();
    qRegisterMetaType<GeolocationSettingsContainer>("GeolocationSettingsContainer");
}

GeolocationSettings::~GeolocationSettings()
{
    delete d;
}

GeolocationSettings* GeolocationSettings::instance()
{
    return &creator->object;
}

void GeolocationSettings::registerWidget(MapWidget* const widget)
{
    if (!widget)
    {
        return;
    }

    if (!d->widgets.contains(widget))
    {
        d->widgets << widget;
    }
}

void GeolocationSettings::applySettingsToWidget(MapWidget* const widget)
{
    d->applySettingsToWidget(widget, d->settings);
}

void GeolocationSettings::unregisterWidget(MapWidget* const widget)
{
    if (!widget)
    {
        return;
    }

    d->widgets.removeAll(widget);
}

void GeolocationSettings::applySettingsToWidgets(const GeolocationSettingsContainer& settings)
{
    for (MapWidget* const w : std::as_const(d->widgets))
    {
        d->applySettingsToWidget(w, settings);
    }
}

MarbleWidget* GeolocationSettings::mainMarbleWidget() const
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Registered Marble Widget:" << d->widgets;

    for (const MapWidget* const w : std::as_const(d->widgets))
    {
        if (w)
        {
            const auto backs = w->backends();

            for (MapBackend* const b : backs)
            {
                BackendMarble* const mb = dynamic_cast<BackendMarble*>(b);

                if (mb)
                {
                    return (static_cast<MarbleWidget*>(mb->mapWidget()));
                }
            }
        }
    }

    return nullptr;
}

void GeolocationSettings::reloadGoogleMaps()
{
    for (const MapWidget* const w : std::as_const(d->widgets))
    {
        if (w)
        {
            const auto backs = w->backends();

            for (MapBackend* const b : backs)
            {
                BackendGoogleMaps* const gb = dynamic_cast<BackendGoogleMaps*>(b);

                if (gb)
                {
                    gb->reload();
                }
            }
        }
    }
}

GeolocationSettingsContainer GeolocationSettings::settings() const
{
    QMutexLocker lock(&d->mutex);
    GeolocationSettingsContainer s(d->settings);

    return s;
}

void GeolocationSettings::setSettings(const GeolocationSettingsContainer& settings)
{
    GeolocationSettingsContainer old = d->setSettings(settings);

    Q_EMIT signalGeolocationSettingsChanged(settings, old);

    Q_EMIT signalSettingsChanged();

    applySettingsToWidgets(settings);
    d->writeToConfig();
}

void GeolocationSettings::readFromConfig()
{
    GeolocationSettingsContainer s   = d->readFromConfig();
    GeolocationSettingsContainer old = d->setSettings(s);

    Q_EMIT signalGeolocationSettingsChanged(s, old);

    Q_EMIT signalSettingsChanged();
}

void GeolocationSettings::openSetupGeolocation(SetupGeolocation::GeolocationTab tab)
{
    Q_EMIT signalSetupGeolocation(tab);
}

} // namespace Digikam

#include "moc_geolocationsettings.cpp"

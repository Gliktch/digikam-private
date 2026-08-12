/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-21
 * Description : Marble Settings View.
 *
 * SPDX-FileCopyrightText: 2009      by Bastian Holst <bastianholst at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Mohammed Nafees <nafees dot technocool at gmail dot com>
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "MarbleConfigView.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFontComboBox>
#include <QComboBox>
#include <QGroupBox>
#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QDateTime>
#include <QTimer>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "MarbleDirs.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "MarbleLocale.h"
#include "MarbleGlobal.h"
#include "RenderPlugin.h"
#include "RenderPluginModel.h"
#include "ParseRunnerPlugin.h"
#include "MarbleClock.h"
#include "MarblePluginSettingsWidget.h"

#include "geolocationsettings.h"
#include "digikam_debug.h"
#include "dexpanderbox.h"

using namespace Digikam;

namespace Marble
{

class Q_DECL_HIDDEN MarbleConfigView::Private
{

public:

    explicit Private(MarbleWidget* const marbleWidget)
        : marbleWidget(marbleWidget),
          pluginModel ()
    {
    }

    MarblePluginSettingsWidget*        pluginSettings   = nullptr;

    QComboBox*                         distCb           = nullptr;
    QComboBox*                         angleCb          = nullptr;
    QComboBox*                         stillCb          = nullptr;
    QComboBox*                         aniCb            = nullptr;

    QCheckBox*                         inertialRotation = nullptr;
    QCheckBox*                         viewRotation     = nullptr;

    QSpinBox*                          phySb            = nullptr;
    QSpinBox*                          discSb           = nullptr;

    QFontComboBox*                     mapFont          = nullptr;
    QCheckBox*                         showGrid         = nullptr;
    QCheckBox*                         showCross        = nullptr;
    QCheckBox*                         showAtmos        = nullptr;
    QCheckBox*                         showSunShading   = nullptr;
    QCheckBox*                         showCities       = nullptr;
    QCheckBox*                         showRelief       = nullptr;

    MarbleWidget* const                marbleWidget;

    RenderPluginModel                  pluginModel;
};

MarbleConfigView::MarbleConfigView(MarbleWidget* const marbleWidget,
                                   QWidget* const parent)
    : QTabWidget(parent),
      d         (new Private(marbleWidget))
{
    // View page

    QWidget* const viewSettings  = new QWidget(this);

    // --- Unit Settings

    QGroupBox* const grpUnits    = new QGroupBox(i18n("Units"), viewSettings);
    QGridLayout* const gridUnits = new QGridLayout(grpUnits);
    QLabel* const distLbl        = new QLabel(i18n("Distance:"), grpUnits);
    d->distCb                    = new QComboBox(grpUnits);
    d->distCb->insertItem(MarbleLocale::MetricSystem,   i18n("Kilometer, Meter"));
    d->distCb->insertItem(MarbleLocale::ImperialSystem, i18n("Miles, Feet"));
    d->distCb->insertItem(MarbleLocale::NauticalSystem, i18n("Nautical miles, Knots"));
    d->distCb->setToolTip(i18n("The unit that gets used to measure altitude, lengths and distances (e.g. km, mi, ft)."));

    QLabel* const angleLbl       = new QLabel(i18n("Angle:"), grpUnits);
    d->angleCb                   = new QComboBox(grpUnits);
    d->angleCb->insertItem(Marble::DMSDegree,     i18n("Degree (DMS)"));
    d->angleCb->insertItem(Marble::DecimalDegree, i18n("Degree (Decimal)"));
    d->angleCb->insertItem(Marble::UTM,           i18n("Universal Transverse Mercator (UTM)"));
    d->angleCb->setToolTip(i18n("Specifies the notation of angles in coordinates.\n"
                                "By default the Degree-Minute-Second notation (e.g. 54\302\26030'00\" ) gets used.\n"
                                "As an alternative you can choose decimal degrees (e.g. 54.5\302\260)."));

    gridUnits->addWidget(distLbl,    0, 0, 1, 1);
    gridUnits->addWidget(d->distCb,  0, 1, 1, 1);
    gridUnits->addWidget(angleLbl,   1, 0, 1, 1);
    gridUnits->addWidget(d->angleCb, 1, 1, 1, 1);

    // --- Map quality settings

    QGroupBox* const grpQa       = new QGroupBox(i18n("Map Quality"), viewSettings);
    QGridLayout* const gridQa    = new QGridLayout(grpQa);
    QLabel* const stillLbl       = new QLabel(i18n("Still Image:"), grpQa);
    d->stillCb                   = new QComboBox(grpQa);
    d->stillCb->insertItem(Marble::OutlineQuality,   i18n("Outline Quality"));
    d->stillCb->insertItem(Marble::LowQuality,       i18n("Low Quality"));
    d->stillCb->insertItem(Marble::NormalQuality,    i18n("Normal"));
    d->stillCb->insertItem(Marble::HighQuality,      i18n("High Quality"));
    d->stillCb->insertItem(Marble::PrintQuality,     i18n("Print Quality"));
    d->stillCb->setToolTip(i18n("Specifies the map quality that gets displayed while there is no user input.\n"
                                "Usually this allows for high map quality as speed is no concern."));

    QLabel* const aniLbl         = new QLabel(i18n("During Animations:"), grpQa);
    d->aniCb                     = new QComboBox(grpQa);
    d->aniCb->insertItem(Marble::OutlineQuality,   i18n("Outline Quality"));
    d->aniCb->insertItem(Marble::LowQuality,       i18n("Low Quality"));
    d->aniCb->insertItem(Marble::NormalQuality,    i18n("Normal"));
    d->aniCb->insertItem(Marble::HighQuality,      i18n("High Quality"));
    d->aniCb->insertItem(Marble::PrintQuality,     i18n("Print Quality"));
    d->aniCb->setToolTip(i18n("Specifies the map quality that gets displayed during map animations\n"
                              "(e.g. while dragging the globe). Especially on slow machines it is\n"
                              "advisable to set this option to \"low quality\" as this will give better speed."));

    gridQa->addWidget(stillLbl,    0, 0, 1, 1);
    gridQa->addWidget(d->stillCb,  0, 1, 1, 1);
    gridQa->addWidget(aniLbl,      1, 0, 1, 1);
    gridQa->addWidget(d->aniCb,    1, 1, 1, 1);

    // --- Mouse behavior settings

    QGroupBox* const grpMouse    = new QGroupBox(i18n("Mouse Behavior"), viewSettings);
    QGridLayout* const gridMouse = new QGridLayout(grpMouse);
    d->inertialRotation          = new QCheckBox(i18n("Inertial Globe Rotation"), grpMouse);
    d->inertialRotation->setToolTip(i18n("Use kinetic spinning when dragging the map"));
    d->viewRotation              = new QCheckBox(i18n("Mouse View Rotation"), grpMouse);
    d->viewRotation->setToolTip(i18n("Use right mouse button to rotate the camera around"));

    gridMouse->addWidget(d->inertialRotation, 0, 0, 1, 1);
    gridMouse->addWidget(d->viewRotation,     1, 0, 1, 1);

    // --- Cache settings

    QGroupBox* const grpCache    = new QGroupBox(i18n("Cache"), viewSettings);
    grpCache->setToolTip(i18n("There are two caches being used:\n"
                              "The \"physical memory\" which is needed to keep map data in the computer's memory.\n"
                              "Increasing the value will make the application more responsive.\n"
                              "The \"hard disc memory\" cache is used by download map data from the Internet.\n"
                              "Decrease this value if you want to save space on the hard disc\n"
                              "and if high usage of the Internet is not an issue."));
    QGridLayout* const gridCache = new QGridLayout(grpCache);
    QLabel* const phyLbl         = new QLabel(i18n("Physical Memory:"), grpCache);
    d->phySb                     = new QSpinBox(grpCache);
    d->phySb->setRange(10, 999999);
    d->phySb->setValue(100);
    d->phySb->setSuffix(i18n(" MB"));
    QPushButton* const phyBtn    = new QPushButton(i18n("Clear"), grpCache);

    QLabel* const discLbl        = new QLabel(i18n("Hard Disc:"), grpCache);
    d->discSb                    = new QSpinBox(grpCache);
    d->discSb->setRange(10, 999999);
    d->discSb->setValue(999999);
    d->discSb->setSuffix(i18n(" MB"));
    QPushButton* const discBtn   = new QPushButton(i18n("Clear"), grpCache);

    gridCache->addWidget(phyLbl,              0, 0, 1, 1);
    gridCache->addWidget(d->phySb,            0, 1, 1, 1);
    gridCache->addWidget(phyBtn,              0, 2, 1, 1);
    gridCache->addWidget(discLbl,             1, 0, 1, 1);
    gridCache->addWidget(d->discSb,           1, 1, 1, 1);
    gridCache->addWidget(discBtn,             1, 2, 1, 1);

    // --- Map content settings

    QGroupBox* const grpMap      = new QGroupBox(i18n("Map Contents"), viewSettings);
    QGridLayout* const gridMap   = new QGridLayout(grpMap);
    QLabel* const fntLbl         = new QLabel(i18n("Default Font:"), grpMap);
    d->mapFont                   = new QFontComboBox(grpMap);
    d->mapFont->setToolTip(i18n("The default font that gets used on the map.\n"
                                "This setting take effect only with the Atlas and Satellite Maps\n"
                                "not the Open-Street Map."));

    d->showGrid                  = new QCheckBox(i18n("Show Coordinate Grid"), grpMap);
    d->showGrid->setToolTip(i18n("Show the world-map coordinate grid overlay.\n"
                                 "This setting take effect only with the Atlas,\n"
                                 "the Satellite, and the Open-Street Maps."));

    d->showCross                 = new QCheckBox(i18n("Show Cross-Hair"), grpMap);
    d->showCross->setToolTip(i18n("Show cross-hair on a center of the map.\n"
                                  "This setting take effect only with the Atlas,\n"
                                  "the Satellite, and the Open-Street Maps."));

    d->showAtmos                 = new QCheckBox(i18n("Show Atmospheric Glow"), grpMap);
    d->showAtmos->setToolTip(i18n("Show the world-map atmospheric glow overlay.\n"
                                  "This setting take effect only with the Atlas,\n"
                                  "the Satellite, and the Open-Street Maps."));

    d->showSunShading            = new QCheckBox(i18n("Show Night Shadow"), grpMap);
    d->showSunShading->setToolTip(i18n("Show the world-map night shadow overlay.\n"
                                       "This setting take effect only with the Atlas,\n"
                                       "the Satellite, and the Open-Street Maps."));

    d->showCities                = new QCheckBox(i18n("Show City Place Mark"), grpMap);
    d->showCities->setToolTip(i18n("Show the world-map city place mark overlay.\n"
                                   "This setting take effect only with the Atlas and Satellite Maps\n"
                                   "not the Open-Street Map."));

    d->showRelief                = new QCheckBox(i18n("Show Relief"), grpMap);
    d->showRelief->setToolTip(i18n("Show the world-map relief overlay.\n"
                                   "This setting take effect only with the Atlas and Satellite Maps\n"
                                   "not the Open-Street Map."));

    // TODO:
    // Add IceLayer
    // Add Borders
    // Add Rivers       ==> Do not works!
    // Add Lakes
    // Add CityLights
    // Add Clouds ???
    // Add Terrain ???
    // Add Places ???
    // Add OtherPlaces ???

    gridMap->addWidget(d->showGrid,                             0, 0, 1, 1);
    gridMap->addWidget(d->showCross,                            0, 1, 1, 1);
    gridMap->addWidget(d->showAtmos,                            1, 0, 1, 1);
    gridMap->addWidget(d->showSunShading,                       1, 1, 1, 1);
    gridMap->addWidget(new DLineWidget(Qt::Horizontal, grpMap), 2, 0, 1, 2);
    gridMap->addWidget(d->showCities,                           3, 0, 1, 1);
    gridMap->addWidget(d->showRelief,                           3, 1, 1, 1);
    gridMap->addWidget(fntLbl,                                  4, 0, 1, 1);
    gridMap->addWidget(d->mapFont,                              4, 1, 1, 1);
    gridMap->setRowStretch(5, 10);

    // ---

    QGridLayout* const viewGrid  = new QGridLayout(viewSettings);
    viewGrid->addWidget(grpUnits, 0, 0, 1, 1);
    viewGrid->addWidget(grpQa,    0, 1, 1, 1);
    viewGrid->addWidget(grpMouse, 1, 0, 1, 1);
    viewGrid->addWidget(grpCache, 1, 1, 1, 1);
    viewGrid->addWidget(grpMap,   2, 0, 1, 2);

    addTab(viewSettings, i18n("Map View"));

    if (d->marbleWidget)
    {
        connect(phyBtn, SIGNAL(clicked()),
                d->marbleWidget, SLOT(clearVolatileTileCache()));

        connect(discBtn, SIGNAL(clicked()),
                d->marbleWidget->model(), SLOT(clearPersistentTileCache()));
    }

    // Plugins page


    d->pluginSettings = new MarblePluginSettingsWidget(this);

    if (d->marbleWidget)
    {
        d->pluginModel.setRenderPlugins(d->marbleWidget->renderPlugins());
        d->pluginSettings->setRunnerPlugins(d->marbleWidget->runnerPlugins());
    }

    d->pluginSettings->setModel(&d->pluginModel);
    d->pluginSettings->setObjectName(QLatin1String("plugin_page"));
    addTab(d->pluginSettings, i18n("Map Plugins"));

    // Setting the icons for the plugin view.

    d->pluginSettings->setConfigIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->pluginSettings->setAboutIcon(QIcon::fromTheme(QLatin1String("help-about")));
}

MarbleConfigView::~MarbleConfigView()
{
    delete d;
}

void MarbleConfigView::cancel()
{
    d->pluginModel.retrievePluginState();
}

void MarbleConfigView::readSettings()
{
    // Read settings from config file.

    GeolocationSettingsContainer settings = GeolocationSettings::instance()->settings();

    // View

    d->distCb->setCurrentIndex(settings.distanceUnit);
    d->angleCb->setCurrentIndex(settings.angleUnit);
    d->stillCb->setCurrentIndex(settings.stillQuality);
    d->aniCb->setCurrentIndex(settings.animationQuality);
    d->inertialRotation->setChecked(settings.inertialRotation);
    d->viewRotation->setChecked(settings.mouseRotation);
    d->phySb->setValue(settings.volatileTileCacheLimit);
    d->discSb->setValue(settings.persistentTileCacheLimit);
    d->mapFont->setCurrentFont(settings.mapFont);
    d->showGrid->setChecked(settings.showGrid);
    d->showCross->setChecked(settings.showCross);
    d->showAtmos->setChecked(settings.showAtmos);
    d->showSunShading->setChecked(settings.showSunShading);
    d->showCities->setChecked(settings.showCities);
    d->showRelief->setChecked(settings.showRelief);

    // Read the settings of the plugins

    if (d->marbleWidget)
    {
        d->marbleWidget->readPluginSettings();
    }
}

void MarbleConfigView::applySettings()
{
    d->pluginModel.applyPluginState();

    GeolocationSettingsContainer settings;

    settings.distanceUnit             = (Marble::MarbleLocale::MeasurementSystem)d->distCb->currentIndex();
    settings.angleUnit                = (Marble::AngleUnit)d->angleCb->currentIndex();
    settings.stillQuality             = (Marble::MapQuality)d->stillCb->currentIndex();
    settings.animationQuality         = (Marble::MapQuality)d->aniCb->currentIndex();
    settings.inertialRotation         = d->inertialRotation->isChecked();
    settings.mouseRotation            = d->viewRotation->isChecked();
    settings.volatileTileCacheLimit   = d->phySb->value();
    settings.persistentTileCacheLimit = d->discSb->value();
    settings.mapFont                  = d->mapFont->currentFont();
    settings.showGrid                 = d->showGrid->isChecked();
    settings.showCross                = d->showCross->isChecked();
    settings.showAtmos                = d->showAtmos->isChecked();
    settings.showSunShading           = d->showSunShading->isChecked();
    settings.showCities               = d->showCities->isChecked();
    settings.showRelief               = d->showRelief->isChecked();

    GeolocationSettings::instance()->setSettings(settings);

    // Plugins

    if (d->marbleWidget)
    {
        d->marbleWidget->writePluginSettings();
    }
}

} // namespace Marble

#include "moc_MarbleConfigView.cpp"

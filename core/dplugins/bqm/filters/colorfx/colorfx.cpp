/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-11-08
 * Description : a batch tool to apply color effects to images.
 *
 * SPDX-FileCopyrightText: 2012 by Alexander Dymo <adymo at develop dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorfx.h"

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "colorfxfilter.h"

namespace DigikamBqmColorFXPlugin
{

ColorFX::ColorFX(QObject* const parent)
    : BatchTool(QLatin1String("ColorFX"), FiltersTool, parent)
{
}

BatchTool* ColorFX::clone(QObject* const parent) const
{
    return new ColorFX(parent);
}

void ColorFX::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new ColorFXSettings(m_settingsWidget);
    m_settingsView->startPreviewFilters();
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ColorFX::defaultSettings()
{
    BatchToolSettings prm;
    ColorFXContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("colorFXType"), (int)defaultPrm.colorFXType);
    prm.insert(QLatin1String("level"),       (int)defaultPrm.level);
    prm.insert(QLatin1String("iterations"),  (int)defaultPrm.iterations);
    prm.insert(QLatin1String("intensity"),   (int)defaultPrm.intensity);
    prm.insert(QLatin1String("path"),        defaultPrm.path);

    return prm;
}

void ColorFX::slotAssignSettings2Widget()
{
    ColorFXContainer prm;

    prm.colorFXType = settings().value(QLatin1String("colorFXType")).toInt();
    prm.level       = settings().value(QLatin1String("level")).toInt();
    prm.iterations  = settings().value(QLatin1String("iterations")).toInt();
    prm.intensity   = settings().value(QLatin1String("intensity")).toInt();
    prm.path        = settings().value(QLatin1String("path")).toString();

    m_settingsView->setSettings(prm);
}

void ColorFX::slotSettingsChanged()
{
    BatchToolSettings prm;
    ColorFXContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("colorFXType"), (int)currentPrm.colorFXType);
    prm.insert(QLatin1String("level"),       (int)currentPrm.level);
    prm.insert(QLatin1String("iterations"),  (int)currentPrm.iterations);
    prm.insert(QLatin1String("intensity"),   (int)currentPrm.intensity);
    prm.insert(QLatin1String("path"),        currentPrm.path);

    BatchTool::slotSettingsChanged(prm);
}

bool ColorFX::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    ColorFXContainer prm;
    prm.colorFXType = settings().value(QLatin1String("colorFXType")).toInt();
    prm.level       = settings().value(QLatin1String("level")).toInt();
    prm.iterations  = settings().value(QLatin1String("iterations")).toInt();
    prm.intensity   = settings().value(QLatin1String("intensity")).toInt();
    prm.path        = settings().value(QLatin1String("path")).toString();

    ColorFXFilter fg(&image(), nullptr, prm);
    applyFilter(&fg);

    return savefromDImg();
}

} // namespace DigikamBqmColorFXPlugin

#include "moc_colorfx.cpp"

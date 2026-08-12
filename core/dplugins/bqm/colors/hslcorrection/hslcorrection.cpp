/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Hue/Saturation/Lightness batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hslcorrection.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "hslfilter.h"

namespace DigikamBqmHSLCorrectionPlugin
{

HSLCorrection::HSLCorrection(QObject* const parent)
    : BatchTool(QLatin1String("HSLCorrection"), ColorTool, parent)
{
}

BatchTool* HSLCorrection::clone(QObject* const parent) const
{
    return new HSLCorrection(parent);
}

void HSLCorrection::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new HSLSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings HSLCorrection::defaultSettings()
{
    BatchToolSettings prm;
    HSLContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("Hue"),        (double)defaultPrm.hue);
    prm.insert(QLatin1String("Saturation"), (double)defaultPrm.saturation);
    prm.insert(QLatin1String("Lightness"),  (double)defaultPrm.lightness);
    prm.insert(QLatin1String("Vibrance"),   (double)defaultPrm.vibrance);

    return prm;
}

void HSLCorrection::slotAssignSettings2Widget()
{
    HSLContainer prm;
    prm.hue        = settings().value(QLatin1String("Hue")).toDouble();
    prm.saturation = settings().value(QLatin1String("Saturation")).toDouble();
    prm.lightness  = settings().value(QLatin1String("Lightness")).toDouble();
    prm.vibrance   = settings().value(QLatin1String("Vibrance")).toDouble();
    m_settingsView->setSettings(prm);
}

void HSLCorrection::slotSettingsChanged()
{
    BatchToolSettings prm;
    HSLContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("Hue"),        (double)currentPrm.hue);
    prm.insert(QLatin1String("Saturation"), (double)currentPrm.saturation);
    prm.insert(QLatin1String("Lightness"),  (double)currentPrm.lightness);
    prm.insert(QLatin1String("Vibrance"),   (double)currentPrm.vibrance);

    BatchTool::slotSettingsChanged(prm);
}

bool HSLCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    HSLContainer prm;
    prm.hue        = settings().value(QLatin1String("Hue")).toDouble();
    prm.saturation = settings().value(QLatin1String("Saturation")).toDouble();
    prm.lightness  = settings().value(QLatin1String("Lightness")).toDouble();
    prm.vibrance   = settings().value(QLatin1String("Vibrance")).toDouble();

    HSLFilter hsl(&image(), nullptr, prm);
    applyFilter(&hsl);

    return (savefromDImg());
}

} // namespace DigikamBqmHSLCorrectionPlugin

#include "moc_hslcorrection.cpp"

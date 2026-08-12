/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "noisereduction.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "nrestimate.h"
#include "nrfilter.h"

namespace DigikamBqmNoiseReductionPlugin
{

NoiseReduction::NoiseReduction(QObject* const parent)
    : BatchTool     (QLatin1String("NoiseReduction"), EnhanceTool, parent)
{
}

BatchTool* NoiseReduction::clone(QObject* const parent) const
{
    return new NoiseReduction(parent);
}

void NoiseReduction::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new NRSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalEstimateNoise()),
            this, SLOT(slotSettingsChanged()));

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings NoiseReduction::defaultSettings()
{
    BatchToolSettings prm;
    NRContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("YThreshold"),    (double)defaultPrm.thresholds[0]);
    prm.insert(QLatin1String("CrThreshold"),   (double)defaultPrm.thresholds[1]);
    prm.insert(QLatin1String("CbThreshold"),   (double)defaultPrm.thresholds[2]);
    prm.insert(QLatin1String("YSoftness"),     (double)defaultPrm.softness[0]);
    prm.insert(QLatin1String("CrSoftness"),    (double)defaultPrm.softness[1]);
    prm.insert(QLatin1String("CbSoftness"),    (double)defaultPrm.softness[2]);
    prm.insert(QLatin1String("EstimateNoise"), false);

    return prm;
}

void NoiseReduction::slotAssignSettings2Widget()
{
    NRContainer prm;
    prm.thresholds[0] = settings().value(QLatin1String("YThreshold")).toDouble();
    prm.thresholds[1] = settings().value(QLatin1String("CrThreshold")).toDouble();
    prm.thresholds[2] = settings().value(QLatin1String("CbThreshold")).toDouble();
    prm.softness[0]   = settings().value(QLatin1String("YSoftness")).toDouble();
    prm.softness[1]   = settings().value(QLatin1String("CrSoftness")).toDouble();
    prm.softness[2]   = settings().value(QLatin1String("CbSoftness")).toDouble();
    m_settingsView->setSettings(prm);
    m_settingsView->setEstimateNoise(settings().value(QLatin1String("EstimateNoise")).toBool());
}

void NoiseReduction::slotSettingsChanged()
{
    BatchToolSettings prm;
    NRContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("YThreshold"),    (double)currentPrm.thresholds[0]);
    prm.insert(QLatin1String("CrThreshold"),   (double)currentPrm.thresholds[1]);
    prm.insert(QLatin1String("CbThreshold"),   (double)currentPrm.thresholds[2]);
    prm.insert(QLatin1String("YSoftness"),     (double)currentPrm.softness[0]);
    prm.insert(QLatin1String("CrSoftness"),    (double)currentPrm.softness[1]);
    prm.insert(QLatin1String("CbSoftness"),    (double)currentPrm.softness[2]);
    prm.insert(QLatin1String("EstimateNoise"), m_settingsView->estimateNoise());

    BatchTool::slotSettingsChanged(prm);
}

bool NoiseReduction::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    NRContainer prm;

    if (settings().value(QLatin1String("EstimateNoise")).toBool())
    {
        NREstimate nre(&image());
        nre.startFilterDirectly();
        prm = nre.settings();
    }
    else
    {
        prm.thresholds[0] = settings().value(QLatin1String("YThreshold")).toDouble();
        prm.thresholds[1] = settings().value(QLatin1String("CrThreshold")).toDouble();
        prm.thresholds[2] = settings().value(QLatin1String("CbThreshold")).toDouble();
        prm.softness[0]   = settings().value(QLatin1String("YSoftness")).toDouble();
        prm.softness[1]   = settings().value(QLatin1String("CrSoftness")).toDouble();
        prm.softness[2]   = settings().value(QLatin1String("CbSoftness")).toDouble();
    }

    NRFilter wnr(&image(), nullptr, prm);
    applyFilter(&wnr);

    return (savefromDImg());
}

} // namespace DigikamBqmNoiseReductionPlugin

#include "moc_noisereduction.cpp"

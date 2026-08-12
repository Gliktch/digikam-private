/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : LDR tonemapper batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "localcontrast.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "localcontrastfilter.h"
#include "localcontrastcontainer.h"

namespace DigikamBqmLocalContrastPlugin
{

LocalContrast::LocalContrast(QObject* const parent)
    : BatchTool(QLatin1String("LocalContrast"), EnhanceTool, parent)
{
}

BatchTool* LocalContrast::clone(QObject* const parent) const
{
    return new LocalContrast(parent);
}

void LocalContrast::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new LocalContrastSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings LocalContrast::defaultSettings()
{
    BatchToolSettings prm;
    LocalContrastContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("stretchContrast"), defaultPrm.stretchContrast);
    prm.insert(QLatin1String("lowSaturation"),   defaultPrm.lowSaturation);
    prm.insert(QLatin1String("highSaturation"),  defaultPrm.highSaturation);
    prm.insert(QLatin1String("functionId"),      defaultPrm.functionId);

    prm.insert(QLatin1String("stage1Enabled"),   defaultPrm.stage[0].enabled);
    prm.insert(QLatin1String("stage1Power"),     defaultPrm.stage[0].power);
    prm.insert(QLatin1String("stage1Blur"),      defaultPrm.stage[0].blur);

    prm.insert(QLatin1String("stage2Enabled"),   defaultPrm.stage[1].enabled);
    prm.insert(QLatin1String("stage2Power"),     defaultPrm.stage[1].power);
    prm.insert(QLatin1String("stage2Blur"),      defaultPrm.stage[1].blur);

    prm.insert(QLatin1String("stage3Enabled"),   defaultPrm.stage[2].enabled);
    prm.insert(QLatin1String("stage3Power"),     defaultPrm.stage[2].power);
    prm.insert(QLatin1String("stage3Blur"),      defaultPrm.stage[2].blur);

    prm.insert(QLatin1String("stage4Enabled"),   defaultPrm.stage[3].enabled);
    prm.insert(QLatin1String("stage4Power"),     defaultPrm.stage[3].power);
    prm.insert(QLatin1String("stage4Blur"),      defaultPrm.stage[3].blur);

    return prm;
}

void LocalContrast::slotAssignSettings2Widget()
{
    LocalContrastContainer prm;

    prm.stretchContrast = settings().value(QLatin1String("stretchContrast")).toBool();
    prm.lowSaturation   = settings().value(QLatin1String("lowSaturation")).toInt();
    prm.highSaturation  = settings().value(QLatin1String("highSaturation")).toInt();
    prm.functionId      = settings().value(QLatin1String("functionId")).toInt();

    prm.stage[0].enabled = settings().value(QLatin1String("stage1Enabled")).toBool();
    prm.stage[0].power   = settings().value(QLatin1String("stage1Power")).toDouble();
    prm.stage[0].blur    = settings().value(QLatin1String("stage1Blur")).toDouble();

    prm.stage[1].enabled = settings().value(QLatin1String("stage2Enabled")).toBool();
    prm.stage[1].power   = settings().value(QLatin1String("stage2Power")).toDouble();
    prm.stage[1].blur    = settings().value(QLatin1String("stage2Blur")).toDouble();

    prm.stage[2].enabled = settings().value(QLatin1String("stage3Enabled")).toBool();
    prm.stage[2].power   = settings().value(QLatin1String("stage3Power")).toDouble();
    prm.stage[2].blur    = settings().value(QLatin1String("stage3Blur")).toDouble();

    prm.stage[3].enabled = settings().value(QLatin1String("stage4Enabled")).toBool();
    prm.stage[3].power   = settings().value(QLatin1String("stage4Power")).toDouble();
    prm.stage[3].blur    = settings().value(QLatin1String("stage4Blur")).toDouble();

    m_settingsView->setSettings(prm);
}

void LocalContrast::slotSettingsChanged()
{
    BatchToolSettings prm;
    LocalContrastContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("stretchContrast"), currentPrm.stretchContrast);
    prm.insert(QLatin1String("lowSaturation"),   currentPrm.lowSaturation);
    prm.insert(QLatin1String("highSaturation"),  currentPrm.highSaturation);
    prm.insert(QLatin1String("functionId"),      currentPrm.functionId);

    prm.insert(QLatin1String("stage1Enabled"),   currentPrm.stage[0].enabled);
    prm.insert(QLatin1String("stage1Power"),     currentPrm.stage[0].power);
    prm.insert(QLatin1String("stage1Blur"),      currentPrm.stage[0].blur);

    prm.insert(QLatin1String("stage2Enabled"),   currentPrm.stage[1].enabled);
    prm.insert(QLatin1String("stage2Power"),     currentPrm.stage[1].power);
    prm.insert(QLatin1String("stage2Blur"),      currentPrm.stage[1].blur);

    prm.insert(QLatin1String("stage3Enabled"),   currentPrm.stage[2].enabled);
    prm.insert(QLatin1String("stage3Power"),     currentPrm.stage[2].power);
    prm.insert(QLatin1String("stage3Blur"),      currentPrm.stage[2].blur);

    prm.insert(QLatin1String("stage4Enabled"),   currentPrm.stage[3].enabled);
    prm.insert(QLatin1String("stage4Power"),     currentPrm.stage[3].power);
    prm.insert(QLatin1String("stage4Blur"),      currentPrm.stage[3].blur);

    BatchTool::slotSettingsChanged(prm);
}

bool LocalContrast::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    LocalContrastContainer prm;

    prm.stretchContrast = settings().value(QLatin1String("stretchContrast")).toBool();
    prm.lowSaturation   = settings().value(QLatin1String("lowSaturation")).toInt();
    prm.highSaturation  = settings().value(QLatin1String("highSaturation")).toInt();
    prm.functionId      = settings().value(QLatin1String("functionId")).toInt();

    prm.stage[0].enabled = settings().value(QLatin1String("stage1Enabled")).toBool();
    prm.stage[0].power   = settings().value(QLatin1String("stage1Power")).toDouble();
    prm.stage[0].blur    = settings().value(QLatin1String("stage1Blur")).toDouble();

    prm.stage[1].enabled = settings().value(QLatin1String("stage2Enabled")).toBool();
    prm.stage[1].power   = settings().value(QLatin1String("stage2Power")).toDouble();
    prm.stage[1].blur    = settings().value(QLatin1String("stage2Blur")).toDouble();

    prm.stage[2].enabled = settings().value(QLatin1String("stage3Enabled")).toBool();
    prm.stage[2].power   = settings().value(QLatin1String("stage3Power")).toDouble();
    prm.stage[2].blur    = settings().value(QLatin1String("stage3Blur")).toDouble();

    prm.stage[3].enabled = settings().value(QLatin1String("stage4Enabled")).toBool();
    prm.stage[3].power   = settings().value(QLatin1String("stage4Power")).toDouble();
    prm.stage[3].blur    = settings().value(QLatin1String("stage4Blur")).toDouble();

    LocalContrastFilter lc(&image(), nullptr, prm);
    applyFilter(&lc);

    return (savefromDImg());
}

} // namespace DigikamBqmLocalContrastPlugin

#include "moc_localcontrast.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-19
 * Description : Channel Mixer batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "channelmixer.h"

// Qt includes

#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "mixerfilter.h"

namespace DigikamBqmChannelMixerPlugin
{

ChannelMixer::ChannelMixer(QObject* const parent)
    : BatchTool(QLatin1String("ChannelMixer"), ColorTool, parent)
{
}

BatchTool* ChannelMixer::clone(QObject* const parent) const
{
    return new ChannelMixer(parent);
}

void ChannelMixer::registerSettingsWidget()
{
    DVBox* const vbox    = new DVBox;
    m_settingsView       = new MixerSettings(vbox);
    QLabel* const space  = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget     = vbox;

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ChannelMixer::defaultSettings()
{
    BatchToolSettings prm;
    MixerContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("bPreserveLum"), (bool)defaultPrm.bPreserveLum);
    prm.insert(QLatin1String("bMonochrome"), (bool)defaultPrm.bMonochrome);

    // Standard settings.
    prm.insert(QLatin1String("redRedGain"), (double)defaultPrm.redRedGain);
    prm.insert(QLatin1String("redGreenGain"), (double)defaultPrm.redGreenGain);
    prm.insert(QLatin1String("redBlueGain"), (double)defaultPrm.redBlueGain);
    prm.insert(QLatin1String("greenRedGain"), (double)defaultPrm.greenRedGain);
    prm.insert(QLatin1String("greenGreenGain"), (double)defaultPrm.greenGreenGain);
    prm.insert(QLatin1String("greenBlueGain"), (double)defaultPrm.greenBlueGain);
    prm.insert(QLatin1String("blueRedGain"), (double)defaultPrm.blueRedGain);
    prm.insert(QLatin1String("blueGreenGain"), (double)defaultPrm.blueGreenGain);
    prm.insert(QLatin1String("blueBlueGain"), (double)defaultPrm.blueBlueGain);

    // Monochrome settings.
    prm.insert(QLatin1String("blackRedGain"), (double)defaultPrm.blackRedGain);
    prm.insert(QLatin1String("blackGreenGain"), (double)defaultPrm.blackGreenGain);
    prm.insert(QLatin1String("blackBlueGain"), (double)defaultPrm.blackBlueGain);

    return prm;
}

void ChannelMixer::slotAssignSettings2Widget()
{
    MixerContainer prm;

    prm.bPreserveLum   = settings().value(QLatin1String("bPreserveLum")).toBool();
    prm.bMonochrome    = settings().value(QLatin1String("bMonochrome")).toBool();

    // Standard settings.
    prm.redRedGain     = settings().value(QLatin1String("redRedGain")).toDouble();
    prm.redGreenGain   = settings().value(QLatin1String("redGreenGain")).toDouble();
    prm.redBlueGain    = settings().value(QLatin1String("redBlueGain")).toDouble();
    prm.greenRedGain   = settings().value(QLatin1String("greenRedGain")).toDouble();
    prm.greenGreenGain = settings().value(QLatin1String("greenGreenGain")).toDouble();
    prm.greenBlueGain  = settings().value(QLatin1String("greenBlueGain")).toDouble();
    prm.blueRedGain    = settings().value(QLatin1String("blueRedGain")).toDouble();
    prm.blueGreenGain  = settings().value(QLatin1String("blueGreenGain")).toDouble();
    prm.blueBlueGain   = settings().value(QLatin1String("blueBlueGain")).toDouble();

    // Monochrome settings.
    prm.blackRedGain   = settings().value(QLatin1String("blackRedGain")).toDouble();
    prm.blackGreenGain = settings().value(QLatin1String("blackGreenGain")).toDouble();
    prm.blackBlueGain  = settings().value(QLatin1String("blackBlueGain")).toDouble();

    m_settingsView->setSettings(prm);
}

void ChannelMixer::slotSettingsChanged()
{
    BatchToolSettings prm;
    MixerContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("bPreserveLum"), (bool)currentPrm.bPreserveLum);
    prm.insert(QLatin1String("bMonochrome"), (bool)currentPrm.bMonochrome);

    // Standard settings.
    prm.insert(QLatin1String("redRedGain"), (double)currentPrm.redRedGain);
    prm.insert(QLatin1String("redGreenGain"), (double)currentPrm.redGreenGain);
    prm.insert(QLatin1String("redBlueGain"), (double)currentPrm.redBlueGain);
    prm.insert(QLatin1String("greenRedGain"), (double)currentPrm.greenRedGain);
    prm.insert(QLatin1String("greenGreenGain"), (double)currentPrm.greenGreenGain);
    prm.insert(QLatin1String("greenBlueGain"), (double)currentPrm.greenBlueGain);
    prm.insert(QLatin1String("blueRedGain"), (double)currentPrm.blueRedGain);
    prm.insert(QLatin1String("blueGreenGain"), (double)currentPrm.blueGreenGain);
    prm.insert(QLatin1String("blueBlueGain"), (double)currentPrm.blueBlueGain);

    // Monochrome settings.
    prm.insert(QLatin1String("blackRedGain"), (double)currentPrm.blackRedGain);
    prm.insert(QLatin1String("blackGreenGain"), (double)currentPrm.blackGreenGain);
    prm.insert(QLatin1String("blackBlueGain"), (double)currentPrm.blackBlueGain);

    BatchTool::slotSettingsChanged(prm);
}

bool ChannelMixer::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    MixerContainer prm;

    prm.bPreserveLum   = settings().value(QLatin1String("bPreserveLum")).toBool();
    prm.bMonochrome    = settings().value(QLatin1String("bMonochrome")).toBool();

    // Standard settings.
    prm.redRedGain     = settings().value(QLatin1String("redRedGain")).toDouble();
    prm.redGreenGain   = settings().value(QLatin1String("redGreenGain")).toDouble();
    prm.redBlueGain    = settings().value(QLatin1String("redBlueGain")).toDouble();
    prm.greenRedGain   = settings().value(QLatin1String("greenRedGain")).toDouble();
    prm.greenGreenGain = settings().value(QLatin1String("greenGreenGain")).toDouble();
    prm.greenBlueGain  = settings().value(QLatin1String("greenBlueGain")).toDouble();
    prm.blueRedGain    = settings().value(QLatin1String("blueRedGain")).toDouble();
    prm.blueGreenGain  = settings().value(QLatin1String("blueGreenGain")).toDouble();
    prm.blueBlueGain   = settings().value(QLatin1String("blueBlueGain")).toDouble();

    // Monochrome settings.
    prm.blackRedGain   = settings().value(QLatin1String("blackRedGain")).toDouble();
    prm.blackGreenGain = settings().value(QLatin1String("blackGreenGain")).toDouble();
    prm.blackBlueGain  = settings().value(QLatin1String("blackBlueGain")).toDouble();

    MixerFilter mixer(&image(), nullptr, prm);
    applyFilter(&mixer);

    return (savefromDImg());
}

} // namespace DigikamBqmChannelMixerPlugin

#include "moc_channelmixer.cpp"

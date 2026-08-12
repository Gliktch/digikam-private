/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Color Balance batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorbalance.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "cbfilter.h"

namespace DigikamBqmColorBalancePlugin
{

ColorBalance::ColorBalance(QObject* const parent)
    : BatchTool(QLatin1String("ColorBalance"), ColorTool, parent)
{

    setToolTitle(i18n("Color Balance"));
    setToolDescription(i18n("Adjust color balance."));
    setToolIconName(QLatin1String("adjustrgb"));
}

BatchTool* ColorBalance::clone(QObject* const parent) const
{
    return new ColorBalance(parent);
}


void ColorBalance::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new CBSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ColorBalance::defaultSettings()
{
    BatchToolSettings prm;
    CBContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("Red"), (double)defaultPrm.red);
    prm.insert(QLatin1String("Green"), (double)defaultPrm.green);
    prm.insert(QLatin1String("Blue"), (double)defaultPrm.blue);

    return prm;
}

void ColorBalance::slotAssignSettings2Widget()
{
    CBContainer prm;
    prm.red   = settings().value(QLatin1String("Red")).toDouble();
    prm.green = settings().value(QLatin1String("Green")).toDouble();
    prm.blue  = settings().value(QLatin1String("Blue")).toDouble();
    m_settingsView->setSettings(prm);
}

void ColorBalance::slotSettingsChanged()
{
    BatchToolSettings prm;
    CBContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("Red"), (double)currentPrm.red);
    prm.insert(QLatin1String("Green"), (double)currentPrm.green);
    prm.insert(QLatin1String("Blue"), (double)currentPrm.blue);

    BatchTool::slotSettingsChanged(prm);
}

bool ColorBalance::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    CBContainer prm;
    prm.red   = settings().value(QLatin1String("Red")).toDouble();
    prm.green = settings().value(QLatin1String("Green")).toDouble();
    prm.blue  = settings().value(QLatin1String("Blue")).toDouble();

    CBFilter cb(&image(), nullptr, prm);
    applyFilter(&cb);

    return (savefromDImg());
}

} // namespace DigikamBqmColorBalancePlugin

#include "moc_colorbalance.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-27
 * Description : White Balance batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "whitebalance.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "wbfilter.h"

namespace DigikamBqmWhiteBalancePlugin
{

WhiteBalance::WhiteBalance(QObject* const parent)
    : BatchTool(QLatin1String("WhiteBalance"), ColorTool, parent)
{
}

BatchTool* WhiteBalance::clone(QObject* const parent) const
{
    return new WhiteBalance(parent);
}

void WhiteBalance::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new WBSettings(m_settingsWidget);
    m_settingsView->showAdvancedButtons(false);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings WhiteBalance::defaultSettings()
{
    BatchToolSettings prm;
    WBContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("black"),          (double)defaultPrm.black);
    prm.insert(QLatin1String("temperature"),    (double)defaultPrm.temperature);
    prm.insert(QLatin1String("green"),          (double)defaultPrm.green);
    prm.insert(QLatin1String("dark"),           (double)defaultPrm.dark);
    prm.insert(QLatin1String("gamma"),          (double)defaultPrm.gamma);
    prm.insert(QLatin1String("saturation"),     (double)defaultPrm.saturation);
    prm.insert(QLatin1String("expositionMain"), (double)defaultPrm.expositionMain);
    prm.insert(QLatin1String("expositionFine"), (double)defaultPrm.expositionFine);

    return prm;
}

void WhiteBalance::slotAssignSettings2Widget()
{
    WBContainer prm;

    prm.black          = settings().value(QLatin1String("black")).toDouble();
    prm.temperature    = settings().value(QLatin1String("temperature")).toDouble();
    prm.green          = settings().value(QLatin1String("green")).toDouble();
    prm.dark           = settings().value(QLatin1String("dark")).toDouble();
    prm.gamma          = settings().value(QLatin1String("gamma")).toDouble();
    prm.saturation     = settings().value(QLatin1String("saturation")).toDouble();
    prm.expositionMain = settings().value(QLatin1String("expositionMain")).toDouble();
    prm.expositionFine = settings().value(QLatin1String("expositionFine")).toDouble();

    m_settingsView->setSettings(prm);
}

void WhiteBalance::slotSettingsChanged()
{
    BatchToolSettings prm;
    WBContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("black"),          (double)currentPrm.black);
    prm.insert(QLatin1String("temperature"),    (double)currentPrm.temperature);
    prm.insert(QLatin1String("green"),          (double)currentPrm.green);
    prm.insert(QLatin1String("dark"),           (double)currentPrm.dark);
    prm.insert(QLatin1String("gamma"),          (double)currentPrm.gamma);
    prm.insert(QLatin1String("saturation"),     (double)currentPrm.saturation);
    prm.insert(QLatin1String("expositionMain"), (double)currentPrm.expositionMain);
    prm.insert(QLatin1String("expositionFine"), (double)currentPrm.expositionFine);

    BatchTool::slotSettingsChanged(prm);
}

bool WhiteBalance::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    WBContainer prm;

    prm.black          = settings().value(QLatin1String("black")).toDouble();
    prm.temperature    = settings().value(QLatin1String("temperature")).toDouble();
    prm.green          = settings().value(QLatin1String("green")).toDouble();
    prm.dark           = settings().value(QLatin1String("dark")).toDouble();
    prm.gamma          = settings().value(QLatin1String("gamma")).toDouble();
    prm.saturation     = settings().value(QLatin1String("saturation")).toDouble();
    prm.expositionMain = settings().value(QLatin1String("expositionMain")).toDouble();
    prm.expositionFine = settings().value(QLatin1String("expositionFine")).toDouble();

    WBFilter wb(&image(), nullptr, prm);
    applyFilter(&wb);

    return (savefromDImg());
}

} // namespace DigikamBqmWhiteBalancePlugin

#include "moc_whitebalance.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Brightness/Contrast/Gamma batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bcgcorrection.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "bcgfilter.h"

namespace DigikamBqmBCGCorrectionPlugin
{

BCGCorrection::BCGCorrection(QObject* const parent)
    : BatchTool(QLatin1String("BCGCorrection"), ColorTool, parent)
{
}

BatchTool* BCGCorrection::clone(QObject* const parent) const
{
    return new BCGCorrection(parent);
}

void BCGCorrection::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new BCGSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings BCGCorrection::defaultSettings()
{
    BatchToolSettings prm;
    BCGContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("Brightness"), (double)defaultPrm.brightness);
    prm.insert(QLatin1String("Contrast"),   (double)defaultPrm.contrast);
    prm.insert(QLatin1String("Gamma"),      (double)defaultPrm.gamma);

    return prm;
}

void BCGCorrection::slotAssignSettings2Widget()
{
    BCGContainer prm;
    prm.brightness = settings().value(QLatin1String("Brightness")).toDouble();
    prm.contrast   = settings().value(QLatin1String("Contrast")).toDouble();
    prm.gamma      = settings().value(QLatin1String("Gamma")).toDouble();
    m_settingsView->setSettings(prm);
}

void BCGCorrection::slotSettingsChanged()
{
    BatchToolSettings prm;
    BCGContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("Brightness"), (double)currentPrm.brightness);
    prm.insert(QLatin1String("Contrast"),   (double)currentPrm.contrast);
    prm.insert(QLatin1String("Gamma"),      (double)currentPrm.gamma);

    BatchTool::slotSettingsChanged(prm);
}

bool BCGCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BCGContainer prm;
    prm.brightness = settings().value(QLatin1String("Brightness")).toDouble();
    prm.contrast   = settings().value(QLatin1String("Contrast")).toDouble();
    prm.gamma      = settings().value(QLatin1String("Gamma")).toDouble();

    BCGFilter bcg(&image(), nullptr, prm);
    applyFilter(&bcg);

    return (savefromDImg());
}

} // namespace DigikamBqmBCGCorrectionPlugin

#include "moc_bcgcorrection.cpp"

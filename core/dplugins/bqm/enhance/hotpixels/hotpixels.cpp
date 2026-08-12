/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-07
 * Description : Hot Pixels Fixer batch tool.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hotpixels.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "hotpixelcontainer.h"
#include "hotpixelfixer.h"
#include "hotpixelprops.h"

namespace DigikamBqmHotPixelsPlugin
{

HotPixels::HotPixels(QObject* const parent)
    : BatchTool(QLatin1String("HotPixels"), EnhanceTool, parent)
{
}

BatchTool* HotPixels::clone(QObject* const parent) const
{
    return new HotPixels(parent);
}

void HotPixels::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new HotPixelSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings HotPixels::defaultSettings()
{
    BatchToolSettings prm;
    HotPixelContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("BlackFrameUrl"), defaultPrm.blackFrameUrl);
    prm.insert(QLatin1String("HotPixelsList"), HotPixelProps::toStringList(defaultPrm.hotPixelsList));
    prm.insert(QLatin1String("FilterMethod"),  (int)defaultPrm.filterMethod);

    return prm;
}

void HotPixels::slotAssignSettings2Widget()
{
    HotPixelContainer prm;
    prm.blackFrameUrl = settings().value(QLatin1String("BlackFrameUrl")).toUrl();
    prm.hotPixelsList = HotPixelProps::fromStringList(settings().value(QLatin1String("HotPixelsList")).toStringList());
    prm.filterMethod  = (HotPixelContainer::InterpolationMethod)settings().value(QLatin1String("FilterMethod")).toInt();
    m_settingsView->setSettings(prm);
}

void HotPixels::slotSettingsChanged()
{
    BatchToolSettings prm;
    HotPixelContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("BlackFrameUrl"), currentPrm.blackFrameUrl);
    prm.insert(QLatin1String("HotPixelsList"), HotPixelProps::toStringList(currentPrm.hotPixelsList));
    prm.insert(QLatin1String("FilterMethod"),  (int)currentPrm.filterMethod);

    BatchTool::slotSettingsChanged(prm);
}

bool HotPixels::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    HotPixelContainer prm;

    prm.blackFrameUrl = settings().value(QLatin1String("BlackFrameUrl")).toUrl();
    prm.hotPixelsList = HotPixelProps::fromStringList(settings().value(QLatin1String("HotPixelsList")).toStringList());
    prm.filterMethod  = (HotPixelContainer::InterpolationMethod)settings().value(QLatin1String("FilterMethod")).toInt();

    HotPixelFixer hpf(&image(), nullptr, prm);
    applyFilter(&hpf);

    return (savefromDImg());
}

} // namespace DigikamBqmHotPixelsPlugin

#include "moc_hotpixels.cpp"

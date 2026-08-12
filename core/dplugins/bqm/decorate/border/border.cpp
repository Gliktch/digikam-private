/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-17
 * Description : batch tool to add border.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "border.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "borderfilter.h"

namespace DigikamBqmBorderPlugin
{

Border::Border(QObject* const parent)
    : BatchTool(QLatin1String("Border"), DecorateTool, parent)
{
}

BatchTool* Border::clone(QObject* const parent) const
{
    return new Border(parent);
}

void Border::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new BorderSettings(m_settingsWidget);
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Border::defaultSettings()
{
    BatchToolSettings prm;
    BorderContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("preserveAspectRatio"),   defaultPrm.preserveAspectRatio);
    prm.insert(QLatin1String("orgWidth"),              defaultPrm.orgWidth);
    prm.insert(QLatin1String("orgHeight"),             defaultPrm.orgHeight);
    prm.insert(QLatin1String("borderType"),            defaultPrm.borderType);
    prm.insert(QLatin1String("borderWidth1"),          defaultPrm.borderWidth1);
    prm.insert(QLatin1String("borderWidth2"),          defaultPrm.borderWidth2);
    prm.insert(QLatin1String("borderWidth3"),          defaultPrm.borderWidth3);
    prm.insert(QLatin1String("borderWidth4"),          defaultPrm.borderWidth4);
    prm.insert(QLatin1String("borderPercent"),         defaultPrm.borderPercent);
    prm.insert(QLatin1String("borderPath"),            defaultPrm.borderPath);
    prm.insert(QLatin1String("solidColor"),            defaultPrm.solidColor);
    prm.insert(QLatin1String("niepceBorderColor"),     defaultPrm.niepceBorderColor);
    prm.insert(QLatin1String("niepceLineColor"),       defaultPrm.niepceLineColor);
    prm.insert(QLatin1String("bevelUpperLeftColor"),   defaultPrm.bevelUpperLeftColor);
    prm.insert(QLatin1String("bevelLowerRightColor"),  defaultPrm.bevelLowerRightColor);
    prm.insert(QLatin1String("decorativeFirstColor"),  defaultPrm.decorativeFirstColor);
    prm.insert(QLatin1String("decorativeSecondColor"), defaultPrm.decorativeSecondColor);

    return prm;
}

void Border::slotAssignSettings2Widget()
{
    BorderContainer prm;

    prm.preserveAspectRatio   = settings().value(QLatin1String("preserveAspectRatio")).toBool();
    prm.borderType            = settings().value(QLatin1String("borderType")).toInt();
    prm.borderWidth1          = settings().value(QLatin1String("borderWidth1")).toInt();
    prm.borderWidth2          = settings().value(QLatin1String("borderWidth2")).toInt();
    prm.borderWidth3          = settings().value(QLatin1String("borderWidth3")).toInt();
    prm.borderWidth4          = settings().value(QLatin1String("borderWidth4")).toInt();
    prm.borderPercent         = settings().value(QLatin1String("borderPercent")).toDouble();
    prm.borderPath            = settings().value(QLatin1String("borderPath")).toString();
    prm.solidColor            = settings().value(QLatin1String("solidColor")).value<QColor>();
    prm.niepceBorderColor     = settings().value(QLatin1String("niepceBorderColor")).value<QColor>();
    prm.niepceLineColor       = settings().value(QLatin1String("niepceLineColor")).value<QColor>();
    prm.bevelUpperLeftColor   = settings().value(QLatin1String("bevelUpperLeftColor")).value<QColor>();
    prm.bevelLowerRightColor  = settings().value(QLatin1String("bevelLowerRightColor")).value<QColor>();
    prm.decorativeFirstColor  = settings().value(QLatin1String("decorativeFirstColor")).value<QColor>();
    prm.decorativeSecondColor = settings().value(QLatin1String("decorativeSecondColor")).value<QColor>();

    m_settingsView->setSettings(prm);
}

void Border::slotSettingsChanged()
{
    BatchToolSettings prm;
    BorderContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("preserveAspectRatio"),   currentPrm.preserveAspectRatio);
    prm.insert(QLatin1String("borderType"),            currentPrm.borderType);
    prm.insert(QLatin1String("borderWidth1"),          currentPrm.borderWidth1);
    prm.insert(QLatin1String("borderWidth2"),          currentPrm.borderWidth2);
    prm.insert(QLatin1String("borderWidth3"),          currentPrm.borderWidth3);
    prm.insert(QLatin1String("borderWidth4"),          currentPrm.borderWidth4);
    prm.insert(QLatin1String("borderPercent"),         currentPrm.borderPercent);
    prm.insert(QLatin1String("borderPath"),            currentPrm.borderPath);
    prm.insert(QLatin1String("solidColor"),            currentPrm.solidColor);
    prm.insert(QLatin1String("niepceBorderColor"),     currentPrm.niepceBorderColor);
    prm.insert(QLatin1String("niepceLineColor"),       currentPrm.niepceLineColor);
    prm.insert(QLatin1String("bevelUpperLeftColor"),   currentPrm.bevelUpperLeftColor);
    prm.insert(QLatin1String("bevelLowerRightColor"),  currentPrm.bevelLowerRightColor);
    prm.insert(QLatin1String("decorativeFirstColor"),  currentPrm.decorativeFirstColor);
    prm.insert(QLatin1String("decorativeSecondColor"), currentPrm.decorativeSecondColor);

    BatchTool::slotSettingsChanged(prm);
}

bool Border::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BorderContainer prm;
    prm.preserveAspectRatio   = settings().value(QLatin1String("preserveAspectRatio")).toBool();
    prm.borderType            = settings().value(QLatin1String("borderType")).toInt();
    prm.borderWidth1          = settings().value(QLatin1String("borderWidth1")).toInt();
    prm.borderWidth2          = settings().value(QLatin1String("borderWidth2")).toInt();
    prm.borderWidth3          = settings().value(QLatin1String("borderWidth3")).toInt();
    prm.borderWidth4          = settings().value(QLatin1String("borderWidth4")).toInt();
    prm.borderPercent         = settings().value(QLatin1String("borderPercent")).toDouble();
    prm.borderPath            = settings().value(QLatin1String("borderPath")).toString();
    prm.solidColor            = settings().value(QLatin1String("solidColor")).value<QColor>();
    prm.niepceBorderColor     = settings().value(QLatin1String("niepceBorderColor")).value<QColor>();
    prm.niepceLineColor       = settings().value(QLatin1String("niepceLineColor")).value<QColor>();
    prm.bevelUpperLeftColor   = settings().value(QLatin1String("bevelUpperLeftColor")).value<QColor>();
    prm.bevelLowerRightColor  = settings().value(QLatin1String("bevelLowerRightColor")).value<QColor>();
    prm.decorativeFirstColor  = settings().value(QLatin1String("decorativeFirstColor")).value<QColor>();
    prm.decorativeSecondColor = settings().value(QLatin1String("decorativeSecondColor")).value<QColor>();
    prm.orgWidth              = image().width();
    prm.orgHeight             = image().height();

    BorderFilter bd(&image(), nullptr, prm);
    applyFilterChangedProperties(&bd);

    return (savefromDImg());
}

} // namespace DigikamBqmBorderPlugin

#include "moc_border.cpp"

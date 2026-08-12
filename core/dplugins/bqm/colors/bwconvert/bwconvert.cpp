/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-23
 * Description : Black and White conversion batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bwconvert.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "bwsepiafilter.h"

namespace DigikamBqmBWConvertPlugin
{

BWConvert::BWConvert(QObject* const parent)
    : BatchTool(QLatin1String("BWConvert"), ColorTool, parent)
{
}

BatchTool* BWConvert::clone(QObject* const parent) const
{
    return new BWConvert(parent);
}

void BWConvert::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new BWSepiaSettings(m_settingsWidget, &m_preview);
    m_settingsView->startPreviewFilters();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

void BWConvert::slotResetSettingsToDefault()
{
    // We need to call this method there to reset all curves points.
    // Curves values are cleaned with default settings passed after.

    m_settingsView->resetToDefault();
    BatchTool::slotResetSettingsToDefault();
}

BatchToolSettings BWConvert::defaultSettings()
{
    BatchToolSettings prm;
    BWSepiaContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("filmType"),   (int)defaultPrm.filmType);
    prm.insert(QLatin1String("filterType"), (int)defaultPrm.filterType);
    prm.insert(QLatin1String("toneType"),   (int)defaultPrm.toneType);
    prm.insert(QLatin1String("contrast"),   (double)defaultPrm.bcgPrm.contrast);
    prm.insert(QLatin1String("strength"),   (double)defaultPrm.strength);
    prm.insert(QLatin1String("curvesType"), defaultPrm.curvesPrm.curvesType);
    prm.insert(QLatin1String("curves"),     defaultPrm.curvesPrm.values[LuminosityChannel]);

    return prm;
}

void BWConvert::slotAssignSettings2Widget()
{
    BWSepiaContainer prm;

    prm.filmType                            = settings().value(QLatin1String("filmType")).toInt();
    prm.filterType                          = settings().value(QLatin1String("filterType")).toInt();
    prm.toneType                            = settings().value(QLatin1String("toneType")).toInt();
    prm.bcgPrm.contrast                     = settings().value(QLatin1String("contrast")).toDouble();
    prm.strength                            = settings().value(QLatin1String("strength")).toDouble();
    prm.curvesPrm.curvesType                = (ImageCurves::CurveType)settings().value(QLatin1String("curvesType")).toInt();
    prm.curvesPrm.values[LuminosityChannel] = settings().value(QLatin1String("curves")).value<QPolygon>();

    m_settingsView->setSettings(prm);
}

void BWConvert::slotSettingsChanged()
{
    BatchToolSettings prm;
    BWSepiaContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("filmType"),   (int)currentPrm.filmType);
    prm.insert(QLatin1String("filterType"), (int)currentPrm.filterType);
    prm.insert(QLatin1String("toneType"),   (int)currentPrm.toneType);
    prm.insert(QLatin1String("contrast"),   (double)currentPrm.bcgPrm.contrast);
    prm.insert(QLatin1String("strength"),   (double)currentPrm.strength);
    prm.insert(QLatin1String("curvesType"), (int)currentPrm.curvesPrm.curvesType);
    prm.insert(QLatin1String("curves"),     currentPrm.curvesPrm.values[LuminosityChannel]);

    BatchTool::slotSettingsChanged(prm);
}

bool BWConvert::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BWSepiaContainer prm;

    prm.filmType                     = settings().value(QLatin1String("filmType")).toInt();
    prm.filterType                   = settings().value(QLatin1String("filterType")).toInt();
    prm.toneType                     = settings().value(QLatin1String("toneType")).toInt();
    prm.bcgPrm.contrast              = settings().value(QLatin1String("contrast")).toDouble();
    prm.strength                     = settings().value(QLatin1String("strength")).toDouble();

    CurvesContainer curves((ImageCurves::CurveType)settings().value(QLatin1String("curvesType")).toInt(), true);
    curves.initialize();
    curves.values[LuminosityChannel] = settings().value(QLatin1String("curves")).value<QPolygon>();
    prm.curvesPrm                    = curves;

    BWSepiaFilter bw(&image(), nullptr, prm);
    applyFilter(&bw);

    return (savefromDImg());
}

} // namespace DigikamBqmBWConvertPlugin

#include "moc_bwconvert.cpp"

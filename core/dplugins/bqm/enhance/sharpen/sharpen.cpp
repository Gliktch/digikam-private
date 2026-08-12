/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : sharpen image batch tool.
 *
 * SPDX-FileCopyrightText: 2009      by Matthias Welwarsky <matze at welwarsky dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sharpen.h"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "dimg.h"
#include "sharpenfilter.h"
#include "unsharpmaskfilter.h"

#ifdef HAVE_EIGEN3
#   include "refocusfilter.h"
#endif // HAVE_EIGEN3

namespace DigikamBqmSharpenPlugin
{

Sharpen::Sharpen(QObject* const parent)
    : BatchTool(QLatin1String("Sharpen"), EnhanceTool, parent)
{
}

BatchTool* Sharpen::clone(QObject* const parent) const
{
    return new Sharpen(parent);
}

void Sharpen::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new SharpSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Sharpen::defaultSettings()
{
    BatchToolSettings settings;
    SharpContainer defaultPrm = m_settingsView->defaultSettings();

    // sharpen method

    settings.insert(QLatin1String("SharpenFilterType"),    (int)defaultPrm.method);

    // simple sharp

    settings.insert(QLatin1String("SimpleSharpRadius"),    (int)defaultPrm.ssRadius);

    // unsharp mask

    settings.insert(QLatin1String("UnsharpMaskRadius"),    (double)defaultPrm.umRadius);
    settings.insert(QLatin1String("UnsharpMaskAmount"),    (double)defaultPrm.umAmount);
    settings.insert(QLatin1String("UnsharpMaskThreshold"), (double)defaultPrm.umThreshold);
    settings.insert(QLatin1String("UnsharpMaskLuma"),      (bool)defaultPrm.umLumaOnly);

#ifdef HAVE_EIGEN3

    // refocus

    settings.insert(QLatin1String("RefocusRadius"),        (double)defaultPrm.rfRadius);
    settings.insert(QLatin1String("RefocusCorrelation"),   (double)defaultPrm.rfCorrelation);
    settings.insert(QLatin1String("RefocusNoise"),         (double)defaultPrm.rfNoise);
    settings.insert(QLatin1String("RefocusGauss"),         (double)defaultPrm.rfGauss);
    settings.insert(QLatin1String("RefocusMatrixSize"),    (int)defaultPrm.rfMatrix);

#endif // HAVE_EIGEN3

    return settings;
}

void Sharpen::slotAssignSettings2Widget()
{
    SharpContainer prm;

    // sharpen method

    prm.method        = settings().value(QLatin1String("SharpenFilterType")).toInt();

    // simple sharp

    prm.ssRadius      = settings().value(QLatin1String("SimpleSharpRadius")).toInt();

    // unsharp mask

    prm.umRadius      = settings().value(QLatin1String("UnsharpMaskRadius")).toDouble();
    prm.umAmount      = settings().value(QLatin1String("UnsharpMaskAmount")).toDouble();
    prm.umThreshold   = settings().value(QLatin1String("UnsharpMaskThreshold")).toDouble();
    prm.umLumaOnly    = settings().value(QLatin1String("UnsharpMaskLuma")).toBool();

#ifdef HAVE_EIGEN3

    // refocus

    prm.rfRadius      = settings().value(QLatin1String("RefocusRadius")).toDouble();
    prm.rfCorrelation = settings().value(QLatin1String("RefocusCorrelation")).toDouble();
    prm.rfNoise       = settings().value(QLatin1String("RefocusNoise")).toDouble();
    prm.rfGauss       = settings().value(QLatin1String("RefocusGauss")).toDouble();
    prm.rfMatrix      = settings().value(QLatin1String("RefocusMatrixSize")).toInt();

#endif // HAVE_EIGEN3

    m_settingsView->setSettings(prm);
}

void Sharpen::slotSettingsChanged()
{
    BatchToolSettings settings;
    SharpContainer prm = m_settingsView->settings();

    // sharpen method

    settings.insert(QLatin1String("SharpenFilterType"),    (int)prm.method);

    // simple sharp

    settings.insert(QLatin1String("SimpleSharpRadius"),    (int)prm.ssRadius);

    // unsharp mask

    settings.insert(QLatin1String("UnsharpMaskRadius"),    (double)prm.umRadius);
    settings.insert(QLatin1String("UnsharpMaskAmount"),    (double)prm.umAmount);
    settings.insert(QLatin1String("UnsharpMaskThreshold"), (double)prm.umThreshold);
    settings.insert(QLatin1String("UnsharpMaskLuma"),      (bool)prm.umLumaOnly);

#ifdef HAVE_EIGEN3

    // refocus

    settings.insert(QLatin1String("RefocusRadius"),        (double)prm.rfRadius);
    settings.insert(QLatin1String("RefocusCorrelation"),   (double)prm.rfCorrelation);
    settings.insert(QLatin1String("RefocusNoise"),         (double)prm.rfNoise);
    settings.insert(QLatin1String("RefocusGauss"),         (double)prm.rfGauss);
    settings.insert(QLatin1String("RefocusMatrixSize"),    (int)prm.rfMatrix);

#endif // HAVE_EIGEN3

    BatchTool::slotSettingsChanged(settings);
}

bool Sharpen::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int filterType  = settings().value(QLatin1String("SharpenFilterType")).toInt();

    switch (filterType)
    {
        case SharpContainer::SimpleSharp:
        {
            double radius = settings().value(QLatin1String("SimpleSharpRadius")).toInt() / 10.0;
            double sigma;

            if (radius < 1.0)
            {
                sigma = radius;
            }
            else
            {
                sigma = sqrt(radius);
            }

            SharpenFilter filter(&image(), nullptr, radius, sigma);
            applyFilter(&filter);
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            double r  = settings().value(QLatin1String("UnsharpMaskRadius")).toDouble();
            double a  = settings().value(QLatin1String("UnsharpMaskAmount")).toDouble();
            double th = settings().value(QLatin1String("UnsharpMaskThreshold")).toDouble();
            bool    l = settings().value(QLatin1String("UnsharpMaskLuma")).toBool();

            UnsharpMaskFilter filter(&image(), nullptr, r, a, th, l);
            applyFilter(&filter);
            break;
        }

        case SharpContainer::Refocus:
        {

#ifdef HAVE_EIGEN3

            double radius      = settings().value(QLatin1String("RefocusRadius")).toDouble();
            double correlation = settings().value(QLatin1String("RefocusCorrelation")).toDouble();
            double noise       = settings().value(QLatin1String("RefocusNoise")).toDouble();
            double gauss       = settings().value(QLatin1String("RefocusGauss")).toDouble();
            int matrixSize     = settings().value(QLatin1String("RefocusMatrixSize")).toInt();

            RefocusFilter filter(&image(), nullptr, matrixSize, radius, gauss, correlation, noise);
            applyFilter(&filter);

#endif // HAVE_EIGEN3

            break;
        }
    }

    return savefromDImg();
}

} // namespace DigikamBqmSharpenPlugin

#include "moc_sharpen.cpp"

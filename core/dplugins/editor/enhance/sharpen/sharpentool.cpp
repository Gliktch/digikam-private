/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sharpentool.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "sharpsettings.h"
#include "sharpenfilter.h"
#include "unsharpmaskfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

#ifdef HAVE_EIGEN3
#   include "refocusfilter.h"
#endif // HAVE_EIGEN3

namespace DigikamEditorSharpenToolPlugin
{

class Q_DECL_HIDDEN SharpenTool::Private
{

public:

    Private() = default;

    const QString       configGroupName = QLatin1String("sharpen Tool");

    SharpSettings*      sharpSettings   = nullptr;
    ImageRegionWidget*  previewWidget   = nullptr;
    EditorToolSettings* gboxSettings    = nullptr;
};

SharpenTool::SharpenTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("sharpen"));
    setToolHelp(QLatin1String("blursharpentool.anchor"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    d->previewWidget = new ImageRegionWidget;

    d->sharpSettings = new SharpSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    connect(d->sharpSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

SharpenTool::~SharpenTool()
{
    delete d;
}

void SharpenTool::slotSettingsChanged()
{
    switch (d->sharpSettings->settings().method)
    {
        case SharpContainer::SimpleSharp:
        {
            d->gboxSettings->enableButton(EditorToolSettings::Load, false);
            d->gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            d->gboxSettings->enableButton(EditorToolSettings::Load, false);
            d->gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case SharpContainer::Refocus:
        {
            break;
        }
    }
}

void SharpenTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->sharpSettings->readSettings(group);
}

void SharpenTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->sharpSettings->writeSettings(group);
    group.sync();
}

void SharpenTool::slotResetSettings()
{
    d->sharpSettings->resetToDefault();
}

void SharpenTool::preparePreview()
{
    SharpContainer settings = d->sharpSettings->settings();

    switch (settings.method)
    {
        case SharpContainer::SimpleSharp:
        {
            DImg img      = d->previewWidget->getOriginalRegionImage();
            double radius = settings.ssRadius/10.0;
            double sigma;

            if (radius < 1.0)
            {
                sigma = radius;
            }
            else
            {
                sigma = sqrt(radius);
            }

            setFilter(new SharpenFilter(&img, this, radius, sigma));
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            DImg img  = d->previewWidget->getOriginalRegionImage();
            double r  = settings.umRadius;
            double a  = settings.umAmount;
            double th = settings.umThreshold;
            bool l    = settings.umLumaOnly;

            setFilter(new UnsharpMaskFilter(&img, this, r, a, th, l));
            break;
        }

        case SharpContainer::Refocus:
        {

#ifdef HAVE_EIGEN3

            DImg   img = d->previewWidget->getOriginalRegionImage();
            double r   = settings.rfRadius;
            double c   = settings.rfCorrelation;
            double n   = settings.rfNoise;
            double g   = settings.rfGauss;
            int    ms  = settings.rfMatrix;

            setFilter(new RefocusFilter(&img, this, ms, r, g, c, n));

#endif // HAVE_EIGEN3

            break;
        }
    }
}

void SharpenTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);
}

void SharpenTool::prepareFinal()
{
    ImageIface iface;
    SharpContainer settings = d->sharpSettings->settings();

    switch (settings.method)
    {
        case SharpContainer::SimpleSharp:
        {
            double radius = settings.ssRadius/10.0;
            double sigma;

            if (radius < 1.0)
            {
                sigma = radius;
            }
            else
            {
                sigma = sqrt(radius);
            }

            setFilter(new SharpenFilter(iface.original(), this, radius, sigma));
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            double r  = settings.umRadius;
            double a  = settings.umAmount;
            double th = settings.umThreshold;

            setFilter(new UnsharpMaskFilter(iface.original(), this, r, a, th));
            break;
        }

        case SharpContainer::Refocus:
        {

#ifdef HAVE_EIGEN3

            double r   = settings.rfRadius;
            double c   = settings.rfCorrelation;
            double n   = settings.rfNoise;
            double g   = settings.rfGauss;
            int    ms  = settings.rfMatrix;

            setFilter(new RefocusFilter(iface.original(), this, ms, r, g, c, n));

#endif // HAVE_EIGEN3

            break;
        }
    }
}

void SharpenTool::setFinalImage()
{
    ImageIface iface;
    SharpContainer settings = d->sharpSettings->settings();

    switch (settings.method)
    {
        case SharpContainer::SimpleSharp:
        {
            iface.setOriginal(i18n("Sharpen"), filter()->filterAction(), filter()->getTargetImage());
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            iface.setOriginal(i18n("Unsharp Mask"), filter()->filterAction(), filter()->getTargetImage());
            break;
        }

        case SharpContainer::Refocus:
        {

#ifdef HAVE_EIGEN3

            iface.setOriginal(i18n("Refocus"), filter()->filterAction(), filter()->getTargetImage());

#endif // HAVE_EIGEN3

            break;
        }
    }
}

void SharpenTool::renderingFinished()
{
    slotSettingsChanged();
}

void SharpenTool::slotLoadSettings()
{
    d->sharpSettings->loadSettings();
}

void SharpenTool::slotSaveAsSettings()
{
    d->sharpSettings->saveAsSettings();
}

} // namespace DigikamEditorSharpenToolPlugin

#include "moc_sharpentool.cpp"

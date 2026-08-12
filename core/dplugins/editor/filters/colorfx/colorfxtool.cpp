/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image tool for to apply a color
 *               effect to an image.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "colorfxtool.h"

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>
#include <QIcon>
#include <QMenu>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "colorfxfilter.h"
#include "colorfxsettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorColorFxToolPlugin
{

class Q_DECL_HIDDEN ColorFxTool::Private
{

public:

    Private() = default;

    const QString configGroupName               = QLatin1String("coloreffect Tool");
    const QString configHistogramChannelEntry   = QLatin1String("Histogram Channel");
    const QString configHistogramScaleEntry     = QLatin1String("Histogram Scale");

    ImageRegionWidget*  previewWidget           = nullptr;
    EditorToolSettings* gboxSettings            = nullptr;
    ColorFXSettings*    settingsView            = nullptr;
};

// --------------------------------------------------------

ColorFxTool::ColorFxTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("coloreffects"));

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    d->previewWidget->setWhatsThis(i18nc("@info", "This is the color effects preview"));
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);

    // -------------------------------------------------------------

    d->settingsView = new ColorFXSettings(d->gboxSettings->plainPage(), false);
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

ColorFxTool::~ColorFxTool()
{
    delete d;
}

void ColorFxTool::slotInit()
{
    EditorToolThreaded::slotInit();
    d->settingsView->startPreviewFilters();
}

void ColorFxTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->settingsView->readSettings(group);
}

void ColorFxTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry,    (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,      (int)d->gboxSettings->histogramBox()->scale());

    d->settingsView->writeSettings(group);

    group.sync();
}

void ColorFxTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void ColorFxTool::slotColorSelectedFromTarget(const Digikam::DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ColorFxTool::preparePreview()
{
    ColorFXContainer prm    = d->settingsView->settings();
    bool useDownscaledImage = true;

    // See bug #237719 : we cannot use downscaled image to render preview.

    if (
        prm.colorFXType == ColorFXFilter::Neon ||
        prm.colorFXType == ColorFXFilter::FindEdges
       )
    {
        useDownscaledImage = false;
    }

    DImg preview = d->previewWidget->getOriginalRegionImage(useDownscaledImage);

    setFilter(new ColorFXFilter(&preview, this, prm));
}

void ColorFxTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview, DImg(), false);
}

void ColorFxTool::prepareFinal()
{
    ColorFXContainer prm = d->settingsView->settings();

    ImageIface iface;

    setFilter(new ColorFXFilter(iface.original(), this, prm));
}

void ColorFxTool::setFinalImage()
{
    ImageIface iface;

    QString name;

    switch (d->settingsView->settings().colorFXType)
    {
        case ColorFXFilter::Solarize:
        {
            name = i18nc("@title: filter", "Solarize");
            break;
        }

        case ColorFXFilter::Vivid:
        {
            name = i18nc("@title: filter", "Vivid");
            break;
        }

        case ColorFXFilter::Neon:
        {
            name = i18nc("@title: filter", "Neon");
            break;
        }

        case ColorFXFilter::FindEdges:
        {
            name = i18nc("@title: filter", "Find Edges");
            break;
        }

        case ColorFXFilter::Lut3D:
        {
            name = i18nc("@title: filter", "Lut3D");
            break;
        }
    }

    iface.setOriginal(name, filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorColorFxToolPlugin

#include "moc_colorfxtool.cpp"

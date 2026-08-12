/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-16
 * Description : digiKam image editor to adjust Hue, Saturation,
 *               and Lightness of picture.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "hsltool.h"

// Qt includes

#include <QLabel>
#include <QColorDialog>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dimg.h"
#include "hslsettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorHSLToolPlugin
{

class Q_DECL_HIDDEN HSLTool::Private
{
public:

    Private() = default;

    const QString configGroupName               = QLatin1String("hsladjust Tool");
    const QString configHistogramChannelEntry   = QLatin1String("Histogram Channel");
    const QString configHistogramScaleEntry     = QLatin1String("Histogram Scale");

    HSLSettings*         hslSettings            = nullptr;
    ImageRegionWidget*   previewWidget          = nullptr;
    EditorToolSettings*  gboxSettings           = nullptr;
};

// --------------------------------------------------------

HSLTool::HSLTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("adjusthsl"));
    setToolHelp(QLatin1String("hsladjusttool.anchor"));
    setInitPreview(true);

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    d->hslSettings = new HSLSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->hslSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

HSLTool::~HSLTool()
{
    delete d;
}

void HSLTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->hslSettings->readSettings(group);
}

void HSLTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->hslSettings->writeSettings(group);

    config->sync();
}

void HSLTool::slotResetSettings()
{
    d->hslSettings->resetToDefault();
    slotPreview();
}

void HSLTool::preparePreview()
{
    HSLContainer settings = d->hslSettings->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new HSLFilter(&preview, this, settings));
}

void HSLTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void HSLTool::prepareFinal()
{
    HSLContainer settings = d->hslSettings->settings();

    ImageIface iface;
    setFilter(new HSLFilter(iface.original(), this, settings));
}

void HSLTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("HSL Adjustments"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorHSLToolPlugin

#include "moc_hsltool.cpp"

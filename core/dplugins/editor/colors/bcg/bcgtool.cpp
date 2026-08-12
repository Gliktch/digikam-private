/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-05
 * Description : digiKam image editor to adjust Brightness,
 *               Contrast, and Gamma of picture.
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bcgtool.h"

// Qt includes

#include <QLabel>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dnuminput.h"
#include "dimg.h"
#include "bcgsettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorBCGToolPlugin
{

class Q_DECL_HIDDEN BCGTool::Private
{
public:

    Private() = default;

    const QString configGroupName               = QLatin1String("bcgadjust Tool");
    const QString configHistogramChannelEntry   = QLatin1String("Histogram Channel");
    const QString configHistogramScaleEntry     = QLatin1String("Histogram Scale");

    BCGSettings*         settingsView           = nullptr;
    ImageRegionWidget*   previewWidget          = nullptr;
    EditorToolSettings*  gboxSettings           = nullptr;
};


// --------------------------------------------------------

BCGTool::BCGTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("bcgadjust"));
    setToolVersion(1);
    setToolHelp(QLatin1String("bcgadjusttool.anchor"));
    setToolCategory(FilterAction::ReproducibleFilter);
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

    d->settingsView = new BCGSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

BCGTool::~BCGTool()
{
    delete d;
}

void BCGTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->settingsView->readSettings(group);
}

void BCGTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->settingsView->writeSettings(group);

    config->sync();
}

void BCGTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void BCGTool::preparePreview()
{
    BCGContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new BCGFilter(&preview, this, settings));
}

void BCGTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void BCGTool::prepareFinal()
{
    BCGContainer settings = d->settingsView->settings();

    ImageIface iface;

    setFilter(new BCGFilter(iface.original(), this, settings));
}

void BCGTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Brightness / Contrast / Gamma"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorBCGToolPlugin

#include "moc_bcgtool.cpp"

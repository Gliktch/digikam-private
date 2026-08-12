/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cbtool.h"

// Qt includes

#include <QLabel>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "cbsettings.h"
#include "cbfilter.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorColorBalanceToolPlugin
{

class Q_DECL_HIDDEN CBTool::Private
{
public:

    Private() = default;

    const QString configGroupName               = QLatin1String("colorbalance Tool");
    const QString configHistogramChannelEntry   = QLatin1String("Histogram Channel");
    const QString configHistogramScaleEntry     = QLatin1String("Histogram Scale");

    CBSettings*          cbSettings             = nullptr;
    ImageRegionWidget*   previewWidget          = nullptr;
    EditorToolSettings*  gboxSettings           = nullptr;
};

// --------------------------------------------------------

CBTool::CBTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("colorbalance"));
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

    d->cbSettings = new CBSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->cbSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

CBTool::~CBTool()
{
    delete d;
}

void CBTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));

    d->cbSettings->readSettings(group);
}

void CBTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->cbSettings->writeSettings(group);

    group.sync();
}

void CBTool::slotResetSettings()
{
    d->cbSettings->resetToDefault();
    slotPreview();
}

void CBTool::preparePreview()
{
    CBContainer settings = d->cbSettings->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new CBFilter(&preview, this, settings));
}

void CBTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void CBTool::prepareFinal()
{
    CBContainer settings = d->cbSettings->settings();

    ImageIface iface;
    setFilter(new CBFilter(iface.original(), this, settings));
}

void CBTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Color Balance"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorColorBalanceToolPlugin

#include "moc_cbtool.cpp"

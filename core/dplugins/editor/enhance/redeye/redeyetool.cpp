/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "redeyetool.h"

// Qt includes

#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>
#include <QColorDialog>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "redeyecorrectionfilter.h"
#include "redeyecorrectionsettings.h"
#include "imageregionwidget.h"

namespace DigikamEditorRedEyeToolPlugin
{

class Q_DECL_HIDDEN RedEyeTool::Private
{

public:

    Private() = default;

    const QString configGroupName               = QLatin1String("redeye Tool");
    const QString configHistogramChannelEntry   = QLatin1String("Histogram Channel");
    const QString configHistogramScaleEntry     = QLatin1String("Histogram Scale");

    RedEyeCorrectionSettings* settingsView      = nullptr;
    ImageRegionWidget*        previewWidget     = nullptr;
    EditorToolSettings*       gboxSettings      = nullptr;
};

// --------------------------------------------------------

RedEyeTool::RedEyeTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("redeye"));
    setToolHelp(QLatin1String("redeyecorrectiontool.anchor"));

    d->previewWidget = new ImageRegionWidget;
    d->previewWidget->setToolTip(i18n("Here you can see the image selection preview with "
                                      "red eye reduction applied."));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->settingsView = new RedEyeCorrectionSettings(d->gboxSettings->plainPage());

    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

RedEyeTool::~RedEyeTool()
{
    delete d;
}

void RedEyeTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->settingsView->readSettings(group);
}

void RedEyeTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->settingsView->writeSettings(group);

    config->sync();
}

void RedEyeTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void RedEyeTool::preparePreview()
{
    RedEyeCorrectionContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg original = d->previewWidget->getOriginalImage();
    setFilter(new RedEyeCorrectionFilter(&original, this, settings));
}

void RedEyeTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage().copy(d->previewWidget->getOriginalImageRegionToRender());
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void RedEyeTool::prepareFinal()
{
    RedEyeCorrectionContainer settings = d->settingsView->settings();

    ImageIface iface;

    setFilter(new RedEyeCorrectionFilter(iface.original(), this, settings));
}

void RedEyeTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Red Eyes Correction"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorRedEyeToolPlugin

#include "moc_redeyetool.cpp"

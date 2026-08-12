/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to blur an image
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "blurtool.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "blurfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorBlurToolPlugin
{

class Q_DECL_HIDDEN BlurTool::Private
{
public:

    Private() = default;

    const QString configGroupName               = QLatin1String("gaussianblur Tool");
    const QString configRadiusAdjustmentEntry   = QLatin1String("RadiusAdjustment");

    DIntNumInput*        radiusInput            = nullptr;
    ImageRegionWidget*   previewWidget          = nullptr;
    EditorToolSettings*  gboxSettings           = nullptr;
};


// --------------------------------------------------------

BlurTool::BlurTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("gaussianblur"));
    setToolHelp(QLatin1String("blursharpentool.anchor"));

    d->gboxSettings  = new EditorToolSettings(nullptr);
    d->previewWidget = new ImageRegionWidget;

    // --------------------------------------------------------

    QLabel* const label  = new QLabel(i18n("Smoothness:"));
    d->radiusInput = new DIntNumInput();
    d->radiusInput->setRange(0, 100, 1);
    d->radiusInput->setDefaultValue(0);
    d->radiusInput->setWhatsThis(i18n("A smoothness of 0 has no effect, "
                                      "1 and above determine the Gaussian blur matrix radius "
                                      "that determines how much to blur the image."));

    // --------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const grid = new QGridLayout( );
    grid->addWidget(label,          0, 0, 1, 2);
    grid->addWidget(d->radiusInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // --------------------------------------------------------

    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);

    // --------------------------------------------------------

    connect(d->radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

BlurTool::~BlurTool()
{
    delete d;
}

void BlurTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry, d->radiusInput->defaultValue()));
}

void BlurTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configRadiusAdjustmentEntry, d->radiusInput->value());
    config->sync();
}

void BlurTool::slotResetSettings()
{
    d->radiusInput->blockSignals(true);
    d->radiusInput->slotReset();
    d->radiusInput->blockSignals(false);
}

void BlurTool::preparePreview()
{
    DImg img = d->previewWidget->getOriginalRegionImage();
    setFilter(new BlurFilter(&img, this, d->radiusInput->value()));
}

void BlurTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);
}

void BlurTool::prepareFinal()
{
    ImageIface iface;
    setFilter(new BlurFilter(iface.original(), this, d->radiusInput->value()));
}

void BlurTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Gaussian Blur"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorBlurToolPlugin

#include "moc_blurtool.cpp"

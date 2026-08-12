/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-08-24
 * Description : image editor plugin to blur the background of an image
 *
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "backgroundblurtool.h"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QTimer>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "blurfilter.h"
#include "dpreviewmanager.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "backgroundblurfilter.h"

namespace DigikamEditorBackgroundBlurToolPlugin
{

class Q_DECL_HIDDEN BackgroundBlurTool::Private
{
public:

    Private() = default;

public:

    const QString configGroupName                 = QLatin1String("backgroundblur Tool");
    const QString configRadiusAdjustmentEntry     = QLatin1String("RadiusAdjustment");
    const QString configTransitionAdjustmentEntry = QLatin1String("TransitionAdjustment");
    const QString configIterationsAdjustmentEntry = QLatin1String("IterationsAdjustment");

    DIntNumInput*        radiusInput              = nullptr;
    DIntNumInput*        transitionInput          = nullptr;
    DIntNumInput*        iterationsInput          = nullptr;
    ImageGuideWidget*    previewWidget            = nullptr;
    EditorToolSettings*  gboxSettings             = nullptr;
    DPreviewManager*     maskPreview              = nullptr;
    QSize                orgSize;
    QRectF               orgSelection;
};

// --------------------------------------------------------

BackgroundBlurTool::BackgroundBlurTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("backgroundblur"));
    setToolHelp(QLatin1String("backgroundblur.anchor"));

    d->gboxSettings     = new EditorToolSettings(nullptr);
    d->previewWidget    = new ImageGuideWidget(nullptr, false, ImageGuideWidget::PickColorMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // --------------------------------------------------------

    QLabel* const label = new QLabel(i18n("Smoothness:"));
    d->radiusInput      = new DIntNumInput();
    d->radiusInput->setRange(1, 100, 1);
    d->radiusInput->setDefaultValue(1);
    d->radiusInput->setWhatsThis(i18n("Set this value to determine the matrix radius that "
                                      "determines how much to blur the background around the subject."));

    // --------------------------------------------------------

    QLabel* const label2 = new QLabel(i18n("Progressive Transition:"));
    d->transitionInput   = new DIntNumInput();
    d->transitionInput->setRange(1, 100, 1);
    d->transitionInput->setDefaultValue(1);
    d->transitionInput->setWhatsThis(i18n("Set this value to apply a variable blur near to far of the subject."
                                          "This allow to simulate a depth of field from a lens."));

    // --------------------------------------------------------

    QLabel* const label3 = new QLabel(i18n("Subject Isolation:"));
    d->iterationsInput   = new DIntNumInput();
    d->iterationsInput->setRange(1, 20, 1);
    d->iterationsInput->setDefaultValue(10);
    d->iterationsInput->setWhatsThis(i18n("Set this value to use a number of iterations used by "
                                          "the segmentation process to isolate more and less the subject."));

    // --------------------------------------------------------

    QLabel* const maskLbl   = new QLabel(i18n("Subject Mask:"));
    d->maskPreview          = new DPreviewManager(d->gboxSettings);
    d->maskPreview->setButtonVisible(false);
    d->maskPreview->setSelectionAreaPossible(true);
    d->maskPreview->setWhatsThis(i18n("This view show the segmentation of the subject determined from the "
                                      "selection. The resulting green mask is superposed to the original image to "
                                      "see if the subject have been isolated properly by the segmentation process. "
                                      "The green area is the subject where the blur effect will not applied. "
                                      "You can re-adjust in this view the rectangle including the subject."));
    ImageIface iface;
    d->orgSelection         = iface.selectionRect();
    d->orgSize              = iface.originalSize();

    // --------------------------------------------------------

    const int spacing       = d->gboxSettings->spacingHint();
    QGridLayout* const grid = new QGridLayout();
    grid->addWidget(label,              0, 0, 1, 1);
    grid->addWidget(d->radiusInput,     1, 0, 1, 1);
    grid->addWidget(label2,             2, 0, 1, 1);
    grid->addWidget(d->transitionInput, 3, 0, 1, 1);
    grid->addWidget(label3,             4, 0, 1, 1);
    grid->addWidget(d->iterationsInput, 5, 0, 1, 1);
    grid->addWidget(maskLbl,            6, 0, 1, 1);
    grid->addWidget(d->maskPreview,     7, 0, 1, 1);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // --------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);

    // --------------------------------------------------------

    connect(d->radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->transitionInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->iterationsInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->maskPreview, SIGNAL(signalSelectionChanged(QRectF)),
            this, SLOT(slotSelectionChanged(QRectF)));
}

BackgroundBlurTool::~BackgroundBlurTool()
{
    delete d;
}

void BackgroundBlurTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry, d->radiusInput->defaultValue()));
    d->transitionInput->setValue(group.readEntry(d->configTransitionAdjustmentEntry, d->transitionInput->defaultValue()));
    d->iterationsInput->setValue(group.readEntry(d->configIterationsAdjustmentEntry, d->iterationsInput->defaultValue()));
}

void BackgroundBlurTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configRadiusAdjustmentEntry, d->radiusInput->value());
    group.writeEntry(d->configTransitionAdjustmentEntry, d->transitionInput->value());
    group.writeEntry(d->configIterationsAdjustmentEntry, d->iterationsInput->value());
    config->sync();
}

void BackgroundBlurTool::slotResetSettings()
{
    d->radiusInput->blockSignals(true);
    d->transitionInput->blockSignals(true);
    d->iterationsInput->blockSignals(true);
    d->radiusInput->slotReset();
    d->transitionInput->slotReset();
    d->iterationsInput->slotReset();
    d->radiusInput->blockSignals(false);
    d->transitionInput->blockSignals(false);
    d->iterationsInput->blockSignals(false);
}

void BackgroundBlurTool::slotSelectionChanged(const QRectF& previewSel)
{
    const ImageIface* const iface = d->previewWidget->imageIface();
    DImg preview                  = iface->preview();

    // Compute the scale factor between mask preview and original canvas sizes.
    // Readjust the selection in editor canvas and normalize.

    double left   = (double)(previewSel.left() * d->orgSize.width()  / preview.width());

    if (left < 0)
    {
        left = 0;
    }

    double top    = (double)(previewSel.top() * d->orgSize.height() / preview.height());

    if (top < 0)
    {
        top = 0;
    }

    double width  = (double)(previewSel.width() * d->orgSize.width() / preview.width());

    if (width > iface->originalSize().width())
    {
        width = iface->originalSize().width();
    }

    double height = (double)(previewSel.height() * d->orgSize.height() / preview.height());

    if (height > iface->originalSize().height())
    {
        height = iface->originalSize().height();
    }

    width           = qMax(width, 1.0);
    height          = qMax(height, 1.0);

    d->orgSelection = QRectF(left, top, width, height);

    QTimer::singleShot(500, this, SLOT(slotTimer()));
}

void BackgroundBlurTool::preparePreview()
{
    d->maskPreview->setBusy(true, i18nc("@info", "Processing mask preview..."));

    const ImageIface* const iface = d->previewWidget->imageIface();
    DImg preview                  = iface->preview();

    // Compute the scale factor between original canvas and mask preview sizes.
    // Readjust the selection in the mask preview and normalize.

    double left   = (double)(d->orgSelection.left() / d->orgSize.width()  * preview.width());

    if (left < 0.0)
    {
        left = 0.0;
    }

    double top    = (double)(d->orgSelection.top() / d->orgSize.height() * preview.height());

    if (top < 0.0)
    {
        top = 0.0;
    }

    double width  = (double)(d->orgSelection.width() / d->orgSize.width() * preview.width());

    if (width > preview.width())
    {
        width = preview.width();
    }

    double height = (double)(d->orgSelection.height() / d->orgSize.height() * preview.height());

    if (height > preview.height())
    {
        height = preview.height();
    }

    width             = qMax(width, 1.0);
    height            = qMax(height, 1.0);

    QRectF previewSel = QRectF(left, top, width, height);

    d->maskPreview->setSelectionArea(previewSel);

    float oWidth                       = d->orgSize.width();
    float pWidth                       = iface->preview().width();

    int radius                         = qRound(d->radiusInput->value() / (oWidth / pWidth));

    BackgroundBlurFilter* const filter = new BackgroundBlurFilter(&preview,
                                                                  previewSel,
                                                                  qMax(radius, 1),
                                                                  d->transitionInput->value(),
                                                                  d->iterationsInput->value(),
                                                                  this);
    connect(filter, SIGNAL(signalSegmentedMask(QImage)),
            this, SLOT(slotPreviewMask(QImage)));

    setFilter(filter);
}

void BackgroundBlurTool::slotPreviewMask(const QImage& mask)
{
    if (mask.isNull())
    {
        d->maskPreview->setBusy(false, i18n("Error while processing image..."));

        return;
    }

    d->maskPreview->setImage(mask);
}

void BackgroundBlurTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg preview            = filter()->getTargetImage();

    iface->setPreview(preview);
    d->previewWidget->updatePreview();
}

void BackgroundBlurTool::prepareFinal()
{
    d->maskPreview->setBusy(true);

    ImageIface iface;
    setFilter(new BackgroundBlurFilter(iface.original(),
                                       d->orgSelection,
                                       d->radiusInput->value(),
                                       d->transitionInput->value(),
                                       d->iterationsInput->value(),
                                       this));
}

void BackgroundBlurTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Background Blur"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorBackgroundBlurToolPlugin

#include "moc_backgroundblurtool.cpp"

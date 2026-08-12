/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensautofixtool.h"

// Qt includes

#include <QBitmap>
#include <QBrush>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dmetadata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "lensfunfilter.h"
#include "lensfunsettings.h"
#include "lensfuncameraselector.h"
#include "dexpanderbox.h"

namespace DigikamEditorLensAutoFixToolPlugin
{

class Q_DECL_HIDDEN LensAutoFixTool::Private
{
public:

    Private() = default;

public:

    const QString          configGroupName  = QLatin1String("Lens Auto-Correction Tool");
    const QString          configShowGrid   = QLatin1String("Show Grid");

    QLabel*                maskPreviewLabel = nullptr;

    QCheckBox*             showGrid         = nullptr;

    LensFunSettings*       settingsView     = nullptr;
    LensFunCameraSelector* cameraSelector   = nullptr;

    ImageGuideWidget*      previewWidget    = nullptr;
    EditorToolSettings*    gboxSettings     = nullptr;
};

LensAutoFixTool::LensAutoFixTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("lensautocorrection"));
    setToolName(i18n("Lens Auto-Correction"));
    setToolIcon(QIcon::fromTheme(QLatin1String("lensautofix")));

    d->previewWidget = new ImageGuideWidget(nullptr, true, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings         = new EditorToolSettings(nullptr);
    QGridLayout* const grid = new QGridLayout(d->gboxSettings->plainPage());

    d->showGrid             = new QCheckBox(i18n("Show grid"), d->gboxSettings->plainPage());
    d->showGrid->setWhatsThis(i18n("Set this option to visualize the correction grid to be applied."));

    // -------------------------------------------------------------

    d->cameraSelector       = new LensFunCameraSelector(d->gboxSettings->plainPage());
    const DImg* const img   = d->previewWidget->imageIface()->original();
    d->cameraSelector->setMetadata(img->getMetadata());
    DLineWidget* const line = new DLineWidget(Qt::Horizontal, d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    d->settingsView         = new LensFunSettings(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    grid->addWidget(d->showGrid,       0, 0, 1, 2);
    grid->addWidget(d->cameraSelector, 1, 0, 1, 2);
    grid->addWidget(line,              2, 0, 1, 2);
    grid->addWidget(d->settingsView,   3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotLensChanged()));

    connect(d->showGrid, SIGNAL(toggled(bool)),
            this, SLOT(slotTimer()));
}

LensAutoFixTool::~LensAutoFixTool()
{
    delete d;
}

void LensAutoFixTool::slotLensChanged()
{
    d->settingsView->setEnabledCCA(d->cameraSelector->supportsCCA());
    d->settingsView->setEnabledVig(d->cameraSelector->supportsVig());
    d->settingsView->setEnabledDist(d->cameraSelector->supportsDistortion());
    d->settingsView->setEnabledGeom(d->cameraSelector->supportsGeometry());

    slotTimer();
}

void LensAutoFixTool::readSettings()
{
    d->gboxSettings->blockSignals(true);
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->showGrid->setChecked(group.readEntry(d->configShowGrid, false));
    d->cameraSelector->readSettings(group);
    d->settingsView->readSettings(group);
    d->gboxSettings->blockSignals(false);
    slotTimer();
}

void LensAutoFixTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configShowGrid, d->showGrid->isChecked());
    d->cameraSelector->writeSettings(group);
    d->settingsView->writeSettings(group);
    group.sync();
}

void LensAutoFixTool::slotResetSettings()
{
    d->gboxSettings->blockSignals(true);
    d->showGrid->setChecked(false);
    d->cameraSelector->resetToDefault();
    d->gboxSettings->blockSignals(false);
}

void LensAutoFixTool::preparePreview()
{
    // Settings information must be get before to disable settings view.

    LensFunContainer settings     = d->cameraSelector->settings();
    d->settingsView->assignFilterSettings(settings);
    const ImageIface* const iface = d->previewWidget->imageIface();
    DImg preview                  = iface->preview();

    if (d->showGrid->isChecked())
    {
        QBitmap pattern(9, 9);
        pattern.clear();
        QPainter p1(&pattern);
        p1.setPen(QPen(Qt::black, 1));
        p1.drawLine(5, 0, 5, 9);
        p1.drawLine(0, 5, 9, 5);
        p1.end();

        QPixmap pix(preview.size());
        pix.fill(Qt::transparent);
        QPainter p2(&pix);
        p2.setPen(QPen(Qt::gray, 1));
        p2.fillRect(0, 0, pix.width(), pix.height(), QBrush(pattern));
        p2.end();
        DImg grid(pix.toImage());

        if (preview.sixteenBit())
        {
            grid.convertDepth(64);
        }

        DColorComposer* const composer            = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
        DColorComposer::MultiplicationFlags flags = DColorComposer::NoMultiplication;

        // Do alpha blending of template on dest image

        preview.bitBlendImage(composer, &grid, 0, 0, preview.width(), preview.height(), 0, 0, flags);

        delete composer;
    }

    setFilter(new LensFunFilter(&preview, this, settings));
}

void LensAutoFixTool::prepareFinal()
{
    // Settings information must be handle before to disable settings view.

    LensFunContainer settings = d->cameraSelector->settings();
    d->settingsView->assignFilterSettings(settings);
    ImageIface iface;
    setFilter(new LensFunFilter(iface.original(), this, settings));
}

void LensAutoFixTool::setPreviewImage()
{
    d->previewWidget->imageIface()->setPreview(filter()->getTargetImage());
    d->previewWidget->updatePreview();
}

void LensAutoFixTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Lens Auto-Correction"), filter()->filterAction(), filter()->getTargetImage());

    MetaEngineData data  = iface.originalMetadata();

    // Note: there are certain situations in the editor where these metadata changes may be undone.
    // Ignore for now, as they are not critical.

    const LensFunFilter* const fltr = dynamic_cast<LensFunFilter*>(filter());

    if (fltr)
    {
        fltr->registerSettingsToXmp(data);
    }

    iface.setOriginalMetadata(data);
}

} // namespace DigikamEditorLensAutoFixToolPlugin

#include "moc_lensautofixtool.cpp"

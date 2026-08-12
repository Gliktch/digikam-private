/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor tool to emboss
 *               an image.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "embosstool.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "editortoolsettings.h"
#include "embossfilter.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorEmbossToolPlugin
{

class Q_DECL_HIDDEN EmbossTool::Private
{
public:

    Private() = default;

    const QString configGroupName               = QLatin1String("emboss Tool");
    const QString configDepthAdjustmentEntry    = QLatin1String("DepthAdjustment");

    DIntNumInput*        depthInput             = nullptr;
    ImageRegionWidget*   previewWidget          = nullptr;
    EditorToolSettings*  gboxSettings           = nullptr;
};

// --------------------------------------------------------

EmbossTool::EmbossTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("emboss"));
    setInitPreview(true);

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings(nullptr);
    d->previewWidget = new ImageRegionWidget;

    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Depth:"));
    d->depthInput  = new DIntNumInput;
    d->depthInput->setRange(10, 300, 1);
    d->depthInput->setDefaultValue(30);
    d->depthInput->setWhatsThis( i18n("Set here the depth of the embossing image effect.") );

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,        0, 0, 1, 2);
    mainLayout->addWidget(d->depthInput, 1, 0, 1, 2);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(d->depthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

EmbossTool::~EmbossTool()
{
    delete d;
}

void EmbossTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->depthInput->blockSignals(true);
    d->depthInput->setValue(group.readEntry(d->configDepthAdjustmentEntry, d->depthInput->defaultValue()));
    d->depthInput->blockSignals(false);
}

void EmbossTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDepthAdjustmentEntry, d->depthInput->value());
    group.sync();
}

void EmbossTool::slotResetSettings()
{
    d->depthInput->blockSignals(true);
    d->depthInput->slotReset();
    d->depthInput->blockSignals(false);

    slotPreview();
}

void EmbossTool::preparePreview()
{
    DImg image = d->previewWidget->getOriginalRegionImage();
    int depth  = d->depthInput->value();

    setFilter(new EmbossFilter(&image, this, depth));
}

void EmbossTool::prepareFinal()
{
    int depth = d->depthInput->value();

    ImageIface iface;
    setFilter(new EmbossFilter(iface.original(), this, depth));
}

void EmbossTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void EmbossTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Emboss"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorEmbossToolPlugin

#include "moc_embosstool.cpp"

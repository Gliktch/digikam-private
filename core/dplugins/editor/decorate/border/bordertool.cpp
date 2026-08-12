/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image tool to add a border
 *               around an image.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bordertool.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "borderfilter.h"
#include "bordersettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace DigikamEditorBorderToolPlugin
{

class Q_DECL_HIDDEN BorderTool::Private
{
public:

    Private() = default;

public:

    const QString       configGroupName = QLatin1String("border Tool");

    EditorToolSettings* gboxSettings    = nullptr;
    ImageGuideWidget*   previewWidget   = nullptr;
    BorderSettings*     settingsView    = nullptr;
};

BorderTool::BorderTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("border"));
    setToolName(i18n("Add Border"));
    setToolIcon(QIcon::fromTheme(QLatin1String("bordertool")));

    d->previewWidget = new ImageGuideWidget(nullptr, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->settingsView = new BorderSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

BorderTool::~BorderTool()
{
    delete d;
}

void BorderTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void BorderTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);

    group.sync();
}

void BorderTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void BorderTool::preparePreview()
{
    const ImageIface* iface  = d->previewWidget->imageIface();
    DImg preview             = iface->preview();
    int w                    = iface->previewSize().width();
    float ratio              = (float)w/(float)iface->originalSize().width();
    BorderContainer settings = d->settingsView->settings();
    settings.orgWidth        = iface->originalSize().width();
    settings.orgHeight       = iface->originalSize().height();
    settings.borderWidth1    = (int)((float)settings.borderWidth1*ratio);
    settings.borderWidth2    = (int)(20.0*ratio);
    settings.borderWidth3    = (int)(20.0*ratio);
    settings.borderWidth4    = 3;

    setFilter(new BorderFilter(&preview, this, settings));
}

void BorderTool::prepareFinal()
{
    ImageIface iface;
    DImg* orgImage           = iface.original();
    BorderContainer settings = d->settingsView->settings();
    settings.orgWidth        = iface.originalSize().width();
    settings.orgHeight       = iface.originalSize().height();

    setFilter(new BorderFilter(orgImage, this, settings));
}

void BorderTool::setPreviewImage()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewSize().width();
    int h             = iface->previewSize().height();
    DImg imTemp       = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(), filter()->getTargetImage().hasAlpha());

    imDest.fill(DColor(d->previewWidget->palette().color(QPalette::Window).rgb(),
                       filter()->getTargetImage().sixteenBit()) );

    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->setPreview(imDest);
    d->previewWidget->updatePreview();
}

void BorderTool::setFinalImage()
{
    ImageIface iface;
    DImg targetImage = filter()->getTargetImage();
    iface.setOriginal(i18n("Add Border"), filter()->filterAction(), targetImage);
}

} // namespace DigikamEditorBorderToolPlugin

#include "moc_bordertool.cpp"

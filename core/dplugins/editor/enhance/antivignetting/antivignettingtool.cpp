/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image tool to reduce
 *               vignetting on an image.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Julien Narboux <julien at narboux dot fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "antivignettingtool.h"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dnuminput.h"
#include "antivignettingfilter.h"
#include "antivignettingsettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace DigikamEditorAntivignettingToolPlugin
{

class Q_DECL_HIDDEN AntiVignettingTool::Private
{
public:

    Private() = default;

public:

    const QString           configGroupName = QLatin1String("antivignetting Tool");

    AntiVignettingSettings* settingsView    = nullptr;
    ImageGuideWidget*       previewWidget   = nullptr;
    EditorToolSettings*     gboxSettings    = nullptr;
};

AntiVignettingTool::AntiVignettingTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("antivignetting"));
    setToolName(i18n("Vignetting Correction"));
    setToolIcon(QIcon::fromTheme(QLatin1String("antivignetting")));

    d->previewWidget = new ImageGuideWidget(nullptr, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->settingsView = new AntiVignettingSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

AntiVignettingTool::~AntiVignettingTool()
{
    delete d;
}

void AntiVignettingTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
    slotPreview();
}

void AntiVignettingTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    group.sync();
}

void AntiVignettingTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void AntiVignettingTool::preparePreview()
{
    AntiVignettingContainer settings = d->settingsView->settings();

    const ImageIface* const iface    = d->previewWidget->imageIface();
    int previewWidth                 = iface->previewSize().width();
    int previewHeight                = iface->previewSize().height();
    DImg imTemp                      = iface->original()->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);

    setFilter(new AntiVignettingFilter(&imTemp, this, settings));
}

void AntiVignettingTool::prepareFinal()
{
    AntiVignettingContainer settings = d->settingsView->settings();

    ImageIface iface;
    setFilter(new AntiVignettingFilter(iface.original(), this, settings));
}

void AntiVignettingTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg preview            = filter()->getTargetImage().smoothScale(iface->previewSize());
    iface->setPreview(preview);
    d->previewWidget->updatePreview();
}

void AntiVignettingTool::setFinalImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg finalImage         = filter()->getTargetImage();

    iface->setOriginal(i18n("Vignetting Correction"), filter()->filterAction(), finalImage);
}

} // namespace DigikamEditorAntivignettingToolPlugin

#include "moc_antivignettingtool.cpp"

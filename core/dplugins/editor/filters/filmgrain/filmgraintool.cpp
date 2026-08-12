/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor tool for add film
 *               grain on an image.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filmgraintool.h"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "filmgrainfilter.h"
#include "filmgrainsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorFilmGrainToolPlugin
{

class Q_DECL_HIDDEN FilmGrainTool::Private
{
public:

    Private() = default;

    const QString       configGroupName = QLatin1String("filmgrain Tool");

    FilmGrainSettings*  settingsView    = nullptr;

    ImageRegionWidget*  previewWidget   = nullptr;
    EditorToolSettings* gboxSettings    = nullptr;
};

FilmGrainTool::FilmGrainTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("filmgrain"));
    setInitPreview(true);

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings(nullptr);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    d->settingsView = new FilmGrainSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------


    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

FilmGrainTool::~FilmGrainTool()
{
    delete d;
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void FilmGrainTool::preparePreview()
{
    FilmGrainContainer prm = d->settingsView->settings();
    DImg image             = d->previewWidget->getOriginalRegionImage();

    setFilter(new FilmGrainFilter(&image, this, prm));
}

void FilmGrainTool::prepareFinal()
{
    FilmGrainContainer prm = d->settingsView->settings();

    ImageIface iface;
    setFilter(new FilmGrainFilter(iface.original(), this, prm));
}

void FilmGrainTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void FilmGrainTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Film Grain"), filter()->filterAction(), filter()->getTargetImage());
}

void FilmGrainTool::renderingFinished()
{
    toolSettings()->enableButton(EditorToolSettings::Ok, d->settingsView->settings().isDirty());
}

} // namespace DigikamEditorFilmGrainToolPlugin

#include "moc_filmgraintool.cpp"

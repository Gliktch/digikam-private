/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a tool to apply texture over an image
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "texturetool.h"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "texturesettings.h"
#include "texturefilter.h"

namespace DigikamEditorTextureToolPlugin
{

class Q_DECL_HIDDEN TextureTool::Private
{
public:

    Private() = default;

    const QString       configGroupName = QLatin1String("texture Tool");

    TextureSettings*    settingsView    = nullptr;
    ImageRegionWidget*  previewWidget   = nullptr;
    EditorToolSettings* gboxSettings    = nullptr;
};

// --------------------------------------------------------

TextureTool::TextureTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("texture"));

    // -------------------------------------------------------------

    d->gboxSettings      = new EditorToolSettings(nullptr);
    d->previewWidget     = new ImageRegionWidget;
    d->settingsView      = new TextureSettings(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

TextureTool::~TextureTool()
{
    delete d;
}

void TextureTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void TextureTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);

    group.sync();
}

void TextureTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void TextureTool::preparePreview()
{
    DImg image                = d->previewWidget->getOriginalRegionImage();
    TextureContainer settings = d->settingsView->settings();
    setFilter(new TextureFilter(&image, this, settings));
}

void TextureTool::prepareFinal()
{
    ImageIface iface;
    TextureContainer settings = d->settingsView->settings();
    setFilter(new TextureFilter(iface.original(), this, settings));
}

void TextureTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void TextureTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Texture"), filter()->filterAction(), filter()->getTargetImage());
}

} // namespace DigikamEditorTextureToolPlugin

#include "moc_texturetool.cpp"

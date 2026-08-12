/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Setup view panel for dplugins.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotosetupplugins.h"

// Qt includes

#include <QTabWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpluginsetup.h"
#include "dpluginconfviewgeneric.h"
#include "dpluginconfvieweditor.h"
#include "dpluginconfviewdimg.h"

using namespace Digikam;

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoSetupPlugins::Private
{
public:

    Private() = default;

    QTabWidget*   tab           = nullptr;

    DPluginSetup* setupGeneric  = nullptr;
    DPluginSetup* setupEditor   = nullptr;
    DPluginSetup* setupDImg     = nullptr;
};

ShowfotoSetupPlugins::ShowfotoSetupPlugins(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->tab          = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    // --------------------

    d->setupGeneric = new DPluginSetup(d->tab);
    d->setupGeneric->setPluginConfView(new Digikam::DPluginConfViewGeneric(d->setupGeneric));
    d->tab->insertTab(Generic, d->setupGeneric, i18nc("@title:tab generic plugins list", "Generic"));

    // --------------------

    d->setupEditor  = new DPluginSetup(d->tab);
    d->setupEditor->setPluginConfView(new Digikam::DPluginConfViewEditor(d->setupEditor));
    d->tab->insertTab(Editor, d->setupEditor, i18nc("@title:tab editor plugins list", "Image Editor"));

    // --------------------

    d->setupDImg    = new DPluginSetup(d->tab);
    d->setupDImg->setPluginConfView(new Digikam::DPluginConfViewDImg(d->setupEditor));
    d->tab->insertTab(Loaders, d->setupDImg, i18nc("@title:tab loader plugins list", "Image Loaders"));
}

ShowfotoSetupPlugins::~ShowfotoSetupPlugins()
{
    delete d;
}

void ShowfotoSetupPlugins::setActiveTab(PluginTab tab)
{
    d->tab->setCurrentIndex(tab);
}

ShowfotoSetupPlugins::PluginTab ShowfotoSetupPlugins::activeTab() const
{
    return (PluginTab)d->tab->currentIndex();
}

void ShowfotoSetupPlugins::applySettings()
{
    d->setupGeneric->applySettings();
    d->setupEditor->applySettings();
    d->setupDImg->applySettings();
}

} // namespace ShowFoto

#include "moc_showfotosetupplugins.cpp"

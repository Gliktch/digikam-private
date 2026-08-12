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

#include "setupplugins.h"

// Qt includes

#include <QTabWidget>
#include <QApplication>
#include <QStyle>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpluginsetup.h"
#include "dpluginaction.h"
#include "dpluginconfviewgeneric.h"
#include "dpluginconfvieweditor.h"
#include "dpluginconfviewbqm.h"
#include "dpluginconfviewdimg.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupPlugins::Private
{
public:

    Private() = default;

public:

    QTabWidget*   tab           = nullptr;

    DPluginSetup* setupGeneric  = nullptr;
    DPluginSetup* setupEditor   = nullptr;
    DPluginSetup* setupBqm      = nullptr;
    DPluginSetup* setupDImg     = nullptr;
};

SetupPlugins::SetupPlugins(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->tab          = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    // --------------------

    d->setupGeneric = new DPluginSetup(d->tab);
    d->setupGeneric->setPluginConfView(new DPluginConfViewGeneric(d->setupGeneric));
    d->tab->insertTab(Generic, d->setupGeneric, i18nc("@title:tab plugins type", "Generic"));

    // --------------------

    d->setupEditor  = new DPluginSetup(d->tab);
    d->setupEditor->setPluginConfView(new DPluginConfViewEditor(d->setupEditor));
    d->tab->insertTab(Editor, d->setupEditor, i18nc("@title:tab plugins type", "Image Editor"));

    // --------------------

    d->setupBqm     = new DPluginSetup(d->tab);
    d->setupBqm->setPluginConfView(new DPluginConfViewBqm(d->setupBqm));
    d->tab->insertTab(Bqm, d->setupBqm, i18nc("@title:tab plugins type", "Batch Queue Manager"));

    // --------------------

    d->setupDImg    = new DPluginSetup(d->tab);
    d->setupDImg->setPluginConfView(new DPluginConfViewDImg(d->setupDImg));
    d->tab->insertTab(Loaders, d->setupDImg, i18nc("@title:tab plugins type", "Image Loaders"));
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::setActiveTab(PluginTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupPlugins::PluginTab SetupPlugins::activeTab() const
{
    return (PluginTab)d->tab->currentIndex();
}

void SetupPlugins::applySettings()
{
    d->setupGeneric->applySettings();
    d->setupEditor->applySettings();
    d->setupBqm->applySettings();
    d->setupDImg->applySettings();
}

} // namespace Digikam

#include "moc_setupplugins.cpp"

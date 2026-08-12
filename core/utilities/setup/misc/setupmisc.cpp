/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmisc_p.h"

namespace Digikam
{

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->parent = parent;
    d->tab            = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    setupBehavior();
    setupAppearance();
    setupGrouping();

    // -- Spell Check and localize Options --------------------------------------

#ifdef HAVE_SONNET

    d->spellCheckWidget = new SpellCheckConfig(d->tab);

    d->tab->insertTab(SpellCheck, d->spellCheckWidget, i18nc("@title:tab", "Spellcheck"));

#endif

    // ---

    d->localizeWidget = new LocalizeConfig(d->tab);

    d->tab->insertTab(Localize, d->localizeWidget, i18nc("@title:tab", "Localize"));

    // -- System Options --------------------------------------------------------

    d->systemSettingsWidget = new SystemSettingsWidget(d->tab);

    d->tab->insertTab(System, d->systemSettingsWidget, i18nc("@title:tab", "System"));

    // ---

    readSettings();
    adjustSize();
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::setActiveTab(MiscTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupMisc::MiscTab SetupMisc::activeTab() const
{
    return (MiscTab)d->tab->currentIndex();
}

} // namespace Digikam

#include "moc_setupmisc.cpp"

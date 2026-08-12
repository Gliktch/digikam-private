/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * SPDX-FileCopyrightText: 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupalbumview_p.h"

namespace Digikam
{

SetupAlbumView::SetupAlbumView(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->tab                     = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    setupIconView();
    setupTreeView();
    setupPreView();

    // ---

    QWidget* const fspanel    = new QWidget(d->tab);
    QVBoxLayout* const layout = new QVBoxLayout(fspanel);
    d->fullScreenSettings     = new FullScreenSettings(FS_ALBUMGUI, d->tab);
    d->fullScreenSettings->setTitle(QString());
    d->fullScreenSettings->setFlat(true);

    layout->setContentsMargins(QMargins());
    layout->setSpacing(d->spacing);
    layout->addWidget(d->fullScreenSettings);
    layout->addStretch();

    d->tab->insertTab(FullScreen, fspanel, i18nc("@title:tab", "Full-Screen"));

    // ---

    d->mimetype = new SetupMime();
    d->tab->insertTab(MimeType, d->mimetype, i18nc("@title:tab", "Mime Types"));

    // ---

    d->category = new SetupCategory(d->tab);
    d->tab->insertTab(Category, d->category, i18nc("@title:tab", "Categories"));

    // ---

    readSettings();
    adjustSize();
}

SetupAlbumView::~SetupAlbumView()
{
    delete d;
}

void SetupAlbumView::setActiveTab(AlbumTab tab)
{
    d->tab->setCurrentIndex(tab);
}

SetupAlbumView::AlbumTab SetupAlbumView::activeTab() const
{
    return (AlbumTab)d->tab->currentIndex();
}

} // namespace Digikam

#include "moc_setupalbumview.cpp"

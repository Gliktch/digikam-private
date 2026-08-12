/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the folder view.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumfolderviewsidebarwidget.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "albummanager.h"
#include "albumselectiontreeview.h"
#include "applicationsettings.h"
#include "searchtextbardb.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumFolderViewSideBarWidget::Private
{
public:

    Private() = default;

public:

    AlbumModificationHelper* albumModificationHelper = nullptr;
    AlbumSelectionTreeView*  albumFolderView         = nullptr;
    SearchTextBarDb*         searchTextBar           = nullptr;
};

AlbumFolderViewSideBarWidget::AlbumFolderViewSideBarWidget(QWidget* const parent,
                                                           AlbumModel* const model,
                                                           AlbumModificationHelper* const albumModificationHelper)
    : SidebarWidget(parent),
      d            (new Private)
{
    setObjectName(QLatin1String("AlbumFolderView Sidebar"));
    setProperty("Shortcut", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F1));
    d->albumModificationHelper = albumModificationHelper;

    const int spacing          = layoutSpacing();

    QVBoxLayout* const layout  = new QVBoxLayout(this);
    d->albumFolderView         = new AlbumSelectionTreeView(this, model, d->albumModificationHelper);
    d->albumFolderView->setObjectName(QLatin1String("AlbumFolderView"));
    d->albumFolderView->setConfigGroup(getConfigGroup());
    d->albumFolderView->setAlbumManagerCurrentAlbum(true);
    d->searchTextBar           = new SearchTextBarDb(this, QLatin1String("ItemIconViewFolderSearchBar"));
    d->searchTextBar->setHighlightOnResult(true);
    d->searchTextBar->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchTextBar->setFilterModel(d->albumFolderView->albumFilterModel());

    layout->addWidget(d->albumFolderView);
    layout->addWidget(d->searchTextBar);
    layout->setContentsMargins(0, 0, spacing, 0);

    // setup connection

    connect(d->albumFolderView, SIGNAL(signalFindDuplicates(QList<PAlbum*>)),
            this, SIGNAL(signalFindDuplicates(QList<PAlbum*>)));
}

AlbumFolderViewSideBarWidget::~AlbumFolderViewSideBarWidget()
{
    delete d;
}

void AlbumFolderViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->albumFolderView->currentPAlbum());
    }
}

void AlbumFolderViewSideBarWidget::doLoadState()
{
    d->albumFolderView->loadState();
}

void AlbumFolderViewSideBarWidget::doSaveState()
{
    d->albumFolderView->saveState();
}

void AlbumFolderViewSideBarWidget::applySettings()
{
    const ApplicationSettings* const settings = ApplicationSettings::instance();
    d->albumFolderView->setEnableToolTips(settings->getShowAlbumToolTips());
    d->albumFolderView->setExpandNewCurrentItem(settings->getExpandNewCurrentItem());
}

void AlbumFolderViewSideBarWidget::applyResorting()
{
    d->albumFolderView->setSortingEnabled(false);
    d->albumFolderView->setSortingEnabled(true);
}

void AlbumFolderViewSideBarWidget::changeAlbumFromHistory(const QList<Album*>& album)
{
    d->albumFolderView->setCurrentAlbums(album);
}

AlbumPointer<PAlbum> AlbumFolderViewSideBarWidget::currentAlbum() const
{
    return AlbumPointer<PAlbum> (d->albumFolderView->currentPAlbum());
}

void AlbumFolderViewSideBarWidget::setCurrentAlbum(PAlbum* album)
{
    // Change the current album in list view.

    d->albumFolderView->setCurrentAlbums(QList<Album*>() << album);
}

const QIcon AlbumFolderViewSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("folder-pictures"));
}

const QString AlbumFolderViewSideBarWidget::getCaption()
{
    return i18n("Albums");
}

} // namespace Digikam

#include "moc_albumfolderviewsidebarwidget.cpp"

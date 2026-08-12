/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select albums using a tab of folder views.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumselecttabs.h"

// Qt includes

#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QString>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "applicationsettings.h"
#include "albumtreeview.h"
#include "tagtreeview.h"
#include "searchtreeview.h"
#include "labelstreeview.h"
#include "abstractalbummodel.h"
#include "searchtextbardb.h"
#include "albummanager.h"
#include "albumlabelssearchhandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumSelectTabs::Private
{
public:

    Private() = default;

    void prepareTreeView(AbstractCheckableAlbumTreeView* const treeView)
    {
        treeView->checkableAlbumModel()->setShowCount(false);
        treeView->checkableAlbumModel()->setCheckable(true);
        treeView->setRootIsDecorated(true);
        treeView->setSortingEnabled(true);
        treeView->setSelectAlbumOnClick(false);
        treeView->setExpandOnSingleClick(false);
        treeView->setEnableContextMenu(false);
        treeView->setDragEnabled(false);
        treeView->setRestoreCheckState(true);
    }

public:

    AlbumModel*               albumModel            = nullptr;
    AlbumTreeView*            albumTreeView         = nullptr;

    TagModel*                 tagModel              = nullptr;
    TagTreeView*              tagTreeView           = nullptr;

    SearchModel*              searchModel           = nullptr;
    SearchTreeView*           searchTreeView        = nullptr;

    SearchTextBarDb*          albumSearchBar        = nullptr;
    SearchTextBarDb*          tagSearchBar          = nullptr;
    SearchTextBarDb*          searchSearchBar       = nullptr;

    LabelsTreeView*           labelsTree            = nullptr;
    AlbumLabelsSearchHandler* labelsSearchHandler   = nullptr;
};

AlbumSelectTabs::AlbumSelectTabs(const QString& name, QWidget* const parent)
    : QTabWidget(parent),
      d         (new Private)
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup configGroup = config->group(QLatin1String("AlbumSelectTabs") +
                               QString::fromLatin1("_%1").arg(name));

    DVBox* const albumBox = new DVBox(this);
    d->albumModel         = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum, albumBox);
    d->albumTreeView      = new AlbumTreeView(albumBox);
    d->albumTreeView->setPAlbumModel(d->albumModel);
    d->albumTreeView->setEntryPrefix(QLatin1String("AlbumTreeView"));
    d->albumTreeView->setConfigGroup(configGroup);
    d->prepareTreeView(d->albumTreeView);

    d->albumSearchBar = new SearchTextBarDb(albumBox, QLatin1String("AlbumSelectTabsAlbumSearchBar"));
    d->albumSearchBar->setEntryPrefix(QLatin1String("AlbumSearchBar"));
    d->albumSearchBar->setConfigGroup(configGroup);
    d->albumSearchBar->setModel(d->albumModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->albumSearchBar->setFilterModel(d->albumTreeView->albumFilterModel());

    albumBox->setContentsMargins(QMargins());
    albumBox->setSpacing(layoutSpacing());

    albumBox->setStretchFactor(d->albumTreeView, 100);
    albumBox->setStretchFactor(d->albumSearchBar, 1);

    // -------------------------------------------------------------------------------

    DVBox* const tagBox = new DVBox(this);
    d->tagModel         = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, tagBox);
    d->tagTreeView      = new TagTreeView(tagBox);
    d->tagTreeView->setTAlbumModel(d->tagModel);
    d->tagTreeView->setEntryPrefix(QLatin1String("TagTreeView"));
    d->tagTreeView->setConfigGroup(configGroup);
    d->prepareTreeView(d->tagTreeView);

    d->tagSearchBar = new SearchTextBarDb(tagBox, QLatin1String("AlbumSelectTabsTagSearchBar"));
    d->tagSearchBar->setEntryPrefix(QLatin1String("TagSearchBar"));
    d->tagSearchBar->setConfigGroup(configGroup);
    d->tagSearchBar->setModel(d->tagTreeView->filteredModel(),
                              AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagTreeView->albumFilterModel());

    tagBox->setContentsMargins(QMargins());
    tagBox->setSpacing(layoutSpacing());

    tagBox->setStretchFactor(d->tagTreeView, 100);
    tagBox->setStretchFactor(d->tagSearchBar, 1);

    // -------------------------------------------------------------------------------

    DVBox* const searchBox  = new DVBox(this);
    d->searchModel          = new SearchModel(searchBox);
    d->searchTreeView       = new SearchTreeView(searchBox);
    d->searchTreeView->setSAlbumModel(d->searchModel);
    d->searchTreeView->setEntryPrefix(QLatin1String("SearchTreeView"));
    d->searchTreeView->setConfigGroup(configGroup);
    d->searchTreeView->filteredModel()->listNormalSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(false);
    d->prepareTreeView(d->searchTreeView);

    d->searchSearchBar = new SearchTextBarDb(searchBox, QLatin1String("AlbumSelectTabsSearchSearchBar"));
    d->searchSearchBar->setEntryPrefix(QLatin1String("SearchSearchBar"));
    d->searchSearchBar->setConfigGroup(configGroup);
    d->searchSearchBar->setModel(d->searchModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchSearchBar->setFilterModel(d->searchTreeView->albumFilterModel());

    searchBox->setContentsMargins(QMargins());
    searchBox->setSpacing(layoutSpacing());

    searchBox->setStretchFactor(d->searchTreeView, 100);
    searchBox->setStretchFactor(d->searchSearchBar, 1);

    // -------------------------------------------------------------------------------

    DVBox* const labelsBox = new DVBox(this);
    d->labelsTree          = new LabelsTreeView(labelsBox, true);
    d->labelsTree->setEntryPrefix(QLatin1String("LabelsTreeView"));
    d->labelsTree->setConfigGroup(configGroup);
    d->labelsSearchHandler = new AlbumLabelsSearchHandler(d->labelsTree);

    labelsBox->setContentsMargins(QMargins());
    labelsBox->setSpacing(layoutSpacing());

    labelsBox->setStretchFactor(d->labelsTree, 100);

    // -------------------------------------------------------------------------------

    addTab(albumBox,  i18n("Albums"));
    addTab(tagBox,    i18n("Tags"));
    addTab(searchBox, i18n("Searches"));
    addTab(labelsBox, i18n("Labels"));

    // ------------------------------------------------------------------------------------

    connect(d->albumModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    connect(d->tagModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    connect(d->searchModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    connect(d->labelsSearchHandler, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    // ------------------------------------------------------------------------------------

    d->albumTreeView->loadState();
    d->albumSearchBar->loadState();
    d->tagTreeView->loadState();
    d->tagSearchBar->loadState();
    d->searchTreeView->loadState();
    d->searchSearchBar->loadState();
    d->labelsTree->loadState();
}

AlbumSelectTabs::~AlbumSelectTabs()
{
    d->albumTreeView->saveState();
    d->albumSearchBar->saveState();
    d->tagTreeView->saveState();
    d->tagSearchBar->saveState();
    d->searchTreeView->saveState();
    d->searchSearchBar->saveState();
    d->labelsTree->saveState();

    delete d;
}

AlbumList AlbumSelectTabs::selectedAlbums() const
{
    AlbumList list;

    list << d->albumModel->checkedAlbums();
    list << d->tagModel->checkedAlbums();
    list << d->searchTreeView->checkableAlbumModel()->checkedAlbums();
    list << d->labelsSearchHandler->albumForSelectedItems();

    // Remove all null albums.

    list.removeAll(nullptr);

    qCDebug(DIGIKAM_GENERAL_LOG) << list.count() << " albums selected";

    return list;
}

void AlbumSelectTabs::enableVirtualAlbums(bool flag)
{
    setTabEnabled(1, flag);
    setTabEnabled(2, flag);
    setTabEnabled(3, flag);
}

QList<AbstractCheckableAlbumModel*> AlbumSelectTabs::albumModels() const
{
    QList<AbstractCheckableAlbumModel*> list;
    list << d->albumModel;
    list << d->tagModel;
    list << d->searchModel;

    return list;
}

AlbumLabelsSearchHandler* AlbumSelectTabs::albumLabelsHandler() const
{
    return d->labelsSearchHandler;
}

} // namespace Digikam

#include "moc_albumselecttabs.cpp"

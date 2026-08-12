/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-26-02
 * Description : a widget to select a physical album
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumselectwidget.h"

// Qt includes

#include <QIcon>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QPushButton>
#include <QAction>
#include <QTimer>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "album.h"
#include "albummodificationhelper.h"
#include "albumthumbnailloader.h"
#include "collectionmanager.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN AlbumSelectTreeView::Private
{
public:

    Private() = default;

public:

    AlbumModificationHelper* albumModificationHelper = nullptr;
    QAction*                 newAlbumAction          = nullptr;
};

AlbumSelectTreeView::AlbumSelectTreeView(AlbumModel* const model,
                                         AlbumModificationHelper* const albumModificationHelper,
                                         QWidget* const parent)
    : AlbumTreeView(parent),
      d            (new Private)
{
    setPAlbumModel(model);
    d->albumModificationHelper = albumModificationHelper;
    d->newAlbumAction          = new QAction(QIcon::fromTheme(QLatin1String("folder-new")),
                                             i18n("Create New Album"), this);
}

AlbumSelectTreeView::~AlbumSelectTreeView()
{
    delete d;
}

void AlbumSelectTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    cmh.addAction(d->newAlbumAction);
    d->newAlbumAction->setEnabled(album);
}

void AlbumSelectTreeView::handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album)
{
    Album* const a       = album;
    PAlbum* const palbum = dynamic_cast<PAlbum*>(a);

    if (palbum && (action == d->newAlbumAction))
    {
        d->albumModificationHelper->slotAlbumNew(palbum);
    }
}

void AlbumSelectTreeView::slotNewAlbum()
{
    PAlbum* const palbum = currentPAlbum();

    if (palbum)
    {
        PAlbum* const createdAlbum = d->albumModificationHelper->slotAlbumNew(palbum);  // cppcheck-suppress constVariablePointer

        if (createdAlbum)
        {
            setCurrentAlbums(QList<Album*>() << createdAlbum, false);
        }
    }
}

// --------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumSelectWidget::Private
{
public:

    Private() = default;

public:

    PAlbum*                  complAlbum                 = nullptr;
    AlbumModel*              albumModel                 = nullptr;
    AlbumSelectTreeView*     albumTreeView              = nullptr;

    AlbumModificationHelper* albumModificationHelper    = nullptr;

    SearchTextBarDb*         searchBar                  = nullptr;

    QPushButton*             newAlbumBtn                = nullptr;
};

AlbumSelectWidget::AlbumSelectWidget(QWidget* const parent,
                                     PAlbum* const albumToSelect, bool completerSelect)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(QLatin1String("AlbumSelectWidget"));

    d->albumModificationHelper = new AlbumModificationHelper(this, this);

    // TODO let this class implement StateSavingObject

    KConfigGroup group = KSharedConfig::openConfig()->group(objectName());

    QGridLayout* const grid = new QGridLayout(this);
    d->albumModel           = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    d->albumTreeView        = new AlbumSelectTreeView(d->albumModel, d->albumModificationHelper, this);
    d->albumTreeView->setDragEnabled(false);
    d->albumTreeView->setRootIsDecorated(true);
    d->albumTreeView->setDropIndicatorShown(false);
    d->albumTreeView->setAcceptDrops(false);
    d->albumTreeView->setSelectAlbumOnClick(false);
    d->albumTreeView->setSelectOnContextMenu(false);
    d->albumTreeView->setEnableContextMenu(true);
    d->albumTreeView->setSortingEnabled(true);
    d->albumTreeView->setConfigGroup(group);
    d->albumTreeView->setEntryPrefix(QLatin1String("AlbumTreeView"));

    d->searchBar   = new SearchTextBarDb(this, QLatin1String("AlbumSelectWidgetSearchBar"));
    d->searchBar->setModel(d->albumModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchBar->setFilterModel(d->albumTreeView->albumFilterModel());
    d->searchBar->setConfigGroup(group);
    d->albumTreeView->setEntryPrefix(QLatin1String("AlbumTreeView"));

    d->newAlbumBtn = new QPushButton(i18n("&New Album"), this);
    d->newAlbumBtn->setToolTip(i18n("Create new album"));
    d->newAlbumBtn->setIcon(QIcon::fromTheme(QLatin1String("folder-new")));

    grid->addWidget(d->albumTreeView, 0, 0, 1, 2);
    grid->addWidget(d->searchBar,     1, 0, 1, 1);
    grid->addWidget(d->newAlbumBtn,   1, 1, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(layoutSpacing());

    // ------------------------------------------------------------------------------------

    d->albumTreeView->loadState();
    d->searchBar->loadState();

    // ------------------------------------------------------------------------------------

    PAlbum* select = albumToSelect;     // cppcheck-suppress constVariablePointer

    if (!select)
    {
        select = AlbumManager::instance()->currentPAlbum();
    }

    d->albumTreeView->setCurrentAlbums(QList<Album*>() << select, false);

    bool enabled = !(!currentAlbum() || currentAlbum()->isRoot());
    d->newAlbumBtn->setEnabled(enabled);

    // ------------------------------------------------------------------------------------

    connect(d->albumTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotSelectionChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    if (completerSelect)
    {
        connect(d->searchBar, SIGNAL(completerHighlighted(int)),
                this, SLOT(slotCompleterHighlighted(int)));

        connect(d->searchBar, SIGNAL(completerActivated()),
                this, SIGNAL(completerActivated()));
    }

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            d->albumTreeView, SLOT(slotNewAlbum()));

    if (completerSelect)
    {
        d->searchBar->setFocus();
    }
}

AlbumSelectWidget::~AlbumSelectWidget()
{
    d->albumTreeView->saveState();
    d->searchBar->saveState();
    delete d;
}

PAlbum* AlbumSelectWidget::currentAlbum() const
{
    return d->albumTreeView->currentPAlbum();
}

void AlbumSelectWidget::setCurrentAlbum(PAlbum* const albumToSelect)
{
    d->albumTreeView->setCurrentAlbums(QList<Album*>() << albumToSelect);
}

QUrl AlbumSelectWidget::currentAlbumUrl() const
{
    const PAlbum* const palbum = d->albumTreeView->currentPAlbum();

    if (palbum)
    {
        return palbum->fileUrl();
    }

    return QUrl();
}

void AlbumSelectWidget::setCurrentAlbumUrl(const QUrl& albumUrl)
{
    PAlbum* const urlAlbum = AlbumManager::instance()->findPAlbum(albumUrl);     // cppcheck-suppress constVariablePointer

    if (urlAlbum)
    {
        d->albumTreeView->setCurrentAlbums(QList<Album*>() << urlAlbum);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot find an album for " << albumUrl;
    }
}

void AlbumSelectWidget::slotAlbumRenamed(Album* album)
{
    if (!album || (album->type() != Album::PHYSICAL))
    {
        return;
    }

    QModelIndex index = d->albumModel->indexForAlbum(album);

    if (index.isValid())
    {
        slotSelectionChanged();
    }
}

void AlbumSelectWidget::slotCompleterHighlighted(int albumId)
{
    PAlbum* const album = AlbumManager::instance()->findPAlbum(albumId);

    if (album)
    {
        d->complAlbum = album;
        QTimer::singleShot(0, this, SLOT(slotCompleterTimer()));
    }
}

void AlbumSelectWidget::slotCompleterTimer()
{
    if (d->complAlbum)
    {
        d->albumTreeView->setCurrentAlbums(QList<Album*>() << d->complAlbum);
    }
}

void AlbumSelectWidget::slotSelectionChanged()
{
    bool enabled = !(!currentAlbum() || currentAlbum()->isRoot());
    d->newAlbumBtn->setEnabled(enabled);

    Q_EMIT itemSelectionChanged();
}

} // namespace Digikam

#include "moc_albumselectwidget.cpp"

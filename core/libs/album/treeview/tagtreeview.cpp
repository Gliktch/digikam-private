/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagtreeview.h"

// Local includes

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

TagTreeView::TagTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags)
{
    m_modificationHelper = new TagModificationHelper(this, this);
    setRootIsDecorated(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(AUTOEXPANDDELAY);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new TagModel(TagModel::IncludeRootAlbum, this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setTAlbumFilterModel(new TagPropertiesFilterModel(this), checkableAlbumFilterModel());
    }
}

void TagTreeView::setTAlbumFilterModel(TagPropertiesFilterModel* const _filteredModel, CheckableAlbumFilterModel* const _filterModel)
{
    m_filteredModel = _filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(_filterModel);

    // hook in: source album model -> filtered model -> album filter model

    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

void TagTreeView::setTAlbumModel(TagModel* const model)
{
    // changing model is not implemented

    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceAlbumModel(model);
    }

    m_dragDropHandler = albumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new TagDragDropHandler(talbumModel());
        albumModel()->setDragDropHandler(m_dragDropHandler);

        connect(albumModel()->dragDropHandler(), SIGNAL(assignTags(QList<qlonglong>,QList<int>)),
                FileActionMngr::instance(), SLOT(assignTags(QList<qlonglong>,QList<int>)));
    }

    if (m_albumModel && (m_albumModel->rootAlbumBehavior() == AbstractAlbumModel::IncludeRootAlbum))
    {
        setRootIsDecorated(false);
    }

    if (m_albumFilterModel)
    {
        expand(m_albumFilterModel->rootAlbumIndex());
    }
}

TagModel* TagTreeView::talbumModel() const
{
    return static_cast<TagModel*>(m_albumModel);
}

TagPropertiesFilterModel* TagTreeView::filteredModel() const
{
    return m_filteredModel;
}

TAlbum* TagTreeView::currentAlbum() const
{
    return dynamic_cast<TAlbum*>(m_albumFilterModel->albumForIndex(currentIndex()));
}

QList<Album*> TagTreeView::selectedTags()
{
    return selectedAlbums<Album>(selectionModel(), m_filteredModel);
}

QList<TAlbum*> TagTreeView::selectedTagAlbums()
{
    return selectedAlbums<TAlbum>(selectionModel(), m_filteredModel);
}

TAlbum* TagTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<TAlbum*>(m_albumFilterModel->albumForIndex(index));
}

TagModificationHelper* TagTreeView::tagModificationHelper() const
{
    return m_modificationHelper;
}

void TagTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    TAlbum* const album = AlbumManager::instance()->findTAlbum(albumId);    // cppcheck-suppress constVariablePointer
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam

#include "moc_tagtreeview.cpp"

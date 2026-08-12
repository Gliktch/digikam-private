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

#include "searchtreeview.h"

// Local includes

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

SearchTreeView::SearchTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags)
{
    setRootIsDecorated(false);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new SearchModel(this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setSAlbumFilterModel(new SearchFilterModel(this), checkableAlbumFilterModel());
    }
}

void SearchTreeView::setSAlbumModel(SearchModel* const model)
{
    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceSearchModel(model);
    }
}

SearchModel* SearchTreeView::salbumModel() const
{
    return static_cast<SearchModel*>(m_albumModel);
}

void SearchTreeView::setSAlbumFilterModel(SearchFilterModel* const _filteredModel, CheckableAlbumFilterModel* const _filterModel)
{
    m_filteredModel = _filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(_filterModel);

    // hook in: source album model -> filtered model -> album filter model

    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

SearchFilterModel* SearchTreeView::filteredModel() const
{
    return m_filteredModel;
}

SAlbum* SearchTreeView::currentAlbum() const
{
    return dynamic_cast<SAlbum*>(m_albumFilterModel->albumForIndex(currentIndex()));
}

void SearchTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    SAlbum* const album = AlbumManager::instance()->findSAlbum(albumId);     // cppcheck-suppress constVariablePointer
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam

#include "moc_searchtreeview.cpp"

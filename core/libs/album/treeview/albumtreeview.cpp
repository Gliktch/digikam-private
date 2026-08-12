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

#include "albumtreeview.h"

// Local includes

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

AlbumTreeView::AlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setRootIsDecorated(false);
    setDropIndicatorShown(false);
    setAutoExpandDelay(AUTOEXPANDDELAY);

    if (flags & CreateDefaultModel)
    {
        setPAlbumModel(new AlbumModel(AlbumModel::IncludeRootAlbum, this));
    }
}

void AlbumTreeView::setPAlbumModel(AlbumModel* const model)
{
    // changing model is not implemented

    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    m_dragDropHandler = palbumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new AlbumDragDropHandler(palbumModel());

        model->setDragDropHandler(m_dragDropHandler);
    }
}

void AlbumTreeView::setCheckableAlbumFilterModel(CheckableAlbumFilterModel* const filterModel)
{
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
}

AlbumModel* AlbumTreeView::palbumModel() const
{
    return dynamic_cast<AlbumModel*>(m_albumModel);
}

PAlbum* AlbumTreeView::currentPAlbum() const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

PAlbum* AlbumTreeView::palbumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void AlbumTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    PAlbum* const album = AlbumManager::instance()->findPAlbum(albumId);     // cppcheck-suppress constVariablePointer
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam

#include "moc_albumtreeview.cpp"

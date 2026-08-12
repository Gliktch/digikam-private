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

#include "datetreeview.h"

// Local includes

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

DateTreeView::DateTreeView(QWidget* const parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags | AlwaysShowInclusiveCounts)
{
    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new DateAlbumModel(this));
    }
}

void DateTreeView::setDAlbumModel(DateAlbumModel* const model)
{
    AbstractCountingAlbumTreeView::setAlbumModel(model);
}

DateAlbumModel* DateTreeView::dalbumModel() const
{
    return static_cast<DateAlbumModel*>(m_albumModel);
}

void DateTreeView::setAlbumFilterModel(AlbumFilterModel* const filterModel)
{
    AbstractCountingAlbumTreeView::setAlbumFilterModel(filterModel);
}

DAlbum* DateTreeView::currentAlbum() const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

DAlbum* DateTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void DateTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    DAlbum* const album = AlbumManager::instance()->findDAlbum(albumId);     // cppcheck-suppress constVariablePointer
    setCurrentAlbums(QList<Album*>() << album, selectInAlbumManager);
}

} // namespace Digikam

#include "moc_datetreeview.cpp"

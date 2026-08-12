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

#include "abstractcountingalbumtreeview.h"

// Local includes

#include "abstractalbumtreeview_p.h"

namespace Digikam
{

AbstractCountingAlbumTreeView::AbstractCountingAlbumTreeView(QWidget* const parent, Flags flags)
    : AbstractAlbumTreeView(parent, flags & ~CreateDefaultFilterModel)
{
    if (flags & CreateDefaultFilterModel)
    {
        this->setAlbumFilterModel(new AlbumFilterModel(this));
    }

    if (!(flags & AlwaysShowInclusiveCounts))
    {
        connect(this, SIGNAL(expanded(QModelIndex)),
                this, SLOT(slotExpanded(QModelIndex)));

        connect(this, SIGNAL(collapsed(QModelIndex)),
                this, SLOT(slotCollapsed(QModelIndex)));
    }

    if (flags & ShowCountAccordingToSettings)
    {
        connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
                this, SLOT(setShowCountFromSettings()));
    }
}

void AbstractCountingAlbumTreeView::setAlbumModel(AbstractCountingAlbumModel* const model)
{
    AbstractAlbumTreeView::setAlbumModel(model);

    if (m_flags & ShowCountAccordingToSettings)
    {
        setShowCountFromSettings();
    }
}

void AbstractCountingAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* const filterModel)
{
    AbstractAlbumTreeView::setAlbumFilterModel(filterModel);

    // Initialize expanded/collapsed showCount state

    updateShowCountState(QModelIndex(), true);
}

void AbstractCountingAlbumTreeView::updateShowCountState(const QModelIndex& index, bool recurse)
{
    if (isExpanded(index) && !(m_flags & AlwaysShowInclusiveCounts))
    {
        slotExpanded(index);
    }
    else
    {
        slotCollapsed(index);
    }

    if (recurse)
    {
        const int rows = m_albumFilterModel->rowCount(index);

        for (int i = 0 ; i < rows ; ++i)
        {
            updateShowCountState(m_albumFilterModel->index(i, 0, index), true);
        }
    }
}

void AbstractCountingAlbumTreeView::slotCollapsed(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->includeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::slotExpanded(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->excludeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::setShowCountFromSettings()
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->setShowCount(ApplicationSettings::instance()->getShowFolderTreeViewItemsCount());
}

void AbstractCountingAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    AbstractAlbumTreeView::rowsInserted(parent, start, end);

    // initialize showCount state when items are added

    for (int i = start ; i <= end ; ++i)
    {
        updateShowCountState(m_albumFilterModel->index(i, 0, parent), false);
    }
}

} // namespace Digikam

#include "moc_abstractcountingalbumtreeview.cpp"

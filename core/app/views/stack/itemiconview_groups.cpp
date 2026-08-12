/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : Item icon view interface - Group methods.
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemiconview_p.h"

namespace Digikam
{

bool ItemIconView::allNeedGroupResolving(const OperationType type) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            return d->tableView->allNeedGroupResolving(type);
        }

        case StackedView::MapWidgetMode:
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::IconViewMode:
        {
            // all of these modes use the same selection model and data as the IconViewMode

            return d->iconView->allNeedGroupResolving(type);
        }

        default:
        {
            return false;
        }
    }
}

bool ItemIconView::selectedNeedGroupResolving(const OperationType type) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
        {
            return d->tableView->selectedNeedGroupResolving(type);
        }

        case StackedView::MapWidgetMode:
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::IconViewMode:
        {
            // all of these modes use the same selection model and data as the IconViewMode

            return d->iconView->selectedNeedGroupResolving(type);
        }

        default:
        {
            return false;
        }
    }
}

void ItemIconView::setAllGroupsOpen(bool open)
{
    ApplicationSettings::instance()->setAllGroupsOpen(open);

    if (!d->iconView->isPeopleViewMode())
    {
        d->iconView->itemFilterModel()->setAllGroupsOpen(open);
    }
}

void ItemIconView::slotCreateGroupFromSelection()
{
    FileActionMngr::instance()->addToGroup(currentInfo(), selectedInfoList(false, true));
}

void ItemIconView::slotCreateGroupByTimeFromSelection()
{
    d->utilities->createGroupByTimeFromInfoList(selectedInfoList(false, true));
}

void ItemIconView::slotCreateGroupByFilenameFromSelection()
{
    d->utilities->createGroupByFilenameFromInfoList(selectedInfoList(false, true));
}

void ItemIconView::slotCreateGroupByTimelapseFromSelection()
{
    d->utilities->createGroupByTimelapseFromInfoList(selectedInfoList(false, true));
}

void ItemIconView::slotRemoveSelectedFromGroup()
{
    FileActionMngr::instance()->removeFromGroup(selectedInfoList(false, true));
}

void ItemIconView::slotUngroupSelected()
{
    FileActionMngr::instance()->ungroup(selectedInfoList(false, true));
}

} // namespace Digikam

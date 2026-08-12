/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class - Groups methods.
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "contextmenuhelper_p.h"

namespace Digikam
{

void ContextMenuHelper::addGroupMenu(const imageIds& ids, const QList<QAction*>& extraMenuItems)
{
    QList<QAction*> actions = groupMenuActions(ids);

    if (actions.isEmpty() && extraMenuItems.isEmpty())
    {
        return;
    }

    if (!extraMenuItems.isEmpty())
    {
        if (!actions.isEmpty())
        {
            QAction* const separator = new QAction(this);
            separator->setSeparator(true);
            actions << separator;
        }

        actions << extraMenuItems;
    }

    QMenu* const menu = new QMenu(i18nc("@action: group items", "Group"));

    for (QAction* const action : std::as_const(actions))
    {
        menu->addAction(action);
    }

    d->parent->addMenu(menu);
}

void ContextMenuHelper::addGroupActions(const imageIds& ids)
{
    const auto acs = groupMenuActions(ids);

    for (QAction* const action : acs)
    {
        d->parent->addAction(action);
    }
}

QList<QAction*> ContextMenuHelper::groupMenuActions(const imageIds& ids)
{
    setSelectedIds(ids);

    QList<QAction*> actions;

    if (ids.isEmpty())
    {
        if (d->imageFilterModel)
        {
            if (!d->imageFilterModel->isAllGroupsOpen())
            {
                QAction* const openAction = new QAction(i18nc("@action:inmenu", "Open All Groups"), this);
                connect(openAction, SIGNAL(triggered()),
                        this, SLOT(slotOpenGroups()));
                actions << openAction;
            }
            else
            {
                QAction* const closeAction = new QAction(i18nc("@action:inmenu", "Close All Groups"), this);
                connect(closeAction, SIGNAL(triggered()),
                        this, SLOT(slotCloseGroups()));
                actions << closeAction;
            }
        }

        return actions;
    }

    ItemInfo info(ids.first());

    if (ids.size() == 1)
    {
        if      (info.hasGroupedImages())
        {
            if (d->imageFilterModel)
            {
                if (!d->imageFilterModel->isGroupOpen(info.id()))
                {
                    QAction* const action = new QAction(i18nc("@action:inmenu", "Show Grouped Images"), this);
                    connect(action, SIGNAL(triggered()),
                            this, SLOT(slotOpenGroups()));
                    actions << action;
                }
                else
                {
                    QAction* const action = new QAction(i18nc("@action:inmenu", "Hide Grouped Images"), this);
                    connect(action, SIGNAL(triggered()),
                            this, SLOT(slotCloseGroups()));
                    actions << action;
                }
            }

            QAction* const separator = new QAction(this);
            separator->setSeparator(true);
            actions << separator;

            QAction* const clearAction = new QAction(i18nc("@action:inmenu", "Ungroup"), this);
            connect(clearAction, SIGNAL(triggered()),
                    this, SIGNAL(signalUngroup()));
            actions << clearAction;
        }
        else if (info.isGrouped())
        {
            QAction* const action = new QAction(i18nc("@action:inmenu", "Remove From Group"), this);
            connect(action, SIGNAL(triggered()),
                    this, SIGNAL(signalRemoveFromGroup()));
            actions << action;

            // TODO: set as group leader / pick image
        }
    }
    else
    {
        QAction* const closeAction = new QAction(i18nc("@action:inmenu", "Group Selected Here"), this);
        connect(closeAction, SIGNAL(triggered()),
                this, SIGNAL(signalCreateGroup()));
        actions << closeAction;

        QAction* const closeActionDate = new QAction(i18nc("@action:inmenu", "Group Selected By Time"), this);
        connect(closeActionDate, SIGNAL(triggered()),
                this, SIGNAL(signalCreateGroupByTime()));
        actions << closeActionDate;

        QAction* const closeActionType = new QAction(i18nc("@action:inmenu", "Group Selected By Filename"), this);
        connect(closeActionType, SIGNAL(triggered()),
                this, SIGNAL(signalCreateGroupByFilename()));
        actions << closeActionType;

        QAction* const closeActionTimelapse = new QAction(i18nc("@action:inmenu", "Group Selected By Timelapse / Burst"), this);
        connect(closeActionTimelapse, SIGNAL(triggered()),
                this, SIGNAL(signalCreateGroupByTimelapse()));
        actions << closeActionTimelapse;

        QAction* const separator = new QAction(this);
        separator->setSeparator(true);
        actions << separator;

        if (d->imageFilterModel)
        {
            QAction* const openAction   = new QAction(i18nc("@action:inmenu", "Show Grouped Images"), this);
            connect(openAction, SIGNAL(triggered()),
                    this, SLOT(slotOpenGroups()));
            actions << openAction;

            QAction* const hideAction = new QAction(i18nc("@action:inmenu", "Hide Grouped Images"), this);
            connect(hideAction, SIGNAL(triggered()),
                    this, SLOT(slotCloseGroups()));
            actions << hideAction;

            QAction* const separator2   = new QAction(this);
            separator2->setSeparator(true);
            actions << separator2;
        }

        QAction* const removeAction = new QAction(i18nc("@action:inmenu", "Remove Selected From Groups"), this);
        connect(removeAction, SIGNAL(triggered()),
                this, SIGNAL(signalRemoveFromGroup()));
        actions << removeAction;

        QAction* const clearAction = new QAction(i18nc("@action:inmenu", "Ungroup Selected"), this);
        connect(clearAction, SIGNAL(triggered()),
                this, SIGNAL(signalUngroup()));
        actions << clearAction;
    }

    return actions;
}

void ContextMenuHelper::setGroupsOpen(bool open)
{
    if (!d->imageFilterModel || d->selectedIds.isEmpty())
    {
        return;
    }

    GroupItemFilterSettings settings = d->imageFilterModel->groupItemFilterSettings();

    for (const qlonglong& id : std::as_const(d->selectedIds))
    {
        ItemInfo info(id);

        if (info.hasGroupedImages())
        {
            settings.setOpen(id, open);
        }
    }

    d->imageFilterModel->setGroupItemFilterSettings(settings);
}

void ContextMenuHelper::slotOpenGroups()
{
    setGroupsOpen(true);
}

void ContextMenuHelper::slotCloseGroups()
{
    setGroupsOpen(false);
}

void ContextMenuHelper::slotOpenAllGroups()
{
    if (!d->imageFilterModel)
    {
        return;
    }

    d->imageFilterModel->setAllGroupsOpen(true);
}

void ContextMenuHelper::slotCloseAllGroups()
{
    if (!d->imageFilterModel)
    {
        return;
    }

    d->imageFilterModel->setAllGroupsOpen(false);
}

} // namespace Digikam

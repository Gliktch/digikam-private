/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class - Tools methods.
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

void ContextMenuHelper::addOpenAndNavigateActions(const imageIds& ids, bool lightTable)
{
    if (lightTable)
    {
        setSelectedIds(ids);
        QAction* const openImageFile = new QAction(QIcon::fromTheme(QLatin1String("quickopen-file")),
                                                   i18nc("@action: file open dialog", "Open..."), this);
        addAction(openImageFile);

        connect(openImageFile, SIGNAL(triggered()),
                this, SLOT(slotOpenImageFile()));
    }
    else
    {
        addAction(QLatin1String("image_edit"));
        addAction(QLatin1String("move_selection_to_album"));
        addAction(QLatin1String("copy_selection_to"));
    }

    addServicesMenu(ItemInfoList(ids).toImageUrlList());

    // addServicesMenu() has stored d->selectedItems

    if (!d->selectedItems.isEmpty())
    {
        QAction* const openFileMngr = new QAction(QIcon::fromTheme(QLatin1String("folder-open")),
                                                  i18nc("@action: context menu", "Open in File Manager"), this);
        addAction(openFileMngr);

        connect(openFileMngr, SIGNAL(triggered()),
                this, SLOT(slotOpenInFileManager()));
    }

    if (!lightTable)
    {
        addGotoMenu(ids);
    }
}

void ContextMenuHelper::slotOpenImageFile()
{
    if (d->selectedIds.isEmpty())
    {
        return;
    }

    ItemInfoList infos = ItemInfoList(d->selectedIds);
    ItemViewUtilities(d->parent).openInfos(infos.first(), infos, nullptr);
}

void ContextMenuHelper::addImportMenu()
{
    QMenu* const menuImport       = new QMenu(i18nc("@action: context menu", "Import"), d->parent);
    KXMLGUIClient* const client   = const_cast<KXMLGUIClient*>(d->stdActionCollection->parentGUIClient());
    QList<DPluginAction*> actions = DPluginLoader::instance()->pluginsActions(DPluginAction::GenericImport,
                                    dynamic_cast<KXmlGuiWindow*>(client));

    if (!actions.isEmpty())
    {
        for (DPluginAction* const ac : std::as_const(actions))  // cppcheck-suppress constVariablePointer
        {
            menuImport->addActions(QList<QAction*>() << ac);
        }
    }
    else
    {
        QAction* const notools = new QAction(i18nc("@action: context menu", "No import tool available"), this);
        notools->setEnabled(false);
        menuImport->addAction(notools);
    }

    d->parent->addMenu(menuImport);
}

void ContextMenuHelper::addExportMenu()
{
    QMenu* const menuExport              = new QMenu(i18nc("@action: context menu", "Export"), d->parent);
    const QList<DPluginAction*>& actions = d->exportPluginActions();

#if 0

    QAction* selectAllAction      = nullptr;
    selectAllAction               = d->stdActionCollection->action("selectAll");

#endif

    if (!actions.isEmpty())
    {
        for (DPluginAction* const ac : std::as_const(actions))
        {
            ac->setData((int)DPluginAction::AlbumData);
            menuExport->addActions(QList<QAction*>() << ac);
        }
    }
    else
    {
        QAction* const notools = new QAction(i18nc("@action: context menu", "No export tool available"), this);
        notools->setEnabled(false);
        menuExport->addAction(notools);
    }

    d->parent->addMenu(menuExport);
}

void ContextMenuHelper::addQueueManagerMenu()
{
    QMenu* const bqmMenu = new QMenu(i18nc("@action: context menu", "Batch Queue Manager"), d->parent);
    bqmMenu->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("run-build")));
    bqmMenu->addAction(d->stdActionCollection->action(QLatin1String("image_add_to_current_queue")));
    bqmMenu->addAction(d->stdActionCollection->action(QLatin1String("image_add_to_new_queue")));

    // if queue list is empty, do not display the queue submenu

    if (
        QueueMgrWindow::queueManagerWindowCreated() &&
        !QueueMgrWindow::queueManagerWindow()->queuesMap().isEmpty()
       )
    {
        const QueueMgrWindow* const qmw = QueueMgrWindow::queueManagerWindow();
        QMenu* const queueMenu          = new QMenu(i18nc("@action: context menu", "Add to Existing Queue"), bqmMenu);

        // queueActions is used by the exec() method to Q_EMIT an appropriate signal.
        // Reset the map before filling in the actions.

        if (!d->queueActions.isEmpty())
        {
            d->queueActions.clear();
        }

        QList<QAction*> queueList;

        // get queue list from BQM window, do not access it directly, it might crash
        // when the list is changed

        QMap<int, QString> qmwMap = qmw->queuesMap();

        for (QMap<int, QString>::const_iterator it = qmwMap.constBegin() ;
             it != qmwMap.constEnd() ; ++it)
        {
            QAction* const action     = new QAction(it.value(), this);
            queueList << action;
            d->queueActions[it.key()] = action;
        }

        queueMenu->addActions(queueList);
        bqmMenu->addMenu(queueMenu);
    }

    d->parent->addMenu(bqmMenu);

    // NOTE: see bug #252130 : we need to disable new items to add on BQM is this one is running.

    bqmMenu->setDisabled(QueueMgrWindow::queueManagerWindow()->isBusy());
}

} // namespace Digikam

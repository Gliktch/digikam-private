/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>
#include <QList>

// Local includes

#include "tagtreeview.h"
#include "album.h"

namespace Digikam
{

class ContextMenuHelper;

class TagFolderView: public TagTreeView
{
    Q_OBJECT

public:

    /**
     * @brief Constructor.
     *
     * @param parent the parent for Qt's parent child mechanism
     * @param model tag model to display
     */
    TagFolderView(QWidget* const parent, TagModel* const model);

    /**
     * @brief Destructor.
     */
    ~TagFolderView()                                                                        override;

    /**
     * @brief Define whether to show the "find duplicate" action in context menus
     * or not.
     *
     * @param show if <code>true</code> the action to find duplicate images in
     *             the tag album is displayed
     */
    void setShowFindDuplicateAction(bool show);

    /**
     * @brief Define whether to show the "Delete People Tags" action in context menus
     * or not.
     *
     * @param show if <code>true</code> the action to delete people tags in
     *             the tag album is displayed
     */
    void setShowDeleteFaceTagsAction(bool show);

    /**
     * @brief Open tag for editing.
     */
    void tagPropsEdit();

Q_SIGNALS:

    void signalFindDuplicates(const QList<TAlbum*>& albums);

protected:

    QString contextMenuTitle()                                                        const override;

    /**
     * @brief Hook method to add custom actions to the generated context menu.
     *
     * The default implementation adds actions to reset the tag icon and to
     * find duplicates in a tag album. If you want to use these actions,
     * remember to call this class' implementation of this method and
     * the handleCustomContextMenuAction in your derived class.
     *
     * @param cmh helper object to create the context menu
     * @param album tag on which the context menu will be created. May be null if
     *              it is requested on no tag entry
     */
    void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)                  override;

    /**
     * @brief Hook method to handle the custom context menu actions that were added
     * with addCustomContextMenuActions.
     *
     * @param action the action that was chosen by the user, may be null if none
     *               of the custom actions were selected
     * @param album the tag on which the context menu was requested. May be null
     *              if there was no
     */
    void handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album)   override;

    /**
     * @brief Reimplement contextMenuEvent from AbstractAlbumTree to support multiple
     * selection
     *
     * @param event context menu event triggered by right click
     */
    void contextMenuEvent(QContextMenuEvent* event)                                         override;

    /**
     * @brief Implementation of AddCustomContextMenuActions(see above) that handle
     * multiple selection. If only one element is selected, only
     * AddCustomContextMenuActions is called
     *
     * @param cmh       - helper object to create context menu
     * @param albums    - vector of selected albums to be used on menu actions
     */
    virtual void setContexMenuItems(ContextMenuHelper& cmh, const QList<TAlbum*>& albums);

    void keyPressEvent(QKeyEvent* event)                                                    override;

public Q_SLOTS:

    void slotTagNewFromABCMenu(const QString& personName);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-05
 * Description : Tag Manager Tree View derived from TagsFolderView to implement
 *               a custom context menu and some batch view options, such as
 *               expanding multiple items
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "album.h"
#include "tagfolderview.h"

namespace Digikam
{

class TagsManager;

class TagMngrTreeView : public TagFolderView
{
    Q_OBJECT

public:

    explicit TagMngrTreeView(TagsManager* const parent, TagModel* const model);
    ~TagMngrTreeView() override;

    void setAlbumFilterModels(TagsManagerFilterModel* const filteredModel,
                              CheckableAlbumFilterModel* const filterModel);

    TagsManagerFilterModel* getFilterModel() const
    {
        return m_tfilteredModel;
    }

protected:

    /**
     * @brief reimplemented method from TagsFolderView.
     *        Will set custom actions for Tags Manager.
     *        Some actions are also available in toolbar.
     *
     * @param cmh    ContextMenuHelper class to help setting some basic actions.
     * @param albums list of currently selected albums.
     */
    void setContexMenuItems(ContextMenuHelper& cmh, const QList<TAlbum*>& albums) override;

    /**
     * @brief reimplement contextMenuEvent from AbstractAlbumTree
     *        to support multiple selection.
     *
     * @param event context menu event triggered by right click.
     */
    void contextMenuEvent(QContextMenuEvent* event) override;

protected:

    TagsManagerFilterModel* m_tfilteredModel = nullptr;

private:

    /// @note disabled
    TagMngrTreeView(QWidget*) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

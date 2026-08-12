/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi at pooh dot tam dot uiuc dot edu>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editablesearchtreeview.h"

namespace Digikam
{

/**
 * Tree view for all saved "normal" searches. Allows editing and creating
 * searches in the context menu.
 *
 * @author jwienke
 */
class NormalSearchTreeView : public EditableSearchTreeView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent qt parent
     * @param searchModel the model this view should act on
     * @param searchModificationHelper the modification helper object used to
     *                                 perform operations on the displayed
     *                                 searches
     */
    NormalSearchTreeView(QWidget* const parent, SearchModel* const searchModel,
                         SearchModificationHelper* const searchModificationHelper);

    /**
     * Destructor.
     */
    ~NormalSearchTreeView() override;

Q_SIGNALS:

    /**
     * Emitted of a new search shall be created.
     */
    void newSearch();

    /**
     * Emitted if the given search shall be copied.
     *
     * @param album search to copy
     */
    void copySearch(SAlbum* album);

    /**
     * Emitted if the given search shall be edited.
     *
     * @param album search to edit
     */
    void editSearch(SAlbum* album);

protected:

    void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)                override;
    void handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

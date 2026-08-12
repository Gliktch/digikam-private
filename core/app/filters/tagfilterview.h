/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : filter view for the right sidebar
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "tagcheckview.h"

namespace Digikam
{

class TagModel;

/**
 * A view to filter the currently displayed album by tags.
 *
 * @author jwienke
 */
class TagFilterView : public TagCheckView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent for qt parent child mechanism
     * @param tagFilterModel tag model to work on
     */
    explicit TagFilterView(QWidget* const parent, TagModel* const tagFilterModel);

    /**
     * Destructor.
     */
    ~TagFilterView()                                                                      override;

private Q_SLOTS:

    void slotDeleteTagByContextMenu(TAlbum* tag);

protected:

    void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)                override;
    void handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // nameSpace Digikam

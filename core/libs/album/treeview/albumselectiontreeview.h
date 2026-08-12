/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeView>

// Local includes

#include "albummodel.h"
#include "albumtreeview.h"
#include "albummodificationhelper.h"

namespace Digikam
{

/**
 * @brief Album tree view used in the left sidebar to select PAlbums and perform
 * operations on them via a context menu.
 */
class AlbumSelectionTreeView: public AlbumTreeView
{
    Q_OBJECT

public:

    AlbumSelectionTreeView(QWidget* const parent,
                           AlbumModel* const model,
                           AlbumModificationHelper* const albumModificationHelper);
    ~AlbumSelectionTreeView()         override;

    /**
     * @brief Sets whether this widget shall display tool tips or not.
     */
    void setEnableToolTips(bool enable);

Q_SIGNALS:

    /**
     * @brief Emitted if a find duplicates search shall be invoked on the given album.
     *
     * @param albums the album to find duplicates in
     */
    void signalFindDuplicates(const QList<PAlbum*>& albums);

private Q_SLOTS:

    void slotFindDuplicates();
    void slotScanForFaces();
    void slotRepairHiddenItems();
    void slotRebuildThumbs();

private:

    /**
     * @brief Re-implemented to handle custom tool tips.
     *
     * @param event the event to process.
     */
    bool viewportEvent(QEvent* event) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

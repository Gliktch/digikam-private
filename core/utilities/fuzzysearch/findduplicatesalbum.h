/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates tree-view search album.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QWidget>
#include <QPixmap>
#include <QTreeWidget>

// Local includes

#include "album.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

/*!
 * @brief The FindDuplicatesAlbum class
 * Widgets used to show all reference images.
 */
class FindDuplicatesAlbum : public QTreeWidget
{
    Q_OBJECT

public:

    explicit FindDuplicatesAlbum(QWidget* const parent = nullptr);
    ~FindDuplicatesAlbum()                       override;

    void updateDuplicatesAlbumItems(const QList<SAlbum*>& sAlbumsToRebuild,
                                    const QList<qlonglong>& deletedImages);

    void selectFirstItem();
    QTreeWidgetItem* firstItem();
    void removeDuplicates();

private:

    void drawRow(QPainter* p,
                 const QStyleOptionViewItem& opt,
                 const QModelIndex& index) const override;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

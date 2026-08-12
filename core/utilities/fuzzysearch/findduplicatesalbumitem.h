/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-06-17
 * Description : Find Duplicates tree-view search album item.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTreeWidget>
#include <QUrl>
#include <QList>
#include <QMutex>
#include <QFuture>

// Local includes

#include "album.h"
#include "iteminfo.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT FindDuplicatesAlbumItem : public QObject,
                                                   public QTreeWidgetItem
{
    Q_OBJECT

public:

    enum Column
    {
        REFERENCE_IMAGE = 0,
        REFERENCE_DATE,
        REFERENCE_ALBUM,
        RESULT_COUNT,
        AVG_SIMILARITY,

        NUMBER_COLUMNS          ///< Must be the last one.
    };

public:

    explicit FindDuplicatesAlbumItem(QTreeWidget* const parent, SAlbum* const album);
    ~FindDuplicatesAlbumItem()                         override;

    bool hasValidThumbnail()                     const;

    /**
     * @brief Calculates the duplicates count and average similarity.
     */
    void calculateInfos(const QList<qlonglong>& deletedImages = QList<qlonglong>());

    /**
     * @brief Waits until calculateInfos() is finished.
     */
    void waitForCalculate();

    /**
     * @return The item count.
     */
    int itemCount()                              const;

    SAlbum* album()                              const;
    QUrl    refUrl()                             const;

    bool operator<(const QTreeWidgetItem& other) const override;

    QList<ItemInfo> duplicatedItems();

    void setThumb(const QPixmap& pix, bool hasThumb = true);

Q_SIGNALS:

    void signalUpdateItemText(int column, const QString& text);
    void signalSetItemHidden(bool hidden);

private:

    void calculateInfosMultithreaded(const QList<qlonglong>& deletedImages);

private:

    class Private;
    Private* const d = nullptr;

private:

    Q_DISABLE_COPY(FindDuplicatesAlbumItem)
};

} // namespace Digikam

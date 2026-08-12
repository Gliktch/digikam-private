/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with database job
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "itemthumbnailmodel.h"
#include "album.h"
#include "itemlisterrecord.h"

namespace Digikam
{

class ImageChangeset;
class CollectionImageChangeset;
class SearchChangeset;
class Album;

class ItemAlbumModel : public ItemThumbnailModel
{
    Q_OBJECT

public:

    explicit ItemAlbumModel(QWidget* const parent);
    ~ItemAlbumModel() override;

    QList<Album*> currentAlbums()       const;

    bool hasScheduledRefresh()          const;
    bool isRecursingAlbums()            const;
    bool isRecursingTags()              const;
    bool isListingOnlyAvailableImages() const;

public Q_SLOTS:

    /**
     * Call this method to populate the model with data from the given album.
     * If called with 0, the model will be empty.
     * Opening the same album again is a no-op.
     */
    void openAlbum(const QList<Album*>& albums);

    /**
     * Reloads the current album
     */
    void refresh();

    void setRecurseAlbums(bool recursiveListing);
    void setRecurseTags(bool recursiveListing);
    void setListOnlyAvailableImages(bool onlyAvailable);

    void setSpecialTagListing(const QString& specialListing);
/*
Q_SIGNALS:

    void listedAlbumChanged(QList<Album*> album);
*/
protected Q_SLOTS:

    void scheduleIncrementalRefresh();
    void slotNextIncrementalRefresh();

    void slotResult();
    void slotData(const QList<ItemListerRecord>& records);

    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
    void slotSearchChange(const SearchChangeset& changeset);

    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotAlbumRenamed(Album* album);
    void slotAlbumsCleared();

    void incrementalRefresh();

    void slotImageChange(const ImageChangeset& changeset) override;
    void slotImageTagChange(const ImageTagChangeset& changeset) override;

protected:

    void startListJob(const QList<Album*>& albums);

private:

    /// @note disabled
    ItemAlbumModel(const ItemAlbumModel&)            = delete;
    ItemAlbumModel& operator=(const ItemAlbumModel&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

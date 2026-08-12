/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "abstractcheckablealbumtreeview.h"

namespace Digikam
{

class TagTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT

public:

    explicit TagTreeView(QWidget* const parent = nullptr, Flags flags = DefaultFlags);
    ~TagTreeView() override = default;

    TagModel* talbumModel()                         const;

    /**
     * @brief Contains only the tags filtered by properties - prefer to albumModel()
     */
    TagPropertiesFilterModel* filteredModel()       const;

    /**
     * @brief currentAlbum
     * Even if multiple selection is enabled current
     * Album can be only one, the last clicked item
     * if you need selected items, see selectedAlbums()
     * It's NOT the same as AlbumManager::currentAlbums()
     */
    TAlbum* currentAlbum() const;

    /**
     * @brief selectedTags - return a list of all selected items in tag model
     */
    QList<Album*>  selectedTags();
    QList<TAlbum*> selectedTagAlbums();

    TAlbum* albumForIndex(const QModelIndex& index) const;
    TagModificationHelper* tagModificationHelper()  const;

    void setTAlbumFilterModel(TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel);
    void setTAlbumModel(TagModel* const model);

public Q_SLOTS:

    void setCurrentAlbum(int tagId, bool selectInAlbumManager = true);

Q_SIGNALS:

    void assignTags(int tagId, const QList<int>& imageIDs);

protected:

    TagPropertiesFilterModel* m_filteredModel       = nullptr;
    TagModificationHelper*    m_modificationHelper  = nullptr;
};

} // namespace Digikam

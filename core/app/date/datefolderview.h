/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-27
 * Description : a folder view for date albums.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2014      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScopedPointer>

// Local includes

#include "dlayoutbox.h"
#include "albummanager.h"
#include "statesavingobject.h"

namespace Digikam
{

class Album;
class DAlbum;
class DateAlbumModel;
class ItemFilterModel;

template <class T>
class AlbumPointer;

class DateFolderView : public DVBox,
                       public StateSavingObject
{
    Q_OBJECT

public:

    explicit DateFolderView(QWidget* const parent, DateAlbumModel* const dateAlbumModel);
    ~DateFolderView()                               override;

    void setItemModel(ItemFilterModel* const model);

    void setActive(const bool val);

    void gotoDate(const QDate& dt);

    void changeAlbumFromHistory(DAlbum* const album);

    AlbumPointer<DAlbum> currentAlbum() const;

    void doLoadState()                              override;
    void doSaveState()                              override;

    void setConfigGroup(const KConfigGroup& group)  override;

private Q_SLOTS:

    void slotSelectionChanged(Album* selectedAlbum);
    void slotAllAlbumsLoaded();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

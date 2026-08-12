/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify physical albums in views
 *
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QWidget>

// Local includes

#include "album.h"

namespace Digikam
{

/**
 * Utility class providing methods to modify physical albums (PAlbum) in a way
 * useful to implement views.
 *
 * @author jwienke
 */
class AlbumModificationHelper : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent for qt parent child mechanism
     * @param dialogParent parent widget for dialogs displayed by this object
     */
    explicit AlbumModificationHelper(QObject* const parent, QWidget* const dialogParent);

    /**
     * Destructor.
     */
    ~AlbumModificationHelper() override;

    /**
     * Sets the album that the given action operates on.
     * You must call bindTag and then connect the action's triggered
     * to the desired slot, slotTagNew(), slotTagEdit() or slotTagDelete().
     * Note: Changes the Action's user data.
     */
    void bindAlbum(QAction* const action, PAlbum* const parent) const;

    /**
     * Returns the album bound with bindAlbum. The given QObject shall be
     * a QAction, but for convenience the given object
     * will be checked with qobject_cast first, so you can pass QObject::sender().
     */
    PAlbum* boundAlbum(QObject* const action) const;

public Q_SLOTS:

    /**
     * Creates a new album under the given parent. The user will be prompted for
     * the settings of the new album.
     *
     * @param parentAlbum parent album for the new one
     * @return the new album or 0 if no album was created
     */
    PAlbum* slotAlbumNew(PAlbum* parentAlbum);
    PAlbum* slotAlbumNew();

    /**
     * Deletes the given album after waiting for a graphical confirmation of the
     * user.
     *
     * @param album the album to delete
     */
    void slotAlbumDelete(PAlbum* album);
    void slotAlbumDelete();

    /**
     * Renames the given album. The user will be prompted for a new name.
     *
     * @param album the album to rename
     */
    void slotAlbumRename(PAlbum* album);
    void slotAlbumRename();

    /**
     * Graphically edits the properties of the given album.
     *
     * @param album the album to edit
     */
    void slotAlbumEdit(PAlbum* album);
    void slotAlbumEdit();

    void slotAlbumResetIcon(PAlbum* album);
    void slotAlbumResetIcon();

private:

    void addAlbumChildrenToList(QList<QUrl>& list, Album* const album);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

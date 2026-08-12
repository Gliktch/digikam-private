/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : a pop-up menu implementation to display a
 *               hierarchical view of digiKam tags.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QMenu>

namespace Digikam
{

class Album;
class TAlbum;

class TagsPopupMenu : public QMenu
{
    Q_OBJECT

public:

    enum Mode
    {
        ASSIGN = 0,
        REMOVE,
        DISPLAY,           ///< @note Used by "GoTo Tag" feature
        RECENTLYASSIGNED
    };

public:

    TagsPopupMenu(qlonglong selectedImageId, Mode mode, QWidget* const parent = nullptr);
    TagsPopupMenu(const QList<qlonglong>& selectedImageIDs, Mode mode, QWidget* const parent = nullptr);
    ~TagsPopupMenu() override;

Q_SIGNALS:

    void signalTagActivated(int id);
    void signalPopupTagsView();

private Q_SLOTS:

    void slotAboutToShow();
    void slotToggleTag(QAction*);
    void slotAddTag(QAction*);
    void slotTagThumbnail(Album*, const QPixmap&);

private:

    void   setup(Mode mode);
    void   clearPopup();
    QMenu* buildSubMenu(int tagid);
    void   iterateAndBuildMenu(QMenu* menu, TAlbum* album);
    void   buildFlatMenu(QMenu* menu);
    void   setAlbumIcon(QAction* action, TAlbum* album);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

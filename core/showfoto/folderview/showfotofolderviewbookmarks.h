/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view bookmark places
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QAction>
#include <QWidget>
#include <QString>
#include <QTreeWidgetItem>

class KConfigGroup;

namespace ShowFoto
{

class ShowfotoFolderViewSideBar;

class ShowfotoFolderViewBookmarks : public QWidget
{
    Q_OBJECT

public:

    explicit ShowfotoFolderViewBookmarks(ShowfotoFolderViewSideBar* const sidebar);
    ~ShowfotoFolderViewBookmarks()                     override;

    void saveSettings(KConfigGroup&);
    void readSettings(const KConfigGroup&);

    QTreeWidgetItem* topBookmarksItem()         const;

    QAction* toolBarAction(const QString& name) const;
    QList<QAction*> pluginActions()             const;

Q_SIGNALS:

    void signalLoadContents();

private Q_SLOTS:

    void slotLoadContents(const QString& path);
    void slotBookmarkActivated(QTreeWidgetItem*);
    void slotBookmarkSelectionChanged();
    void slotAddBookmark(const QString&);
    void slotAddBookmark();
    void slotDelBookmark();
    void slotEdtBookmark();

private:

    ShowfotoFolderViewBookmarks(QWidget*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto

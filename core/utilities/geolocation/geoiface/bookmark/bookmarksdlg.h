/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : Management dialogs for bookmarks
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>
#include <QTreeView>
#include <QComboBox>
#include <QAbstractItemModel>

// Local includes

#include "searchtextbar.h"
#include "bookmarksmngr.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT AddBookmarkDialog : public QDialog
{
    Q_OBJECT

public:

    explicit AddBookmarkDialog(const QString& url,
                               const QString& title,
                               QWidget* const parent = nullptr,
                               BookmarksManager* const mngr = nullptr);
    ~AddBookmarkDialog() override;

private Q_SLOTS:

    void accept()        override;          // cppcheck-suppress virtualCallInConstructor

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT BookmarksDialog : public QDialog
{
    Q_OBJECT

public:

    explicit BookmarksDialog(QWidget* const parent = nullptr,
                             BookmarksManager* const mngr = nullptr);
    ~BookmarksDialog()              override;

private Q_SLOTS:

    void slotCustomContextMenuRequested(const QPoint&);
    void slotOpenInMap(const QModelIndex&);
    void slotNewFolder();
    void slotRemoveOne();

    void accept() override;                 // cppcheck-suppress virtualCallInConstructor

protected:

    void showEvent(QShowEvent*)     override;
    void closeEvent(QCloseEvent*)   override;

private:

    void expandNodes(BookmarkNode* const node);
    bool saveExpandedNodes(const QModelIndex& parent);
    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : low level manager for bookmarks
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QAbstractItemModel>
#include <QUndoCommand>
#include <QSortFilterProxyModel>

// Local includes

#include "searchtextbar.h"

namespace Digikam
{

class BookmarksManager;
class BookmarkNode;

class RemoveBookmarksCommand : public QUndoCommand
{
public:

    explicit RemoveBookmarksCommand(BookmarksManager* const mngr,
                                    BookmarkNode* const parent,
                                    int row);
    ~RemoveBookmarksCommand() override;

    void undo()                override;
    void redo()                override;

protected:

    int               m_row             = 0;
    BookmarksManager* m_bookmarkManager = nullptr;
    BookmarkNode*     m_node            = nullptr;
    BookmarkNode*     m_parent          = nullptr;
    bool              m_done            = false;
};

//---------------------------------------------------------------------------------

class InsertBookmarksCommand : public RemoveBookmarksCommand
{
public:

    explicit InsertBookmarksCommand(BookmarksManager* const mngr,
                                    BookmarkNode* const parent,
                                    BookmarkNode* const node,
                                    int row);

    void undo() override;
    void redo() override;
};

//---------------------------------------------------------------------------------

class ChangeBookmarkCommand : public QUndoCommand
{
public:

    enum BookmarkData
    {
        Url = 0,
        Title,
        Desc
    };

public:

    explicit ChangeBookmarkCommand(BookmarksManager* const mngr,
                                   BookmarkNode* const node,
                                   const QString& newValue,
                                   BookmarkData type);
    ~ChangeBookmarkCommand()  override;

    void undo()               override;
    void redo()               override;

private:

    class Private;
    Private* const d = nullptr;
};

/**
 * BookmarksModel is a QAbstractItemModel wrapper around the BookmarkManager
 */
class BookmarksModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Roles
    {
        TypeRole      = Qt::UserRole + 1,
        UrlRole       = Qt::UserRole + 2,
        UrlStringRole = Qt::UserRole + 3,
        SeparatorRole = Qt::UserRole + 4,
        DateAddedRole = Qt::UserRole + 5
    };

public:

    explicit BookmarksModel(BookmarksManager* const mngr, QObject* const parent = nullptr);
    ~BookmarksModel()                                                                               override;

    BookmarksManager* bookmarksManager()                                                      const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const override;
    int columnCount(const QModelIndex& parent = QModelIndex())                                const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                   const override;
    QModelIndex index(int, int, const QModelIndex& = QModelIndex())                           const override;
    QModelIndex parent(const QModelIndex& index= QModelIndex())                               const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                             const override;
    Qt::DropActions supportedDropActions ()                                                   const override;
    QMimeData* mimeData(const QModelIndexList& indexes)                                       const override;
    QStringList mimeTypes()                                                                   const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex())                               const override;
    BookmarkNode* node(const QModelIndex& index)                                              const;
    QModelIndex index(BookmarkNode* node)                                                     const;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                      int column, const QModelIndex& parent)                                         override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex())                   override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)           override;

public Q_SLOTS:

    void entryAdded(BookmarkNode* item);
    void entryRemoved(BookmarkNode* parent, int row, BookmarkNode* item);
    void entryChanged(BookmarkNode* item);

private:

    class Private;
    Private* const d = nullptr;
};

/**
 *  Proxy model that filters out the bookmarks so only the folders
 *  are left behind.  Used in the add bookmark dialog combobox.
 */
class AddBookmarkProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit AddBookmarkProxyModel(QObject* const parent = nullptr);

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

protected:

    bool filterAcceptsRow(int srow, const QModelIndex& sparent) const override;
};

//---------------------------------------------------------------------------------

class TreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit TreeProxyModel(QObject* const parent = nullptr);

    int columnCount(const QModelIndex&) const                        override;

Q_SIGNALS:

    void signalFilterAccepts(bool);

protected:

    bool filterAcceptsRow(int srow, const QModelIndex& sparent) const override;

private:

    void emitResult(bool v);
};

/**
 *  Bookmark manager, owner of the bookmarks, loads, saves and basic tasks
 */
class BookmarksManager : public QObject
{
    Q_OBJECT

public:

    explicit BookmarksManager(const QString& bookmarksFile, QObject* const parent = nullptr);
    ~BookmarksManager() override;

    void addBookmark(BookmarkNode* const parent, BookmarkNode* const node, int row = -1);
    void removeBookmark(BookmarkNode* const node);
    void setTitle(BookmarkNode* const node, const QString& newTitle);
    void setUrl(BookmarkNode* const node, const QString& newUrl);
    void setComment(BookmarkNode* const node, const QString& newDesc);
    void changeExpanded();

    BookmarkNode*   bookmarks();
    BookmarksModel* bookmarksModel();
    QUndoStack*     undoRedoStack() const;

    void save();
    void load();

Q_SIGNALS:

    void entryAdded(BookmarkNode* item);
    void entryRemoved(BookmarkNode* parent, int row, BookmarkNode* item);
    void entryChanged(BookmarkNode* item);

public Q_SLOTS:

    void importBookmarks();
    void exportBookmarks();

private:

    class Private;
    Private* const d = nullptr;

    friend class RemoveBookmarksCommand;
    friend class ChangeBookmarkCommand;
};

} // namespace Digikam

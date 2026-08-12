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

#include "bookmarksmngr.h"

// Qt includes

#include <QBuffer>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QIcon>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QDebug>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dfiledialog.h"
#include "bookmarknode.h"
#include "digikam_debug.h"

namespace Digikam
{

RemoveBookmarksCommand::RemoveBookmarksCommand(BookmarksManager* const mngr,
                                               BookmarkNode* const parent,
                                               int row)
    : QUndoCommand(i18n("Remove Bookmark")),
      m_row            (row),
      m_bookmarkManager(mngr),
      m_node           (parent->children().value(row)),
      m_parent         (parent)
{
}

RemoveBookmarksCommand::~RemoveBookmarksCommand()
{
    if (m_done && !m_node->parent())
    {
        delete m_node;
    }
}

void RemoveBookmarksCommand::undo()
{
    m_parent->add(m_node, m_row);

    Q_EMIT m_bookmarkManager->entryAdded(m_node);

    m_done = false;
}

void RemoveBookmarksCommand::redo()
{
    m_parent->remove(m_node);

    Q_EMIT m_bookmarkManager->entryRemoved(m_parent, m_row, m_node);

    m_done = true;
}

// --------------------------------------------------------------

InsertBookmarksCommand::InsertBookmarksCommand(BookmarksManager* const mngr,
                                               BookmarkNode* const parent,
                                               BookmarkNode* const node,
                                               int row)
    : RemoveBookmarksCommand(mngr, parent, row)
{
    setText(i18n("Insert Bookmark"));
    m_node = node;
}

void InsertBookmarksCommand::undo()
{
    RemoveBookmarksCommand::redo();
}

void InsertBookmarksCommand::redo()
{
    RemoveBookmarksCommand::undo();
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN ChangeBookmarkCommand::Private
{
public:

    Private() = default;

public:

    BookmarksManager* manager   = nullptr;
    BookmarkData      type      = Url;
    QString           oldValue;
    QString           newValue;
    BookmarkNode*     node      = nullptr;
};

ChangeBookmarkCommand::ChangeBookmarkCommand(BookmarksManager* const mngr,
                                             BookmarkNode* const node,
                                             const QString& newValue,
                                             BookmarkData type)
    : QUndoCommand(),
      d           (new Private)
{
    d->manager  = mngr;
    d->type     = type;
    d->newValue = newValue;
    d->node     = node;

    switch (d->type)
    {
        case Title:
        {
            d->oldValue = d->node->title;
            setText(i18n("Title Change"));
            break;
        }

        case Desc:
        {
            d->oldValue = d->node->desc;
            setText(i18n("Comment Change"));
            break;
        }

        default:    // Url
        {
            d->oldValue = d->node->url;
            setText(i18n("Address Change"));
            break;
        }
    }
}

ChangeBookmarkCommand::~ChangeBookmarkCommand()
{
    delete d;
}

void ChangeBookmarkCommand::undo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title = d->oldValue;
            break;
        }

        case Desc:
        {
            d->node->desc  = d->oldValue;
            break;
        }

        default:    // Url
        {
            d->node->url   = d->oldValue;
            break;
        }
    }

    Q_EMIT d->manager->entryChanged(d->node);
}

void ChangeBookmarkCommand::redo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title = d->newValue;
            break;
        }

        case Desc:
        {
            d->node->desc  = d->newValue;
            break;
        }

        default:    // Url
        {
            d->node->url   = d->newValue;
            break;
        }
    }

    Q_EMIT d->manager->entryChanged(d->node);
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN BookmarksModel::Private
{
public:

    Private() = default;

    BookmarksManager* manager   = nullptr;
    bool              endMacro  = false;
};

BookmarksModel::BookmarksModel(BookmarksManager* const mngr, QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->manager = mngr;

    connect(d->manager, SIGNAL(entryAdded(BookmarkNode*)),
            this, SLOT(entryAdded(BookmarkNode*)));

    connect(d->manager, SIGNAL(entryRemoved(BookmarkNode*,int,BookmarkNode*)),
            this, SLOT(entryRemoved(BookmarkNode*,int,BookmarkNode*)));

    connect(d->manager, SIGNAL(entryChanged(BookmarkNode*)),
            this, SLOT(entryChanged(BookmarkNode*)));
}

BookmarksModel::~BookmarksModel()
{
    delete d;
}

BookmarksManager* BookmarksModel::bookmarksManager() const
{
    return d->manager;
}

QModelIndex BookmarksModel::index(BookmarkNode* node) const
{
    const BookmarkNode* const prnt = node->parent();

    if (!prnt)
    {
        return QModelIndex();
    }

    return createIndex(prnt->children().indexOf(node), 0, node);
}

void BookmarksModel::entryAdded(BookmarkNode* item)
{
    Q_ASSERT(item && item->parent());

    int row                  = item->parent()->children().indexOf(item);
    BookmarkNode* const prnt = item->parent();

    // item was already added so remove before beginInsertRows is called

    prnt->remove(item);
    beginInsertRows(index(prnt), row, row);
    prnt->add(item, row);
    endInsertRows();
}

void BookmarksModel::entryRemoved(BookmarkNode* parent, int row, BookmarkNode* item)
{
    // item was already removed, re-add so beginRemoveRows works

    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void BookmarksModel::entryChanged(BookmarkNode* item)
{
    QModelIndex idx = index(item);

    Q_EMIT dataChanged(idx, idx);
}

bool BookmarksModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if ((row < 0) || (count <= 0) || ((row + count) > rowCount(parent)))
    {
        return false;
    }

    BookmarkNode* const bookmarkNode = node(parent);

    for (int i = (row + count - 1) ; i >= row ; --i)
    {
        BookmarkNode* const nd = bookmarkNode->children().at(i);
        d->manager->removeBookmark(nd);
    }

    if (d->endMacro)
    {
        d->manager->undoRedoStack()->endMacro();
        d->endMacro = false;
    }

    return true;
}

QVariant BookmarksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        switch (section)
        {
            case 0:
            {
                return i18nc("@title: bookmark header", "Title");
            }

            case 1:
            {
                return i18nc("@title: bookmark header", "Comment");
            }
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant BookmarksModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.model() != this))
    {
        return QVariant();
    }

    const BookmarkNode* const bookmarkNode = node(index);

    switch (role)
    {
        case Qt::EditRole:
        case Qt::DisplayRole:
        {
            if (bookmarkNode->type() == BookmarkNode::Separator)
            {
                switch (index.column())
                {
                    case 0:
                    {
                        return QString(50, QChar(0xB7));
                    }

                    case 1:
                    {
                        return QString();
                    }
                }
            }

            switch (index.column())
            {
                case 0:
                {
                    return bookmarkNode->title;
                }

                case 1:
                {
                    return bookmarkNode->desc;
                }
            }

            break;
        }

        case BookmarksModel::UrlRole:
        {
            return QUrl(bookmarkNode->url);
        }

        case BookmarksModel::UrlStringRole:
        {
            return bookmarkNode->url;
        }

        case BookmarksModel::DateAddedRole:
        {
            return bookmarkNode->dateAdded;
        }

        case BookmarksModel::TypeRole:
        {
            return bookmarkNode->type();
        }

        case BookmarksModel::SeparatorRole:
        {
            return (bookmarkNode->type() == BookmarkNode::Separator);
        }

        case Qt::DecorationRole:
        {
            if (index.column() == 0)
            {
                if (bookmarkNode->type() == BookmarkNode::Bookmark)
                {
                    return QIcon::fromTheme(QLatin1String("globe"));
                }
                else
                {
                    return QIcon::fromTheme(QLatin1String("folder"));
                }
            }
        }
    }

    return QVariant();
}

int BookmarksModel::columnCount(const QModelIndex& parent) const
{
    return ((parent.column() > 0) ? 0 : 2);
}

int BookmarksModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        return d->manager->bookmarks()->children().count();
    }

    const BookmarkNode* const item = static_cast<BookmarkNode*>(parent.internalPointer());

    return item->children().count();
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((row < 0) || (column < 0) || (row >= rowCount(parent)) || (column >= columnCount(parent)))
    {
        return QModelIndex();
    }

    // get the parent node

    const BookmarkNode* const parentNode = node(parent);

    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex BookmarksModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    const BookmarkNode* const itemNode = node(index);
    BookmarkNode* const parentNode     = (itemNode ? itemNode->parent() : nullptr);

    if (!parentNode || (parentNode == d->manager->bookmarks()))
    {
        return QModelIndex();
    }

    // get the parent's row

    const BookmarkNode* const grandParentNode = parentNode->parent();
    int parentRow                             = grandParentNode->children().indexOf(parentNode);

    Q_ASSERT(parentRow >= 0);

    return createIndex(parentRow, 0, parentNode);
}

bool BookmarksModel::hasChildren(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return true;
    }

    const BookmarkNode* const parentNode = node(parent);

    return (
            (parentNode->type() == BookmarkNode::Folder) ||
            (parentNode->type() == BookmarkNode::RootFolder)
           );
}

Qt::ItemFlags BookmarksModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags              = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    const BookmarkNode* const bookmarkNode = node(index);

    if (bookmarkNode->type() != BookmarkNode::RootFolder)
    {
        flags |= Qt::ItemIsDragEnabled;
    }

    if (
        (bookmarkNode->type() != BookmarkNode::Separator) &&
        (bookmarkNode->type() != BookmarkNode::RootFolder)
       )
    {
        flags |= Qt::ItemIsEditable;
    }

    if (hasChildren(index))
    {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

Qt::DropActions BookmarksModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

QStringList BookmarksModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/bookmarks.xbel");

    return types;
}

QMimeData* BookmarksModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* const mimeData = new QMimeData();
    QByteArray dt;
    QDataStream stream(&dt, QIODevice::WriteOnly);

    for (const QModelIndex& ind : std::as_const(indexes))
    {
        if ((ind.column() != 0) || !ind.isValid())
        {
            continue;
        }

        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        XbelWriter writer;
        const BookmarkNode* const parentNode = node(ind);
        writer.write(&buffer, parentNode);
        stream << encodedData;
    }

    mimeData->setData(QLatin1String("application/bookmarks.xbel"), dt);

    return mimeData;
}

bool BookmarksModel::dropMimeData(const QMimeData* data,
                                  Qt::DropAction action,
                                  int row, int column,
                                  const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    if (!data->hasFormat(QLatin1String("application/bookmarks.xbel")) || column > 0)
    {
        return false;
    }

    QByteArray ba = data->data(QLatin1String("application/bookmarks.xbel"));
    QDataStream stream(&ba, QIODevice::ReadOnly);

    if (stream.atEnd())
    {
        return false;
    }

    QUndoStack* const undoStack = d->manager->undoRedoStack();
    undoStack->beginMacro(QLatin1String("Move Bookmarks"));

    while (!stream.atEnd())
    {
        QByteArray encodedData;
        stream >> encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadOnly);

        XbelReader reader;
        BookmarkNode* const rootNode  = reader.read(&buffer);
        QList<BookmarkNode*> children = rootNode->children();

        for (int i = 0 ; i < children.count() ; ++i)
        {
            BookmarkNode* const bookmarkNode = children.at(i);
            rootNode->remove(bookmarkNode);
            row                              = qMax(0, row);
            BookmarkNode* const parentNode   = node(parent);
            d->manager->addBookmark(parentNode, bookmarkNode, row);
            d->endMacro                      = true;
        }

        delete rootNode;
    }

    return true;
}

bool BookmarksModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (flags(index) & Qt::ItemIsEditable) == 0)
    {
        return false;
    }

    BookmarkNode* const item = node(index);

    switch (role)
    {
        case Qt::EditRole:
        case Qt::DisplayRole:
        {
            if (index.column() == 0)
            {
                d->manager->setTitle(item, value.toString());
                break;
            }

            if (index.column() == 1)
            {
                d->manager->setComment(item, value.toString());
                break;
            }

            return false;
        }

        case BookmarksModel::UrlRole:
        {
            d->manager->setUrl(item, value.toUrl().toString());
            break;
        }

        case BookmarksModel::UrlStringRole:
        {
            d->manager->setUrl(item, value.toString());
            break;
        }

        default:
        {
            return false;
        }
    }

    return true;
}

BookmarkNode* BookmarksModel::node(const QModelIndex& index) const
{
    BookmarkNode* const itemNode = static_cast<BookmarkNode*>(index.internalPointer());

    if (!itemNode)
    {
        return d->manager->bookmarks();
    }

    return itemNode;
}

// --------------------------------------------------------------

AddBookmarkProxyModel::AddBookmarkProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
}

int AddBookmarkProxyModel::columnCount(const QModelIndex& parent) const
{
    return qMin(1, QSortFilterProxyModel::columnCount(parent));
}

bool AddBookmarkProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
{
    QModelIndex idx = sourceModel()->index(srow, 0, sparent);

    return sourceModel()->hasChildren(idx);
}

// --------------------------------------------------------------

TreeProxyModel::TreeProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

int TreeProxyModel::columnCount(const QModelIndex&) const
{
    // 1th column : Title
    // 2th column : Comment

    return 2;
}

bool TreeProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
{
    QModelIndex index = sourceModel()->index(srow, 0, sparent);

    if (!index.isValid())
    {
        return false;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (index.data().toString().contains(filterRegularExpression()))

#else

    if (index.data().toString().contains(filterRegExp()))

#endif

    {
        return true;
    }

    for (int i = 0 ; i < sourceModel()->rowCount(index) ; ++i)
    {
        if (filterAcceptsRow(i, index))
        {
            return true;
        }
    }

    return false;
}

void TreeProxyModel::emitResult(bool v)
{
    Q_EMIT signalFilterAccepts(v);
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN BookmarksManager::Private
{
public:

    Private() = default;

    bool            loaded              = false;
    BookmarkNode*   bookmarkRootNode    = nullptr;
    BookmarksModel* bookmarkModel       = nullptr;
    QUndoStack      commands;
    QString         bookmarksFile;
};

BookmarksManager::BookmarksManager(const QString& bookmarksFile, QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->bookmarksFile = bookmarksFile;
    load();
}

BookmarksManager::~BookmarksManager()
{
    delete d->bookmarkRootNode;
    delete d;
}

void BookmarksManager::changeExpanded()
{
}

void BookmarksManager::load()
{
    if (d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Loading GPS bookmarks from" << d->bookmarksFile;
    d->loaded = true;

    XbelReader reader;
    d->bookmarkRootNode = reader.read(d->bookmarksFile);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, i18nc("@title:window", "Loading Bookmark"),
                             i18n("Error when loading bookmarks on line %1, column %2:\n%3",
                                  reader.lineNumber(),
                                  reader.columnNumber(),
                                  reader.errorString()));
    }
}

void BookmarksManager::save()
{
    if (!d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Saving GPS bookmarks to" << d->bookmarksFile;

    XbelWriter writer;

    if (!writer.write(d->bookmarksFile, d->bookmarkRootNode))
    {
        qCWarning(DIGIKAM_GEOIFACE_LOG) << "BookmarkManager: error saving to" << d->bookmarksFile;
    }
}

void BookmarksManager::addBookmark(BookmarkNode* const parent, BookmarkNode* const node, int row)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(parent);

    InsertBookmarksCommand* const command = new InsertBookmarksCommand(this, parent, node, row);
    d->commands.push(command);
}

void BookmarksManager::removeBookmark(BookmarkNode* const node)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    BookmarkNode* const parent            = node->parent();
    int row                               = parent->children().indexOf(node);
    RemoveBookmarksCommand* const command = new RemoveBookmarksCommand(this, parent, row);
    d->commands.push(command);
}

void BookmarksManager::setTitle(BookmarkNode* const node, const QString& newTitle)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeBookmarkCommand* const command = new ChangeBookmarkCommand(this, node, newTitle,
                                                                     ChangeBookmarkCommand::Title);
    d->commands.push(command);
}

void BookmarksManager::setUrl(BookmarkNode* const node, const QString& newUrl)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeBookmarkCommand* const command = new ChangeBookmarkCommand(this, node, newUrl,
                                                                     ChangeBookmarkCommand::Url);
    d->commands.push(command);
}

void BookmarksManager::setComment(BookmarkNode* const node, const QString& newDesc)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeBookmarkCommand* const command = new ChangeBookmarkCommand(this, node, newDesc,
                                                                     ChangeBookmarkCommand::Desc);
    d->commands.push(command);
}

BookmarkNode* BookmarksManager::bookmarks()
{
    if (!d->loaded)
    {
        load();
    }

    return d->bookmarkRootNode;
}

BookmarksModel* BookmarksManager::bookmarksModel()
{
    if (!d->bookmarkModel)
    {
        d->bookmarkModel = new BookmarksModel(this, this);
    }

    return d->bookmarkModel;
}

QUndoStack* BookmarksManager::undoRedoStack() const
{
    return &d->commands;
}

void BookmarksManager::importBookmarks()
{
    QString fileName = DFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Open File"),
                                                    QString(),
                                                    i18n("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    XbelReader reader;
    BookmarkNode* const importRootNode = reader.read(fileName);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, i18nc("@title:window", "Loading Bookmark"),
                             i18n("Error when loading bookmarks on line %1, column %2:\n%3",
                                  reader.lineNumber(),
                                  reader.columnNumber(),
                                  reader.errorString()));
    }

    importRootNode->setType(BookmarkNode::Folder);
    importRootNode->title = i18n("Imported %1", QLocale().toString(QDate::currentDate(), QLocale::ShortFormat));
    addBookmark(bookmarks(), importRootNode);
}

void BookmarksManager::exportBookmarks()
{
    QString fileName = DFileDialog::getSaveFileName(nullptr, i18nc("@title:window", "Save File"),
                                                    i18n("%1 Bookmarks.xbel", QCoreApplication::applicationName()),
                                                    i18n("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    XbelWriter writer;

    if (!writer.write(fileName, d->bookmarkRootNode))
    {
        QMessageBox::critical(nullptr, i18nc("@title:window", "Export Bookmark"), i18n("Error saving bookmarks"));
    }
}

} // namespace Digikam

#include "moc_bookmarksmngr.cpp"

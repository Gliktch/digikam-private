/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Side Bar Widget for the Showfoto folder view.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotofolderviewsidebar.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QSplitter>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "showfoto.h"
#include "showfotofolderviewbar.h"
#include "showfotofolderviewundo.h"
#include "showfotofolderviewmodel.h"
#include "showfotofolderviewlist.h"
#include "showfotofolderviewbookmarks.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewSideBar::Private
{
public:

    Private() = default;

public:

    const QString configIconSizeEntry                       = QLatin1String("Icon Size");
    const QString configLastFolderEntry                     = QLatin1String("Last Folder");
    const QString configFolderViewModeEntry                 = QLatin1String("Folder View Mode");
    const QString configFolderViewTypeMimeEntry             = QLatin1String("Folder View Type Mime");
    const QString configBookmarksVisibleEntry               = QLatin1String("Bookmarks Visible");
    const QString configSplitterStateEntry                  = QLatin1String("Splitter State");

    ShowfotoFolderViewModel*               fsmodel          = nullptr;
    ShowfotoFolderViewList*                fsview           = nullptr;
    ShowfotoFolderViewBar*                 fsbar            = nullptr;
    ShowfotoFolderViewBookmarks*           fsmarks          = nullptr;
    QUndoStack*                            fsstack          = nullptr;
    QSplitter*                             splitter         = nullptr;
    Showfoto*                              parent           = nullptr;
    QList<DPluginAction*>                  pluginActions;
    Qt::SortOrder                          fsSortOrder      = Qt::AscendingOrder;
    ShowfotoFolderViewList::FolderViewRole fsRole           = ShowfotoFolderViewList::FileName;
};

ShowfotoFolderViewSideBar::ShowfotoFolderViewSideBar(Showfoto* const parent)
    : QWidget          (parent),
      StateSavingObject(this),
      d                (new Private)
{
    setObjectName(QLatin1String("ShowfotoFolderView Sidebar"));

    d->parent                  = parent;
    d->fsstack                 = new QUndoStack(this);

    // --- Populate the view

    d->fsbar                   = new ShowfotoFolderViewBar(this);
    d->fsview                  = new ShowfotoFolderViewList(this, d->fsbar);
    d->fsmodel                 = new ShowfotoFolderViewModel(d->fsview);
    d->fsview->setModel(d->fsmodel);
    d->fsview->setRootIndex(d->fsmodel->index(QDir::rootPath()));

    d->fsmarks                 = new ShowfotoFolderViewBookmarks(this);

    d->splitter                = new QSplitter(Qt::Vertical, this);
    d->splitter->addWidget(d->fsview);
    d->splitter->addWidget(d->fsmarks);
    d->splitter->setStretchFactor(0, 10);
    d->splitter->setStretchFactor(1, 3);

    QVBoxLayout* const layout  = new QVBoxLayout(this);
    layout->addWidget(d->fsbar);
    layout->addWidget(d->splitter);
    layout->setContentsMargins(QMargins());

    // --- Setup connections

    connect(d->fsbar, SIGNAL(signalSetup()),
            this, SIGNAL(signalSetup()));

    connect(d->fsbar, SIGNAL(signalShowBookmarks(bool)),
            this, SLOT(slotShowBookmarks(bool)));

    connect(d->fsbar, SIGNAL(signalViewModeChanged(int)),
            this, SLOT(slotViewModeChanged(int)));

    connect(d->fsbar, SIGNAL(signalIconSizeChanged(int)),
            d->fsview, SLOT(slotIconSizeChanged(int)));

    connect(d->fsbar, SIGNAL(signalGoHome()),
            this, SLOT(slotGoHome()));

    connect(d->fsbar, SIGNAL(signalGoUp()),
            this, SLOT(slotGoUp()));

    connect(d->fsbar, SIGNAL(signalLoadContents()),
            this, SLOT(slotLoadContents()));

    connect(d->fsbar, SIGNAL(signalAppendContents()),
            this, SLOT(slotAppendContents()));

    connect(d->fsmarks, SIGNAL(signalLoadContents()),
            this, SLOT(slotLoadContents()));

    connect(d->fsbar, SIGNAL(signalCustomPathChanged(QString)),
            this, SLOT(slotCustomPathChanged(QString)));

    connect(d->fsbar, SIGNAL(signalTypeMimesChanged(QString)),
            this, SLOT(slotTypeMimesChanged(QString)));

    connect(d->fsbar, SIGNAL(signalGoNext()),
            this, SLOT(slotRedo()));

    connect(d->fsbar, SIGNAL(signalGoPrevious()),
            this, SLOT(slotUndo()));

    connect(d->fsstack, SIGNAL(canUndoChanged(bool)),
            d->fsbar, SLOT(slotPreviousEnabled(bool)));

    connect(d->fsstack, SIGNAL(canRedoChanged(bool)),
            d->fsbar, SLOT(slotNextEnabled(bool)));

    connect(d->fsview, SIGNAL(signalAddBookmark()),
            this, SIGNAL(signalAddBookmark()));
}

ShowfotoFolderViewSideBar::~ShowfotoFolderViewSideBar()
{
    delete d;
}

void ShowfotoFolderViewSideBar::slotTypeMimesChanged(const QString& patterns)
{
    d->fsmodel->setNameFilters(patterns.split(QLatin1Char(' ')));
}

void ShowfotoFolderViewSideBar::slotLoadContents()
{
    QModelIndex index = d->fsmodel->index(currentPath());
    loadContents(index);
}

void ShowfotoFolderViewSideBar::slotAppendContents()
{
    QModelIndex index = d->fsmodel->index(currentPath());
    loadContents(index, true);
}

void ShowfotoFolderViewSideBar::loadContents(const QModelIndex& index, bool append)
{
    if (!index.isValid())
    {
        return;
    }

    QStringList lst;
    QString currentFile;

    if      (d->fsmodel->isDir(index))
    {
        setCurrentPath(d->fsmodel->filePath(index));

        lst         = d->fsmodel->currentFilesPath();
        currentFile = !lst.isEmpty() ? lst.first() : QString();
    }
    else if (d->fsmodel->fileInfo(index).isFile())
    {
        lst         = d->fsmodel->currentFilesPath();
        currentFile = d->fsmodel->fileInfo(index).filePath();
    }

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Load Contents from:" << currentPath() << "Files:" << lst;

    if (!lst.isEmpty())
    {
        if (!append)
        {
            Q_EMIT signalLoadContentsFromFiles(lst, currentFile);
        }
        else
        {
            Q_EMIT signalAppendContentsFromFiles(lst, currentFile);
        }
    }
}

void ShowfotoFolderViewSideBar::slotCustomPathChanged(const QString& path)
{
    setCurrentPath(path);
}

void ShowfotoFolderViewSideBar::slotUndo()
{
    d->fsstack->undo();
}

void ShowfotoFolderViewSideBar::slotRedo()
{
    d->fsstack->redo();
}

void ShowfotoFolderViewSideBar::slotGoHome()
{
    setCurrentPath(QDir::homePath());
}

void ShowfotoFolderViewSideBar::slotGoUp()
{
    QDir dir(currentFolder());
    Q_UNUSED(dir.cdUp());

    // Is this the same as going back?  If so just go back, so we can keep the view scroll position.

    if (d->fsstack->canUndo())
    {
        const ShowfotoFolderViewUndo* lastDir = static_cast<const ShowfotoFolderViewUndo*>
                                                (d->fsstack->command(d->fsstack->index() - 1));

        if (lastDir->undoPath() == dir.path())
        {
            d->fsstack->undo();
            return;
        }
    }

    setCurrentPath(dir.absolutePath());
}

QString ShowfotoFolderViewSideBar::currentFolder() const
{
    QString path = d->fsmodel->rootPath();

    if (!path.endsWith(QDir::separator()))
    {
        path.append(QDir::separator());
    }

    return path;
}

QString ShowfotoFolderViewSideBar::currentPath() const
{
    QModelIndex index = d->fsview->currentIndex();

    if (index.isValid())
    {
        return (d->fsmodel->filePath(index));
    }

    return currentFolder();
}

void ShowfotoFolderViewSideBar::setCurrentPath(const QString& newPathNative)
{
    QFileInfo infoNative(newPathNative);

    if (!infoNative.exists())
    {
        return;
    }

    QString newPath = QDir::fromNativeSeparators(newPathNative);

    if (infoNative.isDir() && !newPath.endsWith(QDir::separator()))
    {
        newPath.append(QDir::separator());
    }

    QString oldDir(d->fsmodel->rootPath());

    if (!oldDir.endsWith(QDir::separator()))
    {
        oldDir.append(QDir::separator());
    }

    if (oldDir == newPath)
    {
        return;
    }

    QFileInfo info(newPath);

    if (info.isDir())
    {
        QModelIndex index = d->fsmodel->index(newPath);

        if (index.isValid())
        {
            d->fsstack->push(new ShowfotoFolderViewUndo(this, newPath));
            d->fsmodel->setRootPath(newPath);
            d->fsview->setRootIndex(index);
        }
    }
    else
    {
        QModelIndex index = d->fsmodel->index(newPath);

        if (index.isValid())
        {
            QString newDir = info.absolutePath();

            if (!newDir.endsWith(QDir::separator()))
            {
                newDir.append(QDir::separator());
            }

            if (newDir != oldDir)
            {
                d->fsstack->push(new ShowfotoFolderViewUndo(this, newDir));
                d->fsmodel->setRootPath(newDir);
            }

            d->fsview->setCurrentIndex(index);
            d->fsview->scrollTo(index);
        }
    }
}

void ShowfotoFolderViewSideBar::setCurrentPathWithoutUndo(const QString& newPath)
{
    QModelIndex index = d->fsmodel->setRootPath(newPath);

    if (index.isValid())
    {
        d->fsview->setRootIndex(index);
        d->fsbar->setCurrentPath(currentFolder());
    }
}

const QIcon ShowfotoFolderViewSideBar::getIcon()
{
    return QIcon::fromTheme(QLatin1String("document-open-folder"));
}

const QString ShowfotoFolderViewSideBar::getCaption()
{
    return i18nc("@title: file system tree", "Folders");
}

void ShowfotoFolderViewSideBar::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    d->fsmarks->readSettings(group);
    d->fsbar->setFolderViewMode(group.readEntry(entryName(d->configFolderViewModeEntry),         (int)ShowfotoFolderViewList::ShortView));
    d->fsbar->setFolderViewTypeMime(group.readEntry(entryName(d->configFolderViewTypeMimeEntry), (int)ShowfotoFolderViewBar::TYPE_MIME_ALL));
    d->fsbar->setBookmarksVisible(group.readEntry(entryName(d->configBookmarksVisibleEntry),     false));
    slotViewModeChanged(d->fsbar->folderViewMode());
    d->fsbar->setIconSize(group.readEntry(entryName(d->configIconSizeEntry),                     32));

    QByteArray state = group.readEntry(entryName(d->configSplitterStateEntry),                   QByteArray());

    if (!state.isEmpty())
    {
        d->splitter->restoreState(QByteArray::fromBase64(state));
    }

    setCurrentPathWithoutUndo(group.readEntry(entryName(d->configLastFolderEntry),               QDir::rootPath()));
    loadContents(d->fsview->currentIndex());
}

void ShowfotoFolderViewSideBar::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    d->fsmarks->saveSettings(group);
    group.writeEntry(entryName(d->configFolderViewModeEntry),       d->fsbar->folderViewMode());
    group.writeEntry(entryName(d->configFolderViewTypeMimeEntry),   d->fsbar->folderViewTypeMime());
    group.writeEntry(entryName(d->configBookmarksVisibleEntry),     d->fsbar->bookmarksVisible());
    group.writeEntry(entryName(d->configIconSizeEntry),             d->fsbar->iconSize());
    group.writeEntry(entryName(d->configLastFolderEntry),           currentFolder());
    group.writeEntry(entryName(d->configSplitterStateEntry),        d->splitter->saveState().toBase64());
    group.sync();
}

void ShowfotoFolderViewSideBar::slotViewModeChanged(int mode)
{
    switch (mode)
    {
        case ShowfotoFolderViewList::ShortView:
        {
            d->fsview->setColumnHidden(ShowfotoFolderViewList::FileSize, true);
            d->fsview->setColumnHidden(ShowfotoFolderViewList::FileType, true);
            d->fsview->setColumnHidden(ShowfotoFolderViewList::FileDate, true);
            d->fsview->setHeaderHidden(true);
            break;
        }

        default:    // ShowfotoFolderViewList::DetailledView
        {
            d->fsview->setColumnHidden(ShowfotoFolderViewList::FileSize, false);
            d->fsview->setColumnHidden(ShowfotoFolderViewList::FileType, false);
            d->fsview->setColumnHidden(ShowfotoFolderViewList::FileDate, false);
            d->fsview->setHeaderHidden(false);
            break;
        }
    }
}

void ShowfotoFolderViewSideBar::setSortOrder(int order)
{
    d->fsSortOrder = (Qt::SortOrder)order;
    d->fsmodel->sort(d->fsRole, d->fsSortOrder);
}

void ShowfotoFolderViewSideBar::setSortRole(int role)
{
    d->fsRole = (ShowfotoFolderViewList::FolderViewRole)role;
    d->fsmodel->sort(d->fsRole, d->fsSortOrder);
}

void ShowfotoFolderViewSideBar::slotShowBookmarks(bool visible)
{
    d->fsmarks->setVisible(visible);
}

void ShowfotoFolderViewSideBar::registerPluginActions(const QList<DPluginAction*>& actions)
{
    d->pluginActions = actions;

    d->fsbar->registerPluginActions(d->pluginActions);

    connect(d->fsbar, SIGNAL(signalPluginActionTriggered(QAction*)),
            this, SLOT(slotPluginActionTriggered(QAction*)));
}

void ShowfotoFolderViewSideBar::slotPluginActionTriggered(QAction* act)
{
    for (QAction* const dpact : std::as_const(d->pluginActions))
    {
        if (act->objectName() == dpact->objectName())
        {
            slotLoadContents();
            QTimer::singleShot(1000, dpact, SLOT(trigger()));

            return;
        }
    }
}

QList<QAction*> ShowfotoFolderViewSideBar::pluginActions() const
{
    return d->fsbar->pluginActions();
}

} // namespace ShowFoto

#include "moc_showfotofolderviewsidebar.cpp"

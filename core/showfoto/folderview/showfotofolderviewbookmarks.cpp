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

#include "showfotofolderviewbookmarks.h"

// Qt includes

#include <QDir>
#include <QStandardPaths>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QUrl>
#include <QIcon>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "showfotofolderviewsidebar.h"
#include "showfotofolderviewbookmarkdlg.h"
#include "showfotofolderviewbookmarklist.h"
#include "showfotofolderviewbookmarkitem.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoFolderViewBookmarks::Private
{
public:

    Private() = default;

public:

    const QString configBookmarkItemsEntry              = QLatin1String("BookmarkItems");
    const QString configBookmarkPathPrefixEntry         = QLatin1String("BookmarkPath");
    const QString configBookmarkTitlePrefixEntry        = QLatin1String("BookmarkTitle");
    const QString configBookmarkIconPrefixEntry         = QLatin1String("BookmarkIcon");
    const QString configBookmarkTopItemExpandedEntry    = QLatin1String("BookmarkTopItemExpanded");
    const QString configBookmarkTopUsualExpandedEntry   = QLatin1String("BookmarkTopUsualExpanded");

    QList<QAction*>                 actionsList;        ///< used to shared actions with list-view context menu.
    QToolButton*                    addBtn              = nullptr;
    QToolButton*                    delBtn              = nullptr;
    QToolButton*                    edtBtn              = nullptr;
    ShowfotoFolderViewBookmarkList* bookmarksList       = nullptr;
    QTreeWidgetItem*                topBookmarks        = nullptr;
    QTreeWidgetItem*                topUsualPlaces      = nullptr;
    ShowfotoFolderViewSideBar*      sidebar             = nullptr;
};


ShowfotoFolderViewBookmarks::ShowfotoFolderViewBookmarks(ShowfotoFolderViewSideBar* const sidebar)
    : QWidget(sidebar),
      d      (new Private)
{
    d->sidebar              = sidebar;
    QGridLayout* const grid = new QGridLayout(this);

    QLabel* const title     = new QLabel(this);
    title->setText(i18nc("@title", "Places"));

    // --------------------------------------------------------

    QAction* btnAction      = nullptr;

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("AddBookmark"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    btnAction->setText(i18nc("@action", "Add Bookmark"));
    btnAction->setToolTip(i18nc("@info", "Add new bookmark to the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddBookmark()));

    d->actionsList << btnAction;

    d->addBtn               = new QToolButton(this);
    d->addBtn->setDefaultAction(btnAction);
    d->addBtn->setFocusPolicy(Qt::NoFocus);

    // ---

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("DelBookmark"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    btnAction->setText(i18nc("@action", "Del Bookmark"));
    btnAction->setToolTip(i18nc("@info", "Remove selected bookmark from the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDelBookmark()));

    d->actionsList << btnAction;

    d->delBtn               = new QToolButton(this);
    d->delBtn->setDefaultAction(btnAction);
    d->delBtn->setFocusPolicy(Qt::NoFocus);
    d->delBtn->setEnabled(false);

    // ---

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("EditBookmark"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    btnAction->setText(i18nc("@action", "Edit Bookmark"));
    btnAction->setToolTip(i18nc("@info", "Edit current bookmark from the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEdtBookmark()));

    d->actionsList << btnAction;

    d->edtBtn               = new QToolButton(this);
    d->edtBtn->setDefaultAction(btnAction);
    d->edtBtn->setFocusPolicy(Qt::NoFocus);
    d->edtBtn->setEnabled(false);

    // ---

    d->bookmarksList        = new ShowfotoFolderViewBookmarkList(this);

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(title,             0, 0, 1, 1);
    grid->addWidget(d->addBtn,         0, 2, 1, 1);
    grid->addWidget(d->delBtn,         0, 3, 1, 1);
    grid->addWidget(d->edtBtn,         0, 4, 1, 1);
    grid->addWidget(d->bookmarksList,  1, 0, 1, 5);
    grid->setRowStretch(1, 10);
    grid->setColumnStretch(1, 10);
    grid->setContentsMargins(QMargins());

    // --------------------------------------------------------

    connect(d->bookmarksList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotBookmarkSelectionChanged()));

    connect(d->bookmarksList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotBookmarkActivated(QTreeWidgetItem*)));

    connect(d->sidebar, SIGNAL(signalAddBookmark()),
            this, SLOT(slotAddBookmark()));

    connect(d->bookmarksList, SIGNAL(signalAddBookmark(QString)),
            this, SLOT(slotAddBookmark(QString)));

    connect(d->bookmarksList, SIGNAL(signalLoadContents(QString)),
            this, SLOT(slotLoadContents(QString)));
}

ShowfotoFolderViewBookmarks::~ShowfotoFolderViewBookmarks()
{
    delete d;
}

QAction* ShowfotoFolderViewBookmarks::toolBarAction(const QString& name) const
{
    for (QAction* const act : std::as_const(d->actionsList))
    {
        if (act && (act->objectName() == name))
        {
            return act;
        }
    }

    return nullptr;
}

QTreeWidgetItem* ShowfotoFolderViewBookmarks::topBookmarksItem() const
{
    return d->topBookmarks;
}

void ShowfotoFolderViewBookmarks::slotAddBookmark()
{
    QString newBookmark = d->sidebar->currentFolder();
    slotAddBookmark(newBookmark);
}

void ShowfotoFolderViewBookmarks::slotAddBookmark(const QString& newBookmark)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << newBookmark;

    if (newBookmark.isEmpty())
    {
        return;
    }

    ShowfotoFolderViewBookmarkItem* item = d->bookmarksList->bookmarkExists(newBookmark);

    if (!item)
    {
        QString title = d->bookmarksList->bookmarkBaseName(newBookmark);
        QString icon  = QLatin1String("folder");
        QString path  = newBookmark;

        bool ok = ShowfotoFolderViewBookmarkDlg::bookmarkDialog(d->bookmarksList, title, icon, path, true);

        if (ok)
        {
            item = new ShowfotoFolderViewBookmarkItem(d->topBookmarks);
            item->setText(0, title);
            item->setIcon(0, QIcon::fromTheme(icon));
            item->setPath(path);
        }

        return;
    }

    QMessageBox::information(this,
                             i18nc("@title:window", "Add New Bookmark"),
                             i18nc("@info", "This bookmark referencing\n%1\nalready exists in the list with name \"%2\".",
                             item->path(), item->text(0)));
}

void ShowfotoFolderViewBookmarks::slotDelBookmark()
{
    QTreeWidgetItem* const item = d->bookmarksList->currentItem();

    if (!item || (item->parent() != d->topBookmarks))
    {
        return;
    }

    if (QMessageBox::question(this, i18nc("@title:window", "Confirm Bookmark Deletion"),
                              i18nc("@info", "Are you sure you want to remove the bookmark \"%1\"?", item->text(0)))
            != QMessageBox::Yes)
    {
        return;
    }

    d->bookmarksList->removeItemWidget(item, 0);
    delete item;
}

void ShowfotoFolderViewBookmarks::slotEdtBookmark()
{
    ShowfotoFolderViewBookmarkItem* const item = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(d->bookmarksList->currentItem());

    if (!item || (item->parent() != d->topBookmarks))
    {
        return;
    }

    QString title = item->text(0);
    QString icon  = item->icon(0).name();
    QString path  = item->path();

    bool ok = ShowfotoFolderViewBookmarkDlg::bookmarkDialog(d->bookmarksList, title, icon, path);

    if (ok)
    {
        item->setText(0, title);
        item->setIcon(0, QIcon::fromTheme(icon));
        item->setPath(path);
    }
}

void ShowfotoFolderViewBookmarks::slotBookmarkSelectionChanged()
{
    bool b                                     = true;
    ShowfotoFolderViewBookmarkItem* const item = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(d->bookmarksList->currentItem());

    if (!item || (item->parent() != d->topBookmarks))
    {
        b = false;
    }

    d->delBtn->setEnabled(b);
    d->edtBtn->setEnabled(b);
}

void ShowfotoFolderViewBookmarks::slotBookmarkActivated(QTreeWidgetItem* item)
{
    const ShowfotoFolderViewBookmarkItem* const bitem = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(item);

    if (bitem)
    {
        d->sidebar->setCurrentPath(bitem->path());
    }
}

void ShowfotoFolderViewBookmarks::slotLoadContents(const QString& path)
{
    d->sidebar->setCurrentPath(path);

    Q_EMIT signalLoadContents();
}

void ShowfotoFolderViewBookmarks::saveSettings(KConfigGroup& group)
{
    QString confEntry;
    ShowfotoFolderViewBookmarkItem* item = nullptr;
    int nbItems                          = d->topBookmarks->childCount();

    group.writeEntry(d->configBookmarkItemsEntry,            nbItems);
    group.writeEntry(d->configBookmarkTopItemExpandedEntry,  d->topBookmarks->isExpanded());
    group.writeEntry(d->configBookmarkTopUsualExpandedEntry, d->topUsualPlaces->isExpanded());

    for (int i = 0 ; i < nbItems ; ++i)
    {
        item = dynamic_cast<ShowfotoFolderViewBookmarkItem*>(d->topBookmarks->child(i));

        if (item)
        {
            confEntry = QString::fromLatin1("%1_%2").arg(d->configBookmarkPathPrefixEntry).arg(i);
            group.writeEntry(confEntry, item->path());
            confEntry = QString::fromLatin1("%1_%2").arg(d->configBookmarkTitlePrefixEntry).arg(i);
            group.writeEntry(confEntry, item->text(0));
            confEntry = QString::fromLatin1("%1_%2").arg(d->configBookmarkIconPrefixEntry).arg(i);
            group.writeEntry(confEntry, item->icon(0).name());
        }
    }
}

void ShowfotoFolderViewBookmarks::readSettings(const KConfigGroup& group)
{
    d->bookmarksList->clear();

    d->topBookmarks          = new QTreeWidgetItem(d->bookmarksList);
    d->topBookmarks->setFlags(Qt::ItemIsEnabled);
    d->topBookmarks->setExpanded(group.readEntry(d->configBookmarkTopItemExpandedEntry, true));
    d->topBookmarks->setDisabled(false);
    d->topBookmarks->setText(0, i18nc("@title", "My Bookmarks"));

    QString confEntry;
    int nbItems = group.readEntry(d->configBookmarkItemsEntry, 0);

    for (int i = 0 ; i < nbItems ; ++i)
    {
        ShowfotoFolderViewBookmarkItem* const item = new ShowfotoFolderViewBookmarkItem(d->topBookmarks);

        confEntry       = QString::fromLatin1("%1_%2").arg(d->configBookmarkPathPrefixEntry).arg(i);
        item->setPath(group.readEntry(confEntry, QString()));

        if (!item->path().isEmpty())
        {
            confEntry       = QString::fromLatin1("%1_%2").arg(d->configBookmarkTitlePrefixEntry).arg(i);
            item->setText(0, group.readEntry(confEntry, d->bookmarksList->bookmarkBaseName(item->path())));

            confEntry       = QString::fromLatin1("%1_%2").arg(d->configBookmarkIconPrefixEntry).arg(i);
            QString icoName = group.readEntry(confEntry, QString::fromLatin1("folder"));
            item->setIcon(0, QIcon::fromTheme(icoName));
        }
        else
        {
            delete item;
        }
    }

    d->topUsualPlaces                      = new QTreeWidgetItem(d->bookmarksList);
    d->topUsualPlaces->setFlags(Qt::ItemIsEnabled);
    d->topUsualPlaces->setExpanded(group.readEntry(d->configBookmarkTopUsualExpandedEntry, true));
    d->topUsualPlaces->setDisabled(false);
    d->topUsualPlaces->setText(0, i18nc("@title", "Usual Places"));

    ShowfotoFolderViewBookmarkItem* uplace = nullptr;
    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::HomeLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("user-home")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::PicturesLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("folder-image")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::DocumentsLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("folder-document")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::DownloadLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("folder-downloads")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));

    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::DesktopLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("user-desktop")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::MusicLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("folder-music")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    uplace                                 = new ShowfotoFolderViewBookmarkItem(d->topUsualPlaces);
    uplace->setText(0, QStandardPaths::displayName(QStandardPaths::MoviesLocation));
    uplace->setIcon(0, QIcon::fromTheme(QLatin1String("folder-videos")));
    uplace->setPath(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
}

QList<QAction*> ShowfotoFolderViewBookmarks::pluginActions() const
{
    return d->sidebar->pluginActions();
}

} // namespace ShowFoto

#include "moc_showfotofolderviewbookmarks.cpp"

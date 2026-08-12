/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view favorites
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotostackviewfavorites.h"

// Qt includes

#include <QDir>
#include <QStandardPaths>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QFile>
#include <QDate>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#   include <QTextCodec>
#endif

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "showfotostackviewsidebar.h"
#include "showfotostackviewfavoriteitemdlg.h"
#include "showfotostackviewfavoritelist.h"
#include "showfotostackviewfavoriteitem.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoStackViewFavorites::Private
{
public:

    Private() = default;

public:

    QList<QAction*>                 actionsList;                    ///< Used to shared actions with list-view context menu.
    QToolButton*                    addBtn          = nullptr;      ///< Add favorite button.
    QToolButton*                    fldBtn          = nullptr;      ///< Add sub-folder button.
    QToolButton*                    delBtn          = nullptr;      ///< Delete item button.
    QToolButton*                    edtBtn          = nullptr;      ///< Edit item button.
    ShowfotoStackViewFavoriteList*  favoritesList   = nullptr;
    ShowfotoStackViewFavoriteItem*  topFavorites    = nullptr;      ///< Top-level parent of all favorite items.
    ShowfotoStackViewSideBar*       sidebar         = nullptr;
    SearchTextBar*                  favoritesFilter = nullptr;

    /**
     * Path to store favorites XML data file.
     */
    QString                         file            = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                                      QLatin1String("/favorites.xml");
};

ShowfotoStackViewFavorites::ShowfotoStackViewFavorites(ShowfotoStackViewSideBar* const sidebar)
    : QWidget(sidebar),
      d      (new Private)
{
    d->sidebar              = sidebar;
    QGridLayout* const grid = new QGridLayout(this);

    QLabel* const title     = new QLabel(this);
    title->setText(i18nc("@title", "Favorites"));

    // --------------------------------------------------------

    QAction* btnAction      = nullptr;

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("AddFavorite"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    btnAction->setText(i18nc("@action", "Add Favorite"));
    btnAction->setToolTip(i18nc("@info", "Add new bookmark to the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddFavorite()));

    d->actionsList << btnAction;

    d->addBtn               = new QToolButton(this);
    d->addBtn->setDefaultAction(btnAction);
    d->addBtn->setFocusPolicy(Qt::NoFocus);
    d->addBtn->setEnabled(true);

    // ---

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("AddFolder"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("folder-new")));
    btnAction->setText(i18nc("@action", "Add Sub-Folder"));
    btnAction->setToolTip(i18nc("@info", "Add new sub-folder to the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSubFolder()));

    d->actionsList << btnAction;

    d->fldBtn               = new QToolButton(this);
    d->fldBtn->setDefaultAction(btnAction);
    d->fldBtn->setFocusPolicy(Qt::NoFocus);
    d->fldBtn->setEnabled(true);

    // ---

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("DelItem"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    btnAction->setText(i18nc("@action", "Delete Item"));
    btnAction->setToolTip(i18nc("@info", "Remove selected item from the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDelItem()));

    d->actionsList << btnAction;

    d->delBtn               = new QToolButton(this);
    d->delBtn->setDefaultAction(btnAction);
    d->delBtn->setFocusPolicy(Qt::NoFocus);
    d->delBtn->setEnabled(false);

    // ---

    btnAction               = new QAction(this);
    btnAction->setObjectName(QLatin1String("EditItem"));
    btnAction->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));
    btnAction->setText(i18nc("@action", "Edit Item"));
    btnAction->setToolTip(i18nc("@info", "Edit current item from the list"));

    connect(btnAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEditItem()));

    d->actionsList << btnAction;

    d->edtBtn               = new QToolButton(this);
    d->edtBtn->setDefaultAction(btnAction);
    d->edtBtn->setFocusPolicy(Qt::NoFocus);
    d->edtBtn->setEnabled(false);

    // ---

    d->favoritesList        = new ShowfotoStackViewFavoriteList(this);
    d->favoritesFilter      = new SearchTextBar(this,
                                                QLatin1String("FavoritesSearchBar"),
                                                i18nc("@info: search text bar", "Search in Favorites...")
                                               );

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(title,              0, 0, 1, 1);
    grid->addWidget(d->addBtn,          0, 2, 1, 1);
    grid->addWidget(d->fldBtn,          0, 3, 1, 1);
    grid->addWidget(d->delBtn,          0, 4, 1, 1);
    grid->addWidget(d->edtBtn,          0, 5, 1, 1);
    grid->addWidget(d->favoritesList,   1, 0, 1, 6);
    grid->addWidget(d->favoritesFilter, 2, 0, 1, 6);

    grid->setRowStretch(1, 10);
    grid->setColumnStretch(1, 10);
    grid->setContentsMargins(QMargins());

    // --------------------------------------------------------

    connect(d->favoritesList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));

    connect(d->favoritesList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotFavoriteActivated(QTreeWidgetItem*)));

    connect(d->sidebar, SIGNAL(signalAddFavorite()),
            this, SLOT(slotAddFavorite()));

    connect(d->favoritesList, SIGNAL(signalAddFavorite()),
            this, SLOT(slotAddFavorite()));

    connect(d->favoritesList, SIGNAL(signalAddFavoriteList(QList<QUrl>,QUrl)),
            this, SLOT(slotAddFavorite(QList<QUrl>,QUrl)));

    connect(d->favoritesList, SIGNAL(signalLoadContentsFromFiles(QStringList,QString)),
            this, SIGNAL(signalLoadContentsFromFiles(QStringList,QString)));

    connect(d->favoritesFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->favoritesList, SIGNAL(signalSearchResult(int)),
            this, SLOT(slotSearchResult(int)));
}

ShowfotoStackViewFavorites::~ShowfotoStackViewFavorites()
{
    delete d;
}

QAction* ShowfotoStackViewFavorites::toolBarAction(const QString& name) const
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

QTreeWidgetItem* ShowfotoStackViewFavorites::topFavoritesItem() const
{
    return d->topFavorites;
}

void ShowfotoStackViewFavorites::loadContents()
{
    d->favoritesList->slotLoadContents();
}

void ShowfotoStackViewFavorites::slotAddFavorite()
{
    slotAddFavorite(d->sidebar->urls(), d->sidebar->currentUrl());
}

void ShowfotoStackViewFavorites::slotAddSubFolder()
{

    QString name;
    QString desc;
    QDate date;
    QString icon;
    QList<QUrl> urls;
    QUrl currentUrl;
    int favType                                 = ShowfotoStackViewFavoriteItem::FavoriteFolder;
    ShowfotoStackViewFavoriteItem* const parent = d->favoritesList->currentItem() ? dynamic_cast<ShowfotoStackViewFavoriteItem*>(d->favoritesList->currentItem())
                                                                                  : d->topFavorites;
    bool ok                                     = ShowfotoStackViewFavoriteItemDlg::favoriteItemDialog(d->favoritesList,
                                                                                                       name,
                                                                                                       favType,
                                                                                                       desc,
                                                                                                       date,
                                                                                                       icon,
                                                                                                       urls,
                                                                                                       currentUrl,
                                                                                                       d->sidebar->iconSize(),
                                                                                                       d->sidebar->sortOrder(),
                                                                                                       d->sidebar->sortRole(),
                                                                                                       parent,
                                                                                                       true
                                                                                                      );

    if (ok)
    {
        if (favType == ShowfotoStackViewFavoriteItem::FavoriteItem)
        {
            ShowfotoStackViewFavoriteItem* const item = new ShowfotoStackViewFavoriteItem(parent,
                                                                                          ShowfotoStackViewFavoriteItem::FavoriteItem);
            item->setName(name);
            item->setDescription(desc);
            item->setDate(date);
            item->setIcon(0, QIcon::fromTheme(icon));
            item->setUrls(urls);
            item->setCurrentUrl(currentUrl);
        }
        else
        {
            ShowfotoStackViewFavoriteItem* const folder = new ShowfotoStackViewFavoriteItem(parent,
                                                                                            ShowfotoStackViewFavoriteItem::FavoriteFolder);
            folder->setName(name);
        }

        parent->setExpanded(true);
    }
}

void ShowfotoStackViewFavorites::slotAddFavorite(const QList<QUrl>& newUrls, const QUrl& current)
{
    QString name;
    QString desc;
    int favType                                 = ShowfotoStackViewFavoriteItem::FavoriteItem;
    QDate date                                  = QDate::currentDate();
    QString icon                                = QLatin1String("folder-favorites");
    QList<QUrl> urls                            = newUrls;
    QUrl currentUrl                             = current;
    ShowfotoStackViewFavoriteItem* const parent = d->favoritesList->currentItem() ? dynamic_cast<ShowfotoStackViewFavoriteItem*>(d->favoritesList->currentItem())
                                                                                  : d->topFavorites;
    bool ok                                     = ShowfotoStackViewFavoriteItemDlg::favoriteItemDialog(d->favoritesList,
                                                                                                       name,
                                                                                                       favType,
                                                                                                       desc,
                                                                                                       date,
                                                                                                       icon,
                                                                                                       urls,
                                                                                                       currentUrl,
                                                                                                       d->sidebar->iconSize(),
                                                                                                       d->sidebar->sortOrder(),
                                                                                                       d->sidebar->sortRole(),
                                                                                                       parent,
                                                                                                       true
                                                                                                      );

    if (ok)
    {
        if (favType == ShowfotoStackViewFavoriteItem::FavoriteItem)
        {
            ShowfotoStackViewFavoriteItem* const item = new ShowfotoStackViewFavoriteItem(parent,
                                                                                          ShowfotoStackViewFavoriteItem::FavoriteItem);
            item->setName(name);
            item->setDescription(desc);
            item->setDate(date);
            item->setIcon(0, QIcon::fromTheme(icon));
            item->setUrls(urls);
            item->setCurrentUrl(currentUrl);
        }
        else
        {
            ShowfotoStackViewFavoriteItem* const folder = new ShowfotoStackViewFavoriteItem(parent,
                                                                                            ShowfotoStackViewFavoriteItem::FavoriteFolder);
            folder->setName(name);
        }

        parent->setExpanded(true);
    }
}

void ShowfotoStackViewFavorites::slotDelItem()
{
    ShowfotoStackViewFavoriteItem* const fitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(d->favoritesList->currentItem());

    if (!fitem)
    {
        return;
    }

    if (QMessageBox::question(this, i18nc("@title:window", "Confirm Item Deletion"),
                              i18nc("@info", "Are you sure you want to remove the item \"%1\"\n"
                                             "and all nested items if any?", fitem->name()))
            != QMessageBox::Yes)
    {
        return;
    }

    d->favoritesList->removeItemWidget(fitem, 0);
    delete fitem;
}

void ShowfotoStackViewFavorites::slotEditItem()
{
    ShowfotoStackViewFavoriteItem* const item = dynamic_cast<ShowfotoStackViewFavoriteItem*>(d->favoritesList->currentItem());

    if (item && (item->favoriteType() == ShowfotoStackViewFavoriteItem::FavoriteItem))
    {
        int favType                                 = item->favoriteType();
        QString name                                = item->name();
        QString desc                                = item->description();
        QDate date                                  = item->date();
        QString icon                                = item->icon(0).name();
        QList<QUrl> urls                            = item->urls();
        QUrl currentUrl                             = item->currentUrl();
        ShowfotoStackViewFavoriteItem* const parent = dynamic_cast<ShowfotoStackViewFavoriteItem*>(item->parent());
        bool ok                                     = ShowfotoStackViewFavoriteItemDlg::favoriteItemDialog(d->favoritesList,
                                                                                                           name,
                                                                                                           favType,
                                                                                                           desc,
                                                                                                           date,
                                                                                                           icon,
                                                                                                           urls,
                                                                                                           currentUrl,
                                                                                                           d->sidebar->iconSize(),
                                                                                                           d->sidebar->sortOrder(),
                                                                                                           d->sidebar->sortRole(),
                                                                                                           parent
                                                                                                          );

        if (ok)
        {
            if (favType == ShowfotoStackViewFavoriteItem::FavoriteItem)
            {
                item->setName(name);
                item->setDescription(desc);
                item->setDate(date);
                item->setIcon(0, QIcon::fromTheme(icon));
                item->setUrls(urls);
                item->setCurrentUrl(currentUrl);
            }
            else
            {
                item->setFavoriteType(favType);
                item->setName(name);
            }
        }

        return;
    }

    if (item && (item->favoriteType() == ShowfotoStackViewFavoriteItem::FavoriteFolder))
    {
        QString desc;
        QDate date;
        QString icon;
        QList<QUrl> urls;
        QUrl currentUrl;
        int favType                                 = item->favoriteType();
        QString name                                = item->name();
        ShowfotoStackViewFavoriteItem* const parent = dynamic_cast<ShowfotoStackViewFavoriteItem*>(item->parent());
        bool ok                                     = ShowfotoStackViewFavoriteItemDlg::favoriteItemDialog(d->favoritesList,
                                                                                                           name,
                                                                                                           favType,
                                                                                                           desc,
                                                                                                           date,
                                                                                                           icon,
                                                                                                           urls,
                                                                                                           currentUrl,
                                                                                                           d->sidebar->iconSize(),
                                                                                                           d->sidebar->sortOrder(),
                                                                                                           d->sidebar->sortRole(),
                                                                                                           parent
                                                                                                          );

        if (ok)
        {
            if (favType == ShowfotoStackViewFavoriteItem::FavoriteItem)
            {
                item->setFavoriteType(favType);
                item->setName(name);
                item->setDescription(desc);
                item->setDate(date);
                item->setIcon(0, QIcon::fromTheme(icon));
                item->setUrls(urls);
                item->setCurrentUrl(currentUrl);
            }
            else
            {
                item->setName(name);
            }
        }
    }
}

void ShowfotoStackViewFavorites::slotItemSelectionChanged()
{
    bool b                      = false;
    QTreeWidgetItem* const item = d->favoritesList->currentItem();

    if (item)
    {
        const ShowfotoStackViewFavoriteItem* const fitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(item);

        if (fitem && (fitem->favoriteType() != ShowfotoStackViewFavoriteItem::FavoriteRoot))
        {
            b = true;
        }
    }

    d->delBtn->setEnabled(b);
    d->edtBtn->setEnabled(b);
}

void ShowfotoStackViewFavorites::slotFavoriteActivated(QTreeWidgetItem* item)
{
    const ShowfotoStackViewFavoriteItem* const fvitem = dynamic_cast<ShowfotoStackViewFavoriteItem*>(item);

    if (fvitem && (fvitem->favoriteType() == ShowfotoStackViewFavoriteItem::FavoriteItem))
    {
        Q_EMIT signalLoadContentsFromFiles(fvitem->urlsToPaths(), fvitem->currentUrl().toLocalFile());
    }
}

void ShowfotoStackViewFavorites::slotItemListChanged(int nbitems)
{
    d->addBtn->setEnabled(nbitems > 0);
}

bool ShowfotoStackViewFavorites::saveSettings()
{
    QDomDocument doc(QLatin1String("favorites"));
    doc.setContent(QLatin1String("<!DOCTYPE XMLFavorites><favorites version=\"1.0\" client=\"showfoto\" encoding=\"UTF-8\"/>"));
    QDomElement docElem    = doc.documentElement();

    // ---

    QDomElement elemExpand = doc.createElement(QLatin1String("TopExpanded"));
    elemExpand.setAttribute(QLatin1String("value"), d->topFavorites->isExpanded());
    docElem.appendChild(elemExpand);

    // ---

    QDomElement elemList   = doc.createElement(QLatin1String("FavoritesList"));
    docElem.appendChild(elemList);
    QTreeWidgetItemIterator it2(d->favoritesList);

    while (*it2)
    {
        ShowfotoStackViewFavoriteItem* const item = dynamic_cast<ShowfotoStackViewFavoriteItem*>(*it2);

        if (item && (item->favoriteType() != ShowfotoStackViewFavoriteItem::FavoriteRoot))
        {
            QDomElement elem = doc.createElement(QLatin1String("Favorite"));

            QDomElement type = doc.createElement(QLatin1String("Type"));
            type.setAttribute(QLatin1String("value"), item->favoriteType());
            elem.appendChild(type);

            QDomElement name = doc.createElement(QLatin1String("Name"));
            name.setAttribute(QLatin1String("value"), item->name());
            elem.appendChild(name);

            QDomElement desc = doc.createElement(QLatin1String("Description"));
            desc.setAttribute(QLatin1String("value"), item->description());
            elem.appendChild(desc);

            QDomElement hier = doc.createElement(QLatin1String("Hierarchy"));
            hier.setAttribute(QLatin1String("value"), item->hierarchy());
            elem.appendChild(hier);

            QDomElement date = doc.createElement(QLatin1String("Date"));
            date.setAttribute(QLatin1String("value"), item->date().toString());
            elem.appendChild(date);

            QDomElement icon = doc.createElement(QLatin1String("Icon"));
            icon.setAttribute(QLatin1String("value"), item->icon(0).name());
            elem.appendChild(icon);

            QDomElement iexp = doc.createElement(QLatin1String("IsExpanded"));
            iexp.setAttribute(QLatin1String("value"), item->isExpanded());
            elem.appendChild(iexp);

            QDomElement urls = doc.createElement(QLatin1String("UrlsList"));
            elem.appendChild(urls);

            const auto curls = item->urls();

            for (const QUrl& url : curls)
            {
                QDomElement e = doc.createElement(QLatin1String("Url"));
                e.setAttribute(QLatin1String("value"), url.toLocalFile());
                urls.appendChild(e);
            }

            QDomElement curr = doc.createElement(QLatin1String("CurrentUrl"));
            curr.setAttribute(QLatin1String("value"), item->currentUrl().toLocalFile());
            elem.appendChild(curr);

            elemList.appendChild(elem);
        }

        ++it2;
    }

    QFile file(d->file);

    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QTextStream stream(&file);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    stream.setEncoding(QStringConverter::Utf8);

#else

    stream.setCodec(QTextCodec::codecForName("UTF-8"));

#endif

    stream.setAutoDetectUnicode(true);
    stream << doc.toString();
    file.close();

    return true;
}

bool ShowfotoStackViewFavorites::readSettings()
{
    d->favoritesList->clear();

    d->topFavorites = new ShowfotoStackViewFavoriteItem(d->favoritesList);;

    QFile file(d->file);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument doc(QLatin1String("favorites"));

    if (!doc.setContent(&file))
    {
        return false;
    }

    QDomElement docElem = doc.documentElement();

    if (docElem.tagName() != QLatin1String("favorites"))
    {
        return false;
    }

    for (QDomNode n1 = docElem.firstChild() ; !n1.isNull() ; n1 = n1.nextSibling())
    {
        QDomElement e1 = n1.toElement();

        if (e1.isNull())
        {
            continue;
        }

        if (e1.tagName() == QLatin1String("TopExpanded"))
        {
            d->topFavorites->setExpanded((bool)e1.attribute(QLatin1String("value")).toUInt());
            continue;
        }

        // ---

        if (e1.tagName() == QLatin1String("FavoritesList"))
        {
            int unamedID = 1;

            for (QDomNode n2 = e1.firstChild() ; !n2.isNull() ; n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();

                if (e2.tagName() == QLatin1String("Favorite"))
                {
                    bool isExpanded = true;
                    int type        = ShowfotoStackViewFavoriteItem::FavoriteFolder;
                    QDate date      = QDate::currentDate();
                    QString icon    = QString::fromLatin1("folder-favorites");
                    QString name;
                    QString desc;
                    QString hierarchy;
                    QList<QUrl> urls;
                    QString curr;

                    for (QDomNode n3 = e2.firstChild() ; !n3.isNull() ; n3 = n3.nextSibling())
                    {
                        QDomElement e3  = n3.toElement();
                        QString name3   = e3.tagName();
                        QString val3    = e3.attribute(QLatin1String("value"));

                        if      (name3 == QLatin1String("Name"))
                        {
                            if (val3.isEmpty())
                            {
                                val3 = i18nc("@title", "Unnamed") + QString::fromLatin1("_%1").arg(unamedID);
                                unamedID++;
                            }

                            name = val3;
                        }
                        else if (name3 == QLatin1String("Type"))
                        {
                            type = val3.toInt();
                        }
                        else if (name3 == QLatin1String("Description"))
                        {
                            desc = val3;
                        }
                        else if (name3 == QLatin1String("Hierarchy"))
                        {
                            hierarchy = val3;
                        }
                        else if (name3 == QLatin1String("Date"))
                        {
                            if (!val3.isEmpty())
                            {
                                date = QDate::fromString(val3);
                            }
                        }
                        else if (name3 == QLatin1String("Icon"))
                        {
                            if (!val3.isEmpty())
                            {
                                icon = val3;
                            }
                        }
                        else if (name3 == QLatin1String("IsExpanded"))
                        {
                            if (!val3.isEmpty())
                            {
                                isExpanded = val3.toUInt();
                            }
                        }
                        else if (name3 == QLatin1String("UrlsList"))
                        {
                            for (QDomNode n4 = e3.firstChild() ; !n4.isNull() ; n4 = n4.nextSibling())
                            {
                                QDomElement e4 = n4.toElement();
                                QString name4  = e4.tagName();
                                QString val4   = e4.attribute(QLatin1String("value"));

                                if (name4 == QLatin1String("Url"))
                                {
                                    if (!val4.isEmpty())
                                    {
                                        urls.append(QUrl::fromLocalFile(val4));
                                    }
                                }
                            }
                        }
                        else if (name3 == QLatin1String("CurrentUrl"))
                        {
                            curr = val3;
                        }
                    }

                    QString phierarchy            = hierarchy.section(QLatin1Char('/'), 0, -3) + QLatin1String("/");
                    QTreeWidgetItem* const parent = d->favoritesList->findFavoriteByHierarchy(phierarchy);

                    if (!parent)
                    {
                        continue;
                    }

                    ShowfotoStackViewFavoriteItem* const fitem = new ShowfotoStackViewFavoriteItem(parent, type);
                    fitem->setName(name);
                    fitem->setHierarchy(hierarchy);
                    fitem->setExpanded(isExpanded);

                    if (type == ShowfotoStackViewFavoriteItem::FavoriteItem)
                    {
                        fitem->setDescription(desc);
                        fitem->setDate(date);
                        fitem->setIcon(0, QIcon::fromTheme(icon));

                        if (urls.isEmpty())
                        {
                            delete fitem;
                            continue;
                        }
                        else
                        {
                            fitem->setUrls(urls);
                        }

                        if (curr.isEmpty())
                        {
                            if (!fitem->urls().isEmpty())
                            {
                                curr = fitem->urls().constFirst().toLocalFile();
                            }
                            else
                            {
                                curr = QString();
                            }
                        }

                        fitem->setCurrentUrl(QUrl::fromLocalFile(curr));
                    }
                }
            }

            continue;
        }
    }

    return true;
}

QList<QAction*> ShowfotoStackViewFavorites::pluginActions() const
{
    return d->sidebar->pluginActions();
}

void ShowfotoStackViewFavorites::slotSearchTextChanged(const SearchTextSettings& settings)
{
    d->favoritesList->setFilter(settings.text, settings.caseSensitive);
}

void ShowfotoStackViewFavorites::slotSearchResult(int found)
{
    d->favoritesFilter->slotSearchResult(found ? true : false);
}

} // namespace ShowFoto

#include "moc_showfotostackviewfavorites.cpp"

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : menu to manage bookmarks
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bookmarksmenu.h"

// Qt includes

#include <QUrl>
#include <QDebug>

// Local includes

#include "bookmarknode.h"

Q_DECLARE_METATYPE(QModelIndex)

namespace Digikam
{

class Q_DECL_HIDDEN ModelMenu::Private
{
public:

    Private() = default;

public:

    int                   maxRows           = 7;
    int                   firstSeparator    = -1;
    int                   maxWidth          = -1;
    int                   hoverRole         = 0;
    int                   separatorRole     = 0;
    QAbstractItemModel*   model             = nullptr;
    QPersistentModelIndex root;
};

ModelMenu::ModelMenu(QWidget* const parent)
    : QMenu(parent),
      d    (new Private)
{
    // --- NOTE: use dynamic binding as slotAboutToShow() is a virtual method which can be re-implemented in derived classes.

    connect(this, &ModelMenu::aboutToShow,
            this, &ModelMenu::slotAboutToShow);
}

ModelMenu::~ModelMenu()
{
    delete d;
}

bool ModelMenu::prePopulated()
{
    return false;
}

void ModelMenu::postPopulated()
{
}

void ModelMenu::setModel(QAbstractItemModel* model)
{
    d->model = model;
}

QAbstractItemModel* ModelMenu::model() const
{
    return d->model;
}

void ModelMenu::setMaxRows(int max)
{
    d->maxRows = max;
}

int ModelMenu::maxRows() const
{
    return d->maxRows;
}

void ModelMenu::setFirstSeparator(int offset)
{
    d->firstSeparator = offset;
}

int ModelMenu::firstSeparator() const
{
    return d->firstSeparator;
}

void ModelMenu::setRootIndex(const QModelIndex& index)
{
    d->root = index;
}

QModelIndex ModelMenu::rootIndex() const
{
    return d->root;
}

void ModelMenu::setHoverRole(int role)
{
    d->hoverRole = role;
}

int ModelMenu::hoverRole() const
{
    return d->hoverRole;
}

void ModelMenu::setSeparatorRole(int role)
{
    d->separatorRole = role;
}

int ModelMenu::separatorRole() const
{
    return d->separatorRole;
}

void ModelMenu::slotAboutToShow()
{
    if (QMenu* const menu = qobject_cast<QMenu*>(sender()))
    {
        QVariant v = menu->menuAction()->data();

        if (v.canConvert<QModelIndex>())
        {
            QModelIndex idx = qvariant_cast<QModelIndex>(v);
            createMenu(idx, -1, menu, menu);

            disconnect(menu, SIGNAL(aboutToShow()),
                       this, SLOT(slotAboutToShow()));

            return;
        }
    }

    clear();

    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    if (this->prePopulated())
    {
        addSeparator();
    }

    int max = d->maxRows;

    if (max != -1)
    {
        max += d->firstSeparator;
    }

    createMenu(d->root, max, this, this);
    postPopulated();
}

void ModelMenu::createMenu(const QModelIndex& parent, int max, QMenu* parentMenu, QMenu* menu)
{
    if (!menu)
    {
        QString title = parent.data().toString();
        menu          = new QMenu(title, this);
        QIcon icon    = qvariant_cast<QIcon>(parent.data(Qt::DecorationRole));
        menu->setIcon(icon);
        parentMenu->addMenu(menu);
        QVariant v;
        v.setValue(parent);
        menu->menuAction()->setData(v);

        connect(menu, SIGNAL(aboutToShow()),
                this, SLOT(slotAboutToShow()));

        return;
    }

    int end = d->model->rowCount(parent);

    if (max != -1)
    {
        end = qMin(max, end);
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotTriggered(QAction*)));

    connect(menu, SIGNAL(hovered(QAction*)),
            this, SLOT(slotHovered(QAction*)));

    for (int i = 0 ; i < end ; ++i)
    {
        QModelIndex idx = d->model->index(i, 0, parent);

        if (d->model->hasChildren(idx))
        {
            createMenu(idx, -1, menu);
        }
        else
        {
            if ((d->separatorRole != 0) && idx.data(d->separatorRole).toBool())
            {
                addSeparator();
            }
            else
            {
                menu->addAction(makeAction(idx));
            }
        }

        if ((menu == this) && (i == (d->firstSeparator - 1)))
        {
            addSeparator();
        }
    }
}

QAction* ModelMenu::makeAction(const QModelIndex& index)
{
    QIcon icon            = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QAction* const action = makeAction(icon, index.data().toString(), this);
    QVariant v;
    v.setValue(index);
    action->setData(v);

    return action;
}

QAction* ModelMenu::makeAction(const QIcon& icon, const QString& text, QObject* const parent)
{
    QFontMetrics fm(font());

    if (d->maxWidth == -1)
    {
        d->maxWidth = fm.horizontalAdvance(QLatin1Char('m')) * 30;
    }

    QString smallText = fm.elidedText(text, Qt::ElideMiddle, d->maxWidth);

    return (new QAction(icon, smallText, parent));
}

void ModelMenu::slotTriggered(QAction* action)
{
    QVariant v = action->data();

    if (v.canConvert<QModelIndex>())
    {
        QModelIndex idx = qvariant_cast<QModelIndex>(v);

        Q_EMIT activated(idx);
    }
}

void ModelMenu::slotHovered(QAction* action)
{
    QVariant v = action->data();

    if (v.canConvert<QModelIndex>())
    {
        QModelIndex idx       = qvariant_cast<QModelIndex>(v);
        QString hoveredString = idx.data(d->hoverRole).toString();

        if (!hoveredString.isEmpty())
        {
            Q_EMIT hoveredText(hoveredString);
        }
    }
}

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN BookmarksMenu::Private
{
public:

    Private() = default;

public:

    BookmarksManager* manager = nullptr;
    QList<QAction*>   initActions;
};

BookmarksMenu::BookmarksMenu(BookmarksManager* const mngr, QWidget* const parent)
    : ModelMenu(parent),
      d        (new Private)
{
    d->manager = mngr;

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));

    setMaxRows(-1);
    setHoverRole(BookmarksModel::UrlStringRole);
    setSeparatorRole(BookmarksModel::SeparatorRole);
}

BookmarksMenu::~BookmarksMenu()
{
    delete d;
}

void BookmarksMenu::slotActivated(const QModelIndex& index)
{
    Q_EMIT openUrl(index.data(BookmarksModel::UrlRole).toUrl());
}

bool BookmarksMenu::prePopulated()
{
    setModel(d->manager->bookmarksModel());
    setRootIndex(d->manager->bookmarksModel()->index(d->manager->bookmarks()->children().constFirst()));

    // initial actions

    for (QAction* const ac : std::as_const(d->initActions))
    {
        if (ac)
        {
            addAction(ac);
        }
    }

    if (!d->initActions.isEmpty())
    {
        addSeparator();
    }

    createMenu(rootIndex(), 0, this, this);

    return true;
}

void BookmarksMenu::setInitialActions(const QList<QAction*>& actions)
{
    d->initActions = actions;

    for (QAction* const ac : std::as_const(d->initActions))
    {
        if (ac)
        {
            addAction(ac);
        }
    }
}

} // namespace Digikam

#include "moc_bookmarksmenu.cpp"

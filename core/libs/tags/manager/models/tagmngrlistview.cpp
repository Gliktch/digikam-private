/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : Reimplemented QListView for Tags Manager, with support for
 *               drag-n-drop
 *
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagmngrlistview.h"

// Qt includes

#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "tagmngrlistmodel.h"
#include "tagmngrlistitem.h"
#include "taglist.h"

namespace Digikam
{

TagMngrListView::TagMngrListView(QWidget* const parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAlternatingRowColors(true);
}

void TagMngrListView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList list             = selectionModel()->selectedIndexes();
    TagMngrListModel* const tagmodel = dynamic_cast<TagMngrListModel*>(model());

    if (!tagmodel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";

        return;
    }

    QMimeData* const data = tagmodel->mimeData(list);

    if (!data)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no data obtained!";

        return;
    }

    QDrag* const drag = new QDrag(this);
    drag->setMimeData(data);
    drag->exec(supportedActions, Qt::IgnoreAction);
}

QModelIndexList TagMngrListView::mySelectedIndexes()
{
    return selectedIndexes();
}

void TagMngrListView::dropEvent(QDropEvent* e)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    QModelIndex index                = indexVisuallyAt(e->position().toPoint());

#else

    QModelIndex index                = indexVisuallyAt(e->pos());

#endif

    TagMngrListModel* const tagmodel = dynamic_cast<TagMngrListModel*>(model());

    if (!tagmodel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";

        return;
    }

    tagmodel->dropMimeData(e->mimeData(), e->dropAction(),
                           index.row(), index.column(), index.parent());

    QList<int> toSel = tagmodel->getDragNewSelection();

    if (toSel.size() != 2)
    {
        return;
    }

    QItemSelectionModel* const model = selectionModel();

    model->clearSelection();
    setCurrentIndex(tagmodel->index(toSel.first() + 1, 0));

    for (int it = toSel.first() + 1 ; it <= toSel.last() ; ++it)
    {
        model->select(tagmodel->index(it, 0), model->Select);
    }
}

QModelIndex TagMngrListView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p))
    {
        QModelIndex index = indexAt(p);

        if (index.isValid() && visualRect(index).contains(p))
        {
            return index;
        }
    }

    return QModelIndex();
}

void TagMngrListView::contextMenuEvent(QContextMenuEvent* event)
{
    Q_UNUSED(event);

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    TagList* const tagList = dynamic_cast<TagList*>(parent());

    if (!tagList)
    {
        return;
    }

    QAction* const delAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                           i18nc("@action", "Delete Selected from List"), this);
    cmhelper.addAction(delAction, tagList, SLOT(slotDeleteSelected()), false);

    QModelIndexList sel = selectionModel()->selectedIndexes();

    if ((sel.size() == 1) && (sel.first().row() == 0))
    {
        delAction->setDisabled(true);
    }

    cmhelper.exec(QCursor::pos());
}

void TagMngrListView::slotDeleteSelected()
{
    QModelIndexList sel = selectionModel()->selectedIndexes();

    if (sel.isEmpty())
    {
        return;
    }

    TagMngrListModel* const tagmodel = dynamic_cast<TagMngrListModel*>(model());

    if (!tagmodel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error! no model available!";

        return;
    }

    for (const QModelIndex& index : std::as_const(sel))
    {
        ListItem* const item = static_cast<ListItem*>(index.internalPointer());
        tagmodel->deleteItem(item);
    }
}

} // namespace Digikam

#include "moc_tagmngrlistview.cpp"

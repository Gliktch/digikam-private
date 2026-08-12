/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2014      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "abstractalbumtreeview.h"

// Qt includes

#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <QScrollBar>
#include <QQueue>
#include <QTimer>
#include <QDrag>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "albumdragdrop.h"
#include "albummanager.h"
#include "albummodeldragdrophandler.h"
#include "applicationsettings.h"
#include "albumthumbnailloader.h"
#include "contextmenuhelper.h"
#include "fileactionmngr.h"
#include "tagdragdrop.h"
#include "tagmodificationhelper.h"
#include "coredb.h"

namespace Digikam
{

template <class A>
static QList<A*> selectedAlbums(const QItemSelectionModel* const selModel,
                                const AlbumFilterModel* const filterModel)
{
    const QList<QModelIndex> indexes = selModel->selectedIndexes();
    QList<A*> albums;

    for (const QModelIndex& index : std::as_const(indexes))
    {
        albums << static_cast<A*>(filterModel->albumForIndex(index));
    }

    return albums;
}

// -------------------------------------------------------------------------------

struct Q_DECL_HIDDEN State
{
    State() = default;

    bool selected       = false;
    bool expanded       = false;
    bool currentIndex   = false;
};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN AlbumTreeViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit AlbumTreeViewDelegate(AbstractAlbumTreeView* const treeView = nullptr)
        : QStyledItemDelegate(treeView),
          m_treeView         (treeView)
    {
        updateHeight();
    }

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), m_height));

        return size;
    }

    void updateHeight()
    {
        int h = qMax(AlbumThumbnailLoader::instance()->thumbnailSize() + 2,
                     m_treeView->fontMetrics().height());

        if ((h % 2) > 0)
        {
            ++h;
        }

        setHeight(h);
    }

    void setHeight(int height)
    {
        if (m_height == height)
        {
            return;
        }

        m_height = height;

        Q_EMIT sizeHintChanged(QModelIndex());
    }

protected:

    AbstractAlbumTreeView* m_treeView = nullptr;
    int                    m_height   = 0;
};

// -------------------------------------------------------------------------------

class Q_DECL_HIDDEN AbstractAlbumTreeView::Private
{
public:

    Private() = default;

    AlbumTreeViewDelegate*     delegate                 = nullptr;

    bool                       expandOnSingleClick      = false;
    bool                       expandNewCurrent         = false;
    bool                       selectAlbumOnClick       = false;
    bool                       selectOnContextMenu      = true;
    bool                       enableContextMenu        = false;
    bool                       setInAlbumManager        = false;

    QMap<int, Digikam::State>  statesByAlbumId;
    QMap<int, Digikam::State>  searchBackup;

    QTimer*                    resizeColumnsTimer       = nullptr;

    AlbumPointer<Album>        lastSelectedAlbum;

    QList<ContextMenuElement*> contextMenuElements;

    QPixmap                    contextMenuIcon;
    QString                    contextMenuTitle;

    const QString              configSelectionEntry     = QLatin1String("Selection");
    const QString              configExpansionEntry     = QLatin1String("Expansion");
    const QString              configCurrentIndexEntry  = QLatin1String("CurrentIndex");
    const QString              configSortColumnEntry    = QLatin1String("SortColumn");
    const QString              configSortOrderEntry     = QLatin1String("SortOrder");
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
 *
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "applicationsettings.h"
#include "dcategorizedsortfilterproxymodel.h"
#include "digikam_export.h"
#include "digikam_globals.h"
#include "itemviewutilities.h"
#include "iteminfo.h"
#include "statesavingobject.h"

class QMenu;
class QContextMenuEvent;
class QItemDelegate;
class QItemSelectionModel;
class QTreeView;

namespace Digikam
{

class Album;
class ItemFilterModel;
class ThumbnailSize;
class TableViewShared;

class TableView : public QWidget,
                  public StateSavingObject
{
    Q_OBJECT

public:

    explicit TableView(QItemSelectionModel* const selectionModel,
                       DCategorizedSortFilterProxyModel* const imageFilterModel,
                       QWidget* const parent);

    ~TableView()                                                                                  override;

    void setThumbnailSize(const ThumbnailSize& size);
    ThumbnailSize getThumbnailSize()                                                        const;
    ItemInfo currentInfo()                                                                  const;
    Album* currentAlbum()                                                                   const;
    int numberOfSelectedItems()                                                             const;
    ItemInfo nextInfo()                                                                     const;
    ItemInfo previousInfo()                                                                 const;
    ItemInfo deepRowItemInfo(const int rowNumber, const bool relative)                      const;

    void selectAll();
    void clearSelection();
    void invertSelection();

    ItemInfoList allItemInfos(bool grouping = false)                                        const;
    ItemInfoList selectedItemInfos(bool grouping = false)                                   const;
    ItemInfoList selectedItemInfosCurrentFirst(bool grouping = false)                       const;
    bool          allNeedGroupResolving(const OperationType type)                           const;
    bool          selectedNeedGroupResolving(const OperationType type)                      const;

protected:

    void doLoadState()                                                                            override;
    void doSaveState()                                                                            override;

    bool eventFilter(QObject* watched, QEvent* event)                                             override;
    QList<QAction*> getExtraGroupingActions();

public Q_SLOTS:

    void slotGoToRow(const int rowNumber, const bool relativeMove);
    void slotSetCurrentUrlWhenAvailable(const QUrl& url);
    void slotSetCurrentWhenAvailable(const qlonglong id);
    void slotAwayFromSelection();
    void slotDeleteSelected(const ItemViewUtilities::DeleteMode deleteMode = ItemViewUtilities::DeleteUseTrash);
    void slotDeleteSelectedWithoutConfirmation(const ItemViewUtilities::DeleteMode deleteMode = ItemViewUtilities::DeleteUseTrash);
    void slotSetActive(const bool isActive);
    void slotPaste();
    void rename();

protected Q_SLOTS:

    void slotItemActivated(const QModelIndex& tableViewIndex);
    void slotGroupingModeActionTriggered();

Q_SIGNALS:

    void signalPreviewRequested(const ItemInfo& info);
    void signalZoomInStep();
    void signalZoomOutStep();
    void signalPopupTagsView();
    void signalItemsChanged();
    void signalInsertSelectedToExistingQueue(int queue);
    void signalShowContextMenu(QContextMenuEvent* event,
                               const QList<QAction*>& actions);
    void signalShowContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info,
                                     const QList<QAction*>& actions,
                                     ItemFilterModel* filterModel = nullptr);

private:

    class Private;

    const QScopedPointer<Private>         d;
    const QScopedPointer<TableViewShared> s;
};

} // namespace Digikam
